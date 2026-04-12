#include "egui.h"
#include "egui_view_tool_tip.h"
#include "uicode.h"
#include "demo_scaffold.h"

#include <string.h>

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TOOL_TIP_ROOT_WIDTH        224
#define TOOL_TIP_ROOT_HEIGHT       238
#define TOOL_TIP_PRIMARY_WIDTH     196
#define TOOL_TIP_PRIMARY_HEIGHT    118
#define TOOL_TIP_PREVIEW_WIDTH     104
#define TOOL_TIP_PREVIEW_HEIGHT    82
#define TOOL_TIP_BOTTOM_ROW_WIDTH  216
#define TOOL_TIP_BOTTOM_ROW_HEIGHT 82
#define TOOL_TIP_RECORD_WAIT       100
#define TOOL_TIP_RECORD_FRAME_WAIT 180

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))
#define COMPACT_SNAPSHOT_COUNT ((uint8_t)(sizeof(compact_snapshots) / sizeof(compact_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_tool_tip_t tool_tip_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_tool_tip_t tool_tip_compact;
static egui_view_tool_tip_t tool_tip_read_only;
static egui_view_api_t tool_tip_compact_api;
static egui_view_api_t tool_tip_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "ToolTip";

static const egui_view_tool_tip_snapshot_t primary_snapshots[] = {
        {"Save", "Hint", "Quick save", "Appears after a short dwell.", EGUI_VIEW_TOOL_TIP_TONE_ACCENT, EGUI_VIEW_TOOL_TIP_PLACEMENT_BOTTOM, -20},
        {"Search", "Shortcut", "Press slash to search", "Placement above the target.", EGUI_VIEW_TOOL_TIP_TONE_ACCENT, EGUI_VIEW_TOOL_TIP_PLACEMENT_TOP, 20},
        {"Publish", "Warning", "Wait for sync to finish", "Keeps go-live safe.", EGUI_VIEW_TOOL_TIP_TONE_WARNING, EGUI_VIEW_TOOL_TIP_PLACEMENT_BOTTOM, 8},
};

static const egui_view_tool_tip_snapshot_t compact_snapshots[] = {
        {"Filter", "Tip", "Narrow fast", "Compact hint", EGUI_VIEW_TOOL_TIP_TONE_NEUTRAL, EGUI_VIEW_TOOL_TIP_PLACEMENT_BOTTOM, -8},
        {"Search", "Hint", "Find quickly", "Top compact", EGUI_VIEW_TOOL_TIP_TONE_ACCENT, EGUI_VIEW_TOOL_TIP_PLACEMENT_TOP, 8},
};

static const egui_view_tool_tip_snapshot_t read_only_snapshots[] = {
        {"Preview", "Read only", "Review only", "Static comparison.", EGUI_VIEW_TOOL_TIP_TONE_NEUTRAL, EGUI_VIEW_TOOL_TIP_PLACEMENT_BOTTOM, 0},
};

static uint8_t primary_snapshot_index = 0;
static uint8_t compact_snapshot_index = 0;

static void apply_primary_snapshot(uint8_t index)
{
    primary_snapshot_index = (uint8_t)(index % PRIMARY_SNAPSHOT_COUNT);
    egui_view_tool_tip_set_current_snapshot(EGUI_VIEW_OF(&tool_tip_primary), primary_snapshot_index);
}

static void reset_primary_state(uint8_t index)
{
    apply_primary_snapshot(index);
    egui_view_tool_tip_set_open(EGUI_VIEW_OF(&tool_tip_primary), 0);
}

static void apply_compact_snapshot(uint8_t index)
{
    compact_snapshot_index = (uint8_t)(index % COMPACT_SNAPSHOT_COUNT);
    egui_view_tool_tip_set_current_snapshot(EGUI_VIEW_OF(&tool_tip_compact), compact_snapshot_index);
    egui_view_tool_tip_set_open(EGUI_VIEW_OF(&tool_tip_compact), 1);
}

static void apply_read_only_state(void)
{
    egui_view_tool_tip_set_current_snapshot(EGUI_VIEW_OF(&tool_tip_read_only), 0);
    egui_view_tool_tip_set_open(EGUI_VIEW_OF(&tool_tip_read_only), 1);
    egui_view_tool_tip_set_read_only_mode(EGUI_VIEW_OF(&tool_tip_read_only), 1);
}

static void dismiss_primary_tool_tip_focus(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&tool_tip_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_tool_tip_focus();
    }
    return 1;
}

#if EGUI_CONFIG_RECORDING_TEST
static void set_click_view_center(egui_sim_action_t *p_action, egui_view_t *view, int interval_ms)
{
    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = view->region_screen.location.x + view->region_screen.size.width / 2;
    p_action->y1 = view->region_screen.location.y + view->region_screen.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}

static void set_click_tool_tip_part(egui_sim_action_t *p_action, egui_view_t *view, uint8_t part, int interval_ms)
{
    egui_region_t region;

    if (!egui_view_tool_tip_get_part_region(view, part, &region))
    {
        EGUI_SIM_SET_WAIT(p_action, interval_ms);
        return;
    }

    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = region.location.x + region.size.width / 2;
    p_action->y1 = region.location.y + region.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}

static void dispatch_primary_key(uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&tool_tip_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&tool_tip_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&tool_tip_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&tool_tip_primary), &event);
}

static void request_page_snapshot(void)
{
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TOOL_TIP_ROOT_WIDTH, TOOL_TIP_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TOOL_TIP_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_tool_tip_init(EGUI_VIEW_OF(&tool_tip_primary));
    egui_view_set_size(EGUI_VIEW_OF(&tool_tip_primary), TOOL_TIP_PRIMARY_WIDTH, TOOL_TIP_PRIMARY_HEIGHT);
    egui_view_tool_tip_set_font(EGUI_VIEW_OF(&tool_tip_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tool_tip_set_meta_font(EGUI_VIEW_OF(&tool_tip_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tool_tip_set_snapshots(EGUI_VIEW_OF(&tool_tip_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_tool_tip_set_show_delay(EGUI_VIEW_OF(&tool_tip_primary), 420);
    egui_view_tool_tip_set_palette(EGUI_VIEW_OF(&tool_tip_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x1A2734),
                                   EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x748190),
                                   EGUI_COLOR_HEX(0xDDE5EB), EGUI_COLOR_HEX(0xF5F8FB), EGUI_COLOR_HEX(0xD0D9E2));
    egui_view_set_margin(EGUI_VIEW_OF(&tool_tip_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&tool_tip_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TOOL_TIP_BOTTOM_ROW_WIDTH, TOOL_TIP_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_tool_tip_init(EGUI_VIEW_OF(&tool_tip_compact));
    egui_view_set_size(EGUI_VIEW_OF(&tool_tip_compact), TOOL_TIP_PREVIEW_WIDTH, TOOL_TIP_PREVIEW_HEIGHT);
    egui_view_tool_tip_set_font(EGUI_VIEW_OF(&tool_tip_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tool_tip_set_meta_font(EGUI_VIEW_OF(&tool_tip_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tool_tip_set_snapshots(EGUI_VIEW_OF(&tool_tip_compact), compact_snapshots, COMPACT_SNAPSHOT_COUNT);
    egui_view_tool_tip_set_compact_mode(EGUI_VIEW_OF(&tool_tip_compact), 1);
    egui_view_tool_tip_set_palette(EGUI_VIEW_OF(&tool_tip_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x1A2734),
                                   EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x748190),
                                   EGUI_COLOR_HEX(0xDDE5EB), EGUI_COLOR_HEX(0xF5F8FB), EGUI_COLOR_HEX(0xD0D9E2));
    egui_view_tool_tip_override_static_preview_api(EGUI_VIEW_OF(&tool_tip_compact), &tool_tip_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    tool_tip_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&tool_tip_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&tool_tip_compact));

    egui_view_tool_tip_init(EGUI_VIEW_OF(&tool_tip_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&tool_tip_read_only), TOOL_TIP_PREVIEW_WIDTH, TOOL_TIP_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&tool_tip_read_only), 8, 0, 0, 0);
    egui_view_tool_tip_set_font(EGUI_VIEW_OF(&tool_tip_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tool_tip_set_meta_font(EGUI_VIEW_OF(&tool_tip_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tool_tip_set_snapshots(EGUI_VIEW_OF(&tool_tip_read_only), read_only_snapshots, 1);
    egui_view_tool_tip_set_compact_mode(EGUI_VIEW_OF(&tool_tip_read_only), 1);
    egui_view_tool_tip_set_read_only_mode(EGUI_VIEW_OF(&tool_tip_read_only), 1);
    egui_view_tool_tip_set_palette(EGUI_VIEW_OF(&tool_tip_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0x536474),
                                   EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0x9FB0BE), EGUI_COLOR_HEX(0xBFAE95), EGUI_COLOR_HEX(0xB2BDC8),
                                   EGUI_COLOR_HEX(0xE8EDF2), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD9E1E8));
    egui_view_tool_tip_override_static_preview_api(EGUI_VIEW_OF(&tool_tip_read_only), &tool_tip_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    tool_tip_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&tool_tip_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&tool_tip_read_only));

    reset_primary_state(0);
    apply_compact_snapshot(0);
    apply_read_only_state();

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
            reset_primary_state(0);
            apply_compact_snapshot(0);
            apply_read_only_state();
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_FRAME_WAIT);
        return true;
    case 2:
        set_click_tool_tip_part(p_action, EGUI_VIEW_OF(&tool_tip_primary), EGUI_VIEW_TOOL_TIP_PART_TARGET, TOOL_TIP_RECORD_WAIT);
        return true;
    case 3:
        EGUI_SIM_SET_WAIT(p_action, 560);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(1);
            egui_view_tool_tip_set_open(EGUI_VIEW_OF(&tool_tip_primary), 1);
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_snapshot(2);
            egui_view_tool_tip_set_open(EGUI_VIEW_OF(&tool_tip_primary), 0);
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_WAIT);
        return true;
    case 8:
        set_click_tool_tip_part(p_action, EGUI_VIEW_OF(&tool_tip_primary), EGUI_VIEW_TOOL_TIP_PART_TARGET, TOOL_TIP_RECORD_WAIT);
        return true;
    case 9:
        EGUI_SIM_SET_WAIT(p_action, 560);
        return true;
    case 10:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_FRAME_WAIT);
        return true;
    case 11:
        if (first_call)
        {
            dispatch_primary_key(EGUI_KEY_CODE_ESCAPE);
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_WAIT);
        return true;
    case 12:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_FRAME_WAIT);
        return true;
    case 13:
        if (first_call)
        {
            apply_compact_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&tool_tip_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_WAIT);
        return true;
    case 14:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_FRAME_WAIT);
        return true;
    case 15:
        set_click_view_center(p_action, EGUI_VIEW_OF(&tool_tip_compact), TOOL_TIP_RECORD_WAIT);
        return true;
    case 16:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOOL_TIP_RECORD_FRAME_WAIT);
        return true;
    case 17:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
