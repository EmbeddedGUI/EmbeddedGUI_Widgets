#include "egui.h"
#include "egui_view_rating_control.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define RATING_ROOT_WIDTH        224
#define RATING_ROOT_HEIGHT       174
#define RATING_PRIMARY_WIDTH     196
#define RATING_PRIMARY_HEIGHT    92
#define RATING_PREVIEW_WIDTH     104
#define RATING_PREVIEW_HEIGHT    42
#define RATING_BOTTOM_ROW_WIDTH  216
#define RATING_BOTTOM_ROW_HEIGHT 42
#define RATING_RECORD_WAIT       90
#define RATING_RECORD_FRAME_WAIT 130

typedef struct rating_demo_snapshot rating_demo_snapshot_t;
struct rating_demo_snapshot
{
    const char *title;
    const char *low_label;
    const char *high_label;
    const char **value_labels;
    uint8_t label_count;
    uint8_t item_count;
    uint8_t value;
    uint8_t clear_enabled;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_rating_control_t control_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_rating_control_t control_compact;
static egui_view_rating_control_t control_read_only;
static egui_view_api_t compact_api;
static egui_view_api_t read_only_api;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Rating Control";

static const char *primary_labels_0[] = {"No rating", "Poor", "Fair", "Good", "Great", "Excellent"};
static const char *primary_labels_1[] = {"No rating", "Very slow", "Slow", "On time", "Fast", "Rapid"};
static const char *primary_labels_2[] = {"No rating", "Hard", "Rough", "Okay", "Smooth", "Effortless"};

static const rating_demo_snapshot_t primary_snapshots[] = {
        {"Service quality", "Low", "High", primary_labels_0, 6, 5, 4, 1},
        {"Delivery speed", "Slow", "Fast", primary_labels_1, 6, 5, 2, 1},
        {"Setup experience", "Hard", "Easy", primary_labels_2, 6, 5, 5, 1},
};

static const char *compact_labels_0[] = {"No rating", "Poor", "Fair", "Good", "Great", "Excellent"};
static const char *compact_labels_1[] = {"No rating", "Weak", "Okay", "Good", "Strong", "Top"};

static const rating_demo_snapshot_t compact_snapshots[] = {
        {"", "", "", compact_labels_0, 6, 5, 3, 0},
        {"", "", "", compact_labels_1, 6, 5, 4, 0},
};

static const rating_demo_snapshot_t read_only_snapshot = {"", "", "", primary_labels_0, 6, 5, 4, 0};

static void dismiss_primary_rating_control(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_clear_focus(EGUI_VIEW_OF(&control_primary));
#endif
}

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        dismiss_primary_rating_control();
    }
    return 1;
}

static void apply_snapshot(egui_view_t *view, const rating_demo_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return;
    }

    egui_view_rating_control_set_title(view, snapshot->title);
    egui_view_rating_control_set_low_label(view, snapshot->low_label);
    egui_view_rating_control_set_high_label(view, snapshot->high_label);
    egui_view_rating_control_set_value_labels(view, snapshot->value_labels, snapshot->label_count);
    egui_view_rating_control_set_item_count(view, snapshot->item_count);
    egui_view_rating_control_set_clear_enabled(view, snapshot->clear_enabled);
    egui_view_rating_control_set_value(view, snapshot->value);
    egui_view_rating_control_set_current_part(view, snapshot->value > 0 ? snapshot->value : 1);
}

static void apply_primary_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&control_primary), &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)]);
}

static void apply_compact_snapshot(uint8_t index)
{
    apply_snapshot(EGUI_VIEW_OF(&control_compact), &compact_snapshots[index % EGUI_ARRAY_SIZE(compact_snapshots)]);
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&control_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&control_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&control_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&control_primary), &event);
}

static void set_click_rating_part(egui_sim_action_t *p_action, egui_view_t *view, uint8_t part, int interval_ms)
{
    egui_region_t region;

    if (!egui_view_rating_control_get_part_region(view, part, &region))
    {
        EGUI_SIM_SET_WAIT(p_action, interval_ms);
        return;
    }

    p_action->type = EGUI_SIM_ACTION_CLICK;
    p_action->x1 = region.location.x + region.size.width / 2;
    p_action->y1 = region.location.y + region.size.height / 2;
    p_action->x2 = 0;
    p_action->y2 = 0;
    p_action->steps = 0;
    p_action->interval_ms = interval_ms;
}
#endif

void test_init_ui(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), RATING_ROOT_WIDTH, RATING_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), RATING_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 6);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_rating_control_init(EGUI_VIEW_OF(&control_primary));
    egui_view_set_size(EGUI_VIEW_OF(&control_primary), RATING_PRIMARY_WIDTH, RATING_PRIMARY_HEIGHT);
    egui_view_rating_control_set_font(EGUI_VIEW_OF(&control_primary), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_rating_control_set_meta_font(EGUI_VIEW_OF(&control_primary), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rating_control_set_palette(EGUI_VIEW_OF(&control_primary), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD7DFE7), EGUI_COLOR_HEX(0x1E2832),
                                         EGUI_COLOR_HEX(0x71808F), EGUI_COLOR_HEX(0xD59E20), EGUI_COLOR_HEX(0xDBE3EC));
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&control_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&control_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), RATING_BOTTOM_ROW_WIDTH, RATING_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_rating_control_init(EGUI_VIEW_OF(&control_compact));
    egui_view_set_size(EGUI_VIEW_OF(&control_compact), RATING_PREVIEW_WIDTH, RATING_PREVIEW_HEIGHT);
    egui_view_rating_control_set_font(EGUI_VIEW_OF(&control_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rating_control_set_meta_font(EGUI_VIEW_OF(&control_compact), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rating_control_set_compact_mode(EGUI_VIEW_OF(&control_compact), 1);
    egui_view_rating_control_set_palette(EGUI_VIEW_OF(&control_compact), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xDADFE5), EGUI_COLOR_HEX(0x2B3138),
                                         EGUI_COLOR_HEX(0x74808C), EGUI_COLOR_HEX(0xC78C16), EGUI_COLOR_HEX(0xE0E6EC));
    egui_view_rating_control_override_static_preview_api(EGUI_VIEW_OF(&control_compact), &compact_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&control_compact));

    egui_view_rating_control_init(EGUI_VIEW_OF(&control_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&control_read_only), RATING_PREVIEW_WIDTH, RATING_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&control_read_only), 8, 0, 0, 0);
    egui_view_rating_control_set_font(EGUI_VIEW_OF(&control_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rating_control_set_meta_font(EGUI_VIEW_OF(&control_read_only), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rating_control_set_compact_mode(EGUI_VIEW_OF(&control_read_only), 1);
    egui_view_rating_control_set_read_only_mode(EGUI_VIEW_OF(&control_read_only), 1);
    egui_view_rating_control_set_palette(EGUI_VIEW_OF(&control_read_only), EGUI_COLOR_HEX(0xFBFCFD), EGUI_COLOR_HEX(0xD9E1E8), EGUI_COLOR_HEX(0x596878),
                                         EGUI_COLOR_HEX(0x8C98A5), EGUI_COLOR_HEX(0x9DA9B5), EGUI_COLOR_HEX(0xE3E9EF));
    egui_view_rating_control_override_static_preview_api(EGUI_VIEW_OF(&control_read_only), &read_only_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&control_read_only));

    apply_primary_snapshot(0);
    apply_compact_snapshot(0);
    apply_snapshot(EGUI_VIEW_OF(&control_read_only), &read_only_snapshot);

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
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
        }
        EGUI_SIM_SET_WAIT(p_action, RATING_RECORD_WAIT);
        return true;
    case 1:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RATING_RECORD_FRAME_WAIT);
        return true;
    case 2:
        set_click_rating_part(p_action, EGUI_VIEW_OF(&control_primary), 5, RATING_RECORD_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RATING_RECORD_FRAME_WAIT);
        return true;
    case 4:
        set_click_rating_part(p_action, EGUI_VIEW_OF(&control_primary), EGUI_VIEW_RATING_CONTROL_PART_CLEAR, RATING_RECORD_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RATING_RECORD_FRAME_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            apply_primary_snapshot(1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
            egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, RATING_RECORD_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            recording_request_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, RATING_RECORD_FRAME_WAIT);
        return true;
    case 8:
        if (first_call)
        {
            apply_compact_snapshot(1);
        }
        EGUI_SIM_SET_WAIT(p_action, RATING_RECORD_WAIT);
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
