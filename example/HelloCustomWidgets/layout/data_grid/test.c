#include "egui.h"
#include "egui_view_data_grid.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define DATA_GRID_ROOT_WIDTH        224
#define DATA_GRID_ROOT_HEIGHT       248
#define DATA_GRID_PRIMARY_WIDTH     196
#define DATA_GRID_PRIMARY_HEIGHT    126
#define DATA_GRID_PREVIEW_WIDTH     104
#define DATA_GRID_PREVIEW_HEIGHT    80
#define DATA_GRID_BOTTOM_ROW_WIDTH  216
#define DATA_GRID_BOTTOM_ROW_HEIGHT 80
#define DATA_GRID_RECORD_WAIT       90
#define DATA_GRID_RECORD_FRAME_WAIT 180
#define DATA_GRID_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_data_grid_t grid_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_data_grid_t grid_compact;
static egui_view_data_grid_t grid_read_only;
static egui_view_api_t grid_compact_api;
static egui_view_api_t grid_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Data Grid";

static const egui_view_data_grid_column_t primary_columns[] = {
        {"CASE", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"OWNER", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"STATE", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"ETA", EGUI_VIEW_DATA_GRID_ALIGN_RIGHT},
};

static const egui_view_data_grid_row_t primary_rows_0[] = {
        {{"AL-42", "Rina", "Ready", "Today"}, EGUI_VIEW_DATA_GRID_TONE_ACCENT, 1},
        {{"BK-14", "Omar", "Hold", "Tue"}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 0},
        {{"CK-07", "Lea", "Review", "Fri"}, EGUI_VIEW_DATA_GRID_TONE_NEUTRAL, 0},
        {{"DX-88", "Nia", "Done", "Live"}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 1},
};

static const egui_view_data_grid_row_t primary_rows_1[] = {
        {{"QA-10", "Ava", "Queued", "08:30"}, EGUI_VIEW_DATA_GRID_TONE_NEUTRAL, 0},
        {{"QA-14", "Moe", "Run", "09:10"}, EGUI_VIEW_DATA_GRID_TONE_ACCENT, 1},
        {{"QA-18", "Ivy", "Check", "11:00"}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 0},
        {{"QA-21", "Noa", "Done", "12:40"}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 1},
};

static const egui_view_data_grid_row_t primary_rows_2[] = {
        {{"REL-7", "June", "Open", "Wed"}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 1},
        {{"REL-9", "Kai", "Block", "Thu"}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 1},
        {{"REL-12", "Mia", "Watch", "Fri"}, EGUI_VIEW_DATA_GRID_TONE_NEUTRAL, 0},
        {{"REL-16", "Oli", "Done", "Live"}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 1},
};

static const egui_view_data_grid_snapshot_t primary_snapshots[] = {
        {"OPS", "Rollout board", "Selected row stays inside one grid shell.", "4 rows share one selection target.", primary_columns, primary_rows_0, 4, 4, 1},
        {"QA", "Audit board", "Header and rows still keep the same table shell.", "QA rows still use the same row activation path.", primary_columns,
         primary_rows_1, 4, 4, 2},
        {"WARN", "Release board", "Warning rows keep the selection strip but stay low-noise.", "Blocked rows still render in the same grid frame.",
         primary_columns, primary_rows_2, 4, 4, 0},
};

static const egui_view_data_grid_column_t compact_columns[] = {
        {"CASE", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"STATE", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"ETA", EGUI_VIEW_DATA_GRID_ALIGN_RIGHT},
};

static const egui_view_data_grid_row_t compact_rows_0[] = {
        {{"A-42", "Ready", "Today", NULL}, EGUI_VIEW_DATA_GRID_TONE_ACCENT, 1},
        {{"B-14", "Hold", "Tue", NULL}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 0},
        {{"C-07", "Done", "Fri", NULL}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 0},
};

static const egui_view_data_grid_snapshot_t compact_snapshot = {
        "OPS", "Compact grid", "", "", compact_columns, compact_rows_0, 3, 3, 0};

static const egui_view_data_grid_row_t read_only_rows[] = {
        {{"CK-2", "Review", "Today", NULL}, EGUI_VIEW_DATA_GRID_TONE_NEUTRAL, 0},
        {{"CK-6", "Hold", "Thu", NULL}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 1},
        {{"CK-9", "Done", "Live", NULL}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 0},
};

static const egui_view_data_grid_snapshot_t read_only_snapshot = {
        "READ", "Read only grid", "", "", compact_columns, read_only_rows, 3, 3, 1};

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

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_data_grid_set_current_snapshot(EGUI_VIEW_OF(&grid_primary), index % PRIMARY_SNAPSHOT_COUNT);
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
    egui_view_data_grid_set_current_snapshot(EGUI_VIEW_OF(&grid_compact), 0);
    egui_view_data_grid_set_compact_mode(EGUI_VIEW_OF(&grid_compact), 1);

    egui_view_data_grid_set_current_snapshot(EGUI_VIEW_OF(&grid_read_only), 0);
    egui_view_data_grid_set_compact_mode(EGUI_VIEW_OF(&grid_read_only), 1);
    egui_view_data_grid_set_read_only_mode(EGUI_VIEW_OF(&grid_read_only), 1);
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
#endif

void test_init_ui(void)
{
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), DATA_GRID_ROOT_WIDTH, DATA_GRID_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), DATA_GRID_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_data_grid_init(EGUI_VIEW_OF(&grid_primary));
    egui_view_set_size(EGUI_VIEW_OF(&grid_primary), DATA_GRID_PRIMARY_WIDTH, DATA_GRID_PRIMARY_HEIGHT);
    egui_view_data_grid_set_snapshots(EGUI_VIEW_OF(&grid_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_data_grid_set_font(EGUI_VIEW_OF(&grid_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_data_grid_set_meta_font(EGUI_VIEW_OF(&grid_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_set_margin(EGUI_VIEW_OF(&grid_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&grid_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), DATA_GRID_BOTTOM_ROW_WIDTH, DATA_GRID_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_data_grid_init(EGUI_VIEW_OF(&grid_compact));
    egui_view_set_size(EGUI_VIEW_OF(&grid_compact), DATA_GRID_PREVIEW_WIDTH, DATA_GRID_PREVIEW_HEIGHT);
    egui_view_data_grid_set_snapshots(EGUI_VIEW_OF(&grid_compact), &compact_snapshot, 1);
    egui_view_data_grid_set_font(EGUI_VIEW_OF(&grid_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_data_grid_set_meta_font(EGUI_VIEW_OF(&grid_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_data_grid_set_compact_mode(EGUI_VIEW_OF(&grid_compact), 1);
    egui_view_data_grid_override_static_preview_api(EGUI_VIEW_OF(&grid_compact), &grid_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&grid_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&grid_compact));

    egui_view_data_grid_init(EGUI_VIEW_OF(&grid_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&grid_read_only), DATA_GRID_PREVIEW_WIDTH, DATA_GRID_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&grid_read_only), 8, 0, 0, 0);
    egui_view_data_grid_set_snapshots(EGUI_VIEW_OF(&grid_read_only), &read_only_snapshot, 1);
    egui_view_data_grid_set_font(EGUI_VIEW_OF(&grid_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_data_grid_set_meta_font(EGUI_VIEW_OF(&grid_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_data_grid_set_compact_mode(EGUI_VIEW_OF(&grid_read_only), 1);
    egui_view_data_grid_set_read_only_mode(EGUI_VIEW_OF(&grid_read_only), 1);
    egui_view_data_grid_set_palette(EGUI_VIEW_OF(&grid_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF8FAFB), EGUI_COLOR_HEX(0xD8DFE6),
                                    EGUI_COLOR_HEX(0x233241), EGUI_COLOR_HEX(0x708091), EGUI_COLOR_HEX(0x98A5B2), EGUI_COLOR_HEX(0xA7B4BF),
                                    EGUI_COLOR_HEX(0xB8B0A2), EGUI_COLOR_HEX(0xB4BDC8));
    egui_view_data_grid_override_static_preview_api(EGUI_VIEW_OF(&grid_read_only), &grid_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&grid_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&grid_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&grid_primary));
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
            egui_view_request_focus(EGUI_VIEW_OF(&grid_primary));
#endif
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_GRID_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_GRID_RECORD_FINAL_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_GRID_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_GRID_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_GRID_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&grid_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_GRID_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, DATA_GRID_RECORD_FINAL_WAIT);
        return true;
    default:
        break;
    }

    return false;
}
#endif
