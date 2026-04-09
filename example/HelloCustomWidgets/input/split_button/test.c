#include "egui.h"
#include "egui_view_split_button.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SPLIT_BUTTON_ROOT_WIDTH        224
#define SPLIT_BUTTON_ROOT_HEIGHT       154
#define SPLIT_BUTTON_PRIMARY_WIDTH     196
#define SPLIT_BUTTON_PRIMARY_HEIGHT    74
#define SPLIT_BUTTON_PREVIEW_WIDTH     104
#define SPLIT_BUTTON_PREVIEW_HEIGHT    44
#define SPLIT_BUTTON_BOTTOM_ROW_WIDTH  216
#define SPLIT_BUTTON_BOTTOM_ROW_HEIGHT 44
#define SPLIT_BUTTON_RECORD_WAIT       90
#define SPLIT_BUTTON_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_split_button_t button_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_split_button_t button_compact;
static egui_view_split_button_t button_disabled;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Split Button";

static const egui_view_split_button_snapshot_t primary_snapshots[] = {
        {"Save draft", "SV", "Save", "Run save or open more", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 1, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"Share handoff", "SH", "Share", "Send fast or choose route", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_MENU},
        {"Export file", "EX", "Export", "Export PDF or pick format", EGUI_VIEW_SPLIT_BUTTON_TONE_WARNING, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_MENU},
        {"Archive page", "AR", "Archive", "Archive now or choose policy", EGUI_VIEW_SPLIT_BUTTON_TONE_DANGER, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
};

static const egui_view_split_button_snapshot_t compact_snapshots[] = {
        {"Quick", "SV", "Save", "Tight split action", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 1, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"Review", "RV", "Review", "Compact action menu", EGUI_VIEW_SPLIT_BUTTON_TONE_NEUTRAL, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_MENU},
};

static const egui_view_split_button_snapshot_t disabled_snapshots[] = {
        {"Locked", "PB", "Publish", "Visible but inactive", EGUI_VIEW_SPLIT_BUTTON_TONE_NEUTRAL, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
};

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

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&button_primary), index % EGUI_ARRAY_SIZE(primary_snapshots));
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&button_compact), index % EGUI_ARRAY_SIZE(compact_snapshots));
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&button_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&button_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&button_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&button_primary), &event);
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SPLIT_BUTTON_ROOT_WIDTH, SPLIT_BUTTON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SPLIT_BUTTON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_split_button_init(EGUI_VIEW_OF(&button_primary));
    egui_view_set_size(EGUI_VIEW_OF(&button_primary), SPLIT_BUTTON_PRIMARY_WIDTH, SPLIT_BUTTON_PRIMARY_HEIGHT);
    egui_view_split_button_set_font(EGUI_VIEW_OF(&button_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_split_button_set_meta_font(EGUI_VIEW_OF(&button_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_button_set_snapshots(EGUI_VIEW_OF(&button_primary), primary_snapshots, EGUI_ARRAY_SIZE(primary_snapshots));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&button_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&button_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SPLIT_BUTTON_BOTTOM_ROW_WIDTH, SPLIT_BUTTON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_split_button_init(EGUI_VIEW_OF(&button_compact));
    egui_view_set_size(EGUI_VIEW_OF(&button_compact), SPLIT_BUTTON_PREVIEW_WIDTH, SPLIT_BUTTON_PREVIEW_HEIGHT);
    egui_view_split_button_set_font(EGUI_VIEW_OF(&button_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_button_set_meta_font(EGUI_VIEW_OF(&button_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_button_set_snapshots(EGUI_VIEW_OF(&button_compact), compact_snapshots, EGUI_ARRAY_SIZE(compact_snapshots));
    egui_view_split_button_set_compact_mode(EGUI_VIEW_OF(&button_compact), 1);
    static egui_view_api_t button_compact_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&button_compact), &button_compact_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    egui_view_override_api_on_key(EGUI_VIEW_OF(&button_compact), &button_compact_api, consume_preview_key);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&button_compact));

    egui_view_split_button_init(EGUI_VIEW_OF(&button_disabled));
    egui_view_set_size(EGUI_VIEW_OF(&button_disabled), SPLIT_BUTTON_PREVIEW_WIDTH, SPLIT_BUTTON_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&button_disabled), 8, 0, 0, 0);
    egui_view_split_button_set_font(EGUI_VIEW_OF(&button_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_button_set_meta_font(EGUI_VIEW_OF(&button_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_button_set_snapshots(EGUI_VIEW_OF(&button_disabled), disabled_snapshots, EGUI_ARRAY_SIZE(disabled_snapshots));
    egui_view_split_button_set_compact_mode(EGUI_VIEW_OF(&button_disabled), 1);
    egui_view_split_button_set_disabled_mode(EGUI_VIEW_OF(&button_disabled), 1);
    egui_view_split_button_set_palette(EGUI_VIEW_OF(&button_disabled), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD7DEE5), EGUI_COLOR_HEX(0x53606C),
                                       EGUI_COLOR_HEX(0x8E98A3), EGUI_COLOR_HEX(0x97A4B2), EGUI_COLOR_HEX(0x93A594), EGUI_COLOR_HEX(0xB29A67),
                                       EGUI_COLOR_HEX(0xA58A86), EGUI_COLOR_HEX(0x98A3AE));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_disabled), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&button_disabled));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
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
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_BUTTON_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_BUTTON_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_BUTTON_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_LEFT);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_BUTTON_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_snapshot(1);
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_BUTTON_RECORD_WAIT);
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
