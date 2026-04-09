#include "egui.h"
#include "egui_view_badge_group.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define BADGE_GROUP_ROOT_WIDTH        224
#define BADGE_GROUP_ROOT_HEIGHT       242
#define BADGE_GROUP_PRIMARY_WIDTH     196
#define BADGE_GROUP_PRIMARY_HEIGHT    118
#define BADGE_GROUP_PREVIEW_WIDTH     104
#define BADGE_GROUP_PREVIEW_HEIGHT    84
#define BADGE_GROUP_BOTTOM_ROW_WIDTH  216
#define BADGE_GROUP_BOTTOM_ROW_HEIGHT 84
#define BADGE_GROUP_RECORD_WAIT       90
#define BADGE_GROUP_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_badge_group_t group_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_badge_group_t group_compact;
static egui_view_linearlayout_t locked_column;
static egui_view_badge_group_t group_locked;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Badge Group";
static uint8_t primary_snapshot_index = 0;
static uint8_t compact_snapshot_index = 0;

static const egui_view_badge_group_item_t primary_items_0[] = {
        {"Review", "4", 0, 1, 0},
        {"Ready", "12", 1, 1, 0},
        {"Risk", "1", 2, 0, 1},
        {"Archive", "7", 3, 0, 1},
};

static const egui_view_badge_group_item_t primary_items_1[] = {
        {"Online", "8", 1, 1, 0},
        {"Shadow", "2", 3, 0, 1},
        {"Sync", "3", 0, 0, 1},
        {"Alert", "1", 2, 1, 0},
};

static const egui_view_badge_group_item_t primary_items_2[] = {
        {"Queued", "5", 3, 0, 1},
        {"Hold", "2", 2, 1, 0},
        {"Owner", "A", 0, 1, 0},
        {"Done", "9", 1, 0, 1},
};

static const egui_view_badge_group_item_t primary_items_3[] = {
        {"Pinned", "6", 0, 0, 1},
        {"Calm", "3", 3, 1, 0},
        {"Watch", "2", 2, 0, 1},
        {"Live", "4", 1, 0, 1},
};

static const egui_view_badge_group_item_t compact_items_0[] = {
        {"Ready", "8", 0, 1, 0},
        {"Muted", "2", 3, 0, 1},
};

static const egui_view_badge_group_item_t compact_items_1[] = {
        {"Hold", "1", 2, 1, 0},
        {"QA", "6", 0, 0, 1},
};

static const egui_view_badge_group_item_t locked_items_0[] = {
        {"Pinned", "4", 3, 0, 1},
        {"Review", "1", 0, 0, 1},
};

static const egui_view_badge_group_item_t locked_items_1[] = {
        {"Quiet", "3", 3, 1, 0},
        {"Ready", "2", 1, 0, 1},
};

static const egui_view_badge_group_snapshot_t primary_snapshots[] = {
        {"TRIAGE", "Release lanes", "Mixed badges stay aligned.", "Summary follows focus.", primary_items_0, 4, 0},
        {"QUEUE", "Ops handoff", "Success tone leads the row.", "Success drives footer.", primary_items_1, 4, 0},
        {"RISK", "Change review", "Warning focus stays visible.", "Warning stays visible.", primary_items_2, 4, 1},
        {"CALM", "Archive sweep", "Neutral focus softens the card.", "Neutral stays calm.", primary_items_3, 4, 1},
};

static const egui_view_badge_group_snapshot_t compact_snapshots[] = {
        {"SET", "Compact", "", "Short row", compact_items_0, 2, 0},
        {"HOLD", "Compact", "", "Warn focus", compact_items_1, 2, 0},
};

static const egui_view_badge_group_snapshot_t locked_snapshots[] = {
        {"LOCK", "Read only", "", "Muted preview.", locked_items_0, 2, 0},
        {"LOCK", "Read only", "", "Passive", locked_items_1, 2, 0},
};

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}

static void apply_primary_snapshot(uint8_t index)
{
    primary_snapshot_index = index % (sizeof(primary_snapshots) / sizeof(primary_snapshots[0]));
    egui_view_badge_group_set_current_snapshot(EGUI_VIEW_OF(&group_primary), primary_snapshot_index);
}

static void apply_compact_snapshot(uint8_t index)
{
    compact_snapshot_index = index % (sizeof(compact_snapshots) / sizeof(compact_snapshots[0]));
    egui_view_badge_group_set_current_snapshot(EGUI_VIEW_OF(&group_compact), compact_snapshot_index);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), BADGE_GROUP_ROOT_WIDTH, BADGE_GROUP_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), BADGE_GROUP_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_badge_group_init(EGUI_VIEW_OF(&group_primary));
    egui_view_set_size(EGUI_VIEW_OF(&group_primary), BADGE_GROUP_PRIMARY_WIDTH, BADGE_GROUP_PRIMARY_HEIGHT);
    egui_view_badge_group_set_snapshots(EGUI_VIEW_OF(&group_primary), primary_snapshots, 4);
    egui_view_badge_group_set_font(EGUI_VIEW_OF(&group_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_badge_group_set_meta_font(EGUI_VIEW_OF(&group_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_group_set_palette(EGUI_VIEW_OF(&group_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                      EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&group_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&group_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), BADGE_GROUP_BOTTOM_ROW_WIDTH, BADGE_GROUP_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), BADGE_GROUP_PREVIEW_WIDTH, BADGE_GROUP_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_badge_group_init(EGUI_VIEW_OF(&group_compact));
    egui_view_set_size(EGUI_VIEW_OF(&group_compact), BADGE_GROUP_PREVIEW_WIDTH, BADGE_GROUP_PREVIEW_HEIGHT);
    egui_view_badge_group_set_snapshots(EGUI_VIEW_OF(&group_compact), compact_snapshots, 2);
    egui_view_badge_group_set_font(EGUI_VIEW_OF(&group_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_group_set_meta_font(EGUI_VIEW_OF(&group_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_group_set_compact_mode(EGUI_VIEW_OF(&group_compact), 1);
    egui_view_badge_group_set_palette(EGUI_VIEW_OF(&group_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                      EGUI_COLOR_HEX(0x7A8796));
    static egui_view_api_t group_compact_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&group_compact), &group_compact_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&group_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&group_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&locked_column));
    egui_view_set_size(EGUI_VIEW_OF(&locked_column), BADGE_GROUP_PREVIEW_WIDTH, BADGE_GROUP_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&locked_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&locked_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&locked_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&locked_column));

    egui_view_badge_group_init(EGUI_VIEW_OF(&group_locked));
    egui_view_set_size(EGUI_VIEW_OF(&group_locked), BADGE_GROUP_PREVIEW_WIDTH, BADGE_GROUP_PREVIEW_HEIGHT);
    egui_view_badge_group_set_snapshots(EGUI_VIEW_OF(&group_locked), locked_snapshots, 2);
    egui_view_badge_group_set_font(EGUI_VIEW_OF(&group_locked), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_group_set_meta_font(EGUI_VIEW_OF(&group_locked), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_group_set_compact_mode(EGUI_VIEW_OF(&group_locked), 1);
    egui_view_badge_group_set_locked_mode(EGUI_VIEW_OF(&group_locked), 1);
    egui_view_badge_group_set_palette(EGUI_VIEW_OF(&group_locked), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                      EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x9D5D00),
                                      EGUI_COLOR_HEX(0x7A8796));
    static egui_view_api_t group_locked_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&group_locked), &group_locked_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&group_locked), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&locked_column), EGUI_VIEW_OF(&group_locked));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    egui_view_badge_group_set_current_snapshot(EGUI_VIEW_OF(&group_locked), 1);

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
        EGUI_SIM_SET_WAIT(p_action, BADGE_GROUP_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_GROUP_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_GROUP_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_GROUP_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_GROUP_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_GROUP_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_GROUP_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_GROUP_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, BADGE_GROUP_RECORD_WAIT);
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
