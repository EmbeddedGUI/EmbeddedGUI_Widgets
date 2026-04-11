#include <string.h>

#include "egui.h"
#include "egui_view_slider.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SLIDER_ROOT_WIDTH        224
#define SLIDER_ROOT_HEIGHT       174
#define SLIDER_PRIMARY_WIDTH     196
#define SLIDER_PRIMARY_HEIGHT    38
#define SLIDER_PREVIEW_WIDTH     104
#define SLIDER_PREVIEW_HEIGHT    28
#define SLIDER_BOTTOM_ROW_WIDTH  216
#define SLIDER_BOTTOM_ROW_HEIGHT 28
#define SLIDER_RECORD_WAIT       90
#define SLIDER_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_slider_t slider_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_slider_t slider_compact;
static egui_view_slider_t slider_read_only;
static egui_view_api_t slider_primary_api;
static egui_view_api_t slider_compact_api;
static egui_view_api_t slider_read_only_api;
static uint8_t ui_ready = 0;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Slider";
static const uint8_t primary_values[] = {52, 18, 74};
static const uint8_t compact_values[] = {28, 64};
static const uint8_t read_only_value = 42;

static void relayout_demo(void)
{
    if (!ui_ready)
    {
        return;
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void apply_primary_value(uint8_t index)
{
    hcw_slider_set_value(EGUI_VIEW_OF(&slider_primary), primary_values[index % EGUI_ARRAY_SIZE(primary_values)]);
    relayout_demo();
}

static void apply_compact_value(uint8_t index)
{
    hcw_slider_set_value(EGUI_VIEW_OF(&slider_compact), compact_values[index % EGUI_ARRAY_SIZE(compact_values)]);
}

static void apply_read_only_value(void)
{
    hcw_slider_set_value(EGUI_VIEW_OF(&slider_read_only), read_only_value);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SLIDER_ROOT_WIDTH, SLIDER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SLIDER_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_slider_init(EGUI_VIEW_OF(&slider_primary));
    egui_view_set_size(EGUI_VIEW_OF(&slider_primary), SLIDER_PRIMARY_WIDTH, SLIDER_PRIMARY_HEIGHT);
    hcw_slider_apply_standard_style(EGUI_VIEW_OF(&slider_primary));
    hcw_slider_override_interaction_api(EGUI_VIEW_OF(&slider_primary), &slider_primary_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&slider_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&slider_primary), 0, 0, 0, 12);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&slider_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SLIDER_BOTTOM_ROW_WIDTH, SLIDER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_slider_init(EGUI_VIEW_OF(&slider_compact));
    egui_view_set_size(EGUI_VIEW_OF(&slider_compact), SLIDER_PREVIEW_WIDTH, SLIDER_PREVIEW_HEIGHT);
    hcw_slider_apply_compact_style(EGUI_VIEW_OF(&slider_compact));
    hcw_slider_override_static_preview_api(EGUI_VIEW_OF(&slider_compact), &slider_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&slider_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&slider_compact));

    egui_view_slider_init(EGUI_VIEW_OF(&slider_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&slider_read_only), SLIDER_PREVIEW_WIDTH, SLIDER_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&slider_read_only), 8, 0, 0, 0);
    hcw_slider_apply_read_only_style(EGUI_VIEW_OF(&slider_read_only));
    hcw_slider_override_static_preview_api(EGUI_VIEW_OF(&slider_read_only), &slider_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&slider_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&slider_read_only));

    apply_primary_value(0);
    apply_compact_value(0);
    apply_read_only_value();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
    ui_ready = 1;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&slider_primary));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&slider_primary), &event);

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&slider_primary), &event);
    relayout_demo();
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
            apply_primary_value(0);
            apply_compact_value(0);
            apply_read_only_value();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&slider_primary));
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_PLUS);
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_value(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&slider_primary));
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SLIDER_RECORD_FRAME_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            apply_compact_value(1);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
