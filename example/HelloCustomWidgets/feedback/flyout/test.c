#include "egui.h"
#include "egui_view_flyout.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define FLYOUT_ROOT_WIDTH        224
#define FLYOUT_ROOT_HEIGHT       252
#define FLYOUT_PRIMARY_WIDTH     196
#define FLYOUT_PRIMARY_HEIGHT    132
#define FLYOUT_PREVIEW_WIDTH     104
#define FLYOUT_PREVIEW_HEIGHT    80
#define FLYOUT_BOTTOM_ROW_WIDTH  216
#define FLYOUT_BOTTOM_ROW_HEIGHT 80
#define FLYOUT_RECORD_WAIT       100
#define FLYOUT_RECORD_FRAME_WAIT 180
#define FLYOUT_RECORD_FINAL_WAIT 520

#define PRIMARY_SNAPSHOT_COUNT ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_flyout_t flyout_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_flyout_t flyout_compact;
static egui_view_flyout_t flyout_disabled;
static egui_view_api_t flyout_compact_api;
static egui_view_api_t flyout_disabled_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Flyout";
static void layout_page(void);

static const egui_view_flyout_snapshot_t primary_snapshots[] = {
        {"Review", "Flyout", "Review draft", "Keep actions anchored near the current affordance.", "Publish", "Later", "Bottom placement",
         EGUI_VIEW_FLYOUT_TONE_ACCENT, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, -18},
        {"Search", "Top placement", "Find in workspace", "The panel can sit above the target when space is tight.", "Search", "Close", "Top placement",
         EGUI_VIEW_FLYOUT_TONE_SUCCESS, EGUI_VIEW_FLYOUT_PLACEMENT_TOP, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_SECONDARY, 18},
        {"Sync", "Warning", "Reconnect before sending", "Pending changes stay local until sync completes.", "Open sync", "Dismiss", "Warning tone",
         EGUI_VIEW_FLYOUT_TONE_WARNING, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, 0},
        {"Pinned", "Closed", "Tap target to reopen", "", "", "", "", EGUI_VIEW_FLYOUT_TONE_NEUTRAL, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 0, 0, 0,
         EGUI_VIEW_FLYOUT_PART_TARGET, 0},
};

static const egui_view_flyout_snapshot_t compact_snapshots[] = {
        {"Filter", "", "Compact flyout", "", "Open", "Later", "", EGUI_VIEW_FLYOUT_TONE_ACCENT, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1,
         EGUI_VIEW_FLYOUT_PART_PRIMARY, -8},
};

static const egui_view_flyout_snapshot_t disabled_snapshot = {
        "Locked", "Disabled", "Preview only", "", "Primary", "Secondary", "", EGUI_VIEW_FLYOUT_TONE_NEUTRAL, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 0, 0,
        EGUI_VIEW_FLYOUT_PART_TARGET, 4};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_flyout_set_current_snapshot(EGUI_VIEW_OF(&flyout_primary), index % PRIMARY_SNAPSHOT_COUNT);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_preview_states(void)
{
    egui_view_flyout_set_current_snapshot(EGUI_VIEW_OF(&flyout_compact), 0);
    egui_view_flyout_set_current_snapshot(EGUI_VIEW_OF(&flyout_disabled), 0);
    egui_view_flyout_set_disabled_mode(EGUI_VIEW_OF(&flyout_disabled), 1);
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
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), FLYOUT_ROOT_WIDTH, FLYOUT_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), FLYOUT_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_flyout_init(EGUI_VIEW_OF(&flyout_primary));
    egui_view_set_size(EGUI_VIEW_OF(&flyout_primary), FLYOUT_PRIMARY_WIDTH, FLYOUT_PRIMARY_HEIGHT);
    egui_view_flyout_set_font(EGUI_VIEW_OF(&flyout_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_flyout_set_meta_font(EGUI_VIEW_OF(&flyout_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flyout_set_snapshots(EGUI_VIEW_OF(&flyout_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_set_margin(EGUI_VIEW_OF(&flyout_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&flyout_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), FLYOUT_BOTTOM_ROW_WIDTH, FLYOUT_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_flyout_init(EGUI_VIEW_OF(&flyout_compact));
    egui_view_set_size(EGUI_VIEW_OF(&flyout_compact), FLYOUT_PREVIEW_WIDTH, FLYOUT_PREVIEW_HEIGHT);
    egui_view_flyout_set_font(EGUI_VIEW_OF(&flyout_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flyout_set_meta_font(EGUI_VIEW_OF(&flyout_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flyout_set_snapshots(EGUI_VIEW_OF(&flyout_compact), compact_snapshots, 1);
    egui_view_flyout_set_compact_mode(EGUI_VIEW_OF(&flyout_compact), 1);
    egui_view_flyout_override_static_preview_api(EGUI_VIEW_OF(&flyout_compact), &flyout_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flyout_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&flyout_compact));

    egui_view_flyout_init(EGUI_VIEW_OF(&flyout_disabled));
    egui_view_set_size(EGUI_VIEW_OF(&flyout_disabled), FLYOUT_PREVIEW_WIDTH, FLYOUT_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&flyout_disabled), 8, 0, 0, 0);
    egui_view_flyout_set_font(EGUI_VIEW_OF(&flyout_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flyout_set_meta_font(EGUI_VIEW_OF(&flyout_disabled), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_flyout_set_snapshots(EGUI_VIEW_OF(&flyout_disabled), &disabled_snapshot, 1);
    egui_view_flyout_set_compact_mode(EGUI_VIEW_OF(&flyout_disabled), 1);
    egui_view_flyout_set_disabled_mode(EGUI_VIEW_OF(&flyout_disabled), 1);
    egui_view_flyout_set_palette(EGUI_VIEW_OF(&flyout_disabled), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0x536474),
                                 EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xA7B4C1), EGUI_COLOR_HEX(0xB2C4BA), EGUI_COLOR_HEX(0xC4B8A4),
                                 EGUI_COLOR_HEX(0xB4BDC8), EGUI_COLOR_HEX(0xE8EDF2), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD9E1E8));
    egui_view_flyout_override_static_preview_api(EGUI_VIEW_OF(&flyout_disabled), &flyout_disabled_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&flyout_disabled), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&flyout_disabled));

    apply_primary_snapshot(0);
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_snapshot(0);
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
            apply_primary_snapshot(0);
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLYOUT_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, FLYOUT_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLYOUT_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, FLYOUT_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLYOUT_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, FLYOUT_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLYOUT_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_snapshot(0);
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, FLYOUT_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, FLYOUT_RECORD_FINAL_WAIT);
        return true;
    default:
        break;
    }

    return false;
}
#endif
