#include "egui.h"
#include "egui_view_tab_strip.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TAB_STRIP_ROOT_WIDTH        224
#define TAB_STRIP_ROOT_HEIGHT       136
#define TAB_STRIP_PRIMARY_WIDTH     198
#define TAB_STRIP_PRIMARY_HEIGHT    44
#define TAB_STRIP_PREVIEW_WIDTH     104
#define TAB_STRIP_PREVIEW_HEIGHT    36
#define TAB_STRIP_BOTTOM_ROW_WIDTH  216
#define TAB_STRIP_BOTTOM_ROW_HEIGHT 36

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_tab_strip_t bar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_tab_strip_t bar_compact;
static egui_view_linearlayout_t locked_column;
static egui_view_tab_strip_t bar_locked;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Tab Strip";
static const char *primary_tabs[] = {"Overview", "Usage", "Access"};
static const char *compact_tabs[] = {"Home", "Logs"};
static const char *locked_tabs[] = {"Usage", "Audit"};

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TAB_STRIP_ROOT_WIDTH, TAB_STRIP_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TAB_STRIP_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_tab_strip_init(EGUI_VIEW_OF(&bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&bar_primary), TAB_STRIP_PRIMARY_WIDTH, TAB_STRIP_PRIMARY_HEIGHT);
    egui_view_tab_strip_set_tabs(EGUI_VIEW_OF(&bar_primary), primary_tabs, 3);
    egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&bar_primary), 0);
    egui_view_tab_strip_set_font(EGUI_VIEW_OF(&bar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tab_strip_set_palette(EGUI_VIEW_OF(&bar_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                    EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&bar_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TAB_STRIP_BOTTOM_ROW_WIDTH, TAB_STRIP_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), TAB_STRIP_PREVIEW_WIDTH, TAB_STRIP_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_tab_strip_init(EGUI_VIEW_OF(&bar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&bar_compact), TAB_STRIP_PREVIEW_WIDTH, TAB_STRIP_PREVIEW_HEIGHT);
    egui_view_tab_strip_set_tabs(EGUI_VIEW_OF(&bar_compact), compact_tabs, 2);
    egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&bar_compact), 0);
    egui_view_tab_strip_set_font(EGUI_VIEW_OF(&bar_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tab_strip_set_compact_mode(EGUI_VIEW_OF(&bar_compact), 1);
    egui_view_tab_strip_set_palette(EGUI_VIEW_OF(&bar_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                    EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&bar_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&locked_column));
    egui_view_set_size(EGUI_VIEW_OF(&locked_column), TAB_STRIP_PREVIEW_WIDTH, TAB_STRIP_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&locked_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&locked_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&locked_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&locked_column));

    egui_view_tab_strip_init(EGUI_VIEW_OF(&bar_locked));
    egui_view_set_size(EGUI_VIEW_OF(&bar_locked), TAB_STRIP_PREVIEW_WIDTH, TAB_STRIP_PREVIEW_HEIGHT);
    egui_view_tab_strip_set_tabs(EGUI_VIEW_OF(&bar_locked), locked_tabs, 2);
    egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&bar_locked), 1);
    egui_view_tab_strip_set_font(EGUI_VIEW_OF(&bar_locked), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tab_strip_set_compact_mode(EGUI_VIEW_OF(&bar_locked), 1);
    egui_view_tab_strip_set_locked_mode(EGUI_VIEW_OF(&bar_locked), 1);
    egui_view_tab_strip_set_palette(EGUI_VIEW_OF(&bar_locked), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                    EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&bar_locked), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&locked_column), EGUI_VIEW_OF(&bar_locked));

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
            egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&bar_primary), 0);
            egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&bar_compact), 0);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 1:
        if (first_call)
        {
            egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&bar_primary), 1);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 2:
        if (first_call)
        {
            egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&bar_primary), 2);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 3:
        if (first_call)
        {
            egui_view_tab_strip_set_current_index(EGUI_VIEW_OF(&bar_compact), 1);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 640);
        return true;
    default:
        return false;
    }
}
#endif
