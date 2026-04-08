#include <string.h>

#include "egui.h"
#include "egui_view_flip_view.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define FLIP_VIEW_ROOT_WIDTH          224
#define FLIP_VIEW_ROOT_HEIGHT         228
#define FLIP_VIEW_PRIMARY_WIDTH       196
#define FLIP_VIEW_PRIMARY_HEIGHT      122
#define FLIP_VIEW_PREVIEW_WIDTH       104
#define FLIP_VIEW_PREVIEW_HEIGHT      64
#define FLIP_VIEW_BOTTOM_ROW_WIDTH    216
#define FLIP_VIEW_BOTTOM_ROW_HEIGHT   64
#define FLIP_VIEW_RECORD_WAIT         110
#define FLIP_VIEW_RECORD_FRAME_WAIT   150

typedef struct flip_view_track flip_view_track_t;
struct flip_view_track
{
    const char *title;
    const char *helper;
    const egui_view_flip_view_item_t *items;
    uint8_t item_count;
    uint8_t current_index;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_flip_view_t flip_view_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_flip_view_t flip_view_compact;
static egui_view_linearlayout_t locked_column;
static egui_view_flip_view_t flip_view_locked;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Flip View";
static uint8_t primary_track_index = 0;
static uint8_t compact_track_index = 0;

static const egui_view_flip_view_item_t stories_items[] = {
        {"Story", "Night city", "Single card focus with overlay navigation", "Move through the content rail", EGUI_COLOR_HEX(0xF3F7FB), EGUI_COLOR_HEX(0x0F6CBD)},
        {"Story", "Pinned route", "Keyboard and touch use the same surface", "Current page stays centered", EGUI_COLOR_HEX(0xF5F6F8), EGUI_COLOR_HEX(0x5E6A75)},
        {"Story", "Studio notes", "Compact preview keeps the same card logic", "Preview stays low-noise", EGUI_COLOR_HEX(0xF2F7F3), EGUI_COLOR_HEX(0x0F7B45)},
        {"Story", "Quiet review", "Read-only freezes previous and next", "Disabled arrows stay explicit", EGUI_COLOR_HEX(0xF8F4EC), EGUI_COLOR_HEX(0x9D5D00)},
};

static const egui_view_flip_view_item_t planner_items[] = {
        {"Plan", "Planner stack", "One active card replaces perspective layouts", "Single surface, sequential browsing", EGUI_COLOR_HEX(0xF3F7FB), EGUI_COLOR_HEX(0x0F6CBD)},
        {"Plan", "Review board", "Overlay buttons stay secondary to content", "Pager stays in the background", EGUI_COLOR_HEX(0xF4F7F9), EGUI_COLOR_HEX(0x5E6A75)},
        {"Plan", "Focus shelf", "Compact mode trims helper text first", "Same track in a smaller shell", EGUI_COLOR_HEX(0xF2F7F5), EGUI_COLOR_HEX(0x0F766E)},
};

static const egui_view_flip_view_item_t archive_items[] = {
        {"Archive", "Field memo", "Metadata stays short and readable", "No image asset required", EGUI_COLOR_HEX(0xF8F4EC), EGUI_COLOR_HEX(0x9D5D00)},
        {"Archive", "Signal shelf", "Footer only reflects the active snapshot", "State stays lightweight", EGUI_COLOR_HEX(0xF2F7F3), EGUI_COLOR_HEX(0x0F7B45)},
        {"Archive", "Review loop", "Track swaps do not change the widget shell", "Boundary states remain visible", EGUI_COLOR_HEX(0xF4F7F9), EGUI_COLOR_HEX(0x5E6A75)},
        {"Archive", "Paper trail", "Read-only preview keeps disabled arrows", "State boundary remains explicit", EGUI_COLOR_HEX(0xF7F4F0), EGUI_COLOR_HEX(0x8A6B40)},
};

static const egui_view_flip_view_item_t compact_story_items[] = {
        {"Lite", "Pocket", "Compact keeps only essential text", "", EGUI_COLOR_HEX(0xF2F7F5), EGUI_COLOR_HEX(0x0F766E)},
        {"Lite", "Quick", "Overlay buttons stay secondary", "", EGUI_COLOR_HEX(0xF3F7FB), EGUI_COLOR_HEX(0x0F6CBD)},
        {"Lite", "Review", "Footer still anchors context", "", EGUI_COLOR_HEX(0xF4F7F9), EGUI_COLOR_HEX(0x5E6A75)},
};

static const egui_view_flip_view_item_t compact_notes_items[] = {
        {"Lite", "Memo", "Compact mode trims helper rows", "", EGUI_COLOR_HEX(0xF8F4EC), EGUI_COLOR_HEX(0x9D5D00)},
        {"Lite", "Safe", "Track swap keeps compact neutral", "", EGUI_COLOR_HEX(0xF2F7F3), EGUI_COLOR_HEX(0x0F766E)},
};

static const egui_view_flip_view_item_t locked_items[] = {
        {"Lock", "Locked", "Navigation is intentionally frozen", "", EGUI_COLOR_HEX(0xF5F7F9), EGUI_COLOR_HEX(0x94A3B8)},
        {"Lock", "Frozen", "Overlay arrows stay disabled", "", EGUI_COLOR_HEX(0xF5F7F9), EGUI_COLOR_HEX(0xA1AAB5)},
        {"Lock", "Archive", "Hero shell keeps the same layout", "", EGUI_COLOR_HEX(0xF5F7F9), EGUI_COLOR_HEX(0x9CA3AF)},
};

static const flip_view_track_t primary_tracks[] = {
        {"Stories", "", stories_items, 4, 1},
        {"Planner", "", planner_items, 3, 0},
        {"Archive", "", archive_items, 4, 2},
};

static const flip_view_track_t compact_tracks[] = {
        {"Compact", "", compact_story_items, 3, 0},
        {"Compact", "", compact_notes_items, 2, 1},
};

static const flip_view_track_t locked_track = {"Read-only", "", locked_items, 3, 1};

static void apply_track(egui_view_t *view, const flip_view_track_t *track)
{
    egui_view_flip_view_set_title(view, track->title);
    egui_view_flip_view_set_helper(view, track->helper);
    egui_view_flip_view_set_items(view, track->items, track->item_count, track->current_index);
    egui_view_flip_view_set_current_part(view, EGUI_VIEW_FLIP_VIEW_PART_SURFACE);
}

static void apply_primary_track(uint8_t index)
{
    const flip_view_track_t *track = &primary_tracks[index % (sizeof(primary_tracks) / sizeof(primary_tracks[0]))];

    primary_track_index = index % (sizeof(primary_tracks) / sizeof(primary_tracks[0]));
    apply_track(EGUI_VIEW_OF(&flip_view_primary), track);
}

static void apply_compact_track(uint8_t index)
{
    const flip_view_track_t *track = &compact_tracks[index % (sizeof(compact_tracks) / sizeof(compact_tracks[0]))];

    compact_track_index = index % (sizeof(compact_tracks) / sizeof(compact_tracks[0]));
    apply_track(EGUI_VIEW_OF(&flip_view_compact), track);
}

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), FLIP_VIEW_ROOT_WIDTH, FLIP_VIEW_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), FLIP_VIEW_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_flip_view_init(EGUI_VIEW_OF(&flip_view_primary));
    egui_view_set_size(EGUI_VIEW_OF(&flip_view_primary), FLIP_VIEW_PRIMARY_WIDTH, FLIP_VIEW_PRIMARY_HEIGHT);
    egui_view_flip_view_set_font(EGUI_VIEW_OF(&flip_view_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_flip_view_set_meta_font(EGUI_VIEW_OF(&flip_view_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flip_view_set_palette(EGUI_VIEW_OF(&flip_view_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                    EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0xA7B1BC));
    egui_view_set_margin(EGUI_VIEW_OF(&flip_view_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&flip_view_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), FLIP_VIEW_BOTTOM_ROW_WIDTH, FLIP_VIEW_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), FLIP_VIEW_PREVIEW_WIDTH, FLIP_VIEW_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_flip_view_init(EGUI_VIEW_OF(&flip_view_compact));
    egui_view_set_size(EGUI_VIEW_OF(&flip_view_compact), FLIP_VIEW_PREVIEW_WIDTH, FLIP_VIEW_PREVIEW_HEIGHT);
    egui_view_flip_view_set_font(EGUI_VIEW_OF(&flip_view_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_flip_view_set_meta_font(EGUI_VIEW_OF(&flip_view_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flip_view_set_compact_mode(EGUI_VIEW_OF(&flip_view_compact), 1);
    egui_view_flip_view_set_palette(EGUI_VIEW_OF(&flip_view_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                    EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0xA7B1BC));
    static egui_view_api_t flip_view_compact_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&flip_view_compact), &flip_view_compact_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flip_view_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&flip_view_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&locked_column));
    egui_view_set_size(EGUI_VIEW_OF(&locked_column), FLIP_VIEW_PREVIEW_WIDTH, FLIP_VIEW_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&locked_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&locked_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&locked_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&locked_column));

    egui_view_flip_view_init(EGUI_VIEW_OF(&flip_view_locked));
    egui_view_set_size(EGUI_VIEW_OF(&flip_view_locked), FLIP_VIEW_PREVIEW_WIDTH, FLIP_VIEW_PREVIEW_HEIGHT);
    egui_view_flip_view_set_font(EGUI_VIEW_OF(&flip_view_locked), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_flip_view_set_meta_font(EGUI_VIEW_OF(&flip_view_locked), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flip_view_set_compact_mode(EGUI_VIEW_OF(&flip_view_locked), 1);
    egui_view_flip_view_set_read_only_mode(EGUI_VIEW_OF(&flip_view_locked), 1);
    egui_view_flip_view_set_palette(EGUI_VIEW_OF(&flip_view_locked), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xDBE2E8), EGUI_COLOR_HEX(0x536474),
                                    EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xB3BFCA));
    static egui_view_api_t flip_view_locked_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&flip_view_locked), &flip_view_locked_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flip_view_locked), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&locked_column), EGUI_VIEW_OF(&flip_view_locked));

    apply_primary_track(0);
    apply_compact_track(0);
    apply_track(EGUI_VIEW_OF(&flip_view_locked), &locked_track);

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&locked_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&flip_view_primary)->api->on_key_event(EGUI_VIEW_OF(&flip_view_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&flip_view_primary)->api->on_key_event(EGUI_VIEW_OF(&flip_view_primary), &event);
}

bool egui_port_get_recording_action(int action_index, egui_sim_action_t *p_action)
{
    static int last_action = -1;
    int first_call = action_index != last_action;

    last_action = action_index;

    switch (action_index)
    {
    case 0:
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_track((uint8_t)(primary_track_index + 1));
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_track((uint8_t)(compact_track_index + 1));
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FRAME_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
