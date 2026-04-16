#include "egui.h"
#include "egui_view_tab_view.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TAB_VIEW_ROOT_WIDTH        224
#define TAB_VIEW_ROOT_HEIGHT       224
#define TAB_VIEW_PRIMARY_WIDTH     198
#define TAB_VIEW_PRIMARY_HEIGHT    112
#define TAB_VIEW_PREVIEW_WIDTH     104
#define TAB_VIEW_PREVIEW_HEIGHT    72
#define TAB_VIEW_BOTTOM_ROW_WIDTH  216
#define TAB_VIEW_BOTTOM_ROW_HEIGHT 72
#define TAB_VIEW_RECORD_WAIT       110
#define TAB_VIEW_RECORD_FRAME_WAIT 150
#define TAB_VIEW_RECORD_FINAL_WAIT 520
#define PRIMARY_SNAPSHOT_COUNT     ((uint8_t)(sizeof(primary_snapshots) / sizeof(primary_snapshots[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_tab_view_t tab_view_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_tab_view_t tab_view_compact;
static egui_view_tab_view_t tab_view_read_only;
static egui_view_api_t tab_view_compact_api;
static egui_view_api_t tab_view_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Tab View";

static const egui_view_tab_view_tab_t docs_tabs[] = {
        {"Home", "Summary", "Workspace", "Current tab keeps the content shell linked", "Close appears only on the active tab", "Ready", "Draft",
         EGUI_VIEW_TAB_VIEW_TONE_ACCENT, 1, 1},
        {"Review", "Comment pass", "Workspace", "Review track stays in the same shell", "Tab view keeps body and tabs together", "Track", "Sync",
         EGUI_VIEW_TAB_VIEW_TONE_WARNING, 0, 1},
        {"Publish", "Release check", "Workspace", "Footer follows the selected tab", "Restore reopens hidden tabs in place", "Ship", "Ready",
         EGUI_VIEW_TAB_VIEW_TONE_SUCCESS, 0, 1},
        {"Audit", "Snapshot", "Workspace", "Muted reference state", "Compact preview keeps the same shell", "Audit", "Pinned", EGUI_VIEW_TAB_VIEW_TONE_NEUTRAL,
         0, 0},
};

static const egui_view_tab_view_tab_t ops_tabs[] = {
        {"Queue", "Signal wall", "Operations", "Snapshot switch resets the shell state", "One container can hold multiple work tracks", "Live", "Focus",
         EGUI_VIEW_TAB_VIEW_TONE_NEUTRAL, 0, 1},
        {"Alerts", "Escalation", "Operations", "Alert body stays readable", "Keyboard and touch remain aligned", "Watch", "Open",
         EGUI_VIEW_TAB_VIEW_TONE_WARNING, 1, 1},
        {"Repair", "Dispatch", "Operations", "Restore keeps tab order predictable", "Close and reopen stay inside the same frame", "Repair", "Ready",
         EGUI_VIEW_TAB_VIEW_TONE_SUCCESS, 0, 1},
};

static const egui_view_tab_view_tab_t compact_tabs[] = {
        {"Docs", "Pocket", "Compact", "Light shell", "", "Mini", "Focus", EGUI_VIEW_TAB_VIEW_TONE_ACCENT, 0, 1},
        {"Audit", "Pocket", "Compact", "Quiet rail", "", "Mini", "Track", EGUI_VIEW_TAB_VIEW_TONE_WARNING, 0, 0},
};

static const egui_view_tab_view_tab_t read_only_tabs[] = {
        {"Read", "Frozen", "Read only", "Static shell", "", "Mute", "Pinned", EGUI_VIEW_TAB_VIEW_TONE_NEUTRAL, 0, 1},
        {"Safe", "Frozen", "Read only", "Summary track", "", "Mute", "Audit", EGUI_VIEW_TAB_VIEW_TONE_ACCENT, 0, 0},
};

static const egui_view_tab_view_snapshot_t primary_snapshots[] = {
        {"Docs workspace", "Tab shell + content body", docs_tabs, 4, 0, 1},
        {"Ops workspace", "Close and restore in one shell", ops_tabs, 3, 1, 1},
};

static const egui_view_tab_view_snapshot_t compact_snapshots[] = {
        {"Compact docs", "Static preview track", compact_tabs, 2, 0, 1},
};

static const egui_view_tab_view_snapshot_t read_only_snapshots[] = {
        {"Read only", "Static reference preview", read_only_tabs, 2, 0, 0},
};

static void layout_page(void);

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_tab_view_set_current_snapshot(EGUI_VIEW_OF(&tab_view_primary), (uint8_t)(index % PRIMARY_SNAPSHOT_COUNT));
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(0);
    egui_view_tab_view_restore_tabs(EGUI_VIEW_OF(&tab_view_primary));
    egui_view_tab_view_set_current_tab(EGUI_VIEW_OF(&tab_view_primary), 0);
    egui_view_tab_view_set_current_part(EGUI_VIEW_OF(&tab_view_primary), EGUI_VIEW_TAB_VIEW_PART_TAB);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_compact_state(void)
{
    egui_view_tab_view_set_current_snapshot(EGUI_VIEW_OF(&tab_view_compact), 0);
    egui_view_tab_view_set_compact_mode(EGUI_VIEW_OF(&tab_view_compact), 1);
}

static void apply_read_only_state(void)
{
    egui_view_tab_view_set_current_snapshot(EGUI_VIEW_OF(&tab_view_read_only), 0);
    egui_view_tab_view_set_compact_mode(EGUI_VIEW_OF(&tab_view_read_only), 1);
    egui_view_tab_view_set_read_only_mode(EGUI_VIEW_OF(&tab_view_read_only), 1);
}

static void apply_preview_states(void)
{
    apply_compact_state();
    apply_read_only_state();
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

void test_init_ui(void)
{
    ui_ready = 0;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TAB_VIEW_ROOT_WIDTH, TAB_VIEW_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TAB_VIEW_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_tab_view_init(EGUI_VIEW_OF(&tab_view_primary));
    egui_view_set_size(EGUI_VIEW_OF(&tab_view_primary), TAB_VIEW_PRIMARY_WIDTH, TAB_VIEW_PRIMARY_HEIGHT);
    egui_view_tab_view_set_font(EGUI_VIEW_OF(&tab_view_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tab_view_set_meta_font(EGUI_VIEW_OF(&tab_view_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tab_view_set_snapshots(EGUI_VIEW_OF(&tab_view_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_tab_view_set_palette(EGUI_VIEW_OF(&tab_view_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xF4F7F9),
                                   EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                   EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x6B7A89));
    egui_view_set_margin(EGUI_VIEW_OF(&tab_view_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&tab_view_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TAB_VIEW_BOTTOM_ROW_WIDTH, TAB_VIEW_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_tab_view_init(EGUI_VIEW_OF(&tab_view_compact));
    egui_view_set_size(EGUI_VIEW_OF(&tab_view_compact), TAB_VIEW_PREVIEW_WIDTH, TAB_VIEW_PREVIEW_HEIGHT);
    egui_view_tab_view_set_font(EGUI_VIEW_OF(&tab_view_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tab_view_set_meta_font(EGUI_VIEW_OF(&tab_view_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tab_view_set_snapshots(EGUI_VIEW_OF(&tab_view_compact), compact_snapshots, 1);
    egui_view_tab_view_set_compact_mode(EGUI_VIEW_OF(&tab_view_compact), 1);
    egui_view_tab_view_set_palette(EGUI_VIEW_OF(&tab_view_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xF4F7F9),
                                   EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                   EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x6B7A89));
    egui_view_tab_view_override_static_preview_api(EGUI_VIEW_OF(&tab_view_compact), &tab_view_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&tab_view_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&tab_view_compact));

    egui_view_tab_view_init(EGUI_VIEW_OF(&tab_view_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&tab_view_read_only), TAB_VIEW_PREVIEW_WIDTH, TAB_VIEW_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&tab_view_read_only), 8, 0, 0, 0);
    egui_view_tab_view_set_font(EGUI_VIEW_OF(&tab_view_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tab_view_set_meta_font(EGUI_VIEW_OF(&tab_view_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tab_view_set_snapshots(EGUI_VIEW_OF(&tab_view_read_only), read_only_snapshots, 1);
    egui_view_tab_view_set_compact_mode(EGUI_VIEW_OF(&tab_view_read_only), 1);
    egui_view_tab_view_set_read_only_mode(EGUI_VIEW_OF(&tab_view_read_only), 1);
    egui_view_tab_view_set_palette(EGUI_VIEW_OF(&tab_view_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xDBE2E8), EGUI_COLOR_HEX(0xF6F8FA),
                                   EGUI_COLOR_HEX(0x536474), EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xB3BFCA), EGUI_COLOR_HEX(0xA7BDB6),
                                   EGUI_COLOR_HEX(0xC3AE88), EGUI_COLOR_HEX(0x9AA7B3));
    egui_view_tab_view_override_static_preview_api(EGUI_VIEW_OF(&tab_view_read_only), &tab_view_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&tab_view_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&tab_view_read_only));

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
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            egui_view_tab_view_set_current_tab(EGUI_VIEW_OF(&tab_view_primary), 2);
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            egui_view_tab_view_close_current_tab(EGUI_VIEW_OF(&tab_view_primary));
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            egui_view_tab_view_restore_tabs(EGUI_VIEW_OF(&tab_view_primary));
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TAB_VIEW_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
