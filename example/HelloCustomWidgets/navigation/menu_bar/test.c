#include "egui.h"
#include "egui_view_menu_bar.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define MENU_BAR_ROOT_WIDTH     224
#define MENU_BAR_ROOT_HEIGHT    224
#define MENU_BAR_PRIMARY_WIDTH  196
#define MENU_BAR_PRIMARY_HEIGHT 108
#define MENU_BAR_BOTTOM_WIDTH   216
#define MENU_BAR_BOTTOM_HEIGHT  74
#define MENU_BAR_COLUMN_WIDTH   104
#define MENU_BAR_CARD_WIDTH     104
#define MENU_BAR_CARD_HEIGHT    74

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_menu_bar_t menu_bar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_menu_bar_t menu_bar_compact;
static egui_view_linearlayout_t locked_column;
static egui_view_menu_bar_t menu_bar_locked;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Menu Bar";

static const egui_view_menu_bar_menu_t primary_menus[] = {
        {"File", 1, 1},
        {"Edit", 0, 1},
        {"View", 0, 1},
        {"Tools", 0, 1},
};

static const egui_view_menu_bar_panel_item_t primary_items_0[] = {
        {"New window", "Ctrl+N", 0, 1, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Open recent", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_SUBMENU},
        {"Share link", "Ctrl+L", 1, 0, 1, 1, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Settings", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_panel_item_t primary_items_1[] = {
        {"Undo", "Ctrl+Z", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Redo", "Ctrl+Shift+Z", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Find in page", "Ctrl+F", 0, 1, 1, 1, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Preferences", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_SUBMENU},
};

static const egui_view_menu_bar_panel_item_t primary_items_2[] = {
        {"Density compact", "On", 2, 1, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Reading mode", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Show filters", "", 0, 0, 1, 1, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Panels", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_SUBMENU},
};

static const egui_view_menu_bar_panel_item_t primary_items_3[] = {
        {"Sync now", "", 1, 1, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Export review", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Archive item", "", 0, 0, 0, 1, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Delete draft", "", 3, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_snapshot_t primary_snapshots[] = {
        {primary_menus, 4, 0, primary_items_0, 4, 1, 1},
        {primary_menus, 4, 1, primary_items_1, 4, 2, 1},
        {primary_menus, 4, 2, primary_items_2, 4, 0, 1},
        {primary_menus, 4, 3, primary_items_3, 4, 3, 1},
};

static const egui_view_menu_bar_menu_t compact_menus[] = {
        {"File", 1, 1},
        {"View", 0, 1},
        {"Tools", 0, 1},
};

static const egui_view_menu_bar_panel_item_t compact_items_0[] = {
        {"Open", "", 0, 1, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Compact", "On", 2, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_panel_item_t compact_items_1[] = {
        {"Panels", "", 0, 1, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_SUBMENU},
        {"Density", "On", 2, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_panel_item_t compact_items_2[] = {
        {"Review", "", 0, 1, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Sync", "", 1, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_snapshot_t compact_snapshots[] = {
        {compact_menus, 3, 0, compact_items_0, 2, 0, 1},
        {compact_menus, 3, 1, compact_items_1, 2, 0, 1},
        {compact_menus, 3, 2, compact_items_2, 2, 1, 1},
};

static const egui_view_menu_bar_menu_t locked_menus[] = {
        {"Work", 1, 1},
        {"Review", 0, 1},
};

static const egui_view_menu_bar_panel_item_t locked_items[] = {
        {"Pinned", "", 0, 1, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_snapshot_t locked_snapshots[] = {
        {locked_menus, 2, 1, locked_items, 1, 0, 0},
};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&menu_bar_primary), index);
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&menu_bar_compact), index);
}

#if EGUI_CONFIG_RECORDING_TEST
static void clear_recording_pressed_state(void)
{
    menu_bar_primary.pressed_item = EGUI_VIEW_MENU_BAR_ITEM_NONE;
    menu_bar_primary.pressed_menu = EGUI_VIEW_MENU_BAR_HIT_NONE;
    egui_view_set_pressed(EGUI_VIEW_OF(&menu_bar_primary), false);
    egui_view_invalidate(EGUI_VIEW_OF(&menu_bar_primary));

    menu_bar_compact.pressed_item = EGUI_VIEW_MENU_BAR_ITEM_NONE;
    menu_bar_compact.pressed_menu = EGUI_VIEW_MENU_BAR_HIT_NONE;
    egui_view_set_pressed(EGUI_VIEW_OF(&menu_bar_compact), false);
    egui_view_invalidate(EGUI_VIEW_OF(&menu_bar_compact));
}

static void preview_primary_pressed_menu(uint8_t snapshot_index)
{
    clear_recording_pressed_state();
    apply_primary_snapshot(snapshot_index);
    menu_bar_primary.pressed_menu = primary_snapshots[snapshot_index].current_menu;
    egui_view_set_pressed(EGUI_VIEW_OF(&menu_bar_primary), true);
    egui_view_invalidate(EGUI_VIEW_OF(&menu_bar_primary));
}

static void preview_primary_pressed_item(uint8_t snapshot_index, uint8_t item_index)
{
    clear_recording_pressed_state();
    apply_primary_snapshot(snapshot_index);
    egui_view_menu_bar_set_current_item(EGUI_VIEW_OF(&menu_bar_primary), item_index);
    menu_bar_primary.pressed_item = item_index;
    egui_view_set_pressed(EGUI_VIEW_OF(&menu_bar_primary), true);
    egui_view_invalidate(EGUI_VIEW_OF(&menu_bar_primary));
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int on_primary_key(egui_view_t *self, egui_key_event_t *event)
{
    if (!egui_view_get_enable(self))
    {
        return 0;
    }
    if (event->key_code != EGUI_KEY_CODE_TAB)
    {
        return 0;
    }
    if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
    {
        return 1;
    }
    if (event->type == EGUI_KEY_EVENT_ACTION_UP)
    {
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        egui_view_request_focus(EGUI_VIEW_OF(&menu_bar_compact));
#endif
        return 1;
    }
    return 0;
}

static int on_compact_key(egui_view_t *self, egui_key_event_t *event)
{
    if (!egui_view_get_enable(self))
    {
        return 0;
    }
    if (event->key_code != EGUI_KEY_CODE_TAB)
    {
        return 0;
    }
    if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
    {
        return 1;
    }
    if (event->type == EGUI_KEY_EVENT_ACTION_UP)
    {
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        egui_view_request_focus(EGUI_VIEW_OF(&menu_bar_primary));
#endif
        return 1;
    }
    return 0;
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), MENU_BAR_ROOT_WIDTH, MENU_BAR_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), MENU_BAR_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_menu_bar_init(EGUI_VIEW_OF(&menu_bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&menu_bar_primary), MENU_BAR_PRIMARY_WIDTH, MENU_BAR_PRIMARY_HEIGHT);
    egui_view_menu_bar_set_snapshots(EGUI_VIEW_OF(&menu_bar_primary), primary_snapshots, 4);
    egui_view_menu_bar_set_font(EGUI_VIEW_OF(&menu_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_menu_bar_set_meta_font(EGUI_VIEW_OF(&menu_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_palette(EGUI_VIEW_OF(&menu_bar_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                   EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                   EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xD0D7DE));
    egui_view_set_margin(EGUI_VIEW_OF(&menu_bar_primary), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    static egui_view_api_t menu_bar_primary_key_api;
    egui_view_override_api_on_key(EGUI_VIEW_OF(&menu_bar_primary), &menu_bar_primary_key_api, on_primary_key);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&menu_bar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), MENU_BAR_BOTTOM_WIDTH, MENU_BAR_BOTTOM_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), MENU_BAR_COLUMN_WIDTH, MENU_BAR_BOTTOM_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_menu_bar_init(EGUI_VIEW_OF(&menu_bar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&menu_bar_compact), MENU_BAR_CARD_WIDTH, MENU_BAR_CARD_HEIGHT);
    egui_view_menu_bar_set_snapshots(EGUI_VIEW_OF(&menu_bar_compact), compact_snapshots, 3);
    egui_view_menu_bar_set_font(EGUI_VIEW_OF(&menu_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_meta_font(EGUI_VIEW_OF(&menu_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_compact_mode(EGUI_VIEW_OF(&menu_bar_compact), 1);
    egui_view_menu_bar_set_palette(EGUI_VIEW_OF(&menu_bar_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                   EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                   EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xD0D7DE));
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    static egui_view_api_t menu_bar_compact_key_api;
    egui_view_override_api_on_key(EGUI_VIEW_OF(&menu_bar_compact), &menu_bar_compact_key_api, on_compact_key);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&menu_bar_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&locked_column));
    egui_view_set_size(EGUI_VIEW_OF(&locked_column), MENU_BAR_COLUMN_WIDTH, MENU_BAR_BOTTOM_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&locked_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&locked_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&locked_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&locked_column));

    egui_view_menu_bar_init(EGUI_VIEW_OF(&menu_bar_locked));
    egui_view_set_size(EGUI_VIEW_OF(&menu_bar_locked), MENU_BAR_CARD_WIDTH, MENU_BAR_CARD_HEIGHT);
    egui_view_menu_bar_set_snapshots(EGUI_VIEW_OF(&menu_bar_locked), locked_snapshots, 1);
    egui_view_menu_bar_set_font(EGUI_VIEW_OF(&menu_bar_locked), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_meta_font(EGUI_VIEW_OF(&menu_bar_locked), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_compact_mode(EGUI_VIEW_OF(&menu_bar_locked), 1);
    egui_view_menu_bar_set_locked_mode(EGUI_VIEW_OF(&menu_bar_locked), 1);
    egui_view_menu_bar_set_palette(EGUI_VIEW_OF(&menu_bar_locked), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                   EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                   EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xD0D7DE));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&menu_bar_locked), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&locked_column), EGUI_VIEW_OF(&menu_bar_locked));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&locked_column));
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
            clear_recording_pressed_state();
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&menu_bar_primary));
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 1:
        if (first_call)
        {
            preview_primary_pressed_menu(2);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 2:
        if (first_call)
        {
            preview_primary_pressed_item(0, 1);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 3:
        if (first_call)
        {
            clear_recording_pressed_state();
            apply_compact_snapshot(1);
            egui_view_menu_bar_set_current_item(EGUI_VIEW_OF(&menu_bar_compact), 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&menu_bar_compact));
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 4:
        if (first_call)
        {
            clear_recording_pressed_state();
            apply_compact_snapshot(2);
            egui_view_menu_bar_activate_current_item(EGUI_VIEW_OF(&menu_bar_compact));
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 5:
        if (first_call)
        {
            clear_recording_pressed_state();
            apply_primary_snapshot(3);
            egui_view_menu_bar_set_current_item(EGUI_VIEW_OF(&menu_bar_primary), 0);
            egui_view_menu_bar_activate_current_item(EGUI_VIEW_OF(&menu_bar_primary));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&menu_bar_primary));
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 640);
        return true;
    default:
        return false;
    }
}
#endif
