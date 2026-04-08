#include "egui.h"
#include "egui_view_toast_stack.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TOAST_STACK_ROOT_WIDTH        224
#define TOAST_STACK_ROOT_HEIGHT       232
#define TOAST_STACK_PRIMARY_WIDTH     196
#define TOAST_STACK_PRIMARY_HEIGHT    108
#define TOAST_STACK_PREVIEW_WIDTH     104
#define TOAST_STACK_PREVIEW_HEIGHT    83
#define TOAST_STACK_BOTTOM_ROW_WIDTH  216
#define TOAST_STACK_BOTTOM_ROW_HEIGHT 83
#define TOAST_STACK_RECORD_WAIT       90
#define TOAST_STACK_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_toast_stack_t stack_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_toast_stack_t stack_compact;
static egui_view_linearlayout_t locked_column;
static egui_view_toast_stack_t stack_locked;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Toast Stack";
static uint8_t primary_snapshot_index = 0;
static uint8_t compact_snapshot_index = 0;

static const egui_view_toast_stack_snapshot_t primary_snapshots[] = {
        {"Backup ready", "Open the latest note.", "Open", "Now", "Shift starts", "Daily sync", 0, 1},
        {"Draft", "Team can review the build.", "Review", "1 min", "Queue", "Pinned", 1, 1},
        {"Storage low", "Archive logs before sync.", "Manage", "4 min", "Sync waiting", "Quota alert", 2, 1},
        {"Upload failed", "Reconnect to send report.", "Retry", "Offline", "Auth expired", "Queue paused", 3, 1},
};

static const egui_view_toast_stack_snapshot_t compact_snapshots[] = {
        {"Quota alert", "Archive.", "Manage", "Soon", "Sync", "Pinned note", 2, 0},
        {"Upload failed", "Retry send.", "Retry", "Offline", "Draft sent", "Queue paused", 3, 0},
};

static const egui_view_toast_stack_snapshot_t locked_snapshots[] = {
        {"Policy note", "Pinned for this page.", NULL, "Pinned", "Daily sync", "Workspace", 0, 0},
        {"Review ready", "Read only.", NULL, "Locked", "Queue settled", "Pinned note", 1, 0},
};

static void apply_primary_snapshot(uint8_t index)
{
    primary_snapshot_index = index % (sizeof(primary_snapshots) / sizeof(primary_snapshots[0]));
    egui_view_toast_stack_set_current_snapshot(EGUI_VIEW_OF(&stack_primary), primary_snapshot_index);
}

static void apply_compact_snapshot(uint8_t index)
{
    compact_snapshot_index = index % (sizeof(compact_snapshots) / sizeof(compact_snapshots[0]));
    egui_view_toast_stack_set_current_snapshot(EGUI_VIEW_OF(&stack_compact), compact_snapshot_index);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TOAST_STACK_ROOT_WIDTH, TOAST_STACK_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TOAST_STACK_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_toast_stack_init(EGUI_VIEW_OF(&stack_primary));
    egui_view_set_size(EGUI_VIEW_OF(&stack_primary), TOAST_STACK_PRIMARY_WIDTH, TOAST_STACK_PRIMARY_HEIGHT);
    egui_view_toast_stack_set_snapshots(EGUI_VIEW_OF(&stack_primary), primary_snapshots, 4);
    egui_view_toast_stack_set_font(EGUI_VIEW_OF(&stack_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_toast_stack_set_meta_font(EGUI_VIEW_OF(&stack_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_toast_stack_set_palette(EGUI_VIEW_OF(&stack_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                      EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0xC23934));
    egui_view_set_margin(EGUI_VIEW_OF(&stack_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&stack_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TOAST_STACK_BOTTOM_ROW_WIDTH, TOAST_STACK_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), TOAST_STACK_PREVIEW_WIDTH, TOAST_STACK_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_toast_stack_init(EGUI_VIEW_OF(&stack_compact));
    egui_view_set_size(EGUI_VIEW_OF(&stack_compact), TOAST_STACK_PREVIEW_WIDTH, TOAST_STACK_PREVIEW_HEIGHT);
    egui_view_toast_stack_set_snapshots(EGUI_VIEW_OF(&stack_compact), compact_snapshots, 2);
    egui_view_toast_stack_set_font(EGUI_VIEW_OF(&stack_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_toast_stack_set_meta_font(EGUI_VIEW_OF(&stack_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_toast_stack_set_compact_mode(EGUI_VIEW_OF(&stack_compact), 1);
    egui_view_toast_stack_set_palette(EGUI_VIEW_OF(&stack_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                      EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0xC23934));
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&stack_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&locked_column));
    egui_view_set_size(EGUI_VIEW_OF(&locked_column), TOAST_STACK_PREVIEW_WIDTH, TOAST_STACK_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&locked_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&locked_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&locked_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&locked_column));

    egui_view_toast_stack_init(EGUI_VIEW_OF(&stack_locked));
    egui_view_set_size(EGUI_VIEW_OF(&stack_locked), TOAST_STACK_PREVIEW_WIDTH, TOAST_STACK_PREVIEW_HEIGHT);
    egui_view_toast_stack_set_snapshots(EGUI_VIEW_OF(&stack_locked), locked_snapshots, 2);
    egui_view_toast_stack_set_font(EGUI_VIEW_OF(&stack_locked), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_toast_stack_set_meta_font(EGUI_VIEW_OF(&stack_locked), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_toast_stack_set_compact_mode(EGUI_VIEW_OF(&stack_locked), 1);
    egui_view_toast_stack_set_locked_mode(EGUI_VIEW_OF(&stack_locked), 1);
    egui_view_toast_stack_set_palette(EGUI_VIEW_OF(&stack_locked), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                      EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0xC23934));
    egui_view_group_add_child(EGUI_VIEW_OF(&locked_column), EGUI_VIEW_OF(&stack_locked));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    egui_view_toast_stack_set_current_snapshot(EGUI_VIEW_OF(&stack_locked), 1);

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&locked_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
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
        }
        EGUI_SIM_SET_WAIT(p_action, TOAST_STACK_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOAST_STACK_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, TOAST_STACK_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOAST_STACK_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, TOAST_STACK_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOAST_STACK_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, TOAST_STACK_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TOAST_STACK_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, TOAST_STACK_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
