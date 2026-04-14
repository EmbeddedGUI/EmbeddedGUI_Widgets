#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_annotated_scroll_bar.h"

#include "../../HelloCustomWidgets/navigation/annotated_scroll_bar/egui_view_annotated_scroll_bar.h"
#include "../../HelloCustomWidgets/navigation/annotated_scroll_bar/egui_view_annotated_scroll_bar.c"

static egui_view_annotated_scroll_bar_t test_bar;
static egui_view_annotated_scroll_bar_t preview_bar;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static egui_dim_t last_offset;
static egui_dim_t last_max_offset;
static uint8_t last_active_marker;
static uint8_t last_part;

static const egui_view_annotated_scroll_bar_marker_t test_markers[] = {
        {"A", "Intro", 0, EGUI_COLOR_HEX(0x2563EB)},
        {"B", "Board", 180, EGUI_COLOR_HEX(0x2563EB)},
        {"C", "Review", 420, EGUI_COLOR_HEX(0x2563EB)},
        {"D", "Wrap", 780, EGUI_COLOR_HEX(0x2563EB)},
};

static void on_changed(egui_view_t *self, egui_dim_t offset, egui_dim_t max_offset, uint8_t active_marker, uint8_t part)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_offset = offset;
    last_max_offset = max_offset;
    last_active_marker = active_marker;
    last_part = part;
}

static void reset_listener_state(void)
{
    changed_count = 0;
    last_offset = -1;
    last_max_offset = -1;
    last_active_marker = 0xFF;
    last_part = 0xFF;
}

static void setup_bar(egui_dim_t content_length, egui_dim_t viewport_length, egui_dim_t offset, egui_dim_t small_change, egui_dim_t large_change)
{
    egui_view_annotated_scroll_bar_init(EGUI_VIEW_OF(&test_bar));
    egui_view_set_size(EGUI_VIEW_OF(&test_bar), 144, 108);
    egui_view_annotated_scroll_bar_set_markers(EGUI_VIEW_OF(&test_bar), test_markers, (uint8_t)(sizeof(test_markers) / sizeof(test_markers[0])));
    egui_view_annotated_scroll_bar_set_content_metrics(EGUI_VIEW_OF(&test_bar), content_length, viewport_length);
    egui_view_annotated_scroll_bar_set_step_size(EGUI_VIEW_OF(&test_bar), small_change, large_change);
    egui_view_annotated_scroll_bar_set_offset(EGUI_VIEW_OF(&test_bar), offset);
    egui_view_annotated_scroll_bar_set_current_part(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL);
    egui_view_annotated_scroll_bar_set_on_changed_listener(EGUI_VIEW_OF(&test_bar), on_changed);
    reset_listener_state();
}

static void setup_preview_bar(void)
{
    egui_view_annotated_scroll_bar_init(EGUI_VIEW_OF(&preview_bar));
    egui_view_set_size(EGUI_VIEW_OF(&preview_bar), 104, 68);
    egui_view_annotated_scroll_bar_set_markers(EGUI_VIEW_OF(&preview_bar), test_markers, (uint8_t)(sizeof(test_markers) / sizeof(test_markers[0])));
    egui_view_annotated_scroll_bar_set_content_metrics(EGUI_VIEW_OF(&preview_bar), 1000, 200);
    egui_view_annotated_scroll_bar_set_step_size(EGUI_VIEW_OF(&preview_bar), 20, 120);
    egui_view_annotated_scroll_bar_set_offset(EGUI_VIEW_OF(&preview_bar), 420);
    egui_view_annotated_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&preview_bar), 1);
    egui_view_annotated_scroll_bar_set_current_part(EGUI_VIEW_OF(&preview_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL);
    egui_view_annotated_scroll_bar_set_on_changed_listener(EGUI_VIEW_OF(&preview_bar), on_changed);
    egui_view_annotated_scroll_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_bar), &preview_api);
    reset_listener_state();
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

static void layout_bar(void)
{
    layout_view(EGUI_VIEW_OF(&test_bar), 10, 20, 144, 108);
}

static void layout_preview_bar(void)
{
    layout_view(EGUI_VIEW_OF(&preview_bar), 8, 12, 104, 68);
}

static int send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
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

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_bar), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_bar), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_bar), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_bar), key_code);
}

static void test_annotated_scroll_bar_clamps_metrics_and_regions(void)
{
    egui_region_t region;

    setup_bar(10, 5, 3, 0, 0);
    egui_view_annotated_scroll_bar_set_content_metrics(EGUI_VIEW_OF(&test_bar), 0, 0);
    egui_view_annotated_scroll_bar_set_offset(EGUI_VIEW_OF(&test_bar), 90);
    layout_bar();
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_annotated_scroll_bar_get_marker_count(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_annotated_scroll_bar_get_content_length(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_annotated_scroll_bar_get_viewport_length(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_annotated_scroll_bar_get_max_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_marker_region(EGUI_VIEW_OF(&test_bar), 3, &region));
    EGUI_TEST_ASSERT_FALSE(egui_view_annotated_scroll_bar_get_marker_region(EGUI_VIEW_OF(&test_bar), 4, &region));
}

static void test_annotated_scroll_bar_setters_clear_pressed_state_and_clamp(void)
{
    setup_bar(1000, 200, 780, 20, 120);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_MARKER;
    test_bar.pressed_marker = 3;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_annotated_scroll_bar_set_markers(EGUI_VIEW_OF(&test_bar), test_markers, 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_annotated_scroll_bar_get_marker_count(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL;
    test_bar.pressed_marker = 1;
    test_bar.rail_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_annotated_scroll_bar_set_content_metrics(EGUI_VIEW_OF(&test_bar), 300, 120);
    EGUI_TEST_ASSERT_EQUAL_INT(300, egui_view_annotated_scroll_bar_get_content_length(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(120, egui_view_annotated_scroll_bar_get_viewport_length(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_annotated_scroll_bar_get_max_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE;
    test_bar.pressed_marker = 1;
    test_bar.rail_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_annotated_scroll_bar_set_step_size(EGUI_VIEW_OF(&test_bar), 0, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_bar.small_change);
    EGUI_TEST_ASSERT_EQUAL_INT(120, test_bar.large_change);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL;
    test_bar.pressed_marker = 1;
    test_bar.rail_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_annotated_scroll_bar_set_offset(EGUI_VIEW_OF(&test_bar), 180);
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_DECREASE;
    test_bar.pressed_marker = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_annotated_scroll_bar_set_current_part(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, egui_view_annotated_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE;
    test_bar.pressed_marker = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_annotated_scroll_bar_set_current_part(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, egui_view_annotated_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);
}

static void test_annotated_scroll_bar_tab_cycles_parts(void)
{
    setup_bar(1000, 200, 180, 20, 120);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, egui_view_annotated_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_handle_navigation_key(EGUI_VIEW_OF(&test_bar), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, egui_view_annotated_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_handle_navigation_key(EGUI_VIEW_OF(&test_bar), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_DECREASE, egui_view_annotated_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_handle_navigation_key(EGUI_VIEW_OF(&test_bar), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, egui_view_annotated_scroll_bar_get_current_part(EGUI_VIEW_OF(&test_bar)));
}

static void test_annotated_scroll_bar_keyboard_navigation(void)
{
    setup_bar(1000, 200, 180, 20, 120);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(200, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(320, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(800, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
}

static void test_annotated_scroll_bar_touch_increase_button(void)
{
    egui_region_t region;

    setup_bar(1000, 200, 180, 20, 120);
    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(200, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
}

static void test_annotated_scroll_bar_touch_marker_selects_offset(void)
{
    egui_region_t region;

    setup_bar(1000, 200, 180, 20, 120);
    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_marker_region(EGUI_VIEW_OF(&test_bar), 2, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(420, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_annotated_scroll_bar_get_active_marker(EGUI_VIEW_OF(&test_bar)));
}

static void test_annotated_scroll_bar_touch_drag_rail_reaches_end(void)
{
    egui_region_t rail_region;
    egui_dim_t x;
    egui_dim_t start_y;

    setup_bar(1000, 200, 180, 20, 120);
    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, &rail_region));
    x = rail_region.location.x + rail_region.size.width / 2;
    start_y = rail_region.location.y + 2;
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, start_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x, rail_region.location.y + rail_region.size.height - 1));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, rail_region.location.y + rail_region.size.height - 1));
    EGUI_TEST_ASSERT_EQUAL_INT(800, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
}

static void test_annotated_scroll_bar_marker_drag_switches_to_rail(void)
{
    egui_region_t marker_region;
    egui_region_t rail_region;
    egui_dim_t marker_x;
    egui_dim_t marker_y;
    egui_dim_t rail_x;
    egui_dim_t rail_end_y;

    setup_bar(1000, 200, 180, 20, 120);
    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_marker_region(EGUI_VIEW_OF(&test_bar), 1, &marker_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, &rail_region));

    marker_x = marker_region.location.x + marker_region.size.width / 2;
    marker_y = marker_region.location.y + marker_region.size.height / 2;
    rail_x = rail_region.location.x + rail_region.size.width / 2;
    rail_end_y = rail_region.location.y + rail_region.size.height - 1;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, marker_x, marker_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, rail_x, rail_end_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, rail_x, rail_end_y));
    EGUI_TEST_ASSERT_EQUAL_INT(800, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_annotated_scroll_bar_get_active_marker(EGUI_VIEW_OF(&test_bar)));
}

static void test_annotated_scroll_bar_touch_cancel_clears_pressed_state(void)
{
    egui_region_t rail_region;
    egui_dim_t x;
    egui_dim_t y;

    setup_bar(1000, 200, 180, 20, 120);
    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, &rail_region));
    x = rail_region.location.x + rail_region.size.width / 2;
    y = rail_region.location.y + rail_region.size.height - 1;
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
}

static void test_annotated_scroll_bar_compact_mode_clears_pressed_and_ignores_input(void)
{
    egui_region_t region;

    setup_bar(1000, 200, 180, 20, 120);
    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    egui_view_annotated_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_bar.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL;
    test_bar.pressed_marker = 2;
    test_bar.rail_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_annotated_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL;
    test_bar.pressed_marker = 1;
    test_bar.rail_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, &region));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));

    egui_view_annotated_scroll_bar_set_compact_mode(EGUI_VIEW_OF(&test_bar), 0);
    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(200, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
}

static void test_annotated_scroll_bar_read_only_mode_clears_pressed_and_ignores_input(void)
{
    egui_region_t region;

    setup_bar(1000, 200, 180, 20, 120);
    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    egui_view_annotated_scroll_bar_set_read_only_mode(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_bar.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL;
    test_bar.pressed_marker = 2;
    test_bar.rail_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    egui_view_annotated_scroll_bar_set_read_only_mode(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_MARKER;
    test_bar.pressed_marker = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, &region));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));

    egui_view_annotated_scroll_bar_set_read_only_mode(EGUI_VIEW_OF(&test_bar), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(200, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
}

static void test_annotated_scroll_bar_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_region_t region;

    setup_bar(1000, 200, 180, 20, 120);
    layout_bar();
    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_part_region(EGUI_VIEW_OF(&test_bar), EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_bar), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);

    test_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL;
    test_bar.pressed_marker = 1;
    test_bar.rail_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, test_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_bar)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(180, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));

    egui_view_set_enable(EGUI_VIEW_OF(&test_bar), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(200, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&test_bar)));
}

static void test_annotated_scroll_bar_static_preview_consumes_input_and_keeps_state(void)
{
    egui_region_t region;

    setup_preview_bar();
    layout_preview_bar();
    EGUI_TEST_ASSERT_EQUAL_INT(420, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(800, egui_view_annotated_scroll_bar_get_max_offset(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_annotated_scroll_bar_get_active_marker(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, egui_view_annotated_scroll_bar_get_current_part(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_bar.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_bar.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    EGUI_TEST_ASSERT_TRUE(egui_view_annotated_scroll_bar_get_marker_region(EGUI_VIEW_OF(&preview_bar), 2, &region));
    preview_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_MARKER;
    preview_bar.pressed_marker = 2;
    preview_bar.rail_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&preview_bar), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, preview_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(420, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(800, egui_view_annotated_scroll_bar_get_max_offset(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_annotated_scroll_bar_get_active_marker(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, egui_view_annotated_scroll_bar_get_current_part(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_bar.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(-1, last_offset);
    EGUI_TEST_ASSERT_EQUAL_INT(-1, last_max_offset);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, last_active_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, last_part);

    preview_bar.pressed_part = EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_INCREASE;
    preview_bar.pressed_marker = 1;
    preview_bar.rail_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&preview_bar), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_NONE, preview_bar.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_bar.pressed_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_bar.rail_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(420, egui_view_annotated_scroll_bar_get_offset(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(800, egui_view_annotated_scroll_bar_get_max_offset(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_annotated_scroll_bar_get_active_marker(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_ANNOTATED_SCROLL_BAR_PART_RAIL, egui_view_annotated_scroll_bar_get_current_part(EGUI_VIEW_OF(&preview_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_bar.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(-1, last_offset);
    EGUI_TEST_ASSERT_EQUAL_INT(-1, last_max_offset);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, last_active_marker);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, last_part);
}

void test_annotated_scroll_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(annotated_scroll_bar);
    EGUI_TEST_RUN(test_annotated_scroll_bar_clamps_metrics_and_regions);
    EGUI_TEST_RUN(test_annotated_scroll_bar_setters_clear_pressed_state_and_clamp);
    EGUI_TEST_RUN(test_annotated_scroll_bar_tab_cycles_parts);
    EGUI_TEST_RUN(test_annotated_scroll_bar_keyboard_navigation);
    EGUI_TEST_RUN(test_annotated_scroll_bar_touch_increase_button);
    EGUI_TEST_RUN(test_annotated_scroll_bar_touch_marker_selects_offset);
    EGUI_TEST_RUN(test_annotated_scroll_bar_touch_drag_rail_reaches_end);
    EGUI_TEST_RUN(test_annotated_scroll_bar_marker_drag_switches_to_rail);
    EGUI_TEST_RUN(test_annotated_scroll_bar_touch_cancel_clears_pressed_state);
    EGUI_TEST_RUN(test_annotated_scroll_bar_compact_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_annotated_scroll_bar_read_only_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_annotated_scroll_bar_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_annotated_scroll_bar_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
