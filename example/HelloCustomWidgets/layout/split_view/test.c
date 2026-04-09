#include "egui.h"
#include "egui_view_split_view.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SPLIT_VIEW_ROOT_W          224
#define SPLIT_VIEW_ROOT_H          224
#define SPLIT_VIEW_PRIMARY_W       196
#define SPLIT_VIEW_PRIMARY_H       104
#define SPLIT_VIEW_PREVIEW_W       104
#define SPLIT_VIEW_PREVIEW_H       74
#define SPLIT_VIEW_BOTTOM_W        216
#define SPLIT_VIEW_BOTTOM_H        74
#define SPLIT_VIEW_RECORD_WAIT     90
#define SPLIT_VIEW_RECORD_FRAME_WAIT 170

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_split_view_t panel_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_split_view_t panel_compact;
static egui_view_linearlayout_t read_only_column;
static egui_view_split_view_t panel_read_only;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Split View";
static uint8_t primary_index = 0;
static uint8_t primary_pane_expanded = 1;
static uint8_t compact_index = 0;
static uint8_t compact_pane_expanded = 0;

static const egui_view_split_view_item_t primary_items[] = {
        {"OV", "Overview", "8", "Workspace", "Overview board", "Updated 2m ago", "Pinned modules stay visible", "Status cards stay aligned", "Open",
         EGUI_VIEW_SPLIT_VIEW_TONE_ACCENT, 1},
        {"FI", "Files", "12", "Library", "Files library", "Synced 5m ago", "Shared assets stay within reach", "Recent exports stay sorted", "Browse",
         EGUI_VIEW_SPLIT_VIEW_TONE_ACCENT, 0},
        {"RV", "Review", "4", "Review", "Review queue", "Awaiting signoff", "Approvals stay on one rail", "Escalations remain visible", "Assign",
         EGUI_VIEW_SPLIT_VIEW_TONE_WARNING, 0},
        {"AR", "Archive", "1", "Archive", "Archive shelf", "Retention 30 days", "Older work stays tucked away", "Restore before final purge", "Archive",
         EGUI_VIEW_SPLIT_VIEW_TONE_NEUTRAL, 0},
};

static const egui_view_split_view_item_t compact_items[] = {
        {"OV", "Overview", "8", "Focus", "Overview", "Pinned snapshot", "Pane opens in place", "Detail copy stays light", "Open",
         EGUI_VIEW_SPLIT_VIEW_TONE_ACCENT, 1},
        {"RV", "Review", "4", "Queue", "Review", "Compact queue", "Approvals stay visible", "Follow-up stays nearby", "Assign",
         EGUI_VIEW_SPLIT_VIEW_TONE_WARNING, 0},
        {"AR", "Archive", "1", "Store", "Archive", "Compact archive", "Older work stays parked", "Restore when needed", "Store",
         EGUI_VIEW_SPLIT_VIEW_TONE_NEUTRAL, 0},
};

static const egui_view_split_view_item_t read_only_items[] = {
        {"MB", "Members", "7", "People", "Member roster", "Read only", "Names stay visible", "No touch edits allowed", "View",
         EGUI_VIEW_SPLIT_VIEW_TONE_SUCCESS, 0},
        {"FI", "Files", "12", "Library", "Files library", "Read only", "Shared assets stay visible", "Selection remains fixed", "View",
         EGUI_VIEW_SPLIT_VIEW_TONE_ACCENT, 0},
        {"AR", "Archive", "1", "Archive", "Archive shelf", "Read only", "Older work stays tucked away", "Restore from another flow", "View",
         EGUI_VIEW_SPLIT_VIEW_TONE_NEUTRAL, 0},
};

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}

static void apply_primary_state(uint8_t item_index, uint8_t pane_expanded)
{
    primary_index = (uint8_t)(item_index % EGUI_ARRAY_SIZE(primary_items));
    primary_pane_expanded = pane_expanded ? 1 : 0;
    egui_view_split_view_set_current_index(EGUI_VIEW_OF(&panel_primary), primary_index);
    egui_view_split_view_set_pane_expanded(EGUI_VIEW_OF(&panel_primary), primary_pane_expanded);
}

static void apply_compact_state(uint8_t item_index, uint8_t pane_expanded)
{
    compact_index = (uint8_t)(item_index % EGUI_ARRAY_SIZE(compact_items));
    compact_pane_expanded = pane_expanded ? 1 : 0;
    egui_view_split_view_set_current_index(EGUI_VIEW_OF(&panel_compact), compact_index);
    egui_view_split_view_set_pane_expanded(EGUI_VIEW_OF(&panel_compact), compact_pane_expanded);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SPLIT_VIEW_ROOT_W, SPLIT_VIEW_ROOT_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SPLIT_VIEW_ROOT_W, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_split_view_init(EGUI_VIEW_OF(&panel_primary));
    egui_view_set_size(EGUI_VIEW_OF(&panel_primary), SPLIT_VIEW_PRIMARY_W, SPLIT_VIEW_PRIMARY_H);
    egui_view_split_view_set_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_split_view_set_meta_font(EGUI_VIEW_OF(&panel_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_view_set_items(EGUI_VIEW_OF(&panel_primary), primary_items, 4);
    egui_view_split_view_set_palette(EGUI_VIEW_OF(&panel_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                     EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                     EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    egui_view_set_margin(EGUI_VIEW_OF(&panel_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&panel_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SPLIT_VIEW_BOTTOM_W, SPLIT_VIEW_BOTTOM_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), SPLIT_VIEW_PREVIEW_W, SPLIT_VIEW_BOTTOM_H);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_split_view_init(EGUI_VIEW_OF(&panel_compact));
    egui_view_set_size(EGUI_VIEW_OF(&panel_compact), SPLIT_VIEW_PREVIEW_W, SPLIT_VIEW_PREVIEW_H);
    egui_view_split_view_set_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_view_set_meta_font(EGUI_VIEW_OF(&panel_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_view_set_items(EGUI_VIEW_OF(&panel_compact), compact_items, 3);
    egui_view_split_view_set_compact_mode(EGUI_VIEW_OF(&panel_compact), 1);
    egui_view_split_view_set_palette(EGUI_VIEW_OF(&panel_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                     EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                     EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    static egui_view_api_t panel_compact_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&panel_compact), &panel_compact_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&panel_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), SPLIT_VIEW_PREVIEW_W, SPLIT_VIEW_BOTTOM_H);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_split_view_init(EGUI_VIEW_OF(&panel_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&panel_read_only), SPLIT_VIEW_PREVIEW_W, SPLIT_VIEW_PREVIEW_H);
    egui_view_split_view_set_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_view_set_meta_font(EGUI_VIEW_OF(&panel_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_split_view_set_items(EGUI_VIEW_OF(&panel_read_only), read_only_items, 3);
    egui_view_split_view_set_compact_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_split_view_set_read_only_mode(EGUI_VIEW_OF(&panel_read_only), 1);
    egui_view_split_view_set_palette(EGUI_VIEW_OF(&panel_read_only), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0xEAF0F7),
                                     EGUI_COLOR_HEX(0x1A2734), EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x0F7B45),
                                     EGUI_COLOR_HEX(0x9D5D00), EGUI_COLOR_HEX(0x7A8796));
    static egui_view_api_t panel_read_only_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&panel_read_only), &panel_read_only_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&panel_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&panel_read_only));

    apply_primary_state(0, 1);
    apply_compact_state(0, 0);
    egui_view_split_view_set_current_index(EGUI_VIEW_OF(&panel_read_only), 0);
    egui_view_split_view_set_pane_expanded(EGUI_VIEW_OF(&panel_read_only), 0);

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_column));
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
            apply_primary_state(0, 1);
            apply_compact_state(0, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_VIEW_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            apply_primary_state(0, 0);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_VIEW_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_state(2, 1);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_VIEW_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_state(3, 1);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_VIEW_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_VIEW_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_state(1, 1);
        }
        EGUI_SIM_SET_WAIT(p_action, SPLIT_VIEW_RECORD_WAIT);
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
