#include "egui.h"
#include "egui_view_teaching_tip.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TEACHING_TIP_ROOT_WIDTH        224
#define TEACHING_TIP_ROOT_HEIGHT       252
#define TEACHING_TIP_PRIMARY_WIDTH     196
#define TEACHING_TIP_PRIMARY_HEIGHT    132
#define TEACHING_TIP_PREVIEW_WIDTH     104
#define TEACHING_TIP_PREVIEW_HEIGHT    80
#define TEACHING_TIP_BOTTOM_ROW_WIDTH  216
#define TEACHING_TIP_BOTTOM_ROW_HEIGHT 80
#define TEACHING_TIP_RECORD_WAIT       120
#define TEACHING_TIP_RECORD_FRAME_WAIT 180
#define TEACHING_TIP_RECORD_FINAL_WAIT 520
#define PRIMARY_SNAPSHOT_COUNT         ((uint8_t)(sizeof(primary_snapshots_template) / sizeof(primary_snapshots_template[0])))

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_teaching_tip_t tip_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_teaching_tip_t tip_compact;
static egui_view_teaching_tip_t tip_read_only;
static egui_view_api_t tip_compact_api;
static egui_view_api_t tip_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Teaching Tip";
static void layout_page(void);

static const egui_view_teaching_tip_snapshot_t primary_snapshots_template[] = {
        {"Quick filters", "Coachmark", "Pin today view", "Keep ship dates nearby.", "Pin tip", "Later", "Below target", EGUI_VIEW_TEACHING_TIP_TONE_ACCENT,
         EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 1, 1, 1, 1, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, -18},
        {"Cmd palette", "Shortcut", "Press slash to search", "Jump to commands fast.", "Got it", "Tips", "Placement above target",
         EGUI_VIEW_TEACHING_TIP_TONE_ACCENT, EGUI_VIEW_TEACHING_TIP_PLACEMENT_TOP, 1, 1, 1, 1, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, 22},
        {"Sync draft", "Warning", "Reconnect before publish", "Offline edits still queued.", "Review", "Dismiss", "Anchored warning tip",
         EGUI_VIEW_TEACHING_TIP_TONE_WARNING, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 1, 1, 1, 1, EGUI_VIEW_TEACHING_TIP_PART_SECONDARY, -10},
        {"Quick filters", "", "Tip hidden", "Tap target to reopen", "", "", "", EGUI_VIEW_TEACHING_TIP_TONE_NEUTRAL, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 0,
         0, 0, 0, EGUI_VIEW_TEACHING_TIP_PART_TARGET, 0},
};

static const egui_view_teaching_tip_snapshot_t compact_snapshots[] = {
        {"Quick tip", "Hint", "Pin filters", "Keep nearby.", "Open", "", "", EGUI_VIEW_TEACHING_TIP_TONE_ACCENT, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM, 1,
         1, 0, 1, EGUI_VIEW_TEACHING_TIP_PART_PRIMARY, -8},
};

static const egui_view_teaching_tip_snapshot_t read_only_snapshots[] = {
        {"Preview", "Read only", "Review", "Preview only.", "Read", "", "Preview", EGUI_VIEW_TEACHING_TIP_TONE_NEUTRAL, EGUI_VIEW_TEACHING_TIP_PLACEMENT_BOTTOM,
         1, 1, 0, 0, EGUI_VIEW_TEACHING_TIP_PART_TARGET, 6},
};

static egui_view_teaching_tip_snapshot_t primary_snapshots[PRIMARY_SNAPSHOT_COUNT];
static uint8_t primary_snapshot_index = 0;

static void reset_primary_snapshots(void)
{
    uint8_t index;

    for (index = 0; index < PRIMARY_SNAPSHOT_COUNT; index++)
    {
        primary_snapshots[index] = primary_snapshots_template[index];
    }
}

static void sync_closed_snapshot_from(uint8_t source_index)
{
    if (source_index >= PRIMARY_SNAPSHOT_COUNT || source_index == PRIMARY_SNAPSHOT_COUNT - 1)
    {
        return;
    }

    primary_snapshots[PRIMARY_SNAPSHOT_COUNT - 1] = primary_snapshots_template[PRIMARY_SNAPSHOT_COUNT - 1];
    primary_snapshots[PRIMARY_SNAPSHOT_COUNT - 1].target_label = primary_snapshots[source_index].target_label;
    primary_snapshots[PRIMARY_SNAPSHOT_COUNT - 1].tone = primary_snapshots[source_index].tone;
    primary_snapshots[PRIMARY_SNAPSHOT_COUNT - 1].target_offset_x = primary_snapshots[source_index].target_offset_x;
}

static void apply_primary_snapshot(uint8_t index)
{
    primary_snapshot_index = (uint8_t)(index % PRIMARY_SNAPSHOT_COUNT);
    egui_view_teaching_tip_set_current_snapshot(EGUI_VIEW_OF(&tip_primary), primary_snapshot_index);
    if (ui_ready)
    {
        layout_page();
    }
}

static void reset_primary_state(uint8_t index)
{
    reset_primary_snapshots();
    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&tip_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    apply_primary_snapshot(index);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_preview_states(void)
{
    egui_view_teaching_tip_set_current_snapshot(EGUI_VIEW_OF(&tip_compact), 0);
    egui_view_teaching_tip_set_current_snapshot(EGUI_VIEW_OF(&tip_read_only), 0);
    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&tip_read_only), 1);
    if (ui_ready)
    {
        layout_page();
    }
}

static void on_primary_part_changed(egui_view_t *self, uint8_t part)
{
    EGUI_UNUSED(self);

    if (part == EGUI_VIEW_TEACHING_TIP_PART_CLOSE && primary_snapshot_index != PRIMARY_SNAPSHOT_COUNT - 1)
    {
        sync_closed_snapshot_from(primary_snapshot_index);
        apply_primary_snapshot((uint8_t)(PRIMARY_SNAPSHOT_COUNT - 1));
        egui_view_teaching_tip_set_current_part(EGUI_VIEW_OF(&tip_primary), EGUI_VIEW_TEACHING_TIP_PART_TARGET);
        return;
    }

    if (part == EGUI_VIEW_TEACHING_TIP_PART_TARGET && primary_snapshot_index == PRIMARY_SNAPSHOT_COUNT - 1)
    {
        apply_primary_snapshot(0);
        egui_view_teaching_tip_set_current_part(EGUI_VIEW_OF(&tip_primary), EGUI_VIEW_TEACHING_TIP_PART_TARGET);
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
    reset_primary_snapshots();

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TEACHING_TIP_ROOT_WIDTH, TEACHING_TIP_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TEACHING_TIP_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_teaching_tip_init(EGUI_VIEW_OF(&tip_primary));
    egui_view_set_size(EGUI_VIEW_OF(&tip_primary), TEACHING_TIP_PRIMARY_WIDTH, TEACHING_TIP_PRIMARY_HEIGHT);
    egui_view_teaching_tip_set_font(EGUI_VIEW_OF(&tip_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_teaching_tip_set_meta_font(EGUI_VIEW_OF(&tip_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&tip_primary), primary_snapshots, PRIMARY_SNAPSHOT_COUNT);
    egui_view_teaching_tip_set_on_part_changed_listener(EGUI_VIEW_OF(&tip_primary), on_primary_part_changed);
    egui_view_teaching_tip_set_palette(EGUI_VIEW_OF(&tip_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                       EGUI_COLOR_HEX(0x7A8796), EGUI_COLOR_HEX(0xDDE5EB));
    egui_view_set_margin(EGUI_VIEW_OF(&tip_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&tip_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TEACHING_TIP_BOTTOM_ROW_WIDTH, TEACHING_TIP_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_teaching_tip_init(EGUI_VIEW_OF(&tip_compact));
    egui_view_set_size(EGUI_VIEW_OF(&tip_compact), TEACHING_TIP_PREVIEW_WIDTH, TEACHING_TIP_PREVIEW_HEIGHT);
    egui_view_teaching_tip_set_font(EGUI_VIEW_OF(&tip_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_teaching_tip_set_meta_font(EGUI_VIEW_OF(&tip_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&tip_compact), compact_snapshots, 1);
    egui_view_teaching_tip_set_compact_mode(EGUI_VIEW_OF(&tip_compact), 1);
    egui_view_teaching_tip_set_palette(EGUI_VIEW_OF(&tip_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                       EGUI_COLOR_HEX(0x7A8796), EGUI_COLOR_HEX(0xDDE5EB));
    egui_view_teaching_tip_override_static_preview_api(EGUI_VIEW_OF(&tip_compact), &tip_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&tip_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&tip_compact));

    egui_view_teaching_tip_init(EGUI_VIEW_OF(&tip_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&tip_read_only), TEACHING_TIP_PREVIEW_WIDTH, TEACHING_TIP_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&tip_read_only), 8, 0, 0, 0);
    egui_view_teaching_tip_set_font(EGUI_VIEW_OF(&tip_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_teaching_tip_set_meta_font(EGUI_VIEW_OF(&tip_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_teaching_tip_set_snapshots(EGUI_VIEW_OF(&tip_read_only), read_only_snapshots, 1);
    egui_view_teaching_tip_set_compact_mode(EGUI_VIEW_OF(&tip_read_only), 1);
    egui_view_teaching_tip_set_read_only_mode(EGUI_VIEW_OF(&tip_read_only), 1);
    egui_view_teaching_tip_set_palette(EGUI_VIEW_OF(&tip_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0x536474),
                                       EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xA7B4C1), EGUI_COLOR_HEX(0xB2C4BA), EGUI_COLOR_HEX(0xC4B8A4),
                                       EGUI_COLOR_HEX(0xB4BDC8), EGUI_COLOR_HEX(0xE8EDF2));
    egui_view_teaching_tip_override_static_preview_api(EGUI_VIEW_OF(&tip_read_only), &tip_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&tip_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&tip_read_only));

    reset_primary_state(0);
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    reset_primary_state(0);
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
            reset_primary_state(0);
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TEACHING_TIP_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, TEACHING_TIP_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TEACHING_TIP_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, TEACHING_TIP_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TEACHING_TIP_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            sync_closed_snapshot_from(primary_snapshot_index);
            apply_primary_snapshot((uint8_t)(PRIMARY_SNAPSHOT_COUNT - 1));
        }
        EGUI_SIM_SET_WAIT(p_action, TEACHING_TIP_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TEACHING_TIP_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            reset_primary_state(0);
            apply_preview_states();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TEACHING_TIP_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
