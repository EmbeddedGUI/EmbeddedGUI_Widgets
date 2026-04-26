#include "egui.h"
#include "egui_view_spin_button.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SPIN_BUTTON_ROOT_WIDTH        224
#define SPIN_BUTTON_ROOT_HEIGHT       156
#define SPIN_BUTTON_PRIMARY_WIDTH     196
#define SPIN_BUTTON_PRIMARY_HEIGHT    72
#define SPIN_BUTTON_PREVIEW_WIDTH     104
#define SPIN_BUTTON_PREVIEW_HEIGHT    44
#define SPIN_BUTTON_BOTTOM_ROW_WIDTH  216
#define SPIN_BUTTON_BOTTOM_ROW_HEIGHT 44
#define SPIN_BUTTON_RECORD_WAIT       90
#define SPIN_BUTTON_RECORD_FRAME_WAIT 170
#define SPIN_BUTTON_RECORD_FINAL_WAIT 280

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_spin_button_t spin_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_spin_button_t spin_compact;
static egui_view_spin_button_t spin_read_only;
static egui_view_api_t spin_compact_api;
static egui_view_api_t spin_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Spin Button";

static void layout_page(void);

static void apply_primary_default_state(void)
{
    egui_view_spin_button_set_value(EGUI_VIEW_OF(&spin_primary), 6);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_preview_states(void)
{
    egui_view_spin_button_set_value(EGUI_VIEW_OF(&spin_compact), 8);
    egui_view_spin_button_set_value(EGUI_VIEW_OF(&spin_read_only), 16);
    egui_view_spin_button_set_read_only_mode(EGUI_VIEW_OF(&spin_read_only), 1);
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
    egui_core_layout_childs_user_root_view(uicode_get_core(), EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void focus_primary_spin(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&spin_primary));
#endif
}

static void on_primary_value_changed(egui_view_t *self, int16_t value)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(value);
    if (ui_ready)
    {
        layout_page();
    }
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    layout_page();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}

static void dispatch_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    focus_primary_spin();
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&spin_primary), &event);

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&spin_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
}

static void dispatch_primary_part_click(uint8_t part)
{
    egui_motion_event_t event = {0};
    egui_region_t region;

    if (!egui_view_spin_button_get_part_region(EGUI_VIEW_OF(&spin_primary), part, &region))
    {
        return;
    }
    event.location.x = region.location.x + region.size.width / 2;
    event.location.y = region.location.y + region.size.height / 2;

    event.type = EGUI_MOTION_EVENT_ACTION_DOWN;
    egui_view_dispatch_touch_event(EGUI_VIEW_OF(&spin_primary), &event);
    event.type = EGUI_MOTION_EVENT_ACTION_UP;
    egui_view_dispatch_touch_event(EGUI_VIEW_OF(&spin_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
}
#endif

void test_init_ui(void)
{
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SPIN_BUTTON_ROOT_WIDTH, SPIN_BUTTON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SPIN_BUTTON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 6, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_spin_button_init(EGUI_VIEW_OF(&spin_primary));
    egui_view_set_size(EGUI_VIEW_OF(&spin_primary), SPIN_BUTTON_PRIMARY_WIDTH, SPIN_BUTTON_PRIMARY_HEIGHT);
    egui_view_spin_button_set_fonts(EGUI_VIEW_OF(&spin_primary), (const egui_font_t *)&egui_res_font_montserrat_12_4,
                                    (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_spin_button_set_texts(EGUI_VIEW_OF(&spin_primary), "Columns", "cols", "2 to 12, step 2");
    egui_view_spin_button_set_range(EGUI_VIEW_OF(&spin_primary), 2, 12);
    egui_view_spin_button_set_step(EGUI_VIEW_OF(&spin_primary), 2);
    egui_view_spin_button_set_large_step(EGUI_VIEW_OF(&spin_primary), 4);
    egui_view_spin_button_set_palette(EGUI_VIEW_OF(&spin_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF8FAFC),
                                      EGUI_COLOR_HEX(0xD2DCE6), EGUI_COLOR_HEX(0x182433), EGUI_COLOR_HEX(0x667587),
                                      EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_spin_button_set_on_value_changed_listener(EGUI_VIEW_OF(&spin_primary), on_primary_value_changed);
    egui_view_set_margin(EGUI_VIEW_OF(&spin_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&spin_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SPIN_BUTTON_BOTTOM_ROW_WIDTH, SPIN_BUTTON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_spin_button_init(EGUI_VIEW_OF(&spin_compact));
    egui_view_set_size(EGUI_VIEW_OF(&spin_compact), SPIN_BUTTON_PREVIEW_WIDTH, SPIN_BUTTON_PREVIEW_HEIGHT);
    egui_view_spin_button_set_fonts(EGUI_VIEW_OF(&spin_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                    (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_spin_button_set_texts(EGUI_VIEW_OF(&spin_compact), NULL, "px", NULL);
    egui_view_spin_button_set_range(EGUI_VIEW_OF(&spin_compact), 0, 24);
    egui_view_spin_button_set_step(EGUI_VIEW_OF(&spin_compact), 2);
    egui_view_spin_button_set_compact_mode(EGUI_VIEW_OF(&spin_compact), 1);
    egui_view_spin_button_override_static_preview_api(EGUI_VIEW_OF(&spin_compact), &spin_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&spin_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&spin_compact));

    egui_view_spin_button_init(EGUI_VIEW_OF(&spin_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&spin_read_only), SPIN_BUTTON_PREVIEW_WIDTH, SPIN_BUTTON_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&spin_read_only), 8, 0, 0, 0);
    egui_view_spin_button_set_fonts(EGUI_VIEW_OF(&spin_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                    (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_spin_button_set_texts(EGUI_VIEW_OF(&spin_read_only), NULL, "ms", NULL);
    egui_view_spin_button_set_compact_mode(EGUI_VIEW_OF(&spin_read_only), 1);
    egui_view_spin_button_set_read_only_mode(EGUI_VIEW_OF(&spin_read_only), 1);
    egui_view_spin_button_set_palette(EGUI_VIEW_OF(&spin_read_only), EGUI_COLOR_HEX(0xFCFDFE), EGUI_COLOR_HEX(0xF1F4F7),
                                      EGUI_COLOR_HEX(0xDEE5EC), EGUI_COLOR_HEX(0x364452), EGUI_COLOR_HEX(0x7A8793),
                                      EGUI_COLOR_HEX(0x909CAA));
    egui_view_spin_button_override_static_preview_api(EGUI_VIEW_OF(&spin_read_only), &spin_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&spin_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&spin_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
    focus_primary_spin();
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
            apply_primary_default_state();
            apply_preview_states();
            focus_primary_spin();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            dispatch_primary_part_click(EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT);
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            dispatch_primary_key(EGUI_KEY_CODE_UP);
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            dispatch_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            dispatch_primary_part_click(EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT);
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_FINAL_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPIN_BUTTON_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
