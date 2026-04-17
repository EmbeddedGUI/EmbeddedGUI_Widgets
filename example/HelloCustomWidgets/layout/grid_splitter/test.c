#include "egui.h"
#include "egui_view_grid_splitter.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define GRID_SPLITTER_ROOT_WIDTH        224
#define GRID_SPLITTER_ROOT_HEIGHT       236
#define GRID_SPLITTER_PRIMARY_WIDTH     196
#define GRID_SPLITTER_PRIMARY_HEIGHT    118
#define GRID_SPLITTER_PREVIEW_WIDTH     104
#define GRID_SPLITTER_PREVIEW_HEIGHT    74
#define GRID_SPLITTER_BOTTOM_ROW_WIDTH  216
#define GRID_SPLITTER_BOTTOM_ROW_HEIGHT 74
#define GRID_SPLITTER_RECORD_WAIT       90
#define GRID_SPLITTER_RECORD_FRAME_WAIT 170
#define GRID_SPLITTER_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_grid_splitter_t splitter_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_grid_splitter_t splitter_compact;
static egui_view_grid_splitter_t splitter_read_only;
static egui_view_api_t splitter_compact_api;
static egui_view_api_t splitter_read_only_api;
static uint8_t ui_ready;

static void layout_page(void);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Grid Splitter";

static const egui_view_grid_splitter_snapshot_t primary_snapshots[] = {
        {"LAYOUT", "Canvas split", "Drag the rail to rebalance navigation and detail panes.", "Layers", "12 groups", "Pinned tools keep the left rail readable.",
         "Canvas", "Zoom 100%", "Inspector content stays aligned while the rail moves.", "42% left / 58% right", 42,
         EGUI_VIEW_GRID_SPLITTER_EMPHASIS_RIGHT},
        {"REVIEW", "Audit split", "Wider left content keeps issue lists visible before inspection.", "Review queue", "18 items",
         "Priority rows stay anchored near the handle.", "Inspector", "Ready", "Approvals remain stable on the right pane.", "58% left / 42% right", 58,
         EGUI_VIEW_GRID_SPLITTER_EMPHASIS_LEFT},
        {"DETAIL", "Inspector split", "A narrow left rail leaves room for a denser details surface.", "Outline", "5 sections",
         "Selection stays compact when detail needs more room.", "Properties", "Editable", "Right pane keeps the long-form layout payload.", "33% left / 67% right",
         33, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_RIGHT},
};

static const egui_view_grid_splitter_snapshot_t compact_snapshot = {
        "UI", "Compact split", "", "List", "6", "Left rail stays visible.", "Preview", "1:1", "Right pane keeps the detail block.", "", 46,
        EGUI_VIEW_GRID_SPLITTER_EMPHASIS_RIGHT};

static const egui_view_grid_splitter_snapshot_t read_only_snapshot = {
        "LOCK", "Read only split", "", "Library", "8", "Left rail stays fixed in review.", "Properties", "View", "Drag affordance is muted in static preview.", "",
        48, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_NONE};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_grid_splitter_set_current_snapshot(EGUI_VIEW_OF(&splitter_primary), index % PRIMARY_SNAPSHOT_COUNT);
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
    egui_view_grid_splitter_set_current_snapshot(EGUI_VIEW_OF(&splitter_compact), 0);
    egui_view_grid_splitter_set_current_snapshot(EGUI_VIEW_OF(&splitter_read_only), 0);
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
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&splitter_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&splitter_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&splitter_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&splitter_primary), &event);
}

static void request_page_snapshot(void)
{
    layout_page();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}

static void set_drag_primary_handle(egui_sim_action_t *p_action, egui_view_t *view, egui_dim_t target_x, int interval_ms)
{
    egui_region_t handle_region;

    if (!egui_view_grid_splitter_get_handle_region(view, &handle_region))
    {
        EGUI_SIM_SET_WAIT(p_action, interval_ms);
        return;
    }

    p_action->type = EGUI_SIM_ACTION_DRAG;
    p_action->x1 = handle_region.location.x + handle_region.size.width / 2;
    p_action->y1 = handle_region.location.y + handle_region.size.height / 2;
    p_action->x2 = target_x;
    p_action->y2 = p_action->y1;
    p_action->steps = 8;
    p_action->interval_ms = interval_ms;
}
#endif

void test_init_ui(void)
{
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), GRID_SPLITTER_ROOT_WIDTH, GRID_SPLITTER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), GRID_SPLITTER_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_grid_splitter_init(EGUI_VIEW_OF(&splitter_primary));
    egui_view_set_size(EGUI_VIEW_OF(&splitter_primary), GRID_SPLITTER_PRIMARY_WIDTH, GRID_SPLITTER_PRIMARY_HEIGHT);
    egui_view_grid_splitter_set_snapshots(EGUI_VIEW_OF(&splitter_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_grid_splitter_set_font(EGUI_VIEW_OF(&splitter_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_grid_splitter_set_meta_font(EGUI_VIEW_OF(&splitter_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_grid_splitter_set_palette(EGUI_VIEW_OF(&splitter_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF5F8FA), EGUI_COLOR_HEX(0xD4DDE6),
                                        EGUI_COLOR_HEX(0x1B2834), EGUI_COLOR_HEX(0x6D7C8A), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_set_margin(EGUI_VIEW_OF(&splitter_primary), 0, 0, 0, 8);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&splitter_primary), true);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&splitter_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), GRID_SPLITTER_BOTTOM_ROW_WIDTH, GRID_SPLITTER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_grid_splitter_init(EGUI_VIEW_OF(&splitter_compact));
    egui_view_set_size(EGUI_VIEW_OF(&splitter_compact), GRID_SPLITTER_PREVIEW_WIDTH, GRID_SPLITTER_PREVIEW_HEIGHT);
    egui_view_grid_splitter_set_snapshots(EGUI_VIEW_OF(&splitter_compact), &compact_snapshot, 1);
    egui_view_grid_splitter_set_font(EGUI_VIEW_OF(&splitter_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_grid_splitter_set_meta_font(EGUI_VIEW_OF(&splitter_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_grid_splitter_set_compact_mode(EGUI_VIEW_OF(&splitter_compact), 1);
    egui_view_grid_splitter_set_palette(EGUI_VIEW_OF(&splitter_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF3FBF9), EGUI_COLOR_HEX(0xCCE4DE),
                                        EGUI_COLOR_HEX(0x19332F), EGUI_COLOR_HEX(0x5E7E77), EGUI_COLOR_HEX(0x0D9488));
    egui_view_grid_splitter_override_static_preview_api(EGUI_VIEW_OF(&splitter_compact), &splitter_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&splitter_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&splitter_compact));

    egui_view_grid_splitter_init(EGUI_VIEW_OF(&splitter_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&splitter_read_only), GRID_SPLITTER_PREVIEW_WIDTH, GRID_SPLITTER_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&splitter_read_only), 8, 0, 0, 0);
    egui_view_grid_splitter_set_snapshots(EGUI_VIEW_OF(&splitter_read_only), &read_only_snapshot, 1);
    egui_view_grid_splitter_set_font(EGUI_VIEW_OF(&splitter_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_grid_splitter_set_meta_font(EGUI_VIEW_OF(&splitter_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_grid_splitter_set_compact_mode(EGUI_VIEW_OF(&splitter_read_only), 1);
    egui_view_grid_splitter_set_read_only_mode(EGUI_VIEW_OF(&splitter_read_only), 1);
    egui_view_grid_splitter_set_palette(EGUI_VIEW_OF(&splitter_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF4F7FA), EGUI_COLOR_HEX(0xD8E1E8),
                                        EGUI_COLOR_HEX(0x556574), EGUI_COLOR_HEX(0x8B99A6), EGUI_COLOR_HEX(0xA5B2BE));
    egui_view_grid_splitter_override_static_preview_api(EGUI_VIEW_OF(&splitter_read_only), &splitter_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&splitter_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&splitter_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&splitter_primary));
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
            egui_view_request_focus(EGUI_VIEW_OF(&splitter_primary));
#endif
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_RIGHT);
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            set_drag_primary_handle(p_action, EGUI_VIEW_OF(&splitter_primary),
                                    EGUI_VIEW_OF(&splitter_primary)->region_screen.location.x +
                                            EGUI_VIEW_OF(&splitter_primary)->region_screen.size.width - 34,
                                    220);
        }
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_FRAME_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&splitter_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, GRID_SPLITTER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
