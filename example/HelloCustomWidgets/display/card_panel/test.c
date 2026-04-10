#include "egui.h"
#include "egui_view_card_panel.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define CARD_PANEL_ROOT_WIDTH        224
#define CARD_PANEL_ROOT_HEIGHT       252
#define CARD_PANEL_PRIMARY_WIDTH     196
#define CARD_PANEL_PRIMARY_HEIGHT    122
#define CARD_PANEL_PREVIEW_WIDTH     104
#define CARD_PANEL_PREVIEW_HEIGHT    90
#define CARD_PANEL_BOTTOM_ROW_WIDTH  216
#define CARD_PANEL_BOTTOM_ROW_HEIGHT 90
#define CARD_PANEL_RECORD_WAIT       90
#define CARD_PANEL_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_card_panel_t panel_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_card_panel_t panel_compact;
static egui_view_card_panel_t panel_read_only;
static egui_view_api_t panel_compact_api;
static egui_view_api_t panel_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Card Panel";

static const egui_view_card_panel_snapshot_t primary_snapshots[] = {
        {"OVERVIEW", "Workspace status", "Three flows stay aligned.", "98%", "uptime", "Today", "Two checks wait.", "Footer stays readable.", "Open", 0, 1},
        {"SYNC", "Design review", "New handoff needs approval.", "4", "changes", "Next step", "Confirm spacing tokens.", "Summary stays close.", "Review", 2,
         1},
        {"DEPLOY", "Release notes", "Ready for staged publish.", "6", "items", "Channel", "Internal preview for QA.", "Card stays calm on dense pages.",
         "Publish", 1, 0},
        {"ARCHIVE", "Readback summary", "Older detail stays available.", "12", "pages", "History", "Summary stays visible.", "Read only mode still works.",
         "Browse", 3, 0},
};

static const egui_view_card_panel_snapshot_t compact_snapshots[] = {
        {"TASK", "Compact", "Short.", "12", "tasks", "Focus", "", "Clear layout.", "Open", 0, 1},
        {"WARN", "Review", "Warning.", "2", "blocks", "Check", "", "Small card.", "Fix", 2, 1},
};

static const egui_view_card_panel_snapshot_t read_only_snapshots[] = {
        {"ARCHIVE", "Archive", "Muted.", "7", "notes", "History", "", "Preview only.", "", 3, 0},
};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&panel_primary),
                                              index % (sizeof(primary_snapshots) / sizeof(primary_snapshots[0])));
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&panel_compact),
                                              index % (sizeof(compact_snapshots) / sizeof(compact_snapshots[0])));
}

static void apply_read_only_snapshot(void)
{
    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&panel_read_only), 0);
}

static void apply_read_only_state(void)
{
    apply_read_only_snapshot();
    egui_view_card_panel_set_read_only_mode(EGUI_VIEW_OF(&panel_read_only), 1);
}

static void dismiss_primary_card_panel_focus(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&panel_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_card_panel_focus();
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
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), CARD_PANEL_ROOT_WIDTH, CARD_PANEL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), CARD_PANEL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_card_panel_init(EGUI_VIEW_OF(&panel_primary));
    egui_view_set_size(EGUI_VIEW_OF(&panel_primary), CARD_PANEL_PRIMARY_WIDTH, CARD_PANEL_PRIMARY_HEIGHT);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&panel_primary), primary_snapshots, 4);
    egui_view_card_panel_set_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_card_panel_set_meta_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_palette(EGUI_VIEW_OF(&panel_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x1A2734),
                                     EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                     EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&panel_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&panel_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), CARD_PANEL_BOTTOM_ROW_WIDTH, CARD_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_card_panel_init(EGUI_VIEW_OF(&panel_compact));
    egui_view_set_size(EGUI_VIEW_OF(&panel_compact), CARD_PANEL_PREVIEW_WIDTH, CARD_PANEL_PREVIEW_HEIGHT);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&panel_compact), compact_snapshots, 2);
    egui_view_card_panel_set_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_meta_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_compact_mode(EGUI_VIEW_OF(&panel_compact), 1);
    egui_view_card_panel_set_palette(EGUI_VIEW_OF(&panel_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x1A2734),
                                     EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                     EGUI_COLOR_HEX(0x7A8796));
    egui_view_card_panel_override_static_preview_api(EGUI_VIEW_OF(&panel_compact), &panel_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    panel_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&panel_compact));

    egui_view_card_panel_init(EGUI_VIEW_OF(&panel_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&panel_read_only), CARD_PANEL_PREVIEW_WIDTH, CARD_PANEL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&panel_read_only), 8, 0, 0, 0);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&panel_read_only), read_only_snapshots, 1);
    egui_view_card_panel_set_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_meta_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_compact_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_card_panel_set_read_only_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_card_panel_set_palette(EGUI_VIEW_OF(&panel_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0x233241),
                                     EGUI_COLOR_HEX(0x708091), EGUI_COLOR_HEX(0x98A5B2), EGUI_COLOR_HEX(0xA7B4BF), EGUI_COLOR_HEX(0xB8B0A2),
                                     EGUI_COLOR_HEX(0xB4BDC8));
    egui_view_card_panel_override_static_preview_api(EGUI_VIEW_OF(&panel_read_only), &panel_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    panel_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&panel_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_read_only_state();

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

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
            apply_read_only_state();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&panel_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            set_click_view_center(p_action, EGUI_VIEW_OF(&panel_compact), CARD_PANEL_RECORD_WAIT);
        }
        return true;
    case 11:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 12:
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
