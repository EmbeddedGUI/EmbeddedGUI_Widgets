#include <stdio.h>

#include "egui.h"
#include "egui_view_repeat_button.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define REPEAT_BUTTON_ROOT_WIDTH        224
#define REPEAT_BUTTON_ROOT_HEIGHT       128
#define REPEAT_BUTTON_PRIMARY_WIDTH     160
#define REPEAT_BUTTON_PRIMARY_HEIGHT    40
#define REPEAT_BUTTON_PREVIEW_WIDTH     96
#define REPEAT_BUTTON_PREVIEW_HEIGHT    32
#define REPEAT_BUTTON_BOTTOM_ROW_WIDTH  200
#define REPEAT_BUTTON_BOTTOM_ROW_HEIGHT 32
#define REPEAT_BUTTON_RECORD_WAIT       90
#define REPEAT_BUTTON_RECORD_FRAME_WAIT 170
#define REPEAT_BUTTON_RECORD_FINAL_WAIT 280
#define REPEAT_BUTTON_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct repeat_button_snapshot repeat_button_snapshot_t;
struct repeat_button_snapshot
{
    int value;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_repeat_button_t primary_widget;
static egui_view_linearlayout_t bottom_row;
static egui_view_repeat_button_t compact_widget;
static egui_view_repeat_button_t disabled_widget;
static egui_view_api_t compact_widget_api;
static egui_view_api_t disabled_widget_api;
static char primary_button_text[24];
static int primary_value;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "RepeatButton";

static const repeat_button_snapshot_t primary_snapshots[] = {
        {12},
        {15},
        {18},
};

static void layout_page(void);
static void focus_primary_widget(void);

static void update_primary_button_text(void)
{
    snprintf(primary_button_text, sizeof(primary_button_text), "Volume %02d", primary_value);
    egui_view_label_set_text(EGUI_VIEW_OF(&primary_widget), primary_button_text);
}

static void apply_primary_snapshot(uint8_t index)
{
    primary_value = primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT].value;
    update_primary_button_text();
    if (ui_ready)
    {
        layout_page();
        focus_primary_widget();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(REPEAT_BUTTON_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_repeat_button_set_text(EGUI_VIEW_OF(&compact_widget), "Fast");
    egui_view_repeat_button_set_icon(EGUI_VIEW_OF(&compact_widget), EGUI_ICON_MS_REFRESH);
    egui_view_repeat_button_set_icon_text_gap(EGUI_VIEW_OF(&compact_widget), 4);

    egui_view_repeat_button_set_text(EGUI_VIEW_OF(&disabled_widget), "Locked");
    egui_view_repeat_button_set_icon(EGUI_VIEW_OF(&disabled_widget), EGUI_ICON_MS_REMOVE);
    egui_view_repeat_button_set_icon_text_gap(EGUI_VIEW_OF(&disabled_widget), 4);

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

static void focus_primary_widget(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&primary_widget));
#endif
}

static void on_primary_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    if (primary_value < 99)
    {
        primary_value++;
    }
    update_primary_button_text();
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), REPEAT_BUTTON_ROOT_WIDTH, REPEAT_BUTTON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), REPEAT_BUTTON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_repeat_button_init(EGUI_VIEW_OF(&primary_widget));
    egui_view_set_size(EGUI_VIEW_OF(&primary_widget), REPEAT_BUTTON_PRIMARY_WIDTH, REPEAT_BUTTON_PRIMARY_HEIGHT);
    egui_view_label_set_font(EGUI_VIEW_OF(&primary_widget), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_repeat_button_set_icon(EGUI_VIEW_OF(&primary_widget), EGUI_ICON_MS_ADD);
    egui_view_repeat_button_set_icon_font(EGUI_VIEW_OF(&primary_widget), EGUI_FONT_ICON_MS_16);
    egui_view_repeat_button_set_icon_text_gap(EGUI_VIEW_OF(&primary_widget), 6);
    egui_view_set_margin(EGUI_VIEW_OF(&primary_widget), 0, 0, 0, 10);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&primary_widget), on_primary_click);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&primary_widget), 1);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&primary_widget));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), REPEAT_BUTTON_BOTTOM_ROW_WIDTH, REPEAT_BUTTON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_repeat_button_init(EGUI_VIEW_OF(&compact_widget));
    egui_view_set_size(EGUI_VIEW_OF(&compact_widget), REPEAT_BUTTON_PREVIEW_WIDTH, REPEAT_BUTTON_PREVIEW_HEIGHT);
    egui_view_repeat_button_apply_compact_style(EGUI_VIEW_OF(&compact_widget));
    egui_view_repeat_button_set_font(EGUI_VIEW_OF(&compact_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_repeat_button_set_icon_font(EGUI_VIEW_OF(&compact_widget), EGUI_FONT_ICON_MS_16);
    egui_view_repeat_button_override_static_preview_api(EGUI_VIEW_OF(&compact_widget), &compact_widget_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&compact_widget), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_widget));

    egui_view_repeat_button_init(EGUI_VIEW_OF(&disabled_widget));
    egui_view_set_size(EGUI_VIEW_OF(&disabled_widget), REPEAT_BUTTON_PREVIEW_WIDTH, REPEAT_BUTTON_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&disabled_widget), 8, 0, 0, 0);
    egui_view_repeat_button_apply_disabled_style(EGUI_VIEW_OF(&disabled_widget));
    egui_view_repeat_button_set_font(EGUI_VIEW_OF(&disabled_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_repeat_button_set_icon_font(EGUI_VIEW_OF(&disabled_widget), EGUI_FONT_ICON_MS_16);
    egui_view_repeat_button_override_static_preview_api(EGUI_VIEW_OF(&disabled_widget), &disabled_widget_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&disabled_widget), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&disabled_widget));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    layout_page();
    focus_primary_widget();
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
            focus_primary_widget();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, REPEAT_BUTTON_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
