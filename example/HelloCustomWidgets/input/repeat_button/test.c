#include <stdio.h>
#include <string.h>

#include "egui.h"
#include "egui_view_repeat_button.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define REPEAT_BUTTON_ROOT_WIDTH        224
#define REPEAT_BUTTON_ROOT_HEIGHT       252
#define REPEAT_BUTTON_PANEL_WIDTH       196
#define REPEAT_BUTTON_PANEL_HEIGHT      124
#define REPEAT_BUTTON_PRIMARY_WIDTH     140
#define REPEAT_BUTTON_PRIMARY_HEIGHT    40
#define REPEAT_BUTTON_PREVIEW_WIDTH     104
#define REPEAT_BUTTON_PREVIEW_HEIGHT    82
#define REPEAT_BUTTON_PREVIEW_BODY_W    84
#define REPEAT_BUTTON_PREVIEW_BODY_H    32
#define REPEAT_BUTTON_BOTTOM_ROW_WIDTH  216
#define REPEAT_BUTTON_BOTTOM_ROW_HEIGHT 82
#define REPEAT_BUTTON_RECORD_WAIT        90
#define REPEAT_BUTTON_RECORD_FRAME_WAIT 170
#define REPEAT_BUTTON_RECORD_HOLD_WAIT  620
#define REPEAT_BUTTON_RECORD_KEY_WAIT   460

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_linearlayout_t primary_panel;
static egui_view_label_t primary_heading_label;
static egui_view_label_t primary_value_label;
static egui_view_repeat_button_t primary_widget;
static egui_view_label_t primary_status_label;
static egui_view_label_t primary_note_label;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_panel;
static egui_view_label_t compact_heading_label;
static egui_view_repeat_button_t compact_widget;
static egui_view_label_t compact_body_label;
static egui_view_linearlayout_t disabled_panel;
static egui_view_label_t disabled_heading_label;
static egui_view_repeat_button_t disabled_widget;
static egui_view_label_t disabled_body_label;
static egui_view_api_t compact_widget_api;
static egui_view_api_t disabled_widget_api;

static int g_repeat_value = 12;
static uint16_t g_repeat_count = 0;
static char primary_value_text[24];
static char primary_status_text[48];

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_surface_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_surface_panel_params, &bg_surface_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_surface_panel, &bg_surface_panel_params);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_muted_panel_param, EGUI_COLOR_HEX(0xFBFCFD), EGUI_ALPHA_100, 12);
EGUI_BACKGROUND_PARAM_INIT(bg_muted_panel_params, &bg_muted_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_muted_panel, &bg_muted_panel_params);

static const char *title_text = "RepeatButton";

static void init_text_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text, const egui_font_t *font, egui_color_t color,
                            uint8_t align)
{
    egui_view_label_init(EGUI_VIEW_OF(label));
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), align);
    egui_view_label_set_font(EGUI_VIEW_OF(label), font);
    egui_view_label_set_font_color(EGUI_VIEW_OF(label), color, EGUI_ALPHA_100);
}

static void init_panel(egui_view_linearlayout_t *panel, egui_dim_t width, egui_dim_t height, egui_background_t *background, uint8_t align)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(panel));
    egui_view_set_size(EGUI_VIEW_OF(panel), width, height);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(panel), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(panel), align);
    egui_view_set_background(EGUI_VIEW_OF(panel), background);
    egui_view_set_padding(EGUI_VIEW_OF(panel), 6, 6, 6, 6);
}

static void update_primary_labels(void)
{
    snprintf(primary_value_text, sizeof(primary_value_text), "Volume %d", g_repeat_value);
    snprintf(primary_status_text, sizeof(primary_status_text), "Immediate + %u repeats", (unsigned int)(g_repeat_count > 0 ? (g_repeat_count - 1U) : 0U));
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_value_label), primary_value_text);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_status_label), primary_status_text);
}

static void reset_primary_state(void)
{
    g_repeat_value = 12;
    g_repeat_count = 0;
    update_primary_labels();
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_note_label), "Hold to step after the 360 ms delay and 90 ms interval.");
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        egui_view_clear_focus(EGUI_VIEW_OF(&primary_widget));
    }
#else
    EGUI_UNUSED(event);
#endif
    return 0;
}

static void on_primary_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_repeat_value++;
    g_repeat_count++;
    update_primary_labels();
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
static void focus_primary_widget(void)
{
    egui_view_request_focus(EGUI_VIEW_OF(&primary_widget));
}
#endif

#if EGUI_CONFIG_RECORDING_TEST
static void get_primary_center(egui_dim_t *x, egui_dim_t *y)
{
    *x = EGUI_VIEW_OF(&primary_widget)->region_screen.location.x + EGUI_VIEW_OF(&primary_widget)->region_screen.size.width / 2;
    *y = EGUI_VIEW_OF(&primary_widget)->region_screen.location.y + EGUI_VIEW_OF(&primary_widget)->region_screen.size.height / 2;
}

static void dispatch_primary_touch_action(uint8_t type)
{
    egui_motion_event_t event;
    egui_dim_t x;
    egui_dim_t y;

    memset(&event, 0, sizeof(event));
    get_primary_center(&x, &y);
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    EGUI_VIEW_OF(&primary_widget)->api->dispatch_touch_event(EGUI_VIEW_OF(&primary_widget), &event);
}

static void dispatch_primary_key_action(uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    focus_primary_widget();
#endif
    event.type = type;
    event.key_code = key_code;
    EGUI_VIEW_OF(&primary_widget)->api->dispatch_key_event(EGUI_VIEW_OF(&primary_widget), &event);
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), REPEAT_BUTTON_ROOT_WIDTH, REPEAT_BUTTON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    init_text_label(&title_label, REPEAT_BUTTON_ROOT_WIDTH, 18, title_text, (const egui_font_t *)&egui_res_font_montserrat_12_4, EGUI_COLOR_HEX(0x21303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    init_panel(&primary_panel, REPEAT_BUTTON_PANEL_WIDTH, REPEAT_BUTTON_PANEL_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_panel), 0, 0, 0, 10);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_panel));

    init_text_label(&primary_heading_label, 176, 12, "Press and hold", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x51606F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_heading_label));

    init_text_label(&primary_value_label, 176, 20, "Volume 12", (const egui_font_t *)&egui_res_font_montserrat_20_4, EGUI_COLOR_HEX(0x0F6CBD),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_value_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_value_label));

    egui_view_repeat_button_init(EGUI_VIEW_OF(&primary_widget));
    egui_view_set_size(EGUI_VIEW_OF(&primary_widget), REPEAT_BUTTON_PRIMARY_WIDTH, REPEAT_BUTTON_PRIMARY_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_widget), 0, 0, 0, 6);
    egui_view_repeat_button_set_text(EGUI_VIEW_OF(&primary_widget), "Increase");
    egui_view_repeat_button_set_icon(EGUI_VIEW_OF(&primary_widget), EGUI_ICON_MS_ADD);
    egui_view_repeat_button_set_font(EGUI_VIEW_OF(&primary_widget), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_repeat_button_set_icon_font(EGUI_VIEW_OF(&primary_widget), EGUI_FONT_ICON_MS_16);
    egui_view_repeat_button_set_icon_text_gap(EGUI_VIEW_OF(&primary_widget), 6);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&primary_widget), on_primary_click);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_widget), 1);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_widget));

    init_text_label(&primary_status_label, 176, 12, "Immediate + 0 repeats", (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_COLOR_HEX(0x22303F),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_status_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_status_label));

    init_text_label(&primary_note_label, 176, 12, "Hold to step after the 360 ms delay and 90 ms interval.",
                    (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&primary_panel), EGUI_VIEW_OF(&primary_note_label));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), REPEAT_BUTTON_BOTTOM_ROW_WIDTH, REPEAT_BUTTON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    init_panel(&compact_panel, REPEAT_BUTTON_PREVIEW_WIDTH, REPEAT_BUTTON_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_muted_panel), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_panel));

    init_text_label(&compact_heading_label, 84, 12, "Compact", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241), EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_heading_label));

    egui_view_repeat_button_init(EGUI_VIEW_OF(&compact_widget));
    egui_view_set_size(EGUI_VIEW_OF(&compact_widget), REPEAT_BUTTON_PREVIEW_BODY_W, REPEAT_BUTTON_PREVIEW_BODY_H);
    egui_view_set_margin(EGUI_VIEW_OF(&compact_widget), 0, 0, 0, 4);
    egui_view_repeat_button_apply_compact_style(EGUI_VIEW_OF(&compact_widget));
    egui_view_repeat_button_set_text(EGUI_VIEW_OF(&compact_widget), "Fast");
    egui_view_repeat_button_set_icon(EGUI_VIEW_OF(&compact_widget), EGUI_ICON_MS_REFRESH);
    egui_view_repeat_button_set_font(EGUI_VIEW_OF(&compact_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_repeat_button_set_icon_font(EGUI_VIEW_OF(&compact_widget), EGUI_FONT_ICON_MS_16);
    egui_view_repeat_button_override_static_preview_api(EGUI_VIEW_OF(&compact_widget), &compact_widget_api);
    compact_widget_api.on_touch = dismiss_primary_focus_on_preview_touch;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_widget), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_widget));

    init_text_label(&compact_body_label, 84, 18, "Compact repeat\nreference", (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_panel), EGUI_VIEW_OF(&compact_body_label));

    init_panel(&disabled_panel, REPEAT_BUTTON_PREVIEW_WIDTH, REPEAT_BUTTON_PREVIEW_HEIGHT, EGUI_BG_OF(&bg_surface_panel), EGUI_ALIGN_HCENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&disabled_panel), 8, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&disabled_panel));

    init_text_label(&disabled_heading_label, 84, 12, "Disabled", (const egui_font_t *)&egui_res_font_montserrat_8_4, EGUI_COLOR_HEX(0x233241),
                    EGUI_ALIGN_CENTER);
    egui_view_set_margin(EGUI_VIEW_OF(&disabled_heading_label), 0, 0, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&disabled_panel), EGUI_VIEW_OF(&disabled_heading_label));

    egui_view_repeat_button_init(EGUI_VIEW_OF(&disabled_widget));
    egui_view_set_size(EGUI_VIEW_OF(&disabled_widget), REPEAT_BUTTON_PREVIEW_BODY_W, REPEAT_BUTTON_PREVIEW_BODY_H);
    egui_view_set_margin(EGUI_VIEW_OF(&disabled_widget), 0, 0, 0, 4);
    egui_view_repeat_button_apply_disabled_style(EGUI_VIEW_OF(&disabled_widget));
    egui_view_repeat_button_set_text(EGUI_VIEW_OF(&disabled_widget), "Locked");
    egui_view_repeat_button_set_icon(EGUI_VIEW_OF(&disabled_widget), EGUI_ICON_MS_REMOVE);
    egui_view_repeat_button_set_font(EGUI_VIEW_OF(&disabled_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_repeat_button_set_icon_font(EGUI_VIEW_OF(&disabled_widget), EGUI_FONT_ICON_MS_16);
    egui_view_repeat_button_override_static_preview_api(EGUI_VIEW_OF(&disabled_widget), &disabled_widget_api);
    disabled_widget_api.on_touch = dismiss_primary_focus_on_preview_touch;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&disabled_widget), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&disabled_panel), EGUI_VIEW_OF(&disabled_widget));

    init_text_label(&disabled_body_label, 84, 18, "Disabled visual\nreference", (const egui_font_t *)&egui_res_font_montserrat_8_4,
                    EGUI_COLOR_HEX(0x6B7A89), EGUI_ALIGN_CENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&disabled_panel), EGUI_VIEW_OF(&disabled_body_label));

    reset_primary_state();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&primary_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&disabled_panel));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    focus_primary_widget();
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
            reset_primary_state();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            focus_primary_widget();
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            dispatch_primary_touch_action(EGUI_MOTION_EVENT_ACTION_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_HOLD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            dispatch_primary_touch_action(EGUI_MOTION_EVENT_ACTION_UP);
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            dispatch_primary_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_KEY_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            dispatch_primary_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
