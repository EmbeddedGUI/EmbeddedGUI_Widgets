#include "egui.h"
#include "egui_view_segmented_control.h"
#include "uicode.h"
#include "demo_scaffold.h"

#if EGUI_CONFIG_RECORDING_TEST
#include "core/egui_input_simulator.h"
#endif

#define SEGMENTED_CONTROL_ROOT_WIDTH        224
#define SEGMENTED_CONTROL_ROOT_HEIGHT       112
#define SEGMENTED_CONTROL_PRIMARY_WIDTH     196
#define SEGMENTED_CONTROL_PRIMARY_HEIGHT    38
#define SEGMENTED_CONTROL_PREVIEW_WIDTH     104
#define SEGMENTED_CONTROL_PREVIEW_HEIGHT    30
#define SEGMENTED_CONTROL_BOTTOM_ROW_WIDTH  216
#define SEGMENTED_CONTROL_BOTTOM_ROW_HEIGHT 30
#define SEGMENTED_CONTROL_RECORD_WAIT       120
#define SEGMENTED_CONTROL_RECORD_FRAME_WAIT 150

typedef struct segmented_demo_snapshot segmented_demo_snapshot_t;
struct segmented_demo_snapshot
{
    const char **segments;
    uint8_t segment_count;
    uint8_t selected_index;
};

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_segmented_control_t control_primary;
static egui_view_linearlayout_t bottom_row;
static egui_view_segmented_control_t control_compact;
static egui_view_segmented_control_t control_read_only;
static egui_view_api_t control_primary_api;
static egui_view_api_t control_compact_api;
static egui_view_api_t control_read_only_api;
static uint8_t ui_ready;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_page_panel_param, EGUI_COLOR_HEX(0xF5F7F9), EGUI_ALPHA_100, 14);
EGUI_BACKGROUND_PARAM_INIT(bg_page_panel_params, &bg_page_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_page_panel, &bg_page_panel_params);

static const char *title_text = "Segmented Control";

static const char *primary_segments_0[] = {"Overview", "Team", "Usage", "Access"};
static const char *primary_segments_1[] = {"Live", "Pending", "History"};
static const char *primary_segments_2[] = {"Day", "Week", "Month", "Year"};

static const segmented_demo_snapshot_t primary_snapshots[] = {
        {primary_segments_0, 4, 1},
        {primary_segments_1, 3, 0},
        {primary_segments_2, 4, 2},
};

static const char *compact_segments_0[] = {"Day", "Week"};
static const char *compact_segments_1[] = {"Low", "Mid", "High"};

static const segmented_demo_snapshot_t compact_snapshots[] = {
        {compact_segments_0, 2, 0},
        {compact_segments_1, 3, 1},
};

static const char *read_only_segments[] = {"Off", "Auto", "Lock"};

static void layout_page(void);

static int dismiss_primary_focus_on_preview_touch(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(self);

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    if (event->type == EGUI_MOTION_EVENT_ACTION_DOWN)
    {
        egui_view_clear_focus(EGUI_VIEW_OF(&control_primary));
    }
#else
    EGUI_UNUSED(event);
#endif
    return 1;
}

static void apply_snapshot_to_control(egui_view_t *view, const segmented_demo_snapshot_t *snapshot)
{
    hcw_segmented_control_set_segments(view, snapshot->segments, snapshot->segment_count);
    hcw_segmented_control_set_current_index(view, snapshot->selected_index);
}

static void apply_primary_snapshot(uint8_t index)
{
    const segmented_demo_snapshot_t *snapshot = &primary_snapshots[index % EGUI_ARRAY_SIZE(primary_snapshots)];

    apply_snapshot_to_control(EGUI_VIEW_OF(&control_primary), snapshot);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_default_state(void)
{
    apply_primary_snapshot(0);
}

static void apply_preview_states(void)
{
    apply_snapshot_to_control(EGUI_VIEW_OF(&control_compact), &compact_snapshots[0]);
    hcw_segmented_control_set_segments(EGUI_VIEW_OF(&control_read_only), read_only_segments, EGUI_ARRAY_SIZE(read_only_segments));
    hcw_segmented_control_set_current_index(EGUI_VIEW_OF(&control_read_only), 1);
    if (ui_ready)
    {
        layout_page();
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

static void focus_primary_control(void)
{
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_request_focus(EGUI_VIEW_OF(&control_primary));
#endif
}

#if EGUI_CONFIG_RECORDING_TEST
static void apply_compact_snapshot(uint8_t index)
{
    const segmented_demo_snapshot_t *snapshot = &compact_snapshots[index % EGUI_ARRAY_SIZE(compact_snapshots)];

    apply_snapshot_to_control(EGUI_VIEW_OF(&control_compact), snapshot);
    if (ui_ready)
    {
        layout_page();
    }
}

static void apply_primary_key(uint8_t key_code)
{
    egui_key_event_t event = {0};

    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    EGUI_VIEW_OF(&control_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&control_primary), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    EGUI_VIEW_OF(&control_primary)->api->dispatch_key_event(EGUI_VIEW_OF(&control_primary), &event);
    if (ui_ready)
    {
        layout_page();
    }
}

static int resolve_segment_gap(int content_width, uint8_t count, int gap)
{
    if (count <= 1)
    {
        return 0;
    }
    while (gap > 0 && (content_width - gap * (count - 1)) < count)
    {
        gap--;
    }
    return gap;
}

static void get_segment_center(egui_view_t *view, uint8_t count, uint8_t index, uint8_t horizontal_padding, uint8_t segment_gap, int *x, int *y)
{
    int width;
    int height;
    int padding;
    int content_width;
    int content_height;
    int gap;
    int available_width;
    int base_width;
    int remainder;
    int cursor_x;
    uint8_t i;

    if (count == 0)
    {
        *x = view->region_screen.location.x + view->region_screen.size.width / 2;
        *y = view->region_screen.location.y + view->region_screen.size.height / 2;
        return;
    }
    if (index >= count)
    {
        index = count - 1;
    }

    width = view->region_screen.size.width;
    height = view->region_screen.size.height;
    padding = horizontal_padding;
    if (padding * 2 >= width || padding * 2 >= height)
    {
        padding = 0;
    }

    content_width = width - padding * 2;
    content_height = height - padding * 2;
    if (content_width <= 0 || content_height <= 0)
    {
        *x = view->region_screen.location.x + width / 2;
        *y = view->region_screen.location.y + height / 2;
        return;
    }

    gap = resolve_segment_gap(content_width, count, segment_gap);
    available_width = content_width - gap * (count - 1);
    if (available_width < count)
    {
        *x = view->region_screen.location.x + width / 2;
        *y = view->region_screen.location.y + height / 2;
        return;
    }

    base_width = available_width / count;
    remainder = available_width % count;
    cursor_x = view->region_screen.location.x + padding;
    for (i = 0; i < count; i++)
    {
        int segment_width = base_width;

        if (remainder > 0)
        {
            segment_width++;
            remainder--;
        }
        if (i == index)
        {
            *x = cursor_x + segment_width / 2;
            *y = view->region_screen.location.y + padding + content_height / 2;
            return;
        }
        cursor_x += segment_width + gap;
    }

    *x = view->region_screen.location.x + width / 2;
    *y = view->region_screen.location.y + height / 2;
}

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

    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), SEGMENTED_CONTROL_ROOT_WIDTH, SEGMENTED_CONTROL_ROOT_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);
    egui_view_set_background(EGUI_VIEW_OF(&root_layout), EGUI_BG_OF(&bg_page_panel));

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), SEGMENTED_CONTROL_ROOT_WIDTH, 18);
    egui_view_label_set_text(EGUI_VIEW_OF(&title_label), title_text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(&title_label), EGUI_ALIGN_CENTER);
    egui_view_label_set_font(EGUI_VIEW_OF(&title_label), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_label_set_font_color(EGUI_VIEW_OF(&title_label), EGUI_COLOR_HEX(0x21303F), EGUI_ALPHA_100);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 0, 8, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_segmented_control_init(EGUI_VIEW_OF(&control_primary));
    egui_view_set_size(EGUI_VIEW_OF(&control_primary), SEGMENTED_CONTROL_PRIMARY_WIDTH, SEGMENTED_CONTROL_PRIMARY_HEIGHT);
    egui_view_segmented_control_set_font(EGUI_VIEW_OF(&control_primary), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_segmented_control_apply_standard_style(EGUI_VIEW_OF(&control_primary));
    hcw_segmented_control_override_interaction_api(EGUI_VIEW_OF(&control_primary), &control_primary_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_primary), 1);
#endif
    egui_view_set_margin(EGUI_VIEW_OF(&control_primary), 0, 0, 0, 8);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&control_primary));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&bottom_row));
    egui_view_set_size(EGUI_VIEW_OF(&bottom_row), SEGMENTED_CONTROL_BOTTOM_ROW_WIDTH, SEGMENTED_CONTROL_BOTTOM_ROW_HEIGHT);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&bottom_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&bottom_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&bottom_row));

    egui_view_segmented_control_init(EGUI_VIEW_OF(&control_compact));
    egui_view_set_size(EGUI_VIEW_OF(&control_compact), SEGMENTED_CONTROL_PREVIEW_WIDTH, SEGMENTED_CONTROL_PREVIEW_HEIGHT);
    egui_view_segmented_control_set_font(EGUI_VIEW_OF(&control_compact), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_segmented_control_apply_compact_style(EGUI_VIEW_OF(&control_compact));
    hcw_segmented_control_override_static_preview_api(EGUI_VIEW_OF(&control_compact), &control_compact_api);
    control_compact_api.on_touch = dismiss_primary_focus_on_preview_touch;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_compact), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&control_compact));

    egui_view_segmented_control_init(EGUI_VIEW_OF(&control_read_only));
    egui_view_set_size(EGUI_VIEW_OF(&control_read_only), SEGMENTED_CONTROL_PREVIEW_WIDTH, SEGMENTED_CONTROL_PREVIEW_HEIGHT);
    egui_view_set_margin(EGUI_VIEW_OF(&control_read_only), 8, 0, 0, 0);
    egui_view_segmented_control_set_font(EGUI_VIEW_OF(&control_read_only), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_segmented_control_apply_read_only_style(EGUI_VIEW_OF(&control_read_only));
    hcw_segmented_control_override_static_preview_api(EGUI_VIEW_OF(&control_read_only), &control_read_only_api);
    control_read_only_api.on_touch = dismiss_primary_focus_on_preview_touch;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&control_read_only), 0);
#endif
    egui_view_group_add_child(EGUI_VIEW_OF(&bottom_row), EGUI_VIEW_OF(&control_read_only));

    apply_primary_default_state();
    apply_preview_states();

    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), NULL, 0);

    layout_local_views();

    egui_core_add_user_root_view(EGUI_VIEW_OF(&root_layout));
    ui_ready = 1;
    apply_primary_default_state();
    apply_preview_states();
    focus_primary_control();
}

#if EGUI_CONFIG_RECORDING_TEST
bool egui_port_get_recording_action(int action_index, egui_sim_action_t *p_action)
{
    static int last_action = -1;
    int first_call = action_index != last_action;
    int x = 0;
    int y = 0;

    last_action = action_index;

    switch (action_index)
    {
    case 0:
        if (first_call)
        {
            apply_primary_default_state();
            apply_preview_states();
            focus_primary_control();
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SEGMENTED_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 1:
        get_segment_center(EGUI_VIEW_OF(&control_primary), 4, 2, 2, 2, &x, &y);
        p_action->type = EGUI_SIM_ACTION_CLICK;
        p_action->x1 = x;
        p_action->y1 = y;
        p_action->interval_ms = 240;
        return true;
    case 2:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SEGMENTED_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 3:
        if (first_call)
        {
            apply_primary_snapshot(1);
            focus_primary_control();
            apply_primary_key(EGUI_KEY_CODE_END);
        }
        EGUI_SIM_SET_WAIT(p_action, SEGMENTED_CONTROL_RECORD_WAIT);
        return true;
    case 4:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SEGMENTED_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 5:
        if (first_call)
        {
            apply_primary_snapshot(2);
            apply_compact_snapshot(1);
            focus_primary_control();
            apply_primary_key(EGUI_KEY_CODE_HOME);
        }
        EGUI_SIM_SET_WAIT(p_action, SEGMENTED_CONTROL_RECORD_WAIT);
        return true;
    case 6:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, SEGMENTED_CONTROL_RECORD_FRAME_WAIT);
        return true;
    case 7:
        if (first_call)
        {
            focus_primary_control();
        }
        EGUI_SIM_SET_WAIT(p_action, SEGMENTED_CONTROL_RECORD_WAIT);
        return true;
    case 8:
        get_segment_center(EGUI_VIEW_OF(&control_compact), 3, 1, 1, 1, &x, &y);
        p_action->type = EGUI_SIM_ACTION_CLICK;
        p_action->x1 = x;
        p_action->y1 = y;
        p_action->interval_ms = 220;
        return true;
    case 9:
        if (first_call)
        {
            request_page_snapshot();
        }
        EGUI_SIM_SET_WAIT(p_action, 520);
        return true;
    default:
        return false;
    }
}
#endif
