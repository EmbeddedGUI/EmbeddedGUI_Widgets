#include "egui.h"
#include "egui_view_card_panel.h"
#include "uicode_disp0.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define CARD_PANEL_ROOT_WIDTH        224
#define CARD_PANEL_ROOT_HEIGHT       252
#define CARD_PANEL_PRIMARY_WIDTH     196
#define CARD_PANEL_PRIMARY_HEIGHT    122
#define CARD_PANEL_PREVIEW_WIDTH     104
#define CARD_PANEL_PREVIEW_HEIGHT    90
#define CARD_PANEL_BOTTOM_ROW_WIDTH  216
#define CARD_PANEL_BOTTOM_ROW_HEIGHT 90
#define CARD_PANEL_RECORD_WAIT       90
#define CARD_PANEL_RECORD_FRAME_WAIT 170
#define CARD_PANEL_RECORD_FINAL_WAIT 520
#define CARD_PANEL_DEFAULT_SNAPSHOT  0
#define PRIMARY_SNAPSHOT_COUNT       ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_card_panel_t panel_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_card_panel_t panel_secondary;
static egui_view_card_panel_t panel_muted;
static egui_view_api_t panel_secondary_api;
static egui_view_api_t panel_muted_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, HCW_COLOR_PANEL, EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Card Panel";

static const egui_view_card_panel_snapshot_t primary_snapshots[] = {
        {"OVERVIEW", "Workspace status", "Three flows stay aligned.", "98%", "uptime", "Today", "Two checks wait.", "Footer stays readable.", "Open", 0, 1},
        {"SYNC", "Design review", "New handoff needs approval.", "4", "changes", "Next step", "Confirm spacing tokens.", "Summary stays close.", "Review", 2,
         1},
        {"DEPLOY", "Release notes", "Ready for staged publish.", "6", "items", "Channel", "Internal preview for QA.", "Card stays calm on dense pages.",
         "Publish", 1, 0},
        {"ARCHIVE", "Readback summary", "Older detail stays available.", "12", "pages", "History", "Summary stays visible.", "Read only mode still works.",
         "Browse", 3, 0},
};

static const egui_view_card_panel_snapshot_t secondary_snapshots[] = {
        {"TASK", "Small", "Short.", "12", "tasks", "Focus", "", "Clear layout.", "Open", 0, 1},
};

static const egui_view_card_panel_snapshot_t muted_snapshots[] = {
        {"ARCHIVE", "Archive", "Muted.", "7", "notes", "History", "", "Preview only.", "", 3, 0},
};

static void layout_page(void);

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&panel_primary), (uint8_t)(index % PRIMARY_SNAPSHOT_COUNT));
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(CARD_PANEL_DEFAULT_SNAPSHOT);
}

static void apply_secondary_state(void)
{
    egui_view_set_enable(EGUI_VIEW_OF(&panel_secondary), 1);
    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&panel_secondary), 0);
}

static void apply_muted_state(void)
{
    egui_view_card_panel_set_current_snapshot(EGUI_VIEW_OF(&panel_muted), 0);
}

static void apply_preview_states(void)
{
    apply_secondary_state();
    apply_muted_state();
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

void test_init_ui(void)
{
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), CARD_PANEL_ROOT_WIDTH, CARD_PANEL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&title_label), CARD_PANEL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    hello_custom_widgets_demo_set_label_font_with_min_height(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), HCW_COLOR_TEXT, EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_card_panel_init(EGUI_VIEW_OF(&panel_primary));
    egui_view_set_size(EGUI_VIEW_OF(&panel_primary), CARD_PANEL_PRIMARY_WIDTH, CARD_PANEL_PRIMARY_HEIGHT);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&panel_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_card_panel_set_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_card_panel_set_meta_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_palette(EGUI_VIEW_OF(&panel_primary), HCW_COLOR_PANEL, HCW_COLOR_BORDER, HCW_COLOR_TEXT,
                                     HCW_COLOR_TEXT_MUTED, HCW_COLOR_PRIMARY, HCW_COLOR_SUCCESS, HCW_COLOR_WARNING,
                                     HCW_COLOR_NEUTRAL);
    egui_view_set_margin(EGUI_VIEW_OF(&panel_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&panel_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), CARD_PANEL_BOTTOM_ROW_WIDTH, CARD_PANEL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_card_panel_init(EGUI_VIEW_OF(&panel_secondary));
    egui_view_set_size(EGUI_VIEW_OF(&panel_secondary), CARD_PANEL_PREVIEW_WIDTH, CARD_PANEL_PREVIEW_HEIGHT);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&panel_secondary), secondary_snapshots, 1);
    egui_view_card_panel_set_font(EGUI_VIEW_OF(&panel_secondary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_meta_font(EGUI_VIEW_OF(&panel_secondary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_palette(EGUI_VIEW_OF(&panel_secondary), HCW_COLOR_PANEL, HCW_COLOR_BORDER, HCW_COLOR_TEXT,
                                     HCW_COLOR_TEXT_MUTED, HCW_COLOR_PRIMARY, HCW_COLOR_SUCCESS, HCW_COLOR_WARNING,
                                     HCW_COLOR_NEUTRAL);
    egui_view_card_panel_override_static_preview_api(EGUI_VIEW_OF(&panel_secondary), &panel_secondary_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_secondary), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&panel_secondary));

    egui_view_card_panel_init(EGUI_VIEW_OF(&panel_muted));
    egui_view_set_size(EGUI_VIEW_OF(&panel_muted), CARD_PANEL_PREVIEW_WIDTH, CARD_PANEL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&panel_muted), 8, 0, 0, 0);
    egui_view_card_panel_set_snapshots(EGUI_VIEW_OF(&panel_muted), muted_snapshots, 1);
    egui_view_card_panel_set_font(EGUI_VIEW_OF(&panel_muted), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_meta_font(EGUI_VIEW_OF(&panel_muted), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_card_panel_set_palette(EGUI_VIEW_OF(&panel_muted), HCW_COLOR_PANEL, HCW_COLOR_BORDER_STRONG, HCW_COLOR_TEXT,
                                     HCW_COLOR_TEXT_MUTED, HCW_COLOR_NEUTRAL, HCW_COLOR_NEUTRAL, HCW_COLOR_NEUTRAL,
                                     HCW_COLOR_NEUTRAL);
    egui_view_card_panel_override_static_preview_api(EGUI_VIEW_OF(&panel_muted), &panel_muted_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_muted), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&panel_muted));

    apply_primary_default_state();
    apply_preview_states();

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
}

#if EGUI_CONFIG_RECORDING_TEST
static void request_page_snapshot(void)
{
    layout_page();
    egui_view_invalidate(EGUI_VIEW_OF(&root_layout));
    recording_request_snapshot();
}

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
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_default_state();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CARD_PANEL_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
