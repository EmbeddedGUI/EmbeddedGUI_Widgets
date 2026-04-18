#include "egui.h"
#include "egui_view_scroll_viewer.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SCROLL_VIEWER_ROOT_WIDTH        224
#define SCROLL_VIEWER_ROOT_HEIGHT       284
#define SCROLL_VIEWER_PRIMARY_WIDTH     196
#define SCROLL_VIEWER_PRIMARY_HEIGHT    160
#define SCROLL_VIEWER_PREVIEW_WIDTH     104
#define SCROLL_VIEWER_PREVIEW_HEIGHT    88
#define SCROLL_VIEWER_BOTTOM_ROW_WIDTH  216
#define SCROLL_VIEWER_BOTTOM_ROW_HEIGHT 88
#define SCROLL_VIEWER_RECORD_WAIT       90
#define SCROLL_VIEWER_RECORD_FRAME_WAIT 170
#define SCROLL_VIEWER_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_scroll_viewer_t scroll_viewer_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_scroll_viewer_t scroll_viewer_compact;
static egui_view_scroll_viewer_t scroll_viewer_read_only;
static egui_view_api_t scroll_viewer_compact_api;
static egui_view_api_t scroll_viewer_read_only_api;
static uint8_t ui_ready;

static void layout_page(void);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Scroll Viewer";

static const egui_view_scroll_viewer_block_t release_blocks[] = {
        {"REL", "Gate checks", "Artifacts and smoke status", 0, 0, 112, 36, EGUI_VIEW_SCROLL_VIEWER_TONE_ACCENT, 1},
        {"OPS", "Deploy lane", "Ops deck and rollback notes", 16, 52, 128, 42, EGUI_VIEW_SCROLL_VIEWER_TONE_SUCCESS, 0},
        {"QA", "Known issues", "Tracked defects and follow-ups", 8, 112, 138, 40, EGUI_VIEW_SCROLL_VIEWER_TONE_WARNING, 0},
        {"DOC", "Footer appendix", "Linked release notes", 18, 176, 118, 34, EGUI_VIEW_SCROLL_VIEWER_TONE_NEUTRAL, 0},
};

static const egui_view_scroll_viewer_block_t diagnostics_blocks[] = {
        {"OPS", "Signal summary", "CPU, memory and IO", 0, 0, 132, 38, EGUI_VIEW_SCROLL_VIEWER_TONE_ACCENT, 0},
        {"QA", "Incident list", "Escalations in the last hour", 14, 56, 144, 42, EGUI_VIEW_SCROLL_VIEWER_TONE_WARNING, 1},
        {"NET", "Edge peers", "Latency and packet health", 26, 116, 118, 38, EGUI_VIEW_SCROLL_VIEWER_TONE_SUCCESS, 0},
        {"DOC", "Drill notes", "Operator checklist appendix", 8, 176, 140, 36, EGUI_VIEW_SCROLL_VIEWER_TONE_NEUTRAL, 0},
};

static const egui_view_scroll_viewer_block_t backlog_blocks[] = {
        {"UI", "Compact feed", "Grouped notes", 0, 0, 102, 32, EGUI_VIEW_SCROLL_VIEWER_TONE_ACCENT, 0},
        {"PM", "Decision log", "Pinned actions", 10, 46, 112, 34, EGUI_VIEW_SCROLL_VIEWER_TONE_NEUTRAL, 1},
        {"ENG", "Follow-ups", "Deferred tasks", 22, 92, 108, 32, EGUI_VIEW_SCROLL_VIEWER_TONE_SUCCESS, 0},
};

static const egui_view_scroll_viewer_snapshot_t primary_snapshots[] = {
        {"REL", "Release pane", "Viewport metrics stay visible while the content surface scrolls.", "Primary scroll lane", "Release notes",
         release_blocks, 4, 184, 236, 132, 144, 18, 0, 1},
        {"OPS", "Diagnostics lane", "The thumb exposes a second reference state without turning into a data grid.", "Telemetry focus",
         "Incident review", diagnostics_blocks, 4, 196, 248, 132, 144, 72, 12, 1},
        {"PM", "Backlog feed", "Horizontal offset nudges the card stack to show dense content on compact surfaces.", "Feed snapshot", "Weekly archive",
         backlog_blocks, 3, 170, 160, 120, 120, 26, 10, 1},
};

static const egui_view_scroll_viewer_snapshot_t compact_snapshot = {
        "UI", "Compact pane", "", "", "Compact", backlog_blocks, 3, 154, 154, 102, 82, 18, 8, 1};

static const egui_view_scroll_viewer_snapshot_t read_only_snapshot = {
        "LOCK", "Read only pane", "", "", "Static preview", release_blocks, 3, 176, 198, 102, 82, 32, 0, 1};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_scroll_viewer_set_current_snapshot(EGUI_VIEW_OF(&scroll_viewer_primary), index % PRIMARY_SNAPSHOT_COUNT);
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
    egui_view_scroll_viewer_set_current_snapshot(EGUI_VIEW_OF(&scroll_viewer_compact), 0);
    egui_view_scroll_viewer_set_current_snapshot(EGUI_VIEW_OF(&scroll_viewer_read_only), 0);
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SCROLL_VIEWER_ROOT_WIDTH, SCROLL_VIEWER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SCROLL_VIEWER_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_scroll_viewer_init(EGUI_VIEW_OF(&scroll_viewer_primary));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_viewer_primary), SCROLL_VIEWER_PRIMARY_WIDTH, SCROLL_VIEWER_PRIMARY_HEIGHT);
    egui_view_scroll_viewer_set_snapshots(EGUI_VIEW_OF(&scroll_viewer_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_scroll_viewer_set_font(EGUI_VIEW_OF(&scroll_viewer_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_scroll_viewer_set_meta_font(EGUI_VIEW_OF(&scroll_viewer_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_viewer_set_palette(EGUI_VIEW_OF(&scroll_viewer_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD4DDE6),
                                        EGUI_COLOR_HEX(0xF8FBFD), EGUI_COLOR_HEX(0x1B2834), EGUI_COLOR_HEX(0x6D7C8A), EGUI_COLOR_HEX(0x0F6CBD),
                                        EGUI_COLOR_HEX(0x6AA8FF));
    egui_view_set_margin(EGUI_VIEW_OF(&scroll_viewer_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&scroll_viewer_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SCROLL_VIEWER_BOTTOM_ROW_WIDTH, SCROLL_VIEWER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_scroll_viewer_init(EGUI_VIEW_OF(&scroll_viewer_compact));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_viewer_compact), SCROLL_VIEWER_PREVIEW_WIDTH, SCROLL_VIEWER_PREVIEW_HEIGHT);
    egui_view_scroll_viewer_set_snapshots(EGUI_VIEW_OF(&scroll_viewer_compact), &compact_snapshot, 1);
    egui_view_scroll_viewer_set_font(EGUI_VIEW_OF(&scroll_viewer_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_viewer_set_meta_font(EGUI_VIEW_OF(&scroll_viewer_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_viewer_set_compact_mode(EGUI_VIEW_OF(&scroll_viewer_compact), 1);
    egui_view_scroll_viewer_set_palette(EGUI_VIEW_OF(&scroll_viewer_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD3E1DC),
                                         EGUI_COLOR_HEX(0xF7FBFA), EGUI_COLOR_HEX(0x183332), EGUI_COLOR_HEX(0x5E7B76), EGUI_COLOR_HEX(0x0D9488),
                                         EGUI_COLOR_HEX(0x67D4C6));
    egui_view_scroll_viewer_override_static_preview_api(EGUI_VIEW_OF(&scroll_viewer_compact), &scroll_viewer_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&scroll_viewer_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&scroll_viewer_compact));

    egui_view_scroll_viewer_init(EGUI_VIEW_OF(&scroll_viewer_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_viewer_read_only), SCROLL_VIEWER_PREVIEW_WIDTH, SCROLL_VIEWER_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&scroll_viewer_read_only), 8, 0, 0, 0);
    egui_view_scroll_viewer_set_snapshots(EGUI_VIEW_OF(&scroll_viewer_read_only), &read_only_snapshot, 1);
    egui_view_scroll_viewer_set_font(EGUI_VIEW_OF(&scroll_viewer_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_viewer_set_meta_font(EGUI_VIEW_OF(&scroll_viewer_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_viewer_set_compact_mode(EGUI_VIEW_OF(&scroll_viewer_read_only), 1);
    egui_view_scroll_viewer_set_read_only_mode(EGUI_VIEW_OF(&scroll_viewer_read_only), 1);
    egui_view_scroll_viewer_set_palette(EGUI_VIEW_OF(&scroll_viewer_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD9E1E8),
                                         EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0x556575), EGUI_COLOR_HEX(0x8997A4), EGUI_COLOR_HEX(0xA3B2BE),
                                         EGUI_COLOR_HEX(0xC3D1DE));
    egui_view_scroll_viewer_override_static_preview_api(EGUI_VIEW_OF(&scroll_viewer_read_only), &scroll_viewer_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&scroll_viewer_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&scroll_viewer_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&scroll_viewer_primary));
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
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_VIEWER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_VIEWER_RECORD_FINAL_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_VIEWER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_VIEWER_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_VIEWER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_VIEWER_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_VIEWER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
