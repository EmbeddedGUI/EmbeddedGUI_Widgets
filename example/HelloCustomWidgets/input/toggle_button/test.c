#include "egui.h"
#include "egui_view_toggle_button.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TOGGLE_BUTTON_ROOT_WIDTH        224
#define TOGGLE_BUTTON_ROOT_HEIGHT       142
#define TOGGLE_BUTTON_PRIMARY_WIDTH     196
#define TOGGLE_BUTTON_PRIMARY_HEIGHT    52
#define TOGGLE_BUTTON_PREVIEW_WIDTH     104
#define TOGGLE_BUTTON_PREVIEW_HEIGHT    44
#define TOGGLE_BUTTON_BOTTOM_ROW_WIDTH  216
#define TOGGLE_BUTTON_BOTTOM_ROW_HEIGHT 44
#define TOGGLE_BUTTON_RECORD_WAIT       90
#define TOGGLE_BUTTON_RECORD_FRAME_WAIT 170
#define TOGGLE_BUTTON_RECORD_FINAL_WAIT 280
#define TOGGLE_BUTTON_DEFAULT_SNAPSHOT  0

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)EGUI_ARRAY_SIZE(primary_snapshots))

typedef struct toggle_button_snapshot toggle_button_snapshot_t;
struct toggle_button_snapshot
{
    const char *text;
    const char *icon;
    egui_color_t on_color;
    egui_color_t off_color;
    uint8_t toggled;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_toggle_button_t button_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_toggle_button_t button_compact;
static egui_view_toggle_button_t button_read_only;
static egui_view_api_t button_primary_interaction_api;
static egui_view_api_t button_compact_api;
static egui_view_api_t button_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Toggle Button";

static const toggle_button_snapshot_t primary_snapshots[] = {
        {"Alerts", EGUI_ICON_MS_NOTIFICATIONS, EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xE9F1FB), 1},
        {"Visible", EGUI_ICON_MS_VISIBILITY, EGUI_COLOR_HEX(0x0F766E), EGUI_COLOR_HEX(0xE5F3F1), 0},
        {"Favorite", EGUI_ICON_MS_FAVORITE, EGUI_COLOR_HEX(0xB146C2), EGUI_COLOR_HEX(0xF5E9F8), 1},
};

static const toggle_button_snapshot_t compact_snapshot = {
        "Visible", EGUI_ICON_MS_VISIBILITY, EGUI_COLOR_HEX(0x0F766E), EGUI_COLOR_HEX(0xE4F2EF), 1,
};

static const toggle_button_snapshot_t read_only_snapshot = {
        "Pinned", EGUI_ICON_MS_FAVORITE, EGUI_COLOR_HEX(0xA5AFBA), EGUI_COLOR_HEX(0xEEF2F6), 1,
};

static void layout_page(void);

static void apply_snapshot_to_button(egui_view_toggle_button_t *button, const toggle_button_snapshot_t *snapshot)
{
    button->on_color = snapshot->on_color;
    button->off_color = snapshot->off_color;
    egui_view_toggle_button_set_icon(EGUI_VIEW_OF(button), snapshot->icon);
    egui_view_toggle_button_set_text(EGUI_VIEW_OF(button), snapshot->text);
    hcw_toggle_button_set_toggled(EGUI_VIEW_OF(button), snapshot->toggled);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot_to_button(&button_primary, &primary_snapshots[index % PRIMARY_SNAPSHOT_COUNT]);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(TOGGLE_BUTTON_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    apply_snapshot_to_button(&button_compact, &compact_snapshot);
    apply_snapshot_to_button(&button_read_only, &read_only_snapshot);
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

static void focus_primary_button(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&button_primary));
#endif
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
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TOGGLE_BUTTON_ROOT_WIDTH, TOGGLE_BUTTON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TOGGLE_BUTTON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_toggle_button_init(EGUI_VIEW_OF(&button_primary));
    egui_view_set_size(EGUI_VIEW_OF(&button_primary), TOGGLE_BUTTON_PRIMARY_WIDTH, TOGGLE_BUTTON_PRIMARY_HEIGHT);
    egui_view_toggle_button_set_font(EGUI_VIEW_OF(&button_primary), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_toggle_button_set_icon_font(EGUI_VIEW_OF(&button_primary), EGUI_FONT_ICON_MS_16);
    hcw_toggle_button_apply_standard_style(EGUI_VIEW_OF(&button_primary));
    egui_view_set_padding(EGUI_VIEW_OF(&button_primary), 2, 2, 0, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_primary), true);
#endif
    hcw_toggle_button_override_interaction_api(EGUI_VIEW_OF(&button_primary), &button_primary_interaction_api);
    egui_view_set_margin(EGUI_VIEW_OF(&button_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&button_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TOGGLE_BUTTON_BOTTOM_ROW_WIDTH, TOGGLE_BUTTON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_toggle_button_init(EGUI_VIEW_OF(&button_compact));
    egui_view_set_size(EGUI_VIEW_OF(&button_compact), TOGGLE_BUTTON_PREVIEW_WIDTH, TOGGLE_BUTTON_PREVIEW_HEIGHT);
    egui_view_toggle_button_set_font(EGUI_VIEW_OF(&button_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_toggle_button_set_icon_font(EGUI_VIEW_OF(&button_compact), EGUI_FONT_ICON_MS_16);
    hcw_toggle_button_apply_compact_style(EGUI_VIEW_OF(&button_compact));
    egui_view_set_padding(EGUI_VIEW_OF(&button_compact), 1, 1, 0, 0);
    hcw_toggle_button_override_static_preview_api(EGUI_VIEW_OF(&button_compact), &button_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&button_compact));

    egui_view_toggle_button_init(EGUI_VIEW_OF(&button_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&button_read_only), TOGGLE_BUTTON_PREVIEW_WIDTH, TOGGLE_BUTTON_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&button_read_only), 8, 0, 0, 0);
    egui_view_toggle_button_set_font(EGUI_VIEW_OF(&button_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_toggle_button_set_icon_font(EGUI_VIEW_OF(&button_read_only), EGUI_FONT_ICON_MS_16);
    hcw_toggle_button_apply_read_only_style(EGUI_VIEW_OF(&button_read_only));
    egui_view_set_padding(EGUI_VIEW_OF(&button_read_only), 1, 1, 0, 0);
    hcw_toggle_button_override_static_preview_api(EGUI_VIEW_OF(&button_read_only), &button_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&button_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&button_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    layout_page();
    focus_primary_button();
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
            focus_primary_button();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOGGLE_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, TOGGLE_BUTTON_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOGGLE_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, TOGGLE_BUTTON_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOGGLE_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 5:
        EGUI_SIM_SET_WAIT(p_action, TOGGLE_BUTTON_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOGGLE_BUTTON_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
