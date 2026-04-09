#include <string.h>

#include "egui.h"
#include "egui_view_swipe_control.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SWIPE_CONTROL_ROOT_WIDTH        224
#define SWIPE_CONTROL_ROOT_HEIGHT       222
#define SWIPE_CONTROL_PRIMARY_WIDTH     196
#define SWIPE_CONTROL_PRIMARY_HEIGHT    118
#define SWIPE_CONTROL_PREVIEW_WIDTH     104
#define SWIPE_CONTROL_PREVIEW_HEIGHT    64
#define SWIPE_CONTROL_BOTTOM_ROW_WIDTH  216
#define SWIPE_CONTROL_BOTTOM_ROW_HEIGHT 64
#define SWIPE_CONTROL_RECORD_WAIT       110
#define SWIPE_CONTROL_RECORD_FRAME_WAIT 150

typedef struct swipe_control_track swipe_control_track_t;
struct swipe_control_track
{
    const char *title;
    const char *helper;
    const egui_view_swipe_control_item_t *item;
    const egui_view_swipe_control_action_t *start_action;
    const egui_view_swipe_control_action_t *end_action;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_swipe_control_t swipe_control_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_swipe_control_t swipe_control_compact;
static egui_view_swipe_control_t swipe_control_read_only;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Swipe Control";

static const egui_view_swipe_control_item_t inbox_item = {
        "Mail", "Invoice follow-up", "Reveal quick actions without leaving the row.", "Due today", EGUI_COLOR_HEX(0xEDF6FF), EGUI_COLOR_HEX(0x2F6EEA)};
static const egui_view_swipe_control_action_t inbox_start_action = {"Pin", "Keep", EGUI_COLOR_HEX(0x0F766E), EGUI_COLOR_HEX(0xFFFFFF)};
static const egui_view_swipe_control_action_t inbox_end_action = {"Delete", "Remove", EGUI_COLOR_HEX(0xD64545), EGUI_COLOR_HEX(0xFFFFFF)};

static const egui_view_swipe_control_item_t planner_item = {
        "Plan", "Planner sync", "One row, two sides, one calm reveal model.", "Board ready", EGUI_COLOR_HEX(0xF4F1FF), EGUI_COLOR_HEX(0x5F6BCF)};
static const egui_view_swipe_control_action_t planner_start_action = {"Flag", "Review", EGUI_COLOR_HEX(0x6E56CF), EGUI_COLOR_HEX(0xFFFFFF)};
static const egui_view_swipe_control_action_t planner_end_action = {"Archive", "Store", EGUI_COLOR_HEX(0xC9822D), EGUI_COLOR_HEX(0xFFFFFF)};

static const egui_view_swipe_control_item_t review_item = {
        "Build", "Renderer check", "Keys and swipe gestures share the same state.", "Waiting QA", EGUI_COLOR_HEX(0xEAF7EF), EGUI_COLOR_HEX(0x1F8F66)};
static const egui_view_swipe_control_action_t review_start_action = {"Done", "Close", EGUI_COLOR_HEX(0x1C8C61), EGUI_COLOR_HEX(0xFFFFFF)};
static const egui_view_swipe_control_action_t review_end_action = {"Snooze", "Later", EGUI_COLOR_HEX(0x2F6EEA), EGUI_COLOR_HEX(0xFFFFFF)};

static const egui_view_swipe_control_item_t compact_mail_item = {"Mini", "Pocket", "", "", EGUI_COLOR_HEX(0xE9F6F2), EGUI_COLOR_HEX(0x118A7A)};
static const egui_view_swipe_control_action_t compact_mail_start_action = {"Pin", "", EGUI_COLOR_HEX(0x0F766E), EGUI_COLOR_HEX(0xFFFFFF)};
static const egui_view_swipe_control_action_t compact_mail_end_action = {"Delete", "", EGUI_COLOR_HEX(0xD64545), EGUI_COLOR_HEX(0xFFFFFF)};

static const egui_view_swipe_control_item_t compact_queue_item = {"Mini", "Queue", "", "", EGUI_COLOR_HEX(0xFFF4EA), EGUI_COLOR_HEX(0xC98530)};
static const egui_view_swipe_control_action_t compact_queue_start_action = {"Flag", "", EGUI_COLOR_HEX(0x6E56CF), EGUI_COLOR_HEX(0xFFFFFF)};
static const egui_view_swipe_control_action_t compact_queue_end_action = {"Archive", "", EGUI_COLOR_HEX(0xC9822D), EGUI_COLOR_HEX(0xFFFFFF)};

static const egui_view_swipe_control_item_t read_only_item = {"Lock", "Locked row", "", "", EGUI_COLOR_HEX(0xF3F6F9), EGUI_COLOR_HEX(0x9AA7B5)};
static const egui_view_swipe_control_action_t read_only_start_action = {"Pin", "", EGUI_COLOR_HEX(0xA0AAB5), EGUI_COLOR_HEX(0xFFFFFF)};
static const egui_view_swipe_control_action_t read_only_end_action = {"Delete", "", EGUI_COLOR_HEX(0xBAC3CC), EGUI_COLOR_HEX(0xFFFFFF)};

static const swipe_control_track_t primary_tracks[] = {
        {"Inbox", "Swipe right to pin or left to delete.", &inbox_item, &inbox_start_action, &inbox_end_action},
        {"Planner", "One row keeps the same reveal structure.", &planner_item, &planner_start_action, &planner_end_action},
        {"Review", "Keyboard and touch share one reveal state.", &review_item, &review_start_action, &review_end_action},
};

static const swipe_control_track_t compact_tracks[] = {
        {"", "", &compact_mail_item, &compact_mail_start_action, &compact_mail_end_action},
        {"", "", &compact_queue_item, &compact_queue_start_action, &compact_queue_end_action},
};

static const swipe_control_track_t read_only_track = {"", "", &read_only_item, &read_only_start_action, &read_only_end_action};

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_UP || event->type == EGUI_MOTION_EVENT_ACTION_CANCEL)
    {
        egui_view_set_pressed(self, 0);
    }

    return 1;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int consume_preview_key(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}
#endif

static void apply_track(egui_view_t *view, const swipe_control_track_t *track)
{
    egui_view_swipe_control_set_title(view, track->title == NULL ? "" : track->title);
    egui_view_swipe_control_set_helper(view, track->helper == NULL ? "" : track->helper);
    egui_view_swipe_control_set_item(view, track->item);
    egui_view_swipe_control_set_actions(view, track->start_action, track->end_action);
    egui_view_swipe_control_set_reveal_state(view, EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE);
    egui_view_swipe_control_set_current_part(view, EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE);
}

static void apply_primary_track(uint8_t index)
{
    apply_track(EGUI_VIEW_OF(&swipe_control_primary), &primary_tracks[index % EGUI_ARRAY_SIZE(primary_tracks)]);
}

static void apply_compact_track(uint8_t index)
{
    apply_track(EGUI_VIEW_OF(&swipe_control_compact), &compact_tracks[index % EGUI_ARRAY_SIZE(compact_tracks)]);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&swipe_control_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&swipe_control_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&swipe_control_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&swipe_control_primary), &event);
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SWIPE_CONTROL_ROOT_WIDTH, SWIPE_CONTROL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SWIPE_CONTROL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_swipe_control_init(EGUI_VIEW_OF(&swipe_control_primary));
    egui_view_set_size(EGUI_VIEW_OF(&swipe_control_primary), SWIPE_CONTROL_PRIMARY_WIDTH, SWIPE_CONTROL_PRIMARY_HEIGHT);
    egui_view_swipe_control_set_font(EGUI_VIEW_OF(&swipe_control_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_swipe_control_set_meta_font(EGUI_VIEW_OF(&swipe_control_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_swipe_control_set_palette(EGUI_VIEW_OF(&swipe_control_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD7DFE7), EGUI_COLOR_HEX(0x1A2630),
                                        EGUI_COLOR_HEX(0x72808E), EGUI_COLOR_HEX(0xAAB6C3));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&swipe_control_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&swipe_control_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&swipe_control_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SWIPE_CONTROL_BOTTOM_ROW_WIDTH, SWIPE_CONTROL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_swipe_control_init(EGUI_VIEW_OF(&swipe_control_compact));
    egui_view_set_size(EGUI_VIEW_OF(&swipe_control_compact), SWIPE_CONTROL_PREVIEW_WIDTH, SWIPE_CONTROL_PREVIEW_HEIGHT);
    egui_view_swipe_control_set_font(EGUI_VIEW_OF(&swipe_control_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_swipe_control_set_meta_font(EGUI_VIEW_OF(&swipe_control_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_swipe_control_set_compact_mode(EGUI_VIEW_OF(&swipe_control_compact), 1);
    egui_view_swipe_control_set_palette(EGUI_VIEW_OF(&swipe_control_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DDDA), EGUI_COLOR_HEX(0x17302A),
                                        EGUI_COLOR_HEX(0x57756C), EGUI_COLOR_HEX(0x8FB9B1));
    static egui_view_api_t swipe_control_compact_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&swipe_control_compact), &swipe_control_compact_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    egui_view_override_api_on_key(EGUI_VIEW_OF(&swipe_control_compact), &swipe_control_compact_api, consume_preview_key);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&swipe_control_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&swipe_control_compact));

    egui_view_swipe_control_init(EGUI_VIEW_OF(&swipe_control_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&swipe_control_read_only), SWIPE_CONTROL_PREVIEW_WIDTH, SWIPE_CONTROL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&swipe_control_read_only), 8, 0, 0, 0);
    egui_view_swipe_control_set_font(EGUI_VIEW_OF(&swipe_control_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_swipe_control_set_meta_font(EGUI_VIEW_OF(&swipe_control_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_swipe_control_set_compact_mode(EGUI_VIEW_OF(&swipe_control_read_only), 1);
    egui_view_swipe_control_set_read_only_mode(EGUI_VIEW_OF(&swipe_control_read_only), 1);
    egui_view_swipe_control_set_palette(EGUI_VIEW_OF(&swipe_control_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xDBE2E8), EGUI_COLOR_HEX(0x536474),
                                        EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xB3BFCA));
    static egui_view_api_t swipe_control_read_only_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&swipe_control_read_only), &swipe_control_read_only_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    egui_view_override_api_on_key(EGUI_VIEW_OF(&swipe_control_read_only), &swipe_control_read_only_api, consume_preview_key);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&swipe_control_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&swipe_control_read_only));

    apply_primary_track(0);
    apply_compact_track(0);
    apply_track(EGUI_VIEW_OF(&swipe_control_read_only), &read_only_track);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&swipe_control_primary));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
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
            apply_compact_track(0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&swipe_control_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_LEFT);
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_track(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&swipe_control_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_track(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SWIPE_CONTROL_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
