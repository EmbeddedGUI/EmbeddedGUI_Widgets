#include "egui.h"
#include "egui_view_grid_view.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#include "../items_repeater/egui_view_items_repeater.c"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define GRID_VIEW_ROOT_WIDTH        224
#define GRID_VIEW_ROOT_HEIGHT       288
#define GRID_VIEW_PRIMARY_WIDTH     196
#define GRID_VIEW_PRIMARY_HEIGHT    148
#define GRID_VIEW_PREVIEW_WIDTH     104
#define GRID_VIEW_PREVIEW_HEIGHT    86
#define GRID_VIEW_BOTTOM_ROW_WIDTH  216
#define GRID_VIEW_BOTTOM_ROW_HEIGHT 86
#define GRID_VIEW_RECORD_WAIT       90
#define GRID_VIEW_RECORD_FRAME_WAIT 180
#define GRID_VIEW_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static hcw_grid_view_t grid_view_primary;
static egui_view_linearlayout_t bottom_row;
static hcw_grid_view_t grid_view_compact;
static hcw_grid_view_t grid_view_read_only;
static egui_view_api_t grid_view_compact_api;
static egui_view_api_t grid_view_read_only_api;
static uint8_t ui_ready;

static void layout_page(void);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Grid View";

static const hcw_grid_view_item_t primary_items_0[] = {
        {"AST", "Hero", "320", HCW_GRID_VIEW_TONE_ACCENT, 1, HCW_GRID_VIEW_WIDTH_PROMINENT},
        {"KIT", "Cards", "24", HCW_GRID_VIEW_TONE_SUCCESS, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"IMG", "Icons", "86", HCW_GRID_VIEW_TONE_NEUTRAL, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"UI", "Panels", "12", HCW_GRID_VIEW_TONE_WARNING, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"DOC", "Specs", "18", HCW_GRID_VIEW_TONE_ACCENT, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"VID", "Motion", "6", HCW_GRID_VIEW_TONE_SUCCESS, 1, HCW_GRID_VIEW_WIDTH_PROMINENT},
};

static const hcw_grid_view_item_t primary_items_1[] = {
        {"TMP", "Launch", "v3", HCW_GRID_VIEW_TONE_ACCENT, 1, HCW_GRID_VIEW_WIDTH_PROMINENT},
        {"OPS", "Board", "7", HCW_GRID_VIEW_TONE_SUCCESS, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"DOC", "Brief", "2", HCW_GRID_VIEW_TONE_WARNING, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"NAV", "Tiles", "16", HCW_GRID_VIEW_TONE_NEUTRAL, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"REL", "Gate", "Open", HCW_GRID_VIEW_TONE_SUCCESS, 1, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"QA", "Review", "4", HCW_GRID_VIEW_TONE_WARNING, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
};

static const hcw_grid_view_item_t primary_items_2[] = {
        {"A", "Avery", "Design", HCW_GRID_VIEW_TONE_ACCENT, 1, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"B", "Blair", "Dev", HCW_GRID_VIEW_TONE_SUCCESS, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"C", "Casey", "QA", HCW_GRID_VIEW_TONE_WARNING, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"D", "Drew", "PM", HCW_GRID_VIEW_TONE_NEUTRAL, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"E", "Eden", "Ops", HCW_GRID_VIEW_TONE_ACCENT, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"F", "Finley", "Docs", HCW_GRID_VIEW_TONE_SUCCESS, 1, HCW_GRID_VIEW_WIDTH_BALANCED},
};

static const hcw_grid_view_snapshot_t primary_snapshots[] = {
        {"AST", "Assets gallery", "Pinned assets stay tiled while the focused tile keeps selection emphasis.", "Gallery focus", primary_items_0, 6, 1,
         HCW_GRID_VIEW_LAYOUT_WRAP},
        {"TMP", "Template board", "Reusable templates share the same tile shell with a denser board rhythm.", "Board layout", primary_items_1, 6, 0,
         HCW_GRID_VIEW_LAYOUT_WRAP},
        {"TEAM", "Team board", "People tiles keep the same collection focus model without extra page chrome.", "Selection focus", primary_items_2, 6, 2,
         HCW_GRID_VIEW_LAYOUT_WRAP},
};

static const hcw_grid_view_item_t compact_items_0[] = {
        {"A", "Hero", "8", HCW_GRID_VIEW_TONE_ACCENT, 1, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"B", "Docs", "3", HCW_GRID_VIEW_TONE_WARNING, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"C", "Cards", "5", HCW_GRID_VIEW_TONE_NEUTRAL, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"D", "Ship", "1", HCW_GRID_VIEW_TONE_SUCCESS, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
};

static const hcw_grid_view_snapshot_t compact_snapshot = {
        "AST", "Compact grid", "", "", compact_items_0, 4, 0, HCW_GRID_VIEW_LAYOUT_WRAP};

static const hcw_grid_view_item_t read_only_items[] = {
        {"L1", "Locked", "View", HCW_GRID_VIEW_TONE_NEUTRAL, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"L2", "Muted", "Read", HCW_GRID_VIEW_TONE_WARNING, 1, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"L3", "Ready", "Pass", HCW_GRID_VIEW_TONE_SUCCESS, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"L4", "Queue", "Idle", HCW_GRID_VIEW_TONE_ACCENT, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
};

static const hcw_grid_view_snapshot_t read_only_snapshot = {
        "LOCK", "Read only grid", "", "", read_only_items, 4, 1, HCW_GRID_VIEW_LAYOUT_WRAP};

static void apply_primary_snapshot(uint8_t index)
{
    hcw_grid_view_set_current_snapshot(EGUI_VIEW_OF(&grid_view_primary), index % PRIMARY_SNAPSHOT_COUNT);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(0);
}

static void apply_preview_states(void)
{
    hcw_grid_view_set_current_snapshot(EGUI_VIEW_OF(&grid_view_compact), 0);
    hcw_grid_view_set_current_snapshot(EGUI_VIEW_OF(&grid_view_read_only), 0);
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), GRID_VIEW_ROOT_WIDTH, GRID_VIEW_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), GRID_VIEW_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), HCW_COLOR_TEXT, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    hcw_grid_view_init(EGUI_VIEW_OF(&grid_view_primary));
    egui_view_set_size(EGUI_VIEW_OF(&grid_view_primary), GRID_VIEW_PRIMARY_WIDTH, GRID_VIEW_PRIMARY_HEIGHT);
    hcw_grid_view_set_snapshots(EGUI_VIEW_OF(&grid_view_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    hcw_grid_view_set_font(EGUI_VIEW_OF(&grid_view_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_grid_view_set_meta_font(EGUI_VIEW_OF(&grid_view_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_grid_view_set_palette(EGUI_VIEW_OF(&grid_view_primary), HCW_COLOR_PANEL, HCW_COLOR_PANEL, HCW_COLOR_BORDER,
                              HCW_COLOR_TEXT, HCW_COLOR_TEXT_MUTED, HCW_COLOR_PRIMARY, HCW_COLOR_SUCCESS,
                              HCW_COLOR_WARNING, HCW_COLOR_NEUTRAL);
    egui_view_set_margin(EGUI_VIEW_OF(&grid_view_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&grid_view_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), GRID_VIEW_BOTTOM_ROW_WIDTH, GRID_VIEW_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    hcw_grid_view_init(EGUI_VIEW_OF(&grid_view_compact));
    egui_view_set_size(EGUI_VIEW_OF(&grid_view_compact), GRID_VIEW_PREVIEW_WIDTH, GRID_VIEW_PREVIEW_HEIGHT);
    hcw_grid_view_set_snapshots(EGUI_VIEW_OF(&grid_view_compact), &compact_snapshot, 1);
    hcw_grid_view_set_font(EGUI_VIEW_OF(&grid_view_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_grid_view_set_meta_font(EGUI_VIEW_OF(&grid_view_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_grid_view_set_compact_mode(EGUI_VIEW_OF(&grid_view_compact), 1);
    hcw_grid_view_set_palette(EGUI_VIEW_OF(&grid_view_compact), HCW_COLOR_PANEL, HCW_COLOR_PANEL, HCW_COLOR_BORDER,
                              HCW_COLOR_TEXT, HCW_COLOR_TEXT_MUTED, HCW_COLOR_PRIMARY, HCW_COLOR_SUCCESS,
                              HCW_COLOR_WARNING, HCW_COLOR_TEXT_MUTED);
    hcw_grid_view_override_static_preview_api(EGUI_VIEW_OF(&grid_view_compact), &grid_view_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&grid_view_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&grid_view_compact));

    hcw_grid_view_init(EGUI_VIEW_OF(&grid_view_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&grid_view_read_only), GRID_VIEW_PREVIEW_WIDTH, GRID_VIEW_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&grid_view_read_only), 8, 0, 0, 0);
    hcw_grid_view_set_snapshots(EGUI_VIEW_OF(&grid_view_read_only), &read_only_snapshot, 1);
    hcw_grid_view_set_font(EGUI_VIEW_OF(&grid_view_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_grid_view_set_meta_font(EGUI_VIEW_OF(&grid_view_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_grid_view_set_compact_mode(EGUI_VIEW_OF(&grid_view_read_only), 1);
    hcw_grid_view_set_read_only_mode(EGUI_VIEW_OF(&grid_view_read_only), 1);
    hcw_grid_view_set_palette(EGUI_VIEW_OF(&grid_view_read_only), HCW_COLOR_PANEL, HCW_COLOR_PANEL, HCW_COLOR_BORDER_STRONG,
                              HCW_COLOR_TEXT, HCW_COLOR_TEXT_SOFT, HCW_COLOR_TEXT_SOFT, HCW_COLOR_TEXT_SOFT,
                              HCW_COLOR_TEXT_SOFT, HCW_COLOR_BORDER_STRONG);
    hcw_grid_view_override_static_preview_api(EGUI_VIEW_OF(&grid_view_read_only), &grid_view_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&grid_view_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&grid_view_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&grid_view_primary));
#endif
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
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&grid_view_primary));
#endif
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_VIEW_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_VIEW_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&grid_view_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_VIEW_RECORD_FINAL_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_VIEW_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
