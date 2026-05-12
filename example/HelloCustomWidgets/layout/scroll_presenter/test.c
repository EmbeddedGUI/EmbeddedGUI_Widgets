#include "egui.h"
#include "egui_view_scroll_presenter.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SCROLL_PRESENTER_ROOT_WIDTH        224
#define SCROLL_PRESENTER_ROOT_HEIGHT       284
#define SCROLL_PRESENTER_PRIMARY_WIDTH     196
#define SCROLL_PRESENTER_PRIMARY_HEIGHT    160
#define SCROLL_PRESENTER_PREVIEW_WIDTH     104
#define SCROLL_PRESENTER_PREVIEW_HEIGHT    88
#define SCROLL_PRESENTER_BOTTOM_ROW_WIDTH  216
#define SCROLL_PRESENTER_BOTTOM_ROW_HEIGHT 88
#define SCROLL_PRESENTER_RECORD_WAIT       90
#define SCROLL_PRESENTER_RECORD_FRAME_WAIT 170
#define SCROLL_PRESENTER_RECORD_FINAL_WAIT 280

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_scroll_presenter_t scroll_presenter_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_scroll_presenter_t scroll_presenter_compact;
static egui_view_scroll_presenter_t scroll_presenter_read_only;
static egui_view_api_t scroll_presenter_compact_api;
static egui_view_api_t scroll_presenter_read_only_api;
static uint8_t ui_ready;

static void layout_page(void);

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PAGE_BG, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Scroll Presenter";

static const egui_view_scroll_presenter_item_t canvas_items[] = {
        {"MAP", "Origin card", "Primary viewport entry", 0, 0, 118, 48, EGUI_VIEW_SCROLL_PRESENTER_TONE_ACCENT, 1},
        {"OPS", "Signal lane", "Shift summary cluster", 92, 58, 126, 52, EGUI_VIEW_SCROLL_PRESENTER_TONE_SUCCESS, 0},
        {"QA", "Alert branch", "Drill notes and checks", 34, 132, 134, 50, EGUI_VIEW_SCROLL_PRESENTER_TONE_WARNING, 0},
        {"DOC", "Far corner", "Pinned appendix route", 154, 184, 116, 48, EGUI_VIEW_SCROLL_PRESENTER_TONE_NEUTRAL, 0},
};

static const egui_view_scroll_presenter_item_t timeline_items[] = {
        {"REL", "Sprint map", "Milestone stream", 0, 0, 112, 46, EGUI_VIEW_SCROLL_PRESENTER_TONE_ACCENT, 0},
        {"UX", "Review branch", "Linked notes", 138, 22, 118, 48, EGUI_VIEW_SCROLL_PRESENTER_TONE_NEUTRAL, 1},
        {"ENG", "Build lane", "Validation queue", 60, 98, 130, 52, EGUI_VIEW_SCROLL_PRESENTER_TONE_SUCCESS, 0},
        {"OPS", "Rollback shelf", "Fallback deck", 176, 156, 108, 46, EGUI_VIEW_SCROLL_PRESENTER_TONE_WARNING, 0},
};

static const egui_view_scroll_presenter_item_t preview_items[] = {
        {"UI", "Mini map", "Preview route", 0, 0, 92, 42, EGUI_VIEW_SCROLL_PRESENTER_TONE_ACCENT, 0},
};

static const egui_view_scroll_presenter_snapshot_t primary_snapshots[] = {
        {"MAP", "Canvas overview", "Presenter keeps the viewport chrome light while preserving pan metrics.", "Origin focus", "Canvas extent",
         canvas_items, 4, 292, 248, 148, 144, 0, 0},
        {"REL", "Timeline branch", "The surface can slide in both axes without exposing a full scrollbar lane.", "Milestone scan", "Branch extent",
         timeline_items, 4, 308, 228, 148, 144, 34, 46},
        {"OPS", "Far corner", "Preset snapshots jump straight to the lower-right canvas cluster.", "Corner target", "Pinned corner", canvas_items, 4, 292,
         248, 148, 144, 88, 122},
};

static const egui_view_scroll_presenter_snapshot_t compact_snapshot = {
        "UI", "Compact pan", "", "", "Compact", preview_items, 1, 108, 96, 108, 96, 0, 0};

static const egui_view_scroll_presenter_snapshot_t read_only_snapshot = {
        "LOCK", "Read only pan", "", "", "Static preview", preview_items, 1, 108, 96, 108, 96, 0, 0};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_scroll_presenter_set_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_primary), index % PRIMARY_SNAPSHOT_COUNT);
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
    egui_view_scroll_presenter_set_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_compact), 0);
    egui_view_scroll_presenter_set_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_read_only), 0);
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
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SCROLL_PRESENTER_ROOT_WIDTH, SCROLL_PRESENTER_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SCROLL_PRESENTER_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), HCW_COLOR_TEXT, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_scroll_presenter_init(EGUI_VIEW_OF(&scroll_presenter_primary));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_presenter_primary), SCROLL_PRESENTER_PRIMARY_WIDTH, SCROLL_PRESENTER_PRIMARY_HEIGHT);
    egui_view_scroll_presenter_set_snapshots(EGUI_VIEW_OF(&scroll_presenter_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_scroll_presenter_set_font(EGUI_VIEW_OF(&scroll_presenter_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_scroll_presenter_set_meta_font(EGUI_VIEW_OF(&scroll_presenter_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_presenter_set_palette(EGUI_VIEW_OF(&scroll_presenter_primary), HCW_COLOR_PANEL, HCW_COLOR_BORDER,
                                           HCW_COLOR_PANEL, HCW_COLOR_TEXT, HCW_COLOR_TEXT_MUTED, HCW_COLOR_PRIMARY,
                                           HCW_COLOR_PRIMARY_LIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&scroll_presenter_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&scroll_presenter_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SCROLL_PRESENTER_BOTTOM_ROW_WIDTH, SCROLL_PRESENTER_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_scroll_presenter_init(EGUI_VIEW_OF(&scroll_presenter_compact));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_presenter_compact), SCROLL_PRESENTER_PREVIEW_WIDTH, SCROLL_PRESENTER_PREVIEW_HEIGHT);
    egui_view_scroll_presenter_set_snapshots(EGUI_VIEW_OF(&scroll_presenter_compact), &compact_snapshot, 1);
    egui_view_scroll_presenter_set_font(EGUI_VIEW_OF(&scroll_presenter_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_presenter_set_meta_font(EGUI_VIEW_OF(&scroll_presenter_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_presenter_set_compact_mode(EGUI_VIEW_OF(&scroll_presenter_compact), 1);
    egui_view_scroll_presenter_set_palette(EGUI_VIEW_OF(&scroll_presenter_compact), HCW_COLOR_PANEL, HCW_COLOR_BORDER,
                                           HCW_COLOR_PANEL, HCW_COLOR_TEXT, HCW_COLOR_TEXT_MUTED, HCW_COLOR_PRIMARY,
                                           HCW_COLOR_PRIMARY_LIGHT);
    egui_view_scroll_presenter_override_static_preview_api(EGUI_VIEW_OF(&scroll_presenter_compact), &scroll_presenter_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&scroll_presenter_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&scroll_presenter_compact));

    egui_view_scroll_presenter_init(EGUI_VIEW_OF(&scroll_presenter_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_presenter_read_only), SCROLL_PRESENTER_PREVIEW_WIDTH, SCROLL_PRESENTER_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&scroll_presenter_read_only), 8, 0, 0, 0);
    egui_view_scroll_presenter_set_snapshots(EGUI_VIEW_OF(&scroll_presenter_read_only), &read_only_snapshot, 1);
    egui_view_scroll_presenter_set_font(EGUI_VIEW_OF(&scroll_presenter_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_presenter_set_meta_font(EGUI_VIEW_OF(&scroll_presenter_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_scroll_presenter_set_compact_mode(EGUI_VIEW_OF(&scroll_presenter_read_only), 1);
    egui_view_scroll_presenter_set_read_only_mode(EGUI_VIEW_OF(&scroll_presenter_read_only), 1);
    egui_view_scroll_presenter_set_palette(EGUI_VIEW_OF(&scroll_presenter_read_only), HCW_COLOR_PANEL, HCW_COLOR_BORDER,
                                           HCW_COLOR_PANEL, HCW_COLOR_TEXT_MUTED, HCW_COLOR_TEXT_SOFT, HCW_COLOR_TEXT_SOFT,
                                           HCW_COLOR_BORDER);
    egui_view_scroll_presenter_override_static_preview_api(EGUI_VIEW_OF(&scroll_presenter_read_only), &scroll_presenter_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&scroll_presenter_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&scroll_presenter_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&scroll_presenter_primary));
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
        EGUI_SIM_SET_WAIT(p_action, SCROLL_PRESENTER_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_PRESENTER_RECORD_FINAL_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_PRESENTER_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_PRESENTER_RECORD_FINAL_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_PRESENTER_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_PRESENTER_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SCROLL_PRESENTER_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
