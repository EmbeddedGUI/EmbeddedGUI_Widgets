#include "egui.h"
#include "egui_view_menu_button.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define MENU_BUTTON_ROOT_WIDTH        232
#define MENU_BUTTON_ROOT_HEIGHT       176
#define MENU_BUTTON_PRIMARY_WIDTH     206
#define MENU_BUTTON_PRIMARY_HEIGHT    104
#define MENU_BUTTON_PREVIEW_WIDTH     104
#define MENU_BUTTON_PREVIEW_HEIGHT    42
#define MENU_BUTTON_BOTTOM_ROW_WIDTH  216
#define MENU_BUTTON_BOTTOM_ROW_HEIGHT 42
#define MENU_BUTTON_RECORD_WAIT       90
#define MENU_BUTTON_RECORD_FRAME_WAIT 170
#define MENU_BUTTON_RECORD_FINAL_WAIT 280

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_menu_button_t menu_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_menu_button_t menu_compact;
static egui_view_menu_button_t menu_read_only;
static egui_view_api_t menu_compact_api;
static egui_view_api_t menu_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Menu Button";

static const egui_view_menu_button_item_t primary_items[] = {
        {"New page", EGUI_ICON_MS_ADD, "N", EGUI_VIEW_MENU_BUTTON_TONE_ACCENT, 1, 0},
        {"Duplicate", EGUI_ICON_MS_EDIT, "D", EGUI_VIEW_MENU_BUTTON_TONE_NEUTRAL, 0, 0},
        {"Export", EGUI_ICON_MS_DOWNLOAD, "E", EGUI_VIEW_MENU_BUTTON_TONE_SUCCESS, 0, 0},
        {"Archive", EGUI_ICON_MS_LOCK, "A", EGUI_VIEW_MENU_BUTTON_TONE_WARNING, 0, 0},
};

static const egui_view_menu_button_item_t compact_items[] = {
        {"More", EGUI_ICON_MS_SETTINGS, "", EGUI_VIEW_MENU_BUTTON_TONE_ACCENT, 1, 0},
};

static const egui_view_menu_button_item_t read_only_items[] = {
        {"Locked", EGUI_ICON_MS_LOCK, "", EGUI_VIEW_MENU_BUTTON_TONE_NEUTRAL, 1, 0},
};

static void layout_page(void);

static void apply_primary_default_state(void)
{
    egui_view_menu_button_set_items(EGUI_VIEW_OF(&menu_primary), primary_items, (uint8_t)EGUI_ARRAY_SIZE(primary_items));
    egui_view_menu_button_set_selected_index(EGUI_VIEW_OF(&menu_primary), 0);
    egui_view_menu_button_set_open(EGUI_VIEW_OF(&menu_primary), 0);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_preview_states(void)
{
    egui_view_menu_button_set_items(EGUI_VIEW_OF(&menu_compact), compact_items, (uint8_t)EGUI_ARRAY_SIZE(compact_items));
    egui_view_menu_button_set_selected_index(EGUI_VIEW_OF(&menu_compact), 0);
    egui_view_menu_button_set_compact_mode(EGUI_VIEW_OF(&menu_compact), 1);
    egui_view_menu_button_set_open(EGUI_VIEW_OF(&menu_compact), 0);

    egui_view_menu_button_set_items(EGUI_VIEW_OF(&menu_read_only), read_only_items, (uint8_t)EGUI_ARRAY_SIZE(read_only_items));
    egui_view_menu_button_set_selected_index(EGUI_VIEW_OF(&menu_read_only), 0);
    egui_view_menu_button_set_read_only_mode(EGUI_VIEW_OF(&menu_read_only), 1);
    egui_view_menu_button_set_open(EGUI_VIEW_OF(&menu_read_only), 0);
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

static void focus_primary_menu(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&menu_primary));
#endif
}

static void on_primary_action(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(index);
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

    focus_primary_menu();
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&menu_primary), &event);

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&menu_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
}

static void dispatch_primary_trigger_click(void)
{
    egui_motion_event_t event = {0};
    egui_region_t region;

    if (!egui_view_menu_button_get_trigger_region(EGUI_VIEW_OF(&menu_primary), &region))
    {
        return;
    }
    event.location.x = region.location.x + region.size.width / 2;
    event.location.y = region.location.y + region.size.height / 2;

    event.type = EGUI_MOTION_EVENT_ACTION_DOWN;
    egui_view_dispatch_touch_event(EGUI_VIEW_OF(&menu_primary), &event);
    event.type = EGUI_MOTION_EVENT_ACTION_UP;
    egui_view_dispatch_touch_event(EGUI_VIEW_OF(&menu_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
}

static void dispatch_primary_item_click(uint8_t index)
{
    egui_motion_event_t event = {0};
    egui_region_t region;

    if (!egui_view_menu_button_get_item_region(EGUI_VIEW_OF(&menu_primary), index, &region))
    {
        return;
    }
    event.location.x = region.location.x + region.size.width / 2;
    event.location.y = region.location.y + region.size.height / 2;

    event.type = EGUI_MOTION_EVENT_ACTION_DOWN;
    egui_view_dispatch_touch_event(EGUI_VIEW_OF(&menu_primary), &event);
    event.type = EGUI_MOTION_EVENT_ACTION_UP;
    egui_view_dispatch_touch_event(EGUI_VIEW_OF(&menu_primary), &event);
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), MENU_BUTTON_ROOT_WIDTH, MENU_BUTTON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), MENU_BUTTON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), HCW_COLOR_TEXT, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_menu_button_init(EGUI_VIEW_OF(&menu_primary));
    egui_view_set_size(EGUI_VIEW_OF(&menu_primary), MENU_BUTTON_PRIMARY_WIDTH, MENU_BUTTON_PRIMARY_HEIGHT);
    egui_view_menu_button_set_button(EGUI_VIEW_OF(&menu_primary), "Page actions", EGUI_ICON_MS_SETTINGS);
    egui_view_menu_button_set_menu_title(EGUI_VIEW_OF(&menu_primary), "Page menu");
    egui_view_menu_button_set_fonts(EGUI_VIEW_OF(&menu_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                    (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_FONT_ICON_MS_16);
    egui_view_menu_button_set_on_action_listener(EGUI_VIEW_OF(&menu_primary), on_primary_action);
    egui_view_set_margin(EGUI_VIEW_OF(&menu_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&menu_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), MENU_BUTTON_BOTTOM_ROW_WIDTH, MENU_BUTTON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_menu_button_init(EGUI_VIEW_OF(&menu_compact));
    egui_view_set_size(EGUI_VIEW_OF(&menu_compact), MENU_BUTTON_PREVIEW_WIDTH, MENU_BUTTON_PREVIEW_HEIGHT);
    egui_view_menu_button_set_button(EGUI_VIEW_OF(&menu_compact), "More", EGUI_ICON_MS_SETTINGS);
    egui_view_menu_button_set_fonts(EGUI_VIEW_OF(&menu_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                    (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_FONT_ICON_MS_16);
    egui_view_menu_button_override_static_preview_api(EGUI_VIEW_OF(&menu_compact), &menu_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&menu_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&menu_compact));

    egui_view_menu_button_init(EGUI_VIEW_OF(&menu_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&menu_read_only), MENU_BUTTON_PREVIEW_WIDTH, MENU_BUTTON_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&menu_read_only), 8, 0, 0, 0);
    egui_view_menu_button_set_button(EGUI_VIEW_OF(&menu_read_only), "Locked", EGUI_ICON_MS_LOCK);
    egui_view_menu_button_set_fonts(EGUI_VIEW_OF(&menu_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                    (const egui_font_t *)&egui_res_font_montserrat_10_4, EGUI_FONT_ICON_MS_16);
    egui_view_menu_button_set_palette(EGUI_VIEW_OF(&menu_read_only), HCW_COLOR_PANEL, HCW_COLOR_SURFACE,
                                      HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT_STRONG, HCW_COLOR_TEXT_STRONG, HCW_COLOR_PRIMARY_DARK,
                                      HCW_COLOR_SUCCESS, HCW_COLOR_WARNING_DARK, HCW_COLOR_DANGER_DARK, HCW_COLOR_BORDER_STRONG);
    egui_view_menu_button_override_static_preview_api(EGUI_VIEW_OF(&menu_read_only), &menu_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&menu_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&menu_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
    focus_primary_menu();
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
            focus_primary_menu();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, MENU_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            dispatch_primary_trigger_click();
        }
        EGUI_SIM_SET_WAIT(p_action, MENU_BUTTON_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, MENU_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            dispatch_primary_key(EGUI_KEY_CODE_DOWN);
            dispatch_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, MENU_BUTTON_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, MENU_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            dispatch_primary_trigger_click();
            dispatch_primary_item_click(2);
        }
        EGUI_SIM_SET_WAIT(p_action, MENU_BUTTON_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, MENU_BUTTON_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, MENU_BUTTON_RECORD_FINAL_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, MENU_BUTTON_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
