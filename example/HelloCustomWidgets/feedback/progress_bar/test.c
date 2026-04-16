#include <stdio.h>

#include "egui.h"
#include "egui_view_progress_bar.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define PROGRESS_BAR_ROOT_WIDTH        224
#define PROGRESS_BAR_ROOT_HEIGHT       116
#define PROGRESS_BAR_PRIMARY_WIDTH     196
#define PROGRESS_BAR_PRIMARY_HEIGHT    18
#define PROGRESS_BAR_STATUS_WIDTH      196
#define PROGRESS_BAR_STATUS_HEIGHT     14
#define PROGRESS_BAR_PREVIEW_WIDTH     96
#define PROGRESS_BAR_PREVIEW_HEIGHT    12
#define PROGRESS_BAR_BOTTOM_ROW_WIDTH  200
#define PROGRESS_BAR_BOTTOM_ROW_HEIGHT 12
#define PROGRESS_BAR_RECORD_WAIT       90
#define PROGRESS_BAR_RECORD_ANIM_WAIT  260
#define PROGRESS_BAR_RECORD_FRAME_WAIT 170
#define PROGRESS_BAR_RECORD_FINAL_WAIT 520

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_progress_bar_t progress_bar_primary;
static egui_view_label_t progress_bar_status;
static egui_view_linearlayout_t bottom_row;
static egui_view_progress_bar_t progress_bar_paused;
static egui_view_api_t progress_bar_paused_api;
static egui_view_progress_bar_t progress_bar_error;
static egui_view_api_t progress_bar_error_api;
static uint8_t ui_ready;

static char primary_status_text[24];

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "ProgressBar";

static void layout_page(void);

static void update_primary_complete_status(uint8_t value)
{
    snprintf(primary_status_text, sizeof(primary_status_text), "%u%% complete", value);
    egui_view_label_set_text(EGUI_VIEW_OF(&progress_bar_status), primary_status_text);
    if (ui_ready)
    {
        layout_page();
    }
}

static void update_primary_loading_status(void)
{
    egui_view_label_set_text(EGUI_VIEW_OF(&progress_bar_status), "Syncing...");
    if (ui_ready)
    {
        layout_page();
    }
}

static void on_primary_progress_changed(egui_view_t *self, uint8_t progress)
{
    EGUI_UNUSED(self);
    update_primary_complete_status(progress);
}

static void apply_primary_loading_state(void)
{
    hcw_progress_bar_set_value(EGUI_VIEW_OF(&progress_bar_primary), 28);
    hcw_progress_bar_apply_indeterminate_style(EGUI_VIEW_OF(&progress_bar_primary));
    update_primary_loading_status();
}

static void apply_preview_values(void)
{
    hcw_progress_bar_set_value(EGUI_VIEW_OF(&progress_bar_paused), 46);
    hcw_progress_bar_set_value(EGUI_VIEW_OF(&progress_bar_error), 82);
    if (ui_ready)
    {
        layout_page();
    }
}

static void layout_local_views(void)
{
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
}

static void layout_page(void)
{
    layout_local_views();
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    layout_page();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), PROGRESS_BAR_ROOT_WIDTH, PROGRESS_BAR_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), PROGRESS_BAR_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    hcw_progress_bar_init(EGUI_VIEW_OF(&progress_bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&progress_bar_primary), PROGRESS_BAR_PRIMARY_WIDTH, PROGRESS_BAR_PRIMARY_HEIGHT);
    hcw_progress_bar_apply_indeterminate_style(EGUI_VIEW_OF(&progress_bar_primary));
    egui_view_progress_bar_set_on_progress_listener(EGUI_VIEW_OF(&progress_bar_primary), on_primary_progress_changed);
    egui_view_set_margin(EGUI_VIEW_OF(&progress_bar_primary), 0, 0, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&progress_bar_primary));

    egui_view_label_init(EGUI_VIEW_OF(&progress_bar_status));
    egui_view_set_size(EGUI_VIEW_OF(&progress_bar_status), PROGRESS_BAR_STATUS_WIDTH, PROGRESS_BAR_STATUS_HEIGHT);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&progress_bar_status), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&progress_bar_status), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&progress_bar_status), EGUI_COLOR_HEX(0x5F6E7D), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&progress_bar_status), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&progress_bar_status));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), PROGRESS_BAR_BOTTOM_ROW_WIDTH, PROGRESS_BAR_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    hcw_progress_bar_init(EGUI_VIEW_OF(&progress_bar_paused));
    egui_view_set_size(EGUI_VIEW_OF(&progress_bar_paused), PROGRESS_BAR_PREVIEW_WIDTH, PROGRESS_BAR_PREVIEW_HEIGHT);
    hcw_progress_bar_apply_paused_style(EGUI_VIEW_OF(&progress_bar_paused));
    hcw_progress_bar_override_static_preview_api(EGUI_VIEW_OF(&progress_bar_paused), &progress_bar_paused_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&progress_bar_paused), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&progress_bar_paused));

    hcw_progress_bar_init(EGUI_VIEW_OF(&progress_bar_error));
    egui_view_set_size(EGUI_VIEW_OF(&progress_bar_error), PROGRESS_BAR_PREVIEW_WIDTH, PROGRESS_BAR_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&progress_bar_error), 8, 0, 0, 0);
    hcw_progress_bar_apply_error_style(EGUI_VIEW_OF(&progress_bar_error));
    hcw_progress_bar_override_static_preview_api(EGUI_VIEW_OF(&progress_bar_error), &progress_bar_error_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&progress_bar_error), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&progress_bar_error));

    apply_primary_loading_state();
    apply_preview_values();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_loading_state();
    apply_preview_values();
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_complete_state(uint8_t value)
{
    hcw_progress_bar_set_value(EGUI_VIEW_OF(&progress_bar_primary), value);
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
            apply_primary_loading_state();
            apply_preview_values();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PROGRESS_BAR_RECORD_FRAME_WAIT);
        return true;
    case 1:
        EGUI_SIM_SET_WAIT(p_action, PROGRESS_BAR_RECORD_ANIM_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PROGRESS_BAR_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_complete_state(92);
        }
        EGUI_SIM_SET_WAIT(p_action, PROGRESS_BAR_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PROGRESS_BAR_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, PROGRESS_BAR_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
