#include <string.h>

#include "egui.h"
#include "egui_view_switch.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SWITCH_ROOT_WIDTH        224
#define SWITCH_ROOT_HEIGHT       132
#define SWITCH_PRIMARY_WIDTH     108
#define SWITCH_PRIMARY_HEIGHT    44
#define SWITCH_PREVIEW_WIDTH     76
#define SWITCH_PREVIEW_HEIGHT    32
#define SWITCH_BOTTOM_ROW_WIDTH  160
#define SWITCH_BOTTOM_ROW_HEIGHT 32
#define SWITCH_RECORD_WAIT       90
#define SWITCH_RECORD_FRAME_WAIT 170

typedef struct switch_snapshot switch_snapshot_t;
struct switch_snapshot
{
    uint8_t checked;
    const char *icon_on;
    const char *icon_off;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_switch_t primary_switch;
static egui_view_api_t primary_switch_api;
static egui_view_linearlayout_t bottom_row;
static egui_view_switch_t compact_switch;
static egui_view_api_t compact_switch_api;
static egui_view_switch_t read_only_switch;
static egui_view_api_t read_only_switch_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Toggle Switch";

static const switch_snapshot_t primary_snapshots[] = {
        {1, EGUI_ICON_MS_DONE, NULL},
        {0, EGUI_ICON_MS_DONE, NULL},
        {1, EGUI_ICON_MS_DONE, EGUI_ICON_MS_CROSS},
};

static const switch_snapshot_t compact_snapshots[] = {
        {1, EGUI_ICON_MS_DONE, NULL},
        {0, EGUI_ICON_MS_DONE, EGUI_ICON_MS_CROSS},
};

static const switch_snapshot_t read_only_snapshot = {
        1, EGUI_ICON_MS_DONE, NULL,
};

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        egui_view_clear_focus(EGUI_VIEW_OF(&primary_switch));
    }
#else
    EGUI_UNUSED(event);
#endif
    return 1;
}

static void apply_snapshot_to_switch(egui_view_switch_t *control, const switch_snapshot_t *snapshot)
{
    hcw_switch_set_state_icons(EGUI_VIEW_OF(control), snapshot->icon_on, snapshot->icon_off);
    hcw_switch_set_checked(EGUI_VIEW_OF(control), snapshot->checked);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot_to_switch(&primary_switch, &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)]);
}

static void apply_compact_snapshot(uint8_t index)
{
    apply_snapshot_to_switch(&compact_switch, &compact_snapshots[index % EGUI_ARRAY_SIZE(compact_snapshots)]);
}

static void apply_read_only_snapshot(void)
{
    apply_snapshot_to_switch(&read_only_switch, &read_only_snapshot);
}

static void focus_primary_switch(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&primary_switch));
#endif
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SWITCH_ROOT_WIDTH, SWITCH_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SWITCH_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_switch_init(EGUI_VIEW_OF(&primary_switch));
    egui_view_set_size(EGUI_VIEW_OF(&primary_switch), SWITCH_PRIMARY_WIDTH, SWITCH_PRIMARY_HEIGHT);
    hcw_switch_apply_standard_style(EGUI_VIEW_OF(&primary_switch));
    hcw_switch_set_icon_font(EGUI_VIEW_OF(&primary_switch), EGUI_FONT_ICON_MS_20);
    hcw_switch_override_interaction_api(EGUI_VIEW_OF(&primary_switch), &primary_switch_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_switch), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&primary_switch), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_switch));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SWITCH_BOTTOM_ROW_WIDTH, SWITCH_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_switch_init(EGUI_VIEW_OF(&compact_switch));
    egui_view_set_size(EGUI_VIEW_OF(&compact_switch), SWITCH_PREVIEW_WIDTH, SWITCH_PREVIEW_HEIGHT);
    hcw_switch_apply_compact_style(EGUI_VIEW_OF(&compact_switch));
    hcw_switch_set_icon_font(EGUI_VIEW_OF(&compact_switch), EGUI_FONT_ICON_MS_16);
    hcw_switch_override_static_preview_api(EGUI_VIEW_OF(&compact_switch), &compact_switch_api);
    compact_switch_api.on_touch = dismiss_primary_focus_on_preview_touch;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_switch), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_switch));

    egui_view_switch_init(EGUI_VIEW_OF(&read_only_switch));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_switch), SWITCH_PREVIEW_WIDTH, SWITCH_PREVIEW_HEIGHT);
    hcw_switch_apply_read_only_style(EGUI_VIEW_OF(&read_only_switch));
    hcw_switch_set_icon_font(EGUI_VIEW_OF(&read_only_switch), EGUI_FONT_ICON_MS_16);
    hcw_switch_override_static_preview_api(EGUI_VIEW_OF(&read_only_switch), &read_only_switch_api);
    read_only_switch_api.on_touch = dismiss_primary_focus_on_preview_touch;
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_switch), 8, 0, 0, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&read_only_switch), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_switch));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_read_only_snapshot();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
    focus_primary_switch();
}

#if EGUI_CONFIG_RECORDING_TEST
static void dispatch_primary_key(uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    focus_primary_switch();
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&primary_switch)->api->dispatch_key_event(EGUI_VIEW_OF(&primary_switch), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&primary_switch)->api->dispatch_key_event(EGUI_VIEW_OF(&primary_switch), &event);
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
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
            apply_read_only_snapshot();
            focus_primary_switch();
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWITCH_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            dispatch_primary_key(EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, SWITCH_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWITCH_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
            apply_compact_snapshot(1);
            focus_primary_switch();
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SWITCH_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            dispatch_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, SWITCH_RECORD_WAIT);
        return true;
    case 5:
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
