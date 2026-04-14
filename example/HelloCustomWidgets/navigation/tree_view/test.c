#include "egui.h"
#include "egui_view_tree_view.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define TREE_VIEW_ROOT_WIDTH        224
#define TREE_VIEW_ROOT_HEIGHT       236
#define TREE_VIEW_PRIMARY_WIDTH     198
#define TREE_VIEW_PRIMARY_HEIGHT    116
#define TREE_VIEW_PREVIEW_WIDTH     104
#define TREE_VIEW_PREVIEW_HEIGHT    80
#define TREE_VIEW_BOTTOM_ROW_WIDTH  216
#define TREE_VIEW_BOTTOM_ROW_HEIGHT 80
#define TREE_VIEW_RECORD_WAIT       90
#define TREE_VIEW_RECORD_FRAME_WAIT 170
#define TREE_VIEW_RECORD_FINAL_WAIT 520

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_tree_view_t tree_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_tree_view_t tree_compact;
static egui_view_tree_view_t tree_read_only;
static egui_view_api_t tree_compact_api;
static egui_view_api_t tree_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Tree View";

static const egui_view_tree_view_item_t primary_items_0[] = {
        {"Workspaces", "2", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Controls", "12", 1, EGUI_VIEW_TREE_VIEW_TONE_ACCENT, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Buttons", "", 2, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Tree View", "Draft", 2, EGUI_VIEW_TREE_VIEW_TONE_ACCENT, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Resources", "3", 1, EGUI_VIEW_TREE_VIEW_TONE_WARNING, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 0},
        {"Settings", "", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 0},
};

static const egui_view_tree_view_item_t primary_items_1[] = {
        {"Workspaces", "2", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Docs", "7", 1, EGUI_VIEW_TREE_VIEW_TONE_ACCENT, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Overview", "", 2, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"API", "New", 2, EGUI_VIEW_TREE_VIEW_TONE_SUCCESS, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Samples", "", 2, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Settings", "", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 0},
};

static const egui_view_tree_view_item_t primary_items_2[] = {
        {"Workspaces", "2", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Resources", "3", 1, EGUI_VIEW_TREE_VIEW_TONE_WARNING, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Icons", "", 2, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Tokens", "Sync", 2, EGUI_VIEW_TREE_VIEW_TONE_WARNING, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Docs", "7", 1, EGUI_VIEW_TREE_VIEW_TONE_ACCENT, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 0},
        {"Settings", "", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 0},
};

static const egui_view_tree_view_item_t primary_items_3[] = {
        {"Workspaces", "2", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Settings", "4", 1, EGUI_VIEW_TREE_VIEW_TONE_ACCENT, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Preferences", "", 2, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Themes", "Ready", 2, EGUI_VIEW_TREE_VIEW_TONE_SUCCESS, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Accounts", "", 2, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Docs", "7", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 0},
};

static const egui_view_tree_view_snapshot_t primary_snapshots[] = {
        {"Solution", "6 visible", "Controls open", primary_items_0, 6, 3},
        {"Solution", "6 visible", "Docs open", primary_items_1, 6, 3},
        {"Solution", "6 visible", "Resources open", primary_items_2, 6, 3},
        {"Solution", "6 visible", "Settings open", primary_items_3, 6, 3},
};

static const egui_view_tree_view_item_t compact_items_0[] = {
        {"Home", "", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 0},
        {"Library", "4", 0, EGUI_VIEW_TREE_VIEW_TONE_ACCENT, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Recent", "", 1, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Settings", "", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 0},
};

static const egui_view_tree_view_snapshot_t compact_snapshots[] = {
        {"Compact", "4 rows", "Library branch", compact_items_0, 4, 2},
};

static const egui_view_tree_view_item_t read_only_items[] = {
        {"Workspace", "", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Release", "", 1, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Audit", "", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Exports", "", 1, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
};

static const egui_view_tree_view_snapshot_t read_only_snapshots[] = {
        {"Read only", "4 rows", "Static preview", read_only_items, 4, 2},
};

static void apply_primary_snapshot(uint8_t index)
{
    egui_view_tree_view_set_current_snapshot(EGUI_VIEW_OF(&tree_primary), index);
}

static void apply_preview_states(void)
{
    egui_view_tree_view_set_current_snapshot(EGUI_VIEW_OF(&tree_compact), 0);
    egui_view_tree_view_set_current_snapshot(EGUI_VIEW_OF(&tree_read_only), 0);
    egui_view_tree_view_set_read_only_mode(EGUI_VIEW_OF(&tree_read_only), 1);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), TREE_VIEW_ROOT_WIDTH, TREE_VIEW_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), TREE_VIEW_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 6, 0, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_tree_view_init(EGUI_VIEW_OF(&tree_primary));
    egui_view_set_size(EGUI_VIEW_OF(&tree_primary), TREE_VIEW_PRIMARY_WIDTH, TREE_VIEW_PRIMARY_HEIGHT);
    egui_view_tree_view_set_font(EGUI_VIEW_OF(&tree_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tree_view_set_meta_font(EGUI_VIEW_OF(&tree_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tree_view_set_snapshots(EGUI_VIEW_OF(&tree_primary), primary_snapshots, 4);
    egui_view_tree_view_set_palette(EGUI_VIEW_OF(&tree_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF5F7F9), EGUI_COLOR_HEX(0xD2DBE3),
                                    EGUI_COLOR_HEX(0x18222D), EGUI_COLOR_HEX(0x6E7C8B), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x178454),
                                    EGUI_COLOR_HEX(0xB77719), EGUI_COLOR_HEX(0x758391));
    egui_view_set_margin(EGUI_VIEW_OF(&tree_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&tree_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), TREE_VIEW_BOTTOM_ROW_WIDTH, TREE_VIEW_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_tree_view_init(EGUI_VIEW_OF(&tree_compact));
    egui_view_set_size(EGUI_VIEW_OF(&tree_compact), TREE_VIEW_PREVIEW_WIDTH, TREE_VIEW_PREVIEW_HEIGHT);
    egui_view_tree_view_set_font(EGUI_VIEW_OF(&tree_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tree_view_set_meta_font(EGUI_VIEW_OF(&tree_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tree_view_set_snapshots(EGUI_VIEW_OF(&tree_compact), compact_snapshots, 1);
    egui_view_tree_view_set_compact_mode(EGUI_VIEW_OF(&tree_compact), 1);
    egui_view_tree_view_set_palette(EGUI_VIEW_OF(&tree_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xF5F7F9), EGUI_COLOR_HEX(0xD2DBE3),
                                    EGUI_COLOR_HEX(0x18222D), EGUI_COLOR_HEX(0x6E7C8B), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x178454),
                                    EGUI_COLOR_HEX(0xB77719), EGUI_COLOR_HEX(0x758391));
    egui_view_tree_view_override_static_preview_api(EGUI_VIEW_OF(&tree_compact), &tree_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&tree_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&tree_compact));

    egui_view_tree_view_init(EGUI_VIEW_OF(&tree_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&tree_read_only), TREE_VIEW_PREVIEW_WIDTH, TREE_VIEW_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&tree_read_only), 8, 0, 0, 0);
    egui_view_tree_view_set_font(EGUI_VIEW_OF(&tree_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tree_view_set_meta_font(EGUI_VIEW_OF(&tree_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tree_view_set_snapshots(EGUI_VIEW_OF(&tree_read_only), read_only_snapshots, 1);
    egui_view_tree_view_set_compact_mode(EGUI_VIEW_OF(&tree_read_only), 1);
    egui_view_tree_view_set_read_only_mode(EGUI_VIEW_OF(&tree_read_only), 1);
    egui_view_tree_view_set_palette(EGUI_VIEW_OF(&tree_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD9E1E8),
                                    EGUI_COLOR_HEX(0x566675), EGUI_COLOR_HEX(0x8A97A3), EGUI_COLOR_HEX(0xB8C4CF), EGUI_COLOR_HEX(0xABBFB8),
                                    EGUI_COLOR_HEX(0xC9B691), EGUI_COLOR_HEX(0x9CA9B5));
    egui_view_tree_view_override_static_preview_api(EGUI_VIEW_OF(&tree_read_only), &tree_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&tree_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&tree_read_only));

    apply_primary_snapshot(0);
    apply_preview_states();

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
            apply_preview_states();
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TREE_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, TREE_VIEW_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TREE_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(2);
        }
        EGUI_SIM_SET_WAIT(p_action, TREE_VIEW_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TREE_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(3);
        }
        EGUI_SIM_SET_WAIT(p_action, TREE_VIEW_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TREE_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_snapshot(0);
            apply_preview_states();
        }
        EGUI_SIM_SET_WAIT(p_action, TREE_VIEW_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, TREE_VIEW_RECORD_FINAL_WAIT);
        return true;
    default:
        return false;
    }
}
#endif
