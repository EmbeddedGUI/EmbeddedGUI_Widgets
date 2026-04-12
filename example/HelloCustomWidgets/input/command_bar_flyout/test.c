#include "egui.h"
#include "egui_view_command_bar_flyout.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define COMMAND_BAR_FLYOUT_ROOT_WIDTH        224
#define COMMAND_BAR_FLYOUT_ROOT_HEIGHT       272
#define COMMAND_BAR_FLYOUT_PRIMARY_WIDTH     196
#define COMMAND_BAR_FLYOUT_PRIMARY_HEIGHT    160
#define COMMAND_BAR_FLYOUT_PREVIEW_WIDTH     104
#define COMMAND_BAR_FLYOUT_PREVIEW_HEIGHT    72
#define COMMAND_BAR_FLYOUT_BOTTOM_ROW_WIDTH  216
#define COMMAND_BAR_FLYOUT_BOTTOM_ROW_HEIGHT 72
#define COMMAND_BAR_FLYOUT_RECORD_WAIT       90
#define COMMAND_BAR_FLYOUT_RECORD_FRAME_WAIT 180

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_command_bar_flyout_t flyout_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_command_bar_flyout_t flyout_compact;
static egui_view_command_bar_flyout_t flyout_disabled;
static egui_view_api_t flyout_compact_api;
static egui_view_api_t flyout_disabled_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Command Bar Flyout";

static const egui_view_command_bar_flyout_primary_item_t primary_items_edit[] = {
        {"SV", "Save", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 1, 1},
        {"SH", "Share", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 0, 1},
        {"EX", "Export", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_WARNING, 0, 1},
};

static const egui_view_command_bar_flyout_secondary_item_t secondary_items_edit[] = {
        {"LK", "Copy link", "Ctrl+L", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"DP", "Duplicate", "Alt+D", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"AR", "Archive", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_WARNING, 0, 1, 1},
};

static const egui_view_command_bar_flyout_primary_item_t primary_items_review[] = {
        {"OK", "Approve", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_SUCCESS, 1, 1},
        {"CM", "Comment", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 0, 1},
        {"BL", "Block", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER, 0, 1},
};

static const egui_view_command_bar_flyout_secondary_item_t secondary_items_review[] = {
        {"AS", "Assign", "Ctrl+A", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"ES", "Escalate", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_WARNING, 0, 1, 0},
        {"DL", "Delete", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER, 0, 1, 1},
};

static const egui_view_command_bar_flyout_primary_item_t primary_items_quick[] = {
        {"PN", "Pin", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 0, 1},
        {"SH", "Share", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_SUCCESS, 0, 1},
};

static const egui_view_command_bar_flyout_secondary_item_t secondary_items_quick[] = {
        {"OP", "Open", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"RM", "Remove", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER, 0, 1, 1},
};

static const egui_view_command_bar_flyout_primary_item_t primary_items_layout[] = {
        {"AL", "Align", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 0, 1},
        {"DS", "Density", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_WARNING, 1, 1},
        {"GR", "Grid", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1},
};

static const egui_view_command_bar_flyout_secondary_item_t secondary_items_layout[] = {
        {"WP", "Wrap", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"RS", "Resize", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_SUCCESS, 0, 1, 0},
        {"RT", "Reset", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER, 0, 1, 1},
};

static const egui_view_command_bar_flyout_primary_item_t compact_primary_items[] = {
        {"SV", "Save", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 1, 1},
        {"SH", "Share", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 0, 1},
};

static const egui_view_command_bar_flyout_secondary_item_t compact_secondary_items_0[] = {
        {"LK", "Copy", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"MR", "More", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
};

static const egui_view_command_bar_flyout_secondary_item_t compact_secondary_items_1[] = {
        {"AP", "Approve", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_SUCCESS, 0, 1, 0},
        {"DL", "Delete", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER, 0, 1, 1},
};

static const egui_view_command_bar_flyout_primary_item_t disabled_primary_items[] = {
        {"LK", "Locked", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 0},
        {"SH", "Share", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 0},
};

static const egui_view_command_bar_flyout_secondary_item_t disabled_secondary_items[] = {
        {"OP", "Open", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 0, 0},
        {"DL", "Delete", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER, 0, 0, 1},
};

static const egui_view_command_bar_flyout_snapshot_t primary_snapshots[] = {
        {"Edit", "Page actions", "Share surface", "Primary commands stay above the divider", primary_items_edit,
         EGUI_ARRAY_SIZE(primary_items_edit), secondary_items_edit, EGUI_ARRAY_SIZE(secondary_items_edit), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
        {"Review", "Review actions", "Review surface", "Approve now or route from secondary actions", primary_items_review,
         EGUI_ARRAY_SIZE(primary_items_review), secondary_items_review, EGUI_ARRAY_SIZE(secondary_items_review), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(0)},
        {"Quick", "Quick actions", "Quick surface", "Closed trigger uses the same shell", primary_items_quick, EGUI_ARRAY_SIZE(primary_items_quick),
         secondary_items_quick, EGUI_ARRAY_SIZE(secondary_items_quick), 0, EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER},
        {"Layout", "Layout actions", "Layout surface", "Keep the flyout open while tuning density", primary_items_layout,
         EGUI_ARRAY_SIZE(primary_items_layout), secondary_items_layout, EGUI_ARRAY_SIZE(secondary_items_layout), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(1)},
};

static const egui_view_command_bar_flyout_snapshot_t compact_snapshots[] = {
        {"Quick", "Compact flyout", "Compact surface", "", compact_primary_items, EGUI_ARRAY_SIZE(compact_primary_items), compact_secondary_items_0,
         EGUI_ARRAY_SIZE(compact_secondary_items_0), 1, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
        {"Review", "Compact flyout", "Compact review", "", compact_primary_items, EGUI_ARRAY_SIZE(compact_primary_items), compact_secondary_items_1,
         EGUI_ARRAY_SIZE(compact_secondary_items_1), 1, EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(0)},
};

static const egui_view_command_bar_flyout_snapshot_t disabled_snapshot = {
        "Locked", "Disabled flyout", "Disabled surface", "", disabled_primary_items, EGUI_ARRAY_SIZE(disabled_primary_items), disabled_secondary_items,
        EGUI_ARRAY_SIZE(disabled_secondary_items), 1, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)};

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        egui_view_clear_focus(EGUI_VIEW_OF(&flyout_primary));
    }
#else
    EGUI_UNUSED(event);
#endif
    return 1;
}

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_command_bar_flyout_set_current_snapshot(EGUI_VIEW_OF(&flyout_primary), index % EGUI_ARRAY_SIZE(primary_snapshots));
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_command_bar_flyout_set_current_snapshot(EGUI_VIEW_OF(&flyout_compact), index % EGUI_ARRAY_SIZE(compact_snapshots));
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

static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&flyout_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&flyout_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&flyout_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&flyout_primary), &event);
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), COMMAND_BAR_FLYOUT_ROOT_WIDTH, COMMAND_BAR_FLYOUT_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), COMMAND_BAR_FLYOUT_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_command_bar_flyout_init(EGUI_VIEW_OF(&flyout_primary));
    egui_view_set_size(EGUI_VIEW_OF(&flyout_primary), COMMAND_BAR_FLYOUT_PRIMARY_WIDTH, COMMAND_BAR_FLYOUT_PRIMARY_HEIGHT);
    egui_view_command_bar_flyout_set_font(EGUI_VIEW_OF(&flyout_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_command_bar_flyout_set_meta_font(EGUI_VIEW_OF(&flyout_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_flyout_set_snapshots(EGUI_VIEW_OF(&flyout_primary), primary_snapshots, EGUI_ARRAY_SIZE(primary_snapshots));
    egui_view_set_margin(EGUI_VIEW_OF(&flyout_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&flyout_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), COMMAND_BAR_FLYOUT_BOTTOM_ROW_WIDTH, COMMAND_BAR_FLYOUT_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_command_bar_flyout_init(EGUI_VIEW_OF(&flyout_compact));
    egui_view_set_size(EGUI_VIEW_OF(&flyout_compact), COMMAND_BAR_FLYOUT_PREVIEW_WIDTH, COMMAND_BAR_FLYOUT_PREVIEW_HEIGHT);
    egui_view_command_bar_flyout_set_font(EGUI_VIEW_OF(&flyout_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_flyout_set_meta_font(EGUI_VIEW_OF(&flyout_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_flyout_set_snapshots(EGUI_VIEW_OF(&flyout_compact), compact_snapshots, EGUI_ARRAY_SIZE(compact_snapshots));
    egui_view_command_bar_flyout_set_compact_mode(EGUI_VIEW_OF(&flyout_compact), 1);
    egui_view_command_bar_flyout_override_static_preview_api(EGUI_VIEW_OF(&flyout_compact), &flyout_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    flyout_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flyout_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&flyout_compact));

    egui_view_command_bar_flyout_init(EGUI_VIEW_OF(&flyout_disabled));
    egui_view_set_size(EGUI_VIEW_OF(&flyout_disabled), COMMAND_BAR_FLYOUT_PREVIEW_WIDTH, COMMAND_BAR_FLYOUT_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&flyout_disabled), 8, 0, 0, 0);
    egui_view_command_bar_flyout_set_font(EGUI_VIEW_OF(&flyout_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_flyout_set_meta_font(EGUI_VIEW_OF(&flyout_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_command_bar_flyout_set_snapshots(EGUI_VIEW_OF(&flyout_disabled), &disabled_snapshot, 1);
    egui_view_command_bar_flyout_set_compact_mode(EGUI_VIEW_OF(&flyout_disabled), 1);
    egui_view_command_bar_flyout_set_disabled_mode(EGUI_VIEW_OF(&flyout_disabled), 1);
    egui_view_command_bar_flyout_set_palette(EGUI_VIEW_OF(&flyout_disabled), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF6F8FB),
                                             EGUI_COLOR_HEX(0xD8E0E7), EGUI_COLOR_HEX(0x51616F), EGUI_COLOR_HEX(0x8B98A4),
                                             EGUI_COLOR_HEX(0x90A3BF), EGUI_COLOR_HEX(0x90A693), EGUI_COLOR_HEX(0xAF996B),
                                             EGUI_COLOR_HEX(0xA58C87), EGUI_COLOR_HEX(0x95A2AF), EGUI_COLOR_HEX(0xD5DCE3));
    egui_view_command_bar_flyout_override_static_preview_api(EGUI_VIEW_OF(&flyout_disabled), &flyout_disabled_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    flyout_disabled_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flyout_disabled), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&flyout_disabled));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&flyout_primary));
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
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&flyout_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(1);
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_primary_snapshot(3);
            apply_compact_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&flyout_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMMAND_BAR_FLYOUT_RECORD_FRAME_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            set_click_view_center(p_action, EGUI_VIEW_OF(&flyout_compact), 220);
        }
        return true;
    case 11:
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
