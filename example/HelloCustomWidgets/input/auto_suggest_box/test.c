#include <string.h>

#include "egui.h"
#include "egui_view_auto_suggest_box.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define AUTO_SUGGEST_BOX_ROOT_WIDTH        224
#define AUTO_SUGGEST_BOX_ROOT_HEIGHT       206
#define AUTO_SUGGEST_BOX_PRIMARY_WIDTH     196
#define AUTO_SUGGEST_BOX_PRIMARY_HEIGHT    34
#define AUTO_SUGGEST_BOX_PREVIEW_WIDTH     104
#define AUTO_SUGGEST_BOX_PREVIEW_HEIGHT    28
#define AUTO_SUGGEST_BOX_BOTTOM_ROW_WIDTH  216
#define AUTO_SUGGEST_BOX_BOTTOM_ROW_HEIGHT 28
#define AUTO_SUGGEST_BOX_RECORD_WAIT       90
#define AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT 170

typedef struct auto_suggest_snapshot auto_suggest_snapshot_t;
struct auto_suggest_snapshot
{
    const char **suggestions;
    uint8_t suggestion_count;
    uint8_t selected_index;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_autocomplete_t control_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_linearlayout_t compact_column;
static egui_view_autocomplete_t control_compact;
static egui_view_linearlayout_t read_only_column;
static egui_view_autocomplete_t control_read_only;
static uint8_t ui_ready = 0;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "AutoSuggest Box";

static const char *primary_suggestions_0[] = {"Alice Chen", "Alicia Gomez", "Allen Park", "Amelia Stone"};
static const char *primary_suggestions_1[] = {"Deploy API", "Deploy Docs", "Deploy Worker"};
static const char *primary_suggestions_2[] = {"Daily Standup", "Demo Sync", "Design Review", "Docs Review"};

static const auto_suggest_snapshot_t primary_snapshots[] = {
        {primary_suggestions_0, 4, 1},
        {primary_suggestions_1, 3, 0},
        {primary_suggestions_2, 4, 2},
};

static const char *compact_suggestions_0[] = {"Recent", "Pinned"};
static const char *compact_suggestions_1[] = {"Manual", "Auto"};

static const auto_suggest_snapshot_t compact_snapshots[] = {
        {compact_suggestions_0, 2, 0},
        {compact_suggestions_1, 2, 1},
};

static const char *read_only_suggestions[] = {"Pinned", "Recent", "Saved"};

static int consume_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(event);
    return 1;
}

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

static void apply_snapshot(egui_view_t *view, const auto_suggest_snapshot_t *snapshot)
{
    egui_view_autocomplete_set_suggestions(view, snapshot->suggestions, snapshot->suggestion_count);
    egui_view_autocomplete_set_current_index(view, snapshot->selected_index);
    egui_view_autocomplete_collapse(view);
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
    egui_view_autocomplete_set_suggestions(EGUI_VIEW_OF(&control_read_only), read_only_suggestions, EGUI_ARRAY_SIZE(read_only_suggestions));
    egui_view_autocomplete_set_current_index(EGUI_VIEW_OF(&control_read_only), 1);
    egui_view_autocomplete_collapse(EGUI_VIEW_OF(&control_read_only));
}

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), AUTO_SUGGEST_BOX_ROOT_WIDTH, AUTO_SUGGEST_BOX_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), AUTO_SUGGEST_BOX_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_autocomplete_init(EGUI_VIEW_OF(&control_primary));
    egui_view_set_size(EGUI_VIEW_OF(&control_primary), AUTO_SUGGEST_BOX_PRIMARY_WIDTH, AUTO_SUGGEST_BOX_PRIMARY_HEIGHT);
    egui_view_autocomplete_set_font(EGUI_VIEW_OF(&control_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_auto_suggest_box_apply_standard_style(EGUI_VIEW_OF(&control_primary));
    egui_view_set_margin(EGUI_VIEW_OF(&control_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&control_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), AUTO_SUGGEST_BOX_BOTTOM_ROW_WIDTH, AUTO_SUGGEST_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&compact_column));
    egui_view_set_size(EGUI_VIEW_OF(&compact_column), AUTO_SUGGEST_BOX_PREVIEW_WIDTH, AUTO_SUGGEST_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&compact_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&compact_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&compact_column));

    egui_view_autocomplete_init(EGUI_VIEW_OF(&control_compact));
    egui_view_set_size(EGUI_VIEW_OF(&control_compact), AUTO_SUGGEST_BOX_PREVIEW_WIDTH, AUTO_SUGGEST_BOX_PREVIEW_HEIGHT);
    egui_view_autocomplete_set_font(EGUI_VIEW_OF(&control_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_auto_suggest_box_apply_compact_style(EGUI_VIEW_OF(&control_compact));
    static egui_view_api_t control_compact_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&control_compact), &control_compact_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_compact), false);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&compact_column), EGUI_VIEW_OF(&control_compact));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&read_only_column));
    egui_view_set_size(EGUI_VIEW_OF(&read_only_column), AUTO_SUGGEST_BOX_PREVIEW_WIDTH, AUTO_SUGGEST_BOX_BOTTOM_ROW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&read_only_column), 8, 0, 0, 0);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&read_only_column), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&read_only_column), EGUI_ALIGN_HCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&read_only_column));

    egui_view_autocomplete_init(EGUI_VIEW_OF(&control_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&control_read_only), AUTO_SUGGEST_BOX_PREVIEW_WIDTH, AUTO_SUGGEST_BOX_PREVIEW_HEIGHT);
    egui_view_autocomplete_set_font(EGUI_VIEW_OF(&control_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_auto_suggest_box_apply_read_only_style(EGUI_VIEW_OF(&control_read_only));
    static egui_view_api_t control_read_only_touch_api;
    egui_view_override_api_on_touch(EGUI_VIEW_OF(&control_read_only), &control_read_only_touch_api, consume_preview_touch);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_read_only), false);
#endif
    egui_view_set_enable(EGUI_VIEW_OF(&control_read_only), false);
    egui_view_group_add_child(EGUI_VIEW_OF(&read_only_column), EGUI_VIEW_OF(&control_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_read_only_state();

    {
        hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);
    }

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
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            egui_view_autocomplete_expand(EGUI_VIEW_OF(&control_primary));
            relayout_demo();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 2:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_DOWN);
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            apply_primary_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 9:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_ENTER);
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 10:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 11:
        if (first_call)
        {
            apply_primary_key(EGUI_KEY_CODE_END);
            apply_primary_key(EGUI_KEY_CODE_SPACE);
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 12:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_FRAME_WAIT);
        return true;
    case 13:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, AUTO_SUGGEST_BOX_RECORD_WAIT);
        return true;
    case 14:
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
