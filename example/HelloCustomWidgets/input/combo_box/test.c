#include <string.h>

#include "egui.h"
#include "egui_view_combo_box.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define COMBO_BOX_ROOT_WIDTH        224
#define COMBO_BOX_ROOT_HEIGHT       206
#define COMBO_BOX_PRIMARY_WIDTH     196
#define COMBO_BOX_PRIMARY_HEIGHT    34
#define COMBO_BOX_PREVIEW_WIDTH     104
#define COMBO_BOX_PREVIEW_HEIGHT    28
#define COMBO_BOX_BOTTOM_ROW_WIDTH  216
#define COMBO_BOX_BOTTOM_ROW_HEIGHT 28
#define COMBO_BOX_RECORD_WAIT       90
#define COMBO_BOX_RECORD_FRAME_WAIT 170

typedef struct combo_box_snapshot combo_box_snapshot_t;
struct combo_box_snapshot
{
    const char **items;
    uint8_t item_count;
    uint8_t selected_index;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_combobox_t control_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_combobox_t control_compact;
static egui_view_linearlayout_t read_only_column;
static egui_view_combobox_t control_read_only;
static egui_view_api_t control_compact_api;
static egui_view_api_t control_read_only_api;
static uint8_t ui_ready = 0;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Combo Box";

static const char *primary_items_0[] = {"Personal", "Work", "Travel", "Archive"};
static const char *primary_items_1[] = {"Balanced", "Detailed", "Compact"};
static const char *primary_items_2[] = {"Today", "This week", "This month", "Quarter"};

static const combo_box_snapshot_t primary_snapshots[] = {
        {primary_items_0, 4, 1},
        {primary_items_1, 3, 0},
        {primary_items_2, 4, 2},
};

static const char *compact_items_0[] = {"Auto", "Manual"};
static const char *compact_items_1[] = {"Light", "Dark"};

static const combo_box_snapshot_t compact_snapshots[] = {
        {compact_items_0, 2, 0},
        {compact_items_1, 2, 1},
};

static const char *read_only_items[] = {"Desktop", "Tablet", "Mobile"};

static void relayout_demo(void)
{
    if (!ui_ready)
    {
        return;
    }

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
}

static void apply_snapshot(egui_view_t *view, const combo_box_snapshot_t *snapshot)
{
    hcw_combo_box_set_items(view, snapshot->items, snapshot->item_count);
    hcw_combo_box_set_current_index(view, snapshot->selected_index);
    egui_view_combobox_collapse(view);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&control_primary), &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)]);
    relayout_demo();
}

static void apply_compact_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&control_compact), &compact_snapshots[index % EGUI_ARRAY_SIZE(compact_snapshots)]);
}

static void apply_read_only_state(void)
{
    hcw_combo_box_set_items(EGUI_VIEW_OF(&control_read_only), read_only_items, EGUI_ARRAY_SIZE(read_only_items));
    hcw_combo_box_set_current_index(EGUI_VIEW_OF(&control_read_only), 1);
    egui_view_combobox_collapse(EGUI_VIEW_OF(&control_read_only));
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), COMBO_BOX_ROOT_WIDTH, COMBO_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), COMBO_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_combobox_init(EGUI_VIEW_OF(&control_primary));
    egui_view_set_size(EGUI_VIEW_OF(&control_primary), COMBO_BOX_PRIMARY_WIDTH, COMBO_BOX_PRIMARY_HEIGHT);
    egui_view_combobox_set_font(EGUI_VIEW_OF(&control_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_combo_box_apply_standard_style(EGUI_VIEW_OF(&control_primary));
    egui_view_set_margin(EGUI_VIEW_OF(&control_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&control_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), COMBO_BOX_BOTTOM_ROW_WIDTH, COMBO_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), COMBO_BOX_PREVIEW_WIDTH, COMBO_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_combobox_init(EGUI_VIEW_OF(&control_compact));
    egui_view_set_size(EGUI_VIEW_OF(&control_compact), COMBO_BOX_PREVIEW_WIDTH, COMBO_BOX_PREVIEW_HEIGHT);
    egui_view_combobox_set_font(EGUI_VIEW_OF(&control_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_combo_box_apply_compact_style(EGUI_VIEW_OF(&control_compact));
    hcw_combo_box_override_static_preview_api(EGUI_VIEW_OF(&control_compact), &control_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&control_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), COMBO_BOX_PREVIEW_WIDTH, COMBO_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_combobox_init(EGUI_VIEW_OF(&control_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&control_read_only), COMBO_BOX_PREVIEW_WIDTH, COMBO_BOX_PREVIEW_HEIGHT);
    egui_view_combobox_set_font(EGUI_VIEW_OF(&control_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_combo_box_apply_read_only_style(EGUI_VIEW_OF(&control_read_only));
    hcw_combo_box_override_static_preview_api(EGUI_VIEW_OF(&control_read_only), &control_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_read_only), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&control_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_read_only_state();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&compact_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&read_only_column));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
    ui_ready = 1;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&control_primary), &event);

    event.type = EGUI_KEY_EVENT_ACTION_UP;
    egui_view_dispatch_key_event(EGUI_VIEW_OF(&control_primary), &event);
    relayout_demo();
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
            apply_primary_snapshot(0);
            apply_compact_snapshot(0);
            apply_read_only_state();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_WAIT);
        return true;
    case 11:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, COMBO_BOX_RECORD_FRAME_WAIT);
        return true;
    case 12:
        if (first_call)
        {
            apply_compact_snapshot(1);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
