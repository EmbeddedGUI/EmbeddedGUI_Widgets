#include "egui.h"
#include "egui_view_menu_flyout.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define MENU_FLYOUT_ROOT_WIDTH     224
#define MENU_FLYOUT_ROOT_HEIGHT    224
#define MENU_FLYOUT_PRIMARY_WIDTH  188
#define MENU_FLYOUT_PRIMARY_HEIGHT 104
#define MENU_FLYOUT_BOTTOM_WIDTH   214
#define MENU_FLYOUT_BOTTOM_HEIGHT  78
#define MENU_FLYOUT_CARD_WIDTH     104
#define MENU_FLYOUT_CARD_HEIGHT    78

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_menu_flyout_t flyout_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_menu_flyout_t flyout_compact;
static egui_view_menu_flyout_t flyout_disabled;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Menu Flyout";

static const egui_view_menu_flyout_item_t primary_items_0[] = {
        {"OP", "Open panel", "Ctrl+O", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"RV", "Review history", "", 0, 1, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_SUBMENU},
        {"CP", "Copy link", "Ctrl+L", 0, 0, 1, 1, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"RN", "Rename", "F2", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
};

static const egui_view_menu_flyout_item_t primary_items_1[] = {
        {"NM", "Name ascending", "", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"DT", "Date modified", "", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"PN", "Pin summary", "On", 1, 1, 1, 1, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"PR", "Preferences", "", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_SUBMENU},
};

static const egui_view_menu_flyout_item_t primary_items_2[] = {
        {"EX", "Export report", "Ctrl+E", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"CP", "Copy path", "Ctrl+Shift+C", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"DP", "Duplicate", "Ctrl+D", 0, 0, 1, 1, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"DL", "Delete record", "Shift+Del", 3, 1, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
};

static const egui_view_menu_flyout_item_t primary_items_3[] = {
        {"LY", "Layout wide", "", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"DS", "Density compact", "On", 2, 1, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"MV", "Move to group", "", 0, 0, 1, 1, EGUI_VIEW_MENU_FLYOUT_TRAILING_SUBMENU},
        {"AR", "Archive", "", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
};

static const egui_view_menu_flyout_item_t compact_items_0[] = {
        {"OP", "Open", "", 0, 1, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"CP", "Copy", "", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"MO", "More", "", 0, 0, 1, 1, EGUI_VIEW_MENU_FLYOUT_TRAILING_SUBMENU},
};

static const egui_view_menu_flyout_item_t compact_items_1[] = {
        {"PI", "Pin", "On", 1, 1, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"DN", "Density", "On", 2, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"DL", "Delete", "", 3, 0, 1, 1, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
};

static const egui_view_menu_flyout_item_t disabled_items_0[] = {
        {"RN", "Rename", "F2", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"EX", "Export", "Ctrl+E", 0, 1, 0, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"DL", "Delete", "", 3, 0, 0, 1, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
};

static const egui_view_menu_flyout_snapshot_t primary_snapshots[] = {
        {primary_items_0, 4, 1},
        {primary_items_1, 4, 2},
        {primary_items_2, 4, 3},
        {primary_items_3, 4, 1},
};

static const egui_view_menu_flyout_snapshot_t compact_snapshots[] = {
        {compact_items_0, 3, 0},
        {compact_items_1, 3, 0},
};

static const egui_view_menu_flyout_snapshot_t disabled_snapshots[] = {
        {disabled_items_0, 3, 1},
};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_menu_flyout_set_current_snapshot(EGUI_VIEW_OF(&flyout_primary), index);
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_menu_flyout_set_current_snapshot(EGUI_VIEW_OF(&flyout_compact), index);
}

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}

static void on_primary_click(egui_view_t *self)
{
    uint8_t next = (egui_view_menu_flyout_get_current_snapshot(self) + 1) % 4;

    apply_primary_snapshot(next);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), MENU_FLYOUT_ROOT_WIDTH, MENU_FLYOUT_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), MENU_FLYOUT_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_menu_flyout_init(EGUI_VIEW_OF(&flyout_primary));
    egui_view_set_size(EGUI_VIEW_OF(&flyout_primary), MENU_FLYOUT_PRIMARY_WIDTH, MENU_FLYOUT_PRIMARY_HEIGHT);
    egui_view_menu_flyout_set_snapshots(EGUI_VIEW_OF(&flyout_primary), primary_snapshots, 4);
    egui_view_menu_flyout_set_font(EGUI_VIEW_OF(&flyout_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_menu_flyout_set_meta_font(EGUI_VIEW_OF(&flyout_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_flyout_set_palette(EGUI_VIEW_OF(&flyout_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                      EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xD0D7DE));
    egui_view_set_margin(EGUI_VIEW_OF(&flyout_primary), 0, 0, 0, 8);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&flyout_primary), on_primary_click);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&flyout_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), MENU_FLYOUT_BOTTOM_WIDTH, MENU_FLYOUT_BOTTOM_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_menu_flyout_init(EGUI_VIEW_OF(&flyout_compact));
    egui_view_set_size(EGUI_VIEW_OF(&flyout_compact), MENU_FLYOUT_CARD_WIDTH, MENU_FLYOUT_CARD_HEIGHT);
    egui_view_menu_flyout_set_snapshots(EGUI_VIEW_OF(&flyout_compact), compact_snapshots, 2);
    egui_view_menu_flyout_set_font(EGUI_VIEW_OF(&flyout_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_flyout_set_meta_font(EGUI_VIEW_OF(&flyout_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_flyout_set_compact_mode(EGUI_VIEW_OF(&flyout_compact), 1);
    egui_view_menu_flyout_set_palette(EGUI_VIEW_OF(&flyout_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                      EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xD0D7DE));
    static egui_view_api_t flyout_compact_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&flyout_compact), &flyout_compact_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flyout_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&flyout_compact));

    egui_view_menu_flyout_init(EGUI_VIEW_OF(&flyout_disabled));
    egui_view_set_size(EGUI_VIEW_OF(&flyout_disabled), MENU_FLYOUT_CARD_WIDTH, MENU_FLYOUT_CARD_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&flyout_disabled), 6, 0, 0, 0);
    egui_view_menu_flyout_set_snapshots(EGUI_VIEW_OF(&flyout_disabled), disabled_snapshots, 1);
    egui_view_menu_flyout_set_font(EGUI_VIEW_OF(&flyout_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_flyout_set_meta_font(EGUI_VIEW_OF(&flyout_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_menu_flyout_set_compact_mode(EGUI_VIEW_OF(&flyout_disabled), 1);
    egui_view_menu_flyout_set_disabled_mode(EGUI_VIEW_OF(&flyout_disabled), 1);
    egui_view_menu_flyout_set_palette(EGUI_VIEW_OF(&flyout_disabled), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                      EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_HEX(0xD0D7DE));
    static egui_view_api_t flyout_disabled_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&flyout_disabled), &flyout_disabled_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flyout_disabled), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&flyout_disabled));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);

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
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(2);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 3:
        if (first_call)
        {
            apply_compact_snapshot(1);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(3);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 640);
        return true;
    default:
        return false;
    }
}
#endif
