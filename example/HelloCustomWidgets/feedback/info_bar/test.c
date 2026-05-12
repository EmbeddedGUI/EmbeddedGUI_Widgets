#include "egui.h"
#include "egui_view_info_bar.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define INFO_BAR_ROOT_WIDTH        224
#define INFO_BAR_ROOT_HEIGHT       204
#define INFO_BAR_PRIMARY_WIDTH     198
#define INFO_BAR_PRIMARY_HEIGHT    82
#define INFO_BAR_PREVIEW_WIDTH     104
#define INFO_BAR_PREVIEW_HEIGHT    54
#define INFO_BAR_BOTTOM_ROW_WIDTH  216
#define INFO_BAR_BOTTOM_ROW_HEIGHT 54
#define INFO_BAR_RECORD_WAIT       90
#define INFO_BAR_RECORD_FRAME_WAIT 170
#define INFO_BAR_RECORD_FINAL_WAIT 480
#define INFO_BAR_DEFAULT_SNAPSHOT  0
#define PRIMARY_SNAPSHOT_COUNT     ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_info_bar_t info_bar_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_info_bar_t info_bar_compact;
static egui_view_info_bar_t info_bar_read_only;
static egui_view_api_t info_bar_compact_api;
static egui_view_api_t info_bar_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "InfoBar";
static uint8_t primary_snapshot_index = 0;
static void layout_page(void);

static const egui_view_info_bar_snapshot_t primary_snapshots[] = {
        {"Sync complete", "All records are current.", "Details", 1, 1, 1},
        {"Policy update", "Review changes before publish.", "Review", 0, 1, 1},
        {"Storage warning", "Archive logs before next sync.", "Archive", 2, 1, 1},
        {"Sign-in required", "Reconnect account to continue.", "Sign in", 3, 1, 1},
};

static const egui_view_info_bar_snapshot_t compact_snapshots[] = {
        {"Compact note", "Policy synced.", "View", 0, 0, 1},
};

static const egui_view_info_bar_snapshot_t read_only_snapshots[] = {
        {"Read only", "Managed setting.", NULL, 0, 0, 0},
};

static void apply_primary_snapshot(uint8_t index)
{
    primary_snapshot_index = (uint8_t)(index % PRIMARY_SNAPSHOT_COUNT);
    egui_view_info_bar_set_current_snapshot(EGUI_VIEW_OF(&info_bar_primary), primary_snapshot_index);
    egui_view_info_bar_set_opened(EGUI_VIEW_OF(&info_bar_primary), 1);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(INFO_BAR_DEFAULT_SNAPSHOT);
}

static void apply_preview_states(void)
{
    egui_view_info_bar_set_current_snapshot(EGUI_VIEW_OF(&info_bar_compact), 0);
    egui_view_info_bar_set_opened(EGUI_VIEW_OF(&info_bar_compact), 1);
    egui_view_info_bar_set_current_snapshot(EGUI_VIEW_OF(&info_bar_read_only), 0);
    egui_view_info_bar_set_read_only_mode(EGUI_VIEW_OF(&info_bar_read_only), 1);
    egui_view_info_bar_set_opened(EGUI_VIEW_OF(&info_bar_read_only), 1);
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

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), INFO_BAR_ROOT_WIDTH, INFO_BAR_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), INFO_BAR_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), HCW_COLOR_TEXT, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 7, 0, 5);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_info_bar_init(EGUI_VIEW_OF(&info_bar_primary));
    egui_view_set_size(EGUI_VIEW_OF(&info_bar_primary), INFO_BAR_PRIMARY_WIDTH, INFO_BAR_PRIMARY_HEIGHT);
    egui_view_info_bar_set_snapshots(EGUI_VIEW_OF(&info_bar_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_info_bar_set_font(EGUI_VIEW_OF(&info_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_info_bar_set_meta_font(EGUI_VIEW_OF(&info_bar_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_info_bar_set_palette(EGUI_VIEW_OF(&info_bar_primary), HCW_COLOR_SURFACE, HCW_COLOR_BORDER, HCW_COLOR_TEXT,
                                   HCW_COLOR_TEXT_MUTED, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY, HCW_COLOR_SUCCESS,
                                   HCW_COLOR_WARNING, HCW_COLOR_DANGER);
    egui_view_set_margin(EGUI_VIEW_OF(&info_bar_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&info_bar_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), INFO_BAR_BOTTOM_ROW_WIDTH, INFO_BAR_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_info_bar_init(EGUI_VIEW_OF(&info_bar_compact));
    egui_view_set_size(EGUI_VIEW_OF(&info_bar_compact), INFO_BAR_PREVIEW_WIDTH, INFO_BAR_PREVIEW_HEIGHT);
    egui_view_info_bar_set_snapshots(EGUI_VIEW_OF(&info_bar_compact), compact_snapshots, 1);
    egui_view_info_bar_set_font(EGUI_VIEW_OF(&info_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_info_bar_set_meta_font(EGUI_VIEW_OF(&info_bar_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_info_bar_set_compact_mode(EGUI_VIEW_OF(&info_bar_compact), 1);
    egui_view_info_bar_set_palette(EGUI_VIEW_OF(&info_bar_compact), HCW_COLOR_SURFACE, HCW_COLOR_BORDER, HCW_COLOR_TEXT,
                                   HCW_COLOR_TEXT_MUTED, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY, HCW_COLOR_SUCCESS,
                                   HCW_COLOR_WARNING, HCW_COLOR_DANGER);
    egui_view_info_bar_override_static_preview_api(EGUI_VIEW_OF(&info_bar_compact), &info_bar_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&info_bar_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&info_bar_compact));

    egui_view_info_bar_init(EGUI_VIEW_OF(&info_bar_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&info_bar_read_only), INFO_BAR_PREVIEW_WIDTH, INFO_BAR_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&info_bar_read_only), 8, 0, 0, 0);
    egui_view_info_bar_set_snapshots(EGUI_VIEW_OF(&info_bar_read_only), read_only_snapshots, 1);
    egui_view_info_bar_set_font(EGUI_VIEW_OF(&info_bar_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_info_bar_set_meta_font(EGUI_VIEW_OF(&info_bar_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_info_bar_set_compact_mode(EGUI_VIEW_OF(&info_bar_read_only), 1);
    egui_view_info_bar_set_read_only_mode(EGUI_VIEW_OF(&info_bar_read_only), 1);
    egui_view_info_bar_set_palette(EGUI_VIEW_OF(&info_bar_read_only), HCW_COLOR_PANEL, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT_MUTED,
                                   HCW_COLOR_TEXT_MUTED, HCW_COLOR_TEXT_MUTED, HCW_COLOR_PRIMARY, HCW_COLOR_SUCCESS,
                                   HCW_COLOR_WARNING_DARK, HCW_COLOR_DANGER_DARK);
    egui_view_info_bar_override_static_preview_api(EGUI_VIEW_OF(&info_bar_read_only), &info_bar_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&info_bar_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&info_bar_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
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
            apply_primary_default_state();
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BAR_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BAR_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BAR_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BAR_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BAR_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BAR_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BAR_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BAR_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, INFO_BAR_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
