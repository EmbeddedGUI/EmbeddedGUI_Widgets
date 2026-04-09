#include "egui.h"
#include "egui_view_skeleton.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SKELETON_ROOT_WIDTH        224
#define SKELETON_ROOT_HEIGHT       224
#define SKELETON_PRIMARY_WIDTH     196
#define SKELETON_PRIMARY_HEIGHT    124
#define SKELETON_PREVIEW_WIDTH     104
#define SKELETON_PREVIEW_HEIGHT    60
#define SKELETON_BOTTOM_ROW_WIDTH  216
#define SKELETON_BOTTOM_ROW_HEIGHT 60
#define SKELETON_RECORD_WAIT       90
#define SKELETON_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_skeleton_t skeleton_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_skeleton_t skeleton_compact;
static egui_view_skeleton_t skeleton_read_only;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Skeleton";
static uint8_t primary_snapshot_index = 0;
static uint8_t compact_snapshot_index = 0;

static const egui_view_skeleton_block_t primary_blocks_a[] = {
        {0, 1, 22, 22, 11}, {30, 2, 80, 8, 4},  {30, 14, 58, 7, 4}, {0, 34, 176, 20, 7},
        {0, 62, 164, 7, 3}, {0, 74, 170, 7, 3}, {0, 86, 142, 7, 3}, {148, 85, 20, 8, 4},
};
static const egui_view_skeleton_block_t primary_blocks_b[] = {
        {0, 0, 96, 8, 4},   {0, 12, 136, 7, 3}, {0, 28, 52, 18, 7}, {62, 30, 78, 7, 3},  {62, 42, 60, 7, 3},
        {0, 56, 52, 18, 7}, {62, 58, 76, 7, 3}, {62, 70, 58, 7, 3}, {146, 32, 14, 7, 4}, {146, 60, 14, 7, 4},
};
static const egui_view_skeleton_block_t primary_blocks_c[] = {
        {0, 0, 96, 8, 4},   {0, 18, 16, 16, 8}, {24, 20, 76, 6, 3},   {24, 30, 48, 6, 3}, {136, 19, 28, 10, 5}, {0, 42, 16, 16, 8},
        {24, 44, 68, 6, 3}, {24, 54, 50, 6, 3}, {136, 43, 26, 10, 5}, {0, 66, 16, 16, 8}, {24, 68, 82, 6, 3},   {24, 78, 52, 6, 3},
};
static const egui_view_skeleton_snapshot_t primary_snapshots[] = {
        {"Article", "Loading article", primary_blocks_a, 8, 3},
        {"Feed", "Loading feed", primary_blocks_b, 10, 2},
        {"Settings", "Loading settings", primary_blocks_c, 12, 1},
};

static const egui_view_skeleton_block_t compact_blocks_a[] = {
        {0, 4, 11, 11, 6}, {17, 5, 46, 5, 3}, {17, 13, 32, 5, 3}, {0, 24, 78, 10, 4}, {0, 38, 44, 4, 2},
};
static const egui_view_skeleton_block_t compact_blocks_b[] = {
        {0, 3, 30, 15, 5},
        {40, 3, 30, 15, 5},
        {0, 25, 74, 6, 3},
        {0, 35, 48, 5, 3},
};
static const egui_view_skeleton_snapshot_t compact_snapshots[] = {
        {"Compact row", NULL, compact_blocks_a, 5, 3},
        {"Compact tile", NULL, compact_blocks_b, 4, 1},
};

static const egui_view_skeleton_block_t read_only_blocks[] = {
        {0, 2, 14, 14, 7}, {20, 4, 36, 5, 3}, {20, 12, 24, 5, 3}, {0, 24, 74, 10, 4}, {0, 36, 52, 4, 2},
};
static const egui_view_skeleton_snapshot_t read_only_snapshots[] = {
        {"Read only", NULL, read_only_blocks, 5, 3},
};

static void apply_primary_snapshot(uint8_t index)
{
    primary_snapshot_index = index % (sizeof(primary_snapshots) / sizeof(primary_snapshots[0]));
    egui_view_skeleton_set_current_snapshot(EGUI_VIEW_OF(&skeleton_primary), primary_snapshot_index);
    egui_view_skeleton_set_emphasis_block(EGUI_VIEW_OF(&skeleton_primary), primary_snapshots[primary_snapshot_index].emphasis_block);
}

static void apply_compact_snapshot(uint8_t index)
{
    compact_snapshot_index = index % (sizeof(compact_snapshots) / sizeof(compact_snapshots[0]));
    egui_view_skeleton_set_current_snapshot(EGUI_VIEW_OF(&skeleton_compact), compact_snapshot_index);
    egui_view_skeleton_set_emphasis_block(EGUI_VIEW_OF(&skeleton_compact), compact_snapshots[compact_snapshot_index].emphasis_block);
}

static void apply_read_only_snapshot(void)
{
    egui_view_skeleton_set_current_snapshot(EGUI_VIEW_OF(&skeleton_read_only), 0);
    egui_view_skeleton_set_emphasis_block(EGUI_VIEW_OF(&skeleton_read_only), read_only_snapshots[0].emphasis_block);
}

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SKELETON_ROOT_WIDTH, SKELETON_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SKELETON_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_skeleton_init(EGUI_VIEW_OF(&skeleton_primary));
    egui_view_set_size(EGUI_VIEW_OF(&skeleton_primary), SKELETON_PRIMARY_WIDTH, SKELETON_PRIMARY_HEIGHT);
    egui_view_skeleton_set_snapshots(EGUI_VIEW_OF(&skeleton_primary), primary_snapshots, 3);
    egui_view_skeleton_set_font(EGUI_VIEW_OF(&skeleton_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_skeleton_set_palette(EGUI_VIEW_OF(&skeleton_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0xE7EDF3),
                                   EGUI_COLOR_HEX(0x5F6E7D), EGUI_COLOR_HEX(0x8793A0), EGUI_COLOR_HEX(0x8AB7EA));
    egui_view_set_margin(EGUI_VIEW_OF(&skeleton_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&skeleton_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SKELETON_BOTTOM_ROW_WIDTH, SKELETON_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_skeleton_init(EGUI_VIEW_OF(&skeleton_compact));
    egui_view_set_size(EGUI_VIEW_OF(&skeleton_compact), SKELETON_PREVIEW_WIDTH, SKELETON_PREVIEW_HEIGHT);
    egui_view_skeleton_set_snapshots(EGUI_VIEW_OF(&skeleton_compact), compact_snapshots, 2);
    egui_view_skeleton_set_font(EGUI_VIEW_OF(&skeleton_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_skeleton_set_show_footer(EGUI_VIEW_OF(&skeleton_compact), 0);
    egui_view_skeleton_set_compact_mode(EGUI_VIEW_OF(&skeleton_compact), 1);
    egui_view_skeleton_set_animation_mode(EGUI_VIEW_OF(&skeleton_compact), EGUI_VIEW_SKELETON_ANIM_PULSE);
    egui_view_skeleton_set_palette(EGUI_VIEW_OF(&skeleton_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD2DBE3), EGUI_COLOR_HEX(0xE7EDF3),
                                   EGUI_COLOR_HEX(0x5F6E7D), EGUI_COLOR_HEX(0x8793A0), EGUI_COLOR_HEX(0x8AB7EA));
    static egui_view_api_t skeleton_compact_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&skeleton_compact), &skeleton_compact_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&skeleton_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&skeleton_compact));

    egui_view_skeleton_init(EGUI_VIEW_OF(&skeleton_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&skeleton_read_only), SKELETON_PREVIEW_WIDTH, SKELETON_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&skeleton_read_only), 8, 0, 0, 0);
    egui_view_skeleton_set_snapshots(EGUI_VIEW_OF(&skeleton_read_only), read_only_snapshots, 1);
    egui_view_skeleton_set_font(EGUI_VIEW_OF(&skeleton_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_skeleton_set_show_footer(EGUI_VIEW_OF(&skeleton_read_only), 0);
    egui_view_skeleton_set_compact_mode(EGUI_VIEW_OF(&skeleton_read_only), 1);
    egui_view_skeleton_set_locked_mode(EGUI_VIEW_OF(&skeleton_read_only), 1);
    egui_view_skeleton_set_animation_mode(EGUI_VIEW_OF(&skeleton_read_only), EGUI_VIEW_SKELETON_ANIM_NONE);
    egui_view_skeleton_set_palette(EGUI_VIEW_OF(&skeleton_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0xEAF0F5),
                                   EGUI_COLOR_HEX(0x8896A4), EGUI_COLOR_HEX(0xA5B1BD), EGUI_COLOR_HEX(0xB4C9DC));
    static egui_view_api_t skeleton_read_only_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&skeleton_read_only), &skeleton_read_only_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&skeleton_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&skeleton_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_read_only_snapshot();

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

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
            apply_read_only_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SKELETON_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SKELETON_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SKELETON_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SKELETON_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, SKELETON_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SKELETON_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, SKELETON_RECORD_WAIT);
        return true;
    case 7:
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
