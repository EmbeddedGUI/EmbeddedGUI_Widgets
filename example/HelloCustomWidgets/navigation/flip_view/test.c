#include <string.h>

#include "egui.h"
#include "egui_view_flip_view.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define FLIP_VIEW_ROOT_WIDTH          224
#define FLIP_VIEW_ROOT_HEIGHT         224
#define FLIP_VIEW_PRIMARY_WIDTH       196
#define FLIP_VIEW_PRIMARY_HEIGHT      122
#define FLIP_VIEW_PREVIEW_WIDTH       104
#define FLIP_VIEW_PREVIEW_HEIGHT      64
#define FLIP_VIEW_BOTTOM_ROW_WIDTH    216
#define FLIP_VIEW_BOTTOM_ROW_HEIGHT   64
#define FLIP_VIEW_RECORD_WAIT         110
#define FLIP_VIEW_RECORD_FRAME_WAIT   150
#define FLIP_VIEW_RECORD_FINAL_WAIT   520

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
static egui_view_flip_view_t flip_view_compact;
static egui_view_flip_view_t flip_view_read_only;
static egui_view_api_t flip_view_compact_api;
static egui_view_api_t flip_view_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Flip View";

static const egui_view_flip_view_item_t highlights_items[] = {
        {"Overview", "Q1 summary", "One active card keeps the current update in focus", "Primary surface stays centered", EGUI_COLOR_HEX(0xF3F7FB),
         EGUI_COLOR_HEX(0x0F6CBD)},
        {"Overview", "Shipping board", "Previous and next stay secondary to content", "Navigation remains low-noise", EGUI_COLOR_HEX(0xF5F6F8),
         EGUI_COLOR_HEX(0x5E6A75)},
        {"Overview", "Release note", "Compact preview reuses the same card shell", "Preview remains restrained", EGUI_COLOR_HEX(0xF2F7F3),
         EGUI_COLOR_HEX(0x0F7B45)},
        {"Overview", "Archive handoff", "Read-only freezes previous and next", "Boundary state stays explicit", EGUI_COLOR_HEX(0xF8F4EC),
         EGUI_COLOR_HEX(0x9D5D00)},
};

static const egui_view_flip_view_item_t operations_items[] = {
        {"Ops", "Planning board", "Sequential browsing replaces stacked panels", "Single surface, ordered review", EGUI_COLOR_HEX(0xF3F7FB),
         EGUI_COLOR_HEX(0x0F6CBD)},
        {"Ops", "Status deck", "Content changes without changing the shell", "Same frame across snapshots", EGUI_COLOR_HEX(0xF4F7F9),
         EGUI_COLOR_HEX(0x5E6A75)},
        {"Ops", "Review queue", "Compact mode trims helper rows first", "Smaller shell, same semantics", EGUI_COLOR_HEX(0xF2F7F5),
         EGUI_COLOR_HEX(0x0F766E)},
};

static const egui_view_flip_view_item_t records_items[] = {
        {"Record", "Case note", "Metadata stays short and readable", "No image asset required", EGUI_COLOR_HEX(0xF8F4EC), EGUI_COLOR_HEX(0x9D5D00)},
        {"Record", "Approval", "Footer reflects the active item only", "State stays lightweight", EGUI_COLOR_HEX(0xF2F7F3), EGUI_COLOR_HEX(0x0F7B45)},
        {"Record", "Retention", "Track swaps do not change the widget shell", "Pager stays secondary", EGUI_COLOR_HEX(0xF4F7F9), EGUI_COLOR_HEX(0x5E6A75)},
        {"Record", "Readout", "Read-only preview keeps arrows disabled", "State boundary remains visible", EGUI_COLOR_HEX(0xF7F4F0), EGUI_COLOR_HEX(0x8A6B40)},
};

static const egui_view_flip_view_item_t compact_overview_items[] = {
        {"Compact", "Snapshot", "Compact keeps only essential text", "", EGUI_COLOR_HEX(0xF2F7F5), EGUI_COLOR_HEX(0x0F766E)},
        {"Compact", "Queue", "Navigation remains secondary", "", EGUI_COLOR_HEX(0xF3F7FB), EGUI_COLOR_HEX(0x0F6CBD)},
        {"Compact", "Review", "Footer still anchors context", "", EGUI_COLOR_HEX(0xF4F7F9), EGUI_COLOR_HEX(0x5E6A75)},
};

static const egui_view_flip_view_item_t read_only_items[] = {
        {"Read", "Summary", "Navigation is intentionally disabled", "", EGUI_COLOR_HEX(0xF5F7F9), EGUI_COLOR_HEX(0x94A3B8)},
        {"Read", "Snapshot", "Overlay arrows stay dimmed", "", EGUI_COLOR_HEX(0xF5F7F9), EGUI_COLOR_HEX(0xA1AAB5)},
        {"Read", "Record", "Card layout remains unchanged", "", EGUI_COLOR_HEX(0xF5F7F9), EGUI_COLOR_HEX(0x9CA3AF)},
};

static const flip_view_track_t primary_tracks[] = {
        {"Highlights", "", highlights_items, 4, 1},
        {"Operations", "", operations_items, 3, 0},
        {"Records", "", records_items, 4, 2},
};

static const flip_view_track_t compact_track = {"Compact", "", compact_overview_items, 3, 0};
static const flip_view_track_t read_only_track = {"Read only", "", read_only_items, 3, 1};

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

    apply_track(EGUI_VIEW_OF(&flip_view_primary), track);
}

static void apply_preview_states(void)
{
    apply_track(EGUI_VIEW_OF(&flip_view_compact), &compact_track);
    apply_track(EGUI_VIEW_OF(&flip_view_read_only), &read_only_track);
    egui_view_flip_view_set_read_only_mode(EGUI_VIEW_OF(&flip_view_read_only), 1);
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
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
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

    egui_view_flip_view_init(EGUI_VIEW_OF(&flip_view_compact));
    egui_view_set_size(EGUI_VIEW_OF(&flip_view_compact), FLIP_VIEW_PREVIEW_WIDTH, FLIP_VIEW_PREVIEW_HEIGHT);
    egui_view_flip_view_set_font(EGUI_VIEW_OF(&flip_view_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_flip_view_set_meta_font(EGUI_VIEW_OF(&flip_view_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flip_view_set_compact_mode(EGUI_VIEW_OF(&flip_view_compact), 1);
    egui_view_flip_view_set_palette(EGUI_VIEW_OF(&flip_view_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                    EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0xA7B1BC));
    egui_view_flip_view_override_static_preview_api(EGUI_VIEW_OF(&flip_view_compact), &flip_view_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flip_view_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&flip_view_compact));

    egui_view_flip_view_init(EGUI_VIEW_OF(&flip_view_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&flip_view_read_only), FLIP_VIEW_PREVIEW_WIDTH, FLIP_VIEW_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&flip_view_read_only), 8, 0, 0, 0);
    egui_view_flip_view_set_font(EGUI_VIEW_OF(&flip_view_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_flip_view_set_meta_font(EGUI_VIEW_OF(&flip_view_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flip_view_set_compact_mode(EGUI_VIEW_OF(&flip_view_read_only), 1);
    egui_view_flip_view_set_read_only_mode(EGUI_VIEW_OF(&flip_view_read_only), 1);
    egui_view_flip_view_set_palette(EGUI_VIEW_OF(&flip_view_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xDBE2E8), EGUI_COLOR_HEX(0x536474),
                                    EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xB3BFCA));
    egui_view_flip_view_override_static_preview_api(EGUI_VIEW_OF(&flip_view_read_only), &flip_view_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flip_view_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&flip_view_read_only));

    apply_primary_track(0);
    apply_preview_states();

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&flip_view_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&flip_view_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&flip_view_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&flip_view_primary), &event);
}

bool egui_port_get_recording_action(int action_index, egui_sim_action_t *p_action)
{
    static int last_action = -1;
    int first_call = action_index != last_action;

    last_action = action_index;

    switch (action_index)
    {
    case 0:
        if (first_call)
        {
            apply_primary_track(0);
            apply_preview_states();
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_track(1);
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_track(0);
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLIP_VIEW_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
