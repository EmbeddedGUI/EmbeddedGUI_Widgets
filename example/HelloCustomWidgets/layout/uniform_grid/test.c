#include "egui.h"
#include "egui_view_uniform_grid.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define UNIFORM_GRID_ROOT_WIDTH        224
#define UNIFORM_GRID_ROOT_HEIGHT       244
#define UNIFORM_GRID_PRIMARY_WIDTH     196
#define UNIFORM_GRID_PRIMARY_HEIGHT    124
#define UNIFORM_GRID_PREVIEW_WIDTH     104
#define UNIFORM_GRID_PREVIEW_HEIGHT    78
#define UNIFORM_GRID_BOTTOM_ROW_WIDTH  216
#define UNIFORM_GRID_BOTTOM_ROW_HEIGHT 78
#define UNIFORM_GRID_RECORD_WAIT       90
#define UNIFORM_GRID_RECORD_FRAME_WAIT 180

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))
#define COMPACT_SNAPSHOT_COUNT ((uint8_t)(sizeof(compact_snapshots) / sizeof(compact_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_uniform_grid_t grid_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_uniform_grid_t grid_compact;
static egui_view_uniform_grid_t grid_read_only;
static egui_view_api_t grid_compact_api;
static egui_view_api_t grid_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Uniform Grid";

static const egui_view_uniform_grid_cell_t primary_cells_0[] = {
        {"OPS", "Deploy", "2 min", "Queue", EGUI_VIEW_UNIFORM_GRID_TONE_ACCENT, 1},
        {"QA", "Audit", "5 runs", "Check", EGUI_VIEW_UNIFORM_GRID_TONE_WARNING, 0},
        {"REL", "Publish", "3 files", "Ready", EGUI_VIEW_UNIFORM_GRID_TONE_SUCCESS, 1},
        {"DOC", "Notes", "4 items", "Draft", EGUI_VIEW_UNIFORM_GRID_TONE_NEUTRAL, 0},
        {"NAV", "Routes", "12 views", "Map", EGUI_VIEW_UNIFORM_GRID_TONE_ACCENT, 0},
        {"OBS", "Health", "98%", "Live", EGUI_VIEW_UNIFORM_GRID_TONE_SUCCESS, 1},
};

static const egui_view_uniform_grid_cell_t primary_cells_1[] = {
        {"REL", "Launch", "Checklist", "Green", EGUI_VIEW_UNIFORM_GRID_TONE_SUCCESS, 1},
        {"APP", "Approvals", "3 owners", "Hold", EGUI_VIEW_UNIFORM_GRID_TONE_WARNING, 1},
        {"CHK", "Signals", "5 feeds", "Open", EGUI_VIEW_UNIFORM_GRID_TONE_NEUTRAL, 0},
        {"OPS", "Fallback", "1 path", "Ready", EGUI_VIEW_UNIFORM_GRID_TONE_ACCENT, 0},
};

static const egui_view_uniform_grid_cell_t primary_cells_2[] = {
        {"UI", "Compact", "8px", "Dense", EGUI_VIEW_UNIFORM_GRID_TONE_ACCENT, 1},
        {"LOG", "Review", "6 logs", "Trace", EGUI_VIEW_UNIFORM_GRID_TONE_NEUTRAL, 0},
        {"SEC", "Policy", "2 rules", "Guard", EGUI_VIEW_UNIFORM_GRID_TONE_WARNING, 0},
        {"CD", "Stages", "4 jobs", "Run", EGUI_VIEW_UNIFORM_GRID_TONE_SUCCESS, 1},
        {"OPS", "Tiles", "3 cols", "Grid", EGUI_VIEW_UNIFORM_GRID_TONE_ACCENT, 0},
        {"DOC", "Proof", "1 pack", "Done", EGUI_VIEW_UNIFORM_GRID_TONE_SUCCESS, 0},
};

static const egui_view_uniform_grid_snapshot_t primary_snapshots[] = {
        {"OPS", "Operations wall", "Equal tiles keep the launch surface stable.", "6 tiles / 3 columns", primary_cells_0, 6, 3, 1},
        {"REL", "Release wall", "A wider 2-column layout keeps key actions readable.", "4 tiles / 2 columns", primary_cells_1, 4, 2, 2},
        {"GRID", "Density wall", "Snapshot changes can still keep the same tile shell.", "6 tiles / 3 columns", primary_cells_2, 6, 3, 0},
};

static const egui_view_uniform_grid_cell_t compact_cells_0[] = {
        {"A", "Deploy", "2m", "", EGUI_VIEW_UNIFORM_GRID_TONE_ACCENT, 1},
        {"B", "Audit", "5x", "", EGUI_VIEW_UNIFORM_GRID_TONE_WARNING, 0},
        {"C", "Publish", "3f", "", EGUI_VIEW_UNIFORM_GRID_TONE_SUCCESS, 0},
        {"D", "Notes", "4i", "", EGUI_VIEW_UNIFORM_GRID_TONE_NEUTRAL, 0},
};

static const egui_view_uniform_grid_cell_t compact_cells_1[] = {
        {"Q", "Queue", "4", "", EGUI_VIEW_UNIFORM_GRID_TONE_NEUTRAL, 0},
        {"R", "Review", "1", "", EGUI_VIEW_UNIFORM_GRID_TONE_WARNING, 1},
        {"S", "Ship", "2", "", EGUI_VIEW_UNIFORM_GRID_TONE_SUCCESS, 0},
        {"T", "Track", "7", "", EGUI_VIEW_UNIFORM_GRID_TONE_ACCENT, 0},
};

static const egui_view_uniform_grid_snapshot_t compact_snapshots[] = {
        {"OPS", "Compact grid", "", "", compact_cells_0, 4, 2, 0},
        {"QA", "Compact audit", "", "", compact_cells_1, 4, 2, 1},
};

static const egui_view_uniform_grid_cell_t read_only_cells[] = {
        {"L1", "Locked", "View", "", EGUI_VIEW_UNIFORM_GRID_TONE_NEUTRAL, 0},
        {"L2", "Review", "Mute", "", EGUI_VIEW_UNIFORM_GRID_TONE_WARNING, 1},
        {"L3", "Done", "Pass", "", EGUI_VIEW_UNIFORM_GRID_TONE_SUCCESS, 0},
        {"L4", "Queue", "Idle", "", EGUI_VIEW_UNIFORM_GRID_TONE_ACCENT, 0},
};

static const egui_view_uniform_grid_snapshot_t read_only_snapshot = {
        "LOCK", "Read only grid", "", "", read_only_cells, 4, 2, 1};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_uniform_grid_set_current_snapshot(EGUI_VIEW_OF(&grid_primary), index % PRIMARY_SNAPSHOT_COUNT);
}

static void apply_compact_snapshot(uint8_t index)
{
    egui_view_uniform_grid_set_current_snapshot(EGUI_VIEW_OF(&grid_compact), index % COMPACT_SNAPSHOT_COUNT);
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        egui_view_clear_focus(EGUI_VIEW_OF(&grid_primary));
    }
#else
    EGUI_UNUSED(event);
#endif
    return 1;
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&grid_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&grid_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&grid_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&grid_primary), &event);
}

static void request_page_snapshot(void)
{
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}

static void set_click_view_center(egui_sim_action_t *p_action, egui_view_t *view, int interval_ms)
{
    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = view->region_screen.location.x + view->region_screen.size.width / 2;
    p_action->y1 = view->region_screen.location.y + view->region_screen.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}

static void set_click_cell(egui_sim_action_t *p_action, egui_view_t *view, uint8_t cell_index, int interval_ms)
{
    egui_region_t region;

    if (!egui_view_uniform_grid_get_cell_region(view, cell_index, &region))
    {
        EGUI_SIM_SET_WAIT(p_action, interval_ms);
        return;
    }

    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = region.location.x + region.size.width / 2;
    p_action->y1 = region.location.y + region.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), UNIFORM_GRID_ROOT_WIDTH, UNIFORM_GRID_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), UNIFORM_GRID_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_uniform_grid_init(EGUI_VIEW_OF(&grid_primary));
    egui_view_set_size(EGUI_VIEW_OF(&grid_primary), UNIFORM_GRID_PRIMARY_WIDTH, UNIFORM_GRID_PRIMARY_HEIGHT);
    egui_view_uniform_grid_set_snapshots(EGUI_VIEW_OF(&grid_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_uniform_grid_set_font(EGUI_VIEW_OF(&grid_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_uniform_grid_set_meta_font(EGUI_VIEW_OF(&grid_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_uniform_grid_set_palette(EGUI_VIEW_OF(&grid_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF4F7FA), EGUI_COLOR_HEX(0xD6DEE6),
                                       EGUI_COLOR_HEX(0x1B2834), EGUI_COLOR_HEX(0x6D7C8A), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x127A43),
                                       EGUI_COLOR_HEX(0xA15D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&grid_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&grid_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), UNIFORM_GRID_BOTTOM_ROW_WIDTH, UNIFORM_GRID_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_uniform_grid_init(EGUI_VIEW_OF(&grid_compact));
    egui_view_set_size(EGUI_VIEW_OF(&grid_compact), UNIFORM_GRID_PREVIEW_WIDTH, UNIFORM_GRID_PREVIEW_HEIGHT);
    egui_view_uniform_grid_set_snapshots(EGUI_VIEW_OF(&grid_compact), compact_snapshots, COMPACT_SNAPSHOT_COUNT);
    egui_view_uniform_grid_set_font(EGUI_VIEW_OF(&grid_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_uniform_grid_set_meta_font(EGUI_VIEW_OF(&grid_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_uniform_grid_set_compact_mode(EGUI_VIEW_OF(&grid_compact), 1);
    egui_view_uniform_grid_set_palette(EGUI_VIEW_OF(&grid_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF3FBF8), EGUI_COLOR_HEX(0xD0E3DE),
                                       EGUI_COLOR_HEX(0x18312F), EGUI_COLOR_HEX(0x5B7A73), EGUI_COLOR_HEX(0x0D9488), EGUI_COLOR_HEX(0x0B8454),
                                       EGUI_COLOR_HEX(0x957000), EGUI_COLOR_HEX(0x6D7F8D));
    egui_view_uniform_grid_override_static_preview_api(EGUI_VIEW_OF(&grid_compact), &grid_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    grid_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&grid_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&grid_compact));

    egui_view_uniform_grid_init(EGUI_VIEW_OF(&grid_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&grid_read_only), UNIFORM_GRID_PREVIEW_WIDTH, UNIFORM_GRID_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&grid_read_only), 8, 0, 0, 0);
    egui_view_uniform_grid_set_snapshots(EGUI_VIEW_OF(&grid_read_only), &read_only_snapshot, 1);
    egui_view_uniform_grid_set_font(EGUI_VIEW_OF(&grid_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_uniform_grid_set_meta_font(EGUI_VIEW_OF(&grid_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_uniform_grid_set_compact_mode(EGUI_VIEW_OF(&grid_read_only), 1);
    egui_view_uniform_grid_set_read_only_mode(EGUI_VIEW_OF(&grid_read_only), 1);
    egui_view_uniform_grid_set_palette(EGUI_VIEW_OF(&grid_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD9E1E8),
                                       EGUI_COLOR_HEX(0x4F5E6D), EGUI_COLOR_HEX(0x8895A1), EGUI_COLOR_HEX(0xA1AEBA), EGUI_COLOR_HEX(0xA9B8C0),
                                       EGUI_COLOR_HEX(0xB9B0A6), EGUI_COLOR_HEX(0xB0BAC6));
    egui_view_uniform_grid_override_static_preview_api(EGUI_VIEW_OF(&grid_read_only), &grid_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    grid_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&grid_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&grid_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
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
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&grid_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_FRAME_WAIT);
        return true;
    case 8:
        set_click_cell(p_action, EGUI_VIEW_OF(&grid_primary), 3, 220);
        return true;
    case 9:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_FRAME_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_WAIT);
        return true;
    case 11:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, UNIFORM_GRID_RECORD_FRAME_WAIT);
        return true;
    case 12:
        set_click_view_center(p_action, EGUI_VIEW_OF(&grid_compact), 220);
        return true;
    case 13:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
