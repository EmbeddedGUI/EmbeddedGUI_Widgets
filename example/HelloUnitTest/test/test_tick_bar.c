#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_tick_bar.h"

#include "../../HelloCustomWidgets/input/tick_bar/egui_view_tick_bar.h"
#include "../../HelloCustomWidgets/input/tick_bar/egui_view_tick_bar.c"

typedef struct tick_bar_preview_snapshot tick_bar_preview_snapshot_t;
struct tick_bar_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    int16_t minimum;
    int16_t maximum;
    int16_t value;
    int16_t selection_start;
    int16_t selection_end;
    uint8_t tick_frequency;
    uint8_t placement;
    uint8_t reversed;
    uint8_t show_selected_range;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_color_t rail_color;
    egui_color_t tick_color;
    egui_color_t selected_tick_color;
    egui_color_t value_color;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_margin_padding_t padding_left;
    egui_dim_margin_padding_t padding_right;
    egui_dim_margin_padding_t padding_top;
    egui_dim_margin_padding_t padding_bottom;
};

static egui_view_tick_bar_t test_control;
static egui_view_tick_bar_t preview_control;
static egui_view_api_t preview_api;

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_tick_bar(void)
{
    egui_view_tick_bar_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 170, 64);
}

static void setup_preview_control(void)
{
    egui_view_tick_bar_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 72, 34);
    egui_view_tick_bar_apply_compact_style(EGUI_VIEW_OF(&preview_control));
    egui_view_tick_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
}

static void layout_view(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static int send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->dispatch_touch_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= view->api->dispatch_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->dispatch_key_event(view, &event);
    return handled;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(tick_bar_preview_snapshot_t *snapshot)
{
    int16_t selection_start = 0;
    int16_t selection_end = 0;

    egui_view_tick_bar_get_selection_range(EGUI_VIEW_OF(&preview_control), &selection_start, &selection_end);
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->minimum = egui_view_tick_bar_get_minimum(EGUI_VIEW_OF(&preview_control));
    snapshot->maximum = egui_view_tick_bar_get_maximum(EGUI_VIEW_OF(&preview_control));
    snapshot->value = egui_view_tick_bar_get_value(EGUI_VIEW_OF(&preview_control));
    snapshot->selection_start = selection_start;
    snapshot->selection_end = selection_end;
    snapshot->tick_frequency = egui_view_tick_bar_get_tick_frequency(EGUI_VIEW_OF(&preview_control));
    snapshot->placement = egui_view_tick_bar_get_placement(EGUI_VIEW_OF(&preview_control));
    snapshot->reversed = egui_view_tick_bar_get_reversed(EGUI_VIEW_OF(&preview_control));
    snapshot->show_selected_range = egui_view_tick_bar_get_show_selected_range(EGUI_VIEW_OF(&preview_control));
    snapshot->compact_mode = egui_view_tick_bar_get_compact_mode(EGUI_VIEW_OF(&preview_control));
    snapshot->read_only_mode = egui_view_tick_bar_get_read_only_mode(EGUI_VIEW_OF(&preview_control));
    snapshot->rail_color = preview_control.rail_color;
    snapshot->tick_color = preview_control.tick_color;
    snapshot->selected_tick_color = preview_control.selected_tick_color;
    snapshot->value_color = preview_control.value_color;
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_control)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_control)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_control)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_control)->padding.bottom;
}

static void assert_preview_state_unchanged(const tick_bar_preview_snapshot_t *snapshot)
{
    int16_t selection_start = 0;
    int16_t selection_end = 0;

    egui_view_tick_bar_get_selection_range(EGUI_VIEW_OF(&preview_control), &selection_start, &selection_end);
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->minimum, egui_view_tick_bar_get_minimum(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->maximum, egui_view_tick_bar_get_maximum(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->value, egui_view_tick_bar_get_value(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->selection_start, selection_start);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->selection_end, selection_end);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->tick_frequency, egui_view_tick_bar_get_tick_frequency(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->placement, egui_view_tick_bar_get_placement(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->reversed, egui_view_tick_bar_get_reversed(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->show_selected_range, egui_view_tick_bar_get_show_selected_range(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, egui_view_tick_bar_get_compact_mode(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, egui_view_tick_bar_get_read_only_mode(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->rail_color.full, preview_control.rail_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->tick_color.full, preview_control.tick_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->selected_tick_color.full, preview_control.selected_tick_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->value_color.full, preview_control.value_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_control)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_control)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_control)->padding.bottom);
}

static void assert_selection(egui_view_t *view, int16_t expected_start, int16_t expected_end)
{
    int16_t selection_start = 0;
    int16_t selection_end = 0;

    egui_view_tick_bar_get_selection_range(view, &selection_start, &selection_end);
    EGUI_TEST_ASSERT_EQUAL_INT(expected_start, selection_start);
    EGUI_TEST_ASSERT_EQUAL_INT(expected_end, selection_end);
}

static void test_tick_bar_init_defaults(void)
{
    setup_tick_bar();

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tick_bar_get_minimum(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(100, egui_view_tick_bar_get_maximum(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(40, egui_view_tick_bar_get_value(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_tick_bar_get_tick_frequency(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TICK_BAR_PLACEMENT_BOTTOM, egui_view_tick_bar_get_placement(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tick_bar_get_reversed(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tick_bar_get_show_selected_range(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tick_bar_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tick_bar_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    assert_selection(EGUI_VIEW_OF(&test_control), 20, 70);
}

static void test_tick_bar_setters_clamp_and_normalize(void)
{
    setup_tick_bar();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_tick_bar_set_range(EGUI_VIEW_OF(&test_control), 50, 10);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(50, egui_view_tick_bar_get_minimum(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(51, egui_view_tick_bar_get_maximum(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(50, egui_view_tick_bar_get_value(EGUI_VIEW_OF(&test_control)));

    egui_view_tick_bar_set_range(EGUI_VIEW_OF(&test_control), -20, 20);
    egui_view_tick_bar_set_value(EGUI_VIEW_OF(&test_control), 45);
    EGUI_TEST_ASSERT_EQUAL_INT(20, egui_view_tick_bar_get_value(EGUI_VIEW_OF(&test_control)));
    egui_view_tick_bar_set_value(EGUI_VIEW_OF(&test_control), -45);
    EGUI_TEST_ASSERT_EQUAL_INT(-20, egui_view_tick_bar_get_value(EGUI_VIEW_OF(&test_control)));

    egui_view_tick_bar_set_selection_range(EGUI_VIEW_OF(&test_control), 18, -12);
    assert_selection(EGUI_VIEW_OF(&test_control), -12, 18);

    egui_view_tick_bar_set_tick_frequency(EGUI_VIEW_OF(&test_control), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tick_bar_get_tick_frequency(EGUI_VIEW_OF(&test_control)));
    egui_view_tick_bar_set_placement(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TICK_BAR_PLACEMENT_BOTTOM, egui_view_tick_bar_get_placement(EGUI_VIEW_OF(&test_control)));
}

static void test_tick_bar_styles(void)
{
    setup_tick_bar();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_tick_bar_apply_accent_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_tick_bar_get_maximum(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(75, egui_view_tick_bar_get_value(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(15, egui_view_tick_bar_get_tick_frequency(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TICK_BAR_PLACEMENT_TOP, egui_view_tick_bar_get_placement(EGUI_VIEW_OF(&test_control)));

    egui_view_tick_bar_apply_vertical_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TICK_BAR_PLACEMENT_LEFT, egui_view_tick_bar_get_placement(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tick_bar_get_reversed(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_tick_bar_get_value(EGUI_VIEW_OF(&test_control)));
    assert_selection(EGUI_VIEW_OF(&test_control), 3, 7);

    egui_view_tick_bar_apply_read_only_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TICK_BAR_PLACEMENT_RIGHT, egui_view_tick_bar_get_placement(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tick_bar_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tick_bar_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
}

static void test_tick_bar_static_preview_consumes_input_and_keeps_state(void)
{
    tick_bar_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_control();
    layout_view(EGUI_VIEW_OF(&preview_control), 12, 18, 72, 34);
    get_view_center(EGUI_VIEW_OF(&preview_control), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_control), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_control), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_control), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_control), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_control), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_tick_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(tick_bar);
    EGUI_TEST_RUN(test_tick_bar_init_defaults);
    EGUI_TEST_RUN(test_tick_bar_setters_clamp_and_normalize);
    EGUI_TEST_RUN(test_tick_bar_styles);
    EGUI_TEST_RUN(test_tick_bar_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
