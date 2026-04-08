#include "egui.h"
#include "egui_view_breadcrumb_bar.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define BREADCRUMB_ROOT_WIDTH        224
#define BREADCRUMB_ROOT_HEIGHT       140
#define BREADCRUMB_PRIMARY_WIDTH     198
#define BREADCRUMB_PRIMARY_HEIGHT    48
#define BREADCRUMB_PREVIEW_WIDTH     104
#define BREADCRUMB_PREVIEW_HEIGHT    36
#define BREADCRUMB_BOTTOM_ROW_WIDTH  216
#define BREADCRUMB_BOTTOM_ROW_HEIGHT 36

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_breadcrumb_bar_t bar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_breadcrumb_bar_t bar_compact;
static egui_view_linearlayout_t locked_column;
static egui_view_breadcrumb_bar_t bar_locked;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Breadcrumb Bar";

static const char *primary_items_a[] = {"Home", "Docs", "Nav", "Details"};
static const char *primary_items_b[] = {"Home", "Demos", "Forms", "Value"};
static const char *primary_items_c[] = {"Home", "Files", "Grid", "Review"};
static const egui_view_breadcrumb_bar_snapshot_t primary_snapshots[] = {
        {"Docs path", primary_items_a, 4, 3},
        {"Forms path", primary_items_b, 4, 3},
        {"Review path", primary_items_c, 4, 3},
};

static const char *compact_items_a[] = {"Home", "System", "Alerts", "Bills"};
static const char *compact_items_b[] = {"Home", "Teams", "Review", "Access"};
static const egui_view_breadcrumb_bar_snapshot_t compact_snapshots[] = {
        {"Compact bills", compact_items_a, 4, 3},
        {"Compact access", compact_items_b, 4, 3},
};

static const char *locked_items[] = {"Home", "Admin", "Reports", "Audit"};
static const egui_view_breadcrumb_bar_snapshot_t locked_snapshots[] = {
        {"Read only", locked_items, 4, 3},
};

static void on_primary_click(egui_view_t *self)
{
    uint8_t next = (egui_view_breadcrumb_bar_get_current_snapshot(self) + 1) % 3;

    egui_view_breadcrumb_bar_set_current_snapshot(self, next);
}

static void on_compact_click(egui_view_t *self)
{
    uint8_t next = (egui_view_breadcrumb_bar_get_current_snapshot(self) + 1) % 2;

    egui_view_breadcrumb_bar_set_current_snapshot(self, next);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), BREADCRUMB_ROOT_WIDTH, BREADCRUMB_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), BREADCRUMB_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_breadcrumb_bar_init(EGUI_VIEW_OF(&bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&bar_primary), BREADCRUMB_PRIMARY_WIDTH, BREADCRUMB_PRIMARY_HEIGHT);
    egui_view_breadcrumb_bar_set_snapshots(EGUI_VIEW_OF(&bar_primary), primary_snapshots, 3);
    egui_view_breadcrumb_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_primary), 0);
    egui_view_breadcrumb_bar_set_font(EGUI_VIEW_OF(&bar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_breadcrumb_bar_set_palette(EGUI_VIEW_OF(&bar_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                         EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&bar_primary), 0, 0, 0, 8);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&bar_primary), on_primary_click);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), BREADCRUMB_BOTTOM_ROW_WIDTH, BREADCRUMB_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), BREADCRUMB_PREVIEW_WIDTH, BREADCRUMB_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_breadcrumb_bar_init(EGUI_VIEW_OF(&bar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&bar_compact), BREADCRUMB_PREVIEW_WIDTH, BREADCRUMB_PREVIEW_HEIGHT);
    egui_view_breadcrumb_bar_set_snapshots(EGUI_VIEW_OF(&bar_compact), compact_snapshots, 2);
    egui_view_breadcrumb_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_compact), 0);
    egui_view_breadcrumb_bar_set_font(EGUI_VIEW_OF(&bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_breadcrumb_bar_set_compact_mode(EGUI_VIEW_OF(&bar_compact), 1);
    egui_view_breadcrumb_bar_set_palette(EGUI_VIEW_OF(&bar_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                         EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&bar_compact), on_compact_click);
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&bar_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&locked_column));
    egui_view_set_size(EGUI_VIEW_OF(&locked_column), BREADCRUMB_PREVIEW_WIDTH, BREADCRUMB_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&locked_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&locked_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&locked_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&locked_column));

    egui_view_breadcrumb_bar_init(EGUI_VIEW_OF(&bar_locked));
    egui_view_set_size(EGUI_VIEW_OF(&bar_locked), BREADCRUMB_PREVIEW_WIDTH, BREADCRUMB_PREVIEW_HEIGHT);
    egui_view_breadcrumb_bar_set_snapshots(EGUI_VIEW_OF(&bar_locked), locked_snapshots, 1);
    egui_view_breadcrumb_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_locked), 0);
    egui_view_breadcrumb_bar_set_font(EGUI_VIEW_OF(&bar_locked), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_breadcrumb_bar_set_compact_mode(EGUI_VIEW_OF(&bar_locked), 1);
    egui_view_breadcrumb_bar_set_locked_mode(EGUI_VIEW_OF(&bar_locked), 1);
    egui_view_breadcrumb_bar_set_palette(EGUI_VIEW_OF(&bar_locked), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
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
            egui_view_breadcrumb_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_primary), 0);
            egui_view_breadcrumb_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_compact), 0);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 1:
        if (first_call)
        {
            egui_view_breadcrumb_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_primary), 1);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 2:
        if (first_call)
        {
            egui_view_breadcrumb_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_primary), 2);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 240);
        return true;
    case 3:
        if (first_call)
        {
            egui_view_breadcrumb_bar_set_current_snapshot(EGUI_VIEW_OF(&bar_compact), 1);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 640);
        return true;
    default:
        return false;
    }
}
#endif
