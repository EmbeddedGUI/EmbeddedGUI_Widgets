#include <string.h>

#include "egui.h"
#include "egui_view_check_box.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define CHECK_BOX_ROOT_WIDTH        224
#define CHECK_BOX_ROOT_HEIGHT       158
#define CHECK_BOX_PRIMARY_WIDTH     196
#define CHECK_BOX_PRIMARY_HEIGHT    34
#define CHECK_BOX_PREVIEW_WIDTH     104
#define CHECK_BOX_PREVIEW_HEIGHT    28
#define CHECK_BOX_BOTTOM_ROW_WIDTH  216
#define CHECK_BOX_BOTTOM_ROW_HEIGHT 28
#define CHECK_BOX_RECORD_WAIT       90
#define CHECK_BOX_RECORD_FRAME_WAIT 170

typedef struct check_box_snapshot check_box_snapshot_t;
struct check_box_snapshot
{
    const char *text;
    uint8_t checked;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_checkbox_t control_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_checkbox_t control_compact;
static egui_view_checkbox_t control_read_only;
static egui_view_api_t control_primary_api;
static egui_view_api_t control_compact_api;
static egui_view_api_t control_read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Check Box";

static const check_box_snapshot_t primary_snapshots[] = {
        {"Email alerts", 0},
        {"Offline sync", 0},
        {"Archive reports", 1},
};

static const check_box_snapshot_t compact_snapshots[] = {
        {"Auto", 1},
        {"Muted", 0},
};

static const check_box_snapshot_t read_only_snapshot = {
        "Accepted", 1,
};

static void apply_snapshot_to_box(egui_view_checkbox_t *box, const check_box_snapshot_t *snapshot)
{
    hcw_check_box_set_text(EGUI_VIEW_OF(box), snapshot->text);
    hcw_check_box_set_checked(EGUI_VIEW_OF(box), snapshot->checked);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot_to_box(&control_primary, &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)]);
}

static void apply_compact_snapshot(uint8_t index)
{
    apply_snapshot_to_box(&control_compact, &compact_snapshots[index % EGUI_ARRAY_SIZE(compact_snapshots)]);
}

static void apply_read_only_snapshot(void)
{
    apply_snapshot_to_box(&control_read_only, &read_only_snapshot);
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), CHECK_BOX_ROOT_WIDTH, CHECK_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), CHECK_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_checkbox_init(EGUI_VIEW_OF(&control_primary));
    egui_view_set_size(EGUI_VIEW_OF(&control_primary), CHECK_BOX_PRIMARY_WIDTH, CHECK_BOX_PRIMARY_HEIGHT);
    egui_view_checkbox_set_font(EGUI_VIEW_OF(&control_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_check_box_apply_standard_style(EGUI_VIEW_OF(&control_primary));
    hcw_check_box_override_interaction_api(EGUI_VIEW_OF(&control_primary), &control_primary_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&control_primary), 0, 0, 0, 12);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&control_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), CHECK_BOX_BOTTOM_ROW_WIDTH, CHECK_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_checkbox_init(EGUI_VIEW_OF(&control_compact));
    egui_view_set_size(EGUI_VIEW_OF(&control_compact), CHECK_BOX_PREVIEW_WIDTH, CHECK_BOX_PREVIEW_HEIGHT);
    egui_view_checkbox_set_font(EGUI_VIEW_OF(&control_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_check_box_apply_compact_style(EGUI_VIEW_OF(&control_compact));
    hcw_check_box_override_static_preview_api(EGUI_VIEW_OF(&control_compact), &control_compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&control_compact));

    egui_view_checkbox_init(EGUI_VIEW_OF(&control_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&control_read_only), CHECK_BOX_PREVIEW_WIDTH, CHECK_BOX_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&control_read_only), 8, 0, 0, 0);
    egui_view_checkbox_set_font(EGUI_VIEW_OF(&control_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_check_box_apply_read_only_style(EGUI_VIEW_OF(&control_read_only));
    hcw_check_box_override_static_preview_api(EGUI_VIEW_OF(&control_read_only), &control_read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&control_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_read_only_snapshot();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&bottom_row));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    egui_core_layout_childs_user_root_view(EGUI_LAYOUT_VERTICAL, EGUI_ALIGN_HCENTER | EGUI_ALIGN_VCENTER);
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
            apply_read_only_snapshot();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CHECK_BOX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, CHECK_BOX_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CHECK_BOX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CHECK_BOX_RECORD_FRAME_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, CHECK_BOX_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, CHECK_BOX_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_compact_snapshot(1);
            apply_primary_snapshot(2);
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
