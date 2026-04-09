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
#define MENU_BAR_CARD_WIDTH     104
#define MENU_BAR_CARD_HEIGHT    74

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_menu_bar_t menu_bar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_menu_bar_t menu_bar_compact;
static egui_view_menu_bar_t menu_bar_read_only;

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
        {"New workspace", "Ctrl+N", 0, 1, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Open recent", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_SUBMENU},
        {"Copy link", "Ctrl+L", 1, 0, 1, 1, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Properties", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
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
        {"Export report", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Archive record", "", 0, 0, 0, 1, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Delete record", "", 3, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
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

static const egui_view_menu_bar_menu_t read_only_menus[] = {
        {"Home", 1, 1},
        {"Review", 0, 1},
};

static const egui_view_menu_bar_panel_item_t read_only_items[] = {
        {"Pinned", "", 0, 1, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_snapshot_t read_only_snapshots[] = {
        {read_only_menus, 2, 1, read_only_items, 1, 0, 0},
};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&menu_bar_primary), index);
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&menu_bar_compact), index);
}

static void apply_read_only_snapshot(uint8_t index)
{
    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&menu_bar_read_only), index);
}

static void apply_read_only_state(uint8_t index)
{
    apply_read_only_snapshot(index);
    egui_view_menu_bar_set_read_only_mode(EGUI_VIEW_OF(&menu_bar_read_only), 1);
}

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
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

    menu_bar_read_only.pressed_item = EGUI_VIEW_MENU_BAR_ITEM_NONE;
    menu_bar_read_only.pressed_menu = EGUI_VIEW_MENU_BAR_HIT_NONE;
    egui_view_set_pressed(EGUI_VIEW_OF(&menu_bar_read_only), false);
    egui_view_invalidate(EGUI_VIEW_OF(&menu_bar_read_only));
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
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 6, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_menu_bar_init(EGUI_VIEW_OF(&menu_bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&menu_bar_primary), MENU_BAR_PRIMARY_WIDTH, MENU_BAR_PRIMARY_HEIGHT);
    egui_view_menu_bar_set_snapshots(EGUI_VIEW_OF(&menu_bar_primary), primary_snapshots, 4);
    egui_view_menu_bar_set_font(EGUI_VIEW_OF(&menu_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_menu_bar_set_meta_font(EGUI_VIEW_OF(&menu_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_palette(EGUI_VIEW_OF(&menu_bar_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x18222D),
                                   EGUI_COLOR_HEX(0x6E7C8B), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                   EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xD0D7DE));
    egui_view_set_margin(EGUI_VIEW_OF(&menu_bar_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&menu_bar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), MENU_BAR_BOTTOM_WIDTH, MENU_BAR_BOTTOM_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_menu_bar_init(EGUI_VIEW_OF(&menu_bar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&menu_bar_compact), MENU_BAR_CARD_WIDTH, MENU_BAR_CARD_HEIGHT);
    egui_view_menu_bar_set_snapshots(EGUI_VIEW_OF(&menu_bar_compact), compact_snapshots, 3);
    egui_view_menu_bar_set_font(EGUI_VIEW_OF(&menu_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_meta_font(EGUI_VIEW_OF(&menu_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_compact_mode(EGUI_VIEW_OF(&menu_bar_compact), 1);
    egui_view_menu_bar_set_palette(EGUI_VIEW_OF(&menu_bar_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x18222D),
                                   EGUI_COLOR_HEX(0x6E7C8B), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                   EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xD0D7DE));
    static egui_view_api_t menu_bar_compact_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&menu_bar_compact), &menu_bar_compact_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&menu_bar_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&menu_bar_compact));

    egui_view_menu_bar_init(EGUI_VIEW_OF(&menu_bar_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&menu_bar_read_only), MENU_BAR_CARD_WIDTH, MENU_BAR_CARD_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&menu_bar_read_only), 8, 0, 0, 0);
    egui_view_menu_bar_set_snapshots(EGUI_VIEW_OF(&menu_bar_read_only), read_only_snapshots, 1);
    egui_view_menu_bar_set_font(EGUI_VIEW_OF(&menu_bar_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_meta_font(EGUI_VIEW_OF(&menu_bar_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_bar_set_compact_mode(EGUI_VIEW_OF(&menu_bar_read_only), 1);
    egui_view_menu_bar_set_read_only_mode(EGUI_VIEW_OF(&menu_bar_read_only), 1);
    egui_view_menu_bar_set_palette(EGUI_VIEW_OF(&menu_bar_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xDBE2E8), EGUI_COLOR_HEX(0x566675),
                                   EGUI_COLOR_HEX(0x8A97A3), EGUI_COLOR_HEX(0xB8C4CF), EGUI_COLOR_HEX(0xAEBFB8), EGUI_COLOR_HEX(0xC7B592),
                                   EGUI_COLOR_HEX(0xCBA9A9), EGUI_COLOR_HEX(0xD8DFE6));
    static egui_view_api_t menu_bar_read_only_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&menu_bar_read_only), &menu_bar_read_only_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&menu_bar_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&menu_bar_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_read_only_state(0);

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
            clear_recording_pressed_state();
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
            apply_read_only_state(0);
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
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 4:
        if (first_call)
        {
            clear_recording_pressed_state();
            apply_compact_snapshot(2);
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
