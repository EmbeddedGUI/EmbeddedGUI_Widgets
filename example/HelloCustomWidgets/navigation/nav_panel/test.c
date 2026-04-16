#include "egui.h"
#include "egui_view_nav_panel.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define NAV_PANEL_ROOT_WIDTH         224
#define NAV_PANEL_ROOT_HEIGHT        224
#define NAV_PANEL_PRIMARY_WIDTH      198
#define NAV_PANEL_PRIMARY_HEIGHT     108
#define NAV_PANEL_COMPACT_WIDTH      58
#define NAV_PANEL_COMPACT_HEIGHT     74
#define NAV_PANEL_BOTTOM_ROW_WIDTH   152
#define NAV_PANEL_BOTTOM_ROW_HEIGHT  74
#define NAV_PANEL_RECORD_WAIT        90
#define NAV_PANEL_RECORD_FRAME_WAIT  170
#define NAV_PANEL_RECORD_FINAL_WAIT  520

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_nav_panel_t panel_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_nav_panel_t panel_compact;
static egui_view_nav_panel_t panel_read_only;
static egui_view_api_t panel_compact_api;
static egui_view_api_t panel_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Nav Panel";

static const egui_view_nav_panel_item_t primary_items[] = {
        {"Overview", "O"},
        {"Library", "L"},
        {"People", "P"},
};

static const egui_view_nav_panel_item_t compact_items[] = {
        {"Home", "H"},
        {"Files", "F"},
        {"Rules", "R"},
};

static const egui_view_nav_panel_item_t read_only_items[] = {
        {"Feed", "F"},
        {"Teams", "T"},
        {"Audit", "A"},
};

static void layout_page(void);

static void apply_primary_index(uint8_t index)
{
    egui_view_nav_panel_set_current_index(EGUI_VIEW_OF(&panel_primary), index);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_compact_state(void)
{
    egui_view_nav_panel_set_compact_mode(EGUI_VIEW_OF(&panel_compact), 1);
    egui_view_nav_panel_set_current_index(EGUI_VIEW_OF(&panel_compact), 0);
}

static void apply_read_only_state(void)
{
    egui_view_nav_panel_set_compact_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_nav_panel_set_current_index(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_nav_panel_set_read_only_mode(EGUI_VIEW_OF(&panel_read_only), 1);
}

static void apply_preview_states(void)
{
    apply_compact_state();
    apply_read_only_state();
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
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), NAV_PANEL_ROOT_WIDTH, NAV_PANEL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), NAV_PANEL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 6, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_nav_panel_init(EGUI_VIEW_OF(&panel_primary));
    egui_view_set_size(EGUI_VIEW_OF(&panel_primary), NAV_PANEL_PRIMARY_WIDTH, NAV_PANEL_PRIMARY_HEIGHT);
    egui_view_nav_panel_set_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_nav_panel_set_meta_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_nav_panel_set_items(EGUI_VIEW_OF(&panel_primary), primary_items, 3);
    egui_view_nav_panel_set_header_text(EGUI_VIEW_OF(&panel_primary), "Workspace");
    egui_view_nav_panel_set_footer_text(EGUI_VIEW_OF(&panel_primary), "Settings");
    egui_view_nav_panel_set_footer_badge(EGUI_VIEW_OF(&panel_primary), "S");
    egui_view_nav_panel_set_palette(EGUI_VIEW_OF(&panel_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x18222D),
                                    EGUI_COLOR_HEX(0x6E7C8B), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&panel_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&panel_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), NAV_PANEL_BOTTOM_ROW_WIDTH, NAV_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_nav_panel_init(EGUI_VIEW_OF(&panel_compact));
    egui_view_set_size(EGUI_VIEW_OF(&panel_compact), NAV_PANEL_COMPACT_WIDTH, NAV_PANEL_COMPACT_HEIGHT);
    egui_view_nav_panel_set_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_nav_panel_set_meta_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_nav_panel_set_items(EGUI_VIEW_OF(&panel_compact), compact_items, 3);
    egui_view_nav_panel_set_footer_badge(EGUI_VIEW_OF(&panel_compact), "S");
    egui_view_nav_panel_set_compact_mode(EGUI_VIEW_OF(&panel_compact), 1);
    egui_view_nav_panel_set_palette(EGUI_VIEW_OF(&panel_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x18222D),
                                    EGUI_COLOR_HEX(0x6E7C8B), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_nav_panel_override_static_preview_api(EGUI_VIEW_OF(&panel_compact), &panel_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&panel_compact));

    egui_view_nav_panel_init(EGUI_VIEW_OF(&panel_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&panel_read_only), NAV_PANEL_COMPACT_WIDTH, NAV_PANEL_COMPACT_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&panel_read_only), 8, 0, 0, 0);
    egui_view_nav_panel_set_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_nav_panel_set_meta_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_nav_panel_set_items(EGUI_VIEW_OF(&panel_read_only), read_only_items, 3);
    egui_view_nav_panel_set_footer_badge(EGUI_VIEW_OF(&panel_read_only), "S");
    egui_view_nav_panel_set_compact_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_nav_panel_set_read_only_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_nav_panel_set_palette(EGUI_VIEW_OF(&panel_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD9E1E8), EGUI_COLOR_HEX(0x566675),
                                    EGUI_COLOR_HEX(0x8A97A3), EGUI_COLOR_HEX(0xB8C4CF));
    egui_view_nav_panel_override_static_preview_api(EGUI_VIEW_OF(&panel_read_only), &panel_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&panel_read_only));

    apply_primary_index(0);
    apply_preview_states();

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_index(0);
    apply_preview_states();
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
            apply_primary_index(0);
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, NAV_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_index(1);
        }
        EGUI_SIM_SET_WAIT(p_action, NAV_PANEL_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, NAV_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_index(2);
        }
        EGUI_SIM_SET_WAIT(p_action, NAV_PANEL_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, NAV_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_index(0);
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, NAV_PANEL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
