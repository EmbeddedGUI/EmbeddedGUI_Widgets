#include "egui.h"
#include "egui_view_dialog_sheet.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define DIALOG_SHEET_ROOT_WIDTH        224
#define DIALOG_SHEET_ROOT_HEIGHT       258
#define DIALOG_SHEET_PRIMARY_WIDTH     196
#define DIALOG_SHEET_PRIMARY_HEIGHT    132
#define DIALOG_SHEET_PREVIEW_WIDTH     104
#define DIALOG_SHEET_PREVIEW_HEIGHT    86
#define DIALOG_SHEET_BOTTOM_ROW_WIDTH  216
#define DIALOG_SHEET_BOTTOM_ROW_HEIGHT 86
#define DIALOG_SHEET_RECORD_WAIT       90
#define DIALOG_SHEET_RECORD_FRAME_WAIT 170
#define PRIMARY_SNAPSHOT_COUNT         ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))
#define COMPACT_SNAPSHOT_COUNT         ((uint8_t)(sizeof(compact_snapshots) / sizeof(compact_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_dialog_sheet_t sheet_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_dialog_sheet_t sheet_compact;
static egui_view_dialog_sheet_t sheet_read_only;
static egui_view_api_t sheet_compact_api;
static egui_view_api_t sheet_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Dialog Sheet";
static uint8_t primary_snapshot_index = 0;
static uint8_t compact_snapshot_index = 0;

static const egui_view_dialog_sheet_snapshot_t primary_snapshots[] = {
        {"Sync issue", "Reconnect account?", "Resume sync for review.", "Reconnect", "Later", "Sync", "Queue paused", EGUI_VIEW_DIALOG_SHEET_TONE_WARNING, 1, 1,
         EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
        {"Delete draft", "Delete unfinished layout?", "Remove local draft.", "Delete", "Cancel", "Draft", "Cannot undo", EGUI_VIEW_DIALOG_SHEET_TONE_ERROR, 1,
         1, EGUI_VIEW_DIALOG_SHEET_ACTION_SECONDARY},
        {"Template", "Apply starter scene?", "Load base panels.", "Apply", NULL, "Template", "Saved", EGUI_VIEW_DIALOG_SHEET_TONE_ACCENT, 0, 0,
         EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
        {"Publishing", "Send build to review?", "Share build now.", "Send", NULL, "Review", "Ready", EGUI_VIEW_DIALOG_SHEET_TONE_SUCCESS, 0, 1,
         EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
};

static const egui_view_dialog_sheet_snapshot_t compact_snapshots[] = {
        {"Network", "Reconnect?", "Resume sync.", "Retry", NULL, "Sync", "", EGUI_VIEW_DIALOG_SHEET_TONE_WARNING, 0, 0, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
        {"Review", "Send build?", "Share build.", "Send", NULL, "Build", "", EGUI_VIEW_DIALOG_SHEET_TONE_ACCENT, 0, 0, EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
};

static const egui_view_dialog_sheet_snapshot_t read_only_snapshots[] = {
        {"Read only", "Review", "Static state.", NULL, NULL, "Read only", "", EGUI_VIEW_DIALOG_SHEET_TONE_NEUTRAL, 0, 0,
         EGUI_VIEW_DIALOG_SHEET_ACTION_PRIMARY},
};

static void dismiss_primary_dialog_sheet_focus(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&sheet_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_dialog_sheet_focus();
    }
    return 1;
}

static void apply_primary_snapshot(uint8_t index)
{
    primary_snapshot_index = (uint8_t)(index % PRIMARY_SNAPSHOT_COUNT);
    egui_view_dialog_sheet_set_current_snapshot(EGUI_VIEW_OF(&sheet_primary), primary_snapshot_index);
}

static void apply_compact_snapshot(uint8_t index)
{
    compact_snapshot_index = (uint8_t)(index % COMPACT_SNAPSHOT_COUNT);
    egui_view_dialog_sheet_set_current_snapshot(EGUI_VIEW_OF(&sheet_compact), compact_snapshot_index);
}

static void apply_read_only_state(void)
{
    egui_view_dialog_sheet_set_current_snapshot(EGUI_VIEW_OF(&sheet_read_only), 0);
    egui_view_dialog_sheet_set_read_only_mode(EGUI_VIEW_OF(&sheet_read_only), 1);
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), DIALOG_SHEET_ROOT_WIDTH, DIALOG_SHEET_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), DIALOG_SHEET_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 6, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_dialog_sheet_init(EGUI_VIEW_OF(&sheet_primary));
    egui_view_set_size(EGUI_VIEW_OF(&sheet_primary), DIALOG_SHEET_PRIMARY_WIDTH, DIALOG_SHEET_PRIMARY_HEIGHT);
    egui_view_dialog_sheet_set_font(EGUI_VIEW_OF(&sheet_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_dialog_sheet_set_meta_font(EGUI_VIEW_OF(&sheet_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&sheet_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_dialog_sheet_set_palette(EGUI_VIEW_OF(&sheet_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD7DFE6), EGUI_COLOR_HEX(0xD7E0E7),
                                       EGUI_COLOR_HEX(0x1C2835), EGUI_COLOR_HEX(0x6E7B88), EGUI_COLOR_HEX(0x1E69A8), EGUI_COLOR_HEX(0x1D6F4A),
                                       EGUI_COLOR_HEX(0x946019), EGUI_COLOR_HEX(0xB84B45), EGUI_COLOR_HEX(0x7D8996));
    egui_view_set_margin(EGUI_VIEW_OF(&sheet_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&sheet_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), DIALOG_SHEET_BOTTOM_ROW_WIDTH, DIALOG_SHEET_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_dialog_sheet_init(EGUI_VIEW_OF(&sheet_compact));
    egui_view_set_size(EGUI_VIEW_OF(&sheet_compact), DIALOG_SHEET_PREVIEW_WIDTH, DIALOG_SHEET_PREVIEW_HEIGHT);
    egui_view_dialog_sheet_set_font(EGUI_VIEW_OF(&sheet_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_dialog_sheet_set_meta_font(EGUI_VIEW_OF(&sheet_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&sheet_compact), compact_snapshots, COMPACT_SNAPSHOT_COUNT);
    egui_view_dialog_sheet_set_compact_mode(EGUI_VIEW_OF(&sheet_compact), 1);
    egui_view_dialog_sheet_set_palette(EGUI_VIEW_OF(&sheet_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD7DFE6), EGUI_COLOR_HEX(0xD7E0E7),
                                       EGUI_COLOR_HEX(0x1C2835), EGUI_COLOR_HEX(0x6E7B88), EGUI_COLOR_HEX(0x1E69A8), EGUI_COLOR_HEX(0x1D6F4A),
                                       EGUI_COLOR_HEX(0x946019), EGUI_COLOR_HEX(0xB84B45), EGUI_COLOR_HEX(0x7D8996));
    egui_view_dialog_sheet_override_static_preview_api(EGUI_VIEW_OF(&sheet_compact), &sheet_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    sheet_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&sheet_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&sheet_compact));

    egui_view_dialog_sheet_init(EGUI_VIEW_OF(&sheet_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&sheet_read_only), DIALOG_SHEET_PREVIEW_WIDTH, DIALOG_SHEET_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&sheet_read_only), 8, 0, 0, 0);
    egui_view_dialog_sheet_set_font(EGUI_VIEW_OF(&sheet_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_dialog_sheet_set_meta_font(EGUI_VIEW_OF(&sheet_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_dialog_sheet_set_snapshots(EGUI_VIEW_OF(&sheet_read_only), read_only_snapshots, 1);
    egui_view_dialog_sheet_set_compact_mode(EGUI_VIEW_OF(&sheet_read_only), 1);
    egui_view_dialog_sheet_set_read_only_mode(EGUI_VIEW_OF(&sheet_read_only), 1);
    egui_view_dialog_sheet_set_palette(EGUI_VIEW_OF(&sheet_read_only), EGUI_COLOR_HEX(0xFCFDFE), EGUI_COLOR_HEX(0xDCE3E9), EGUI_COLOR_HEX(0xD0D8DF),
                                       EGUI_COLOR_HEX(0x8A97A4), EGUI_COLOR_HEX(0x97A3AE), EGUI_COLOR_HEX(0xA5B1BC), EGUI_COLOR_HEX(0xB1BEB8),
                                       EGUI_COLOR_HEX(0xBFB39F), EGUI_COLOR_HEX(0xC2B2AE), EGUI_COLOR_HEX(0xB1BBC5));
    egui_view_dialog_sheet_override_static_preview_api(EGUI_VIEW_OF(&sheet_read_only), &sheet_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    sheet_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&sheet_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&sheet_read_only));

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
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&sheet_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_FRAME_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            set_click_view_center(p_action, EGUI_VIEW_OF(&sheet_compact), DIALOG_SHEET_RECORD_WAIT);
        }
        return true;
    case 11:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DIALOG_SHEET_RECORD_FRAME_WAIT);
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
