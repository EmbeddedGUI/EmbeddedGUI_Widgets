#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_counter_badge.h"

#include "../../HelloCustomWidgets/display/counter_badge/egui_view_counter_badge.h"
#include "../../HelloCustomWidgets/display/counter_badge/egui_view_counter_badge.c"

static egui_view_counter_badge_t test_badge_widget;
static egui_view_counter_badge_t preview_badge_widget;
static egui_view_api_t preview_api;

static void layout_badge(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void setup_counter_badge(void)
{
    egui_view_counter_badge_init(EGUI_VIEW_OF(&test_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_badge_widget), 42, 24);
}

static void setup_preview_badge(void)
{
    egui_view_counter_badge_init(EGUI_VIEW_OF(&preview_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_badge_widget), 18, 16);
    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&preview_badge_widget), 4);
    egui_view_counter_badge_set_compact_mode(EGUI_VIEW_OF(&preview_badge_widget), 1);
    egui_view_counter_badge_override_static_preview_api(EGUI_VIEW_OF(&preview_badge_widget), &preview_api);
}

static int send_touch(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int send_key(egui_view_t *view, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= view->api->on_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->on_key_event(view, &event);
    return handled;
}

static void test_counter_badge_init_uses_defaults(void)
{
    setup_counter_badge();

    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_counter_badge_get_count(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(99, egui_view_counter_badge_get_max_display(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_counter_badge_get_dot_mode(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_counter_badge_get_compact_mode(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_counter_badge_get_read_only_mode(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC42B1C).full, test_badge_widget.badge_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, test_badge_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, test_badge_widget.outline_color.full);
}

static void test_counter_badge_setters_clear_pressed_state_and_update_palette(void)
{
    setup_counter_badge();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&test_badge_widget), 128);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(128, egui_view_counter_badge_get_count(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_max_display(EGUI_VIEW_OF(&test_badge_widget), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_counter_badge_get_max_display(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_dot_mode(EGUI_VIEW_OF(&test_badge_widget), 3);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_dot_mode(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_palette(EGUI_VIEW_OF(&test_badge_widget), EGUI_COLOR_HEX(0x13579B), EGUI_COLOR_HEX(0xF8F9FA), EGUI_COLOR_HEX(0x102030));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x13579B).full, test_badge_widget.badge_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF8F9FA).full, test_badge_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x102030).full, test_badge_widget.outline_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_compact_mode(EGUI_VIEW_OF(&test_badge_widget), 2);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_compact_mode(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_read_only_mode(EGUI_VIEW_OF(&test_badge_widget), 4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_read_only_mode(EGUI_VIEW_OF(&test_badge_widget)));
}

static void test_counter_badge_helpers_compute_regions_and_modes(void)
{
    egui_region_t region;
    egui_dim_t single_width;
    egui_dim_t overflow_width;
    egui_dim_t dot_width;
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_counter_badge();
    layout_badge(EGUI_VIEW_OF(&test_badge_widget), 10, 20, 42, 24);

    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_badge_region(EGUI_VIEW_OF(&test_badge_widget), &region));
    single_width = region.size.width;

    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&test_badge_widget), 128);
    egui_view_counter_badge_set_max_display(EGUI_VIEW_OF(&test_badge_widget), 99);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_badge_region(EGUI_VIEW_OF(&test_badge_widget), &region));
    overflow_width = region.size.width;
    EGUI_TEST_ASSERT_TRUE(overflow_width > single_width);

    egui_view_counter_badge_set_dot_mode(EGUI_VIEW_OF(&test_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_badge_region(EGUI_VIEW_OF(&test_badge_widget), &region));
    dot_width = region.size.width;
    EGUI_TEST_ASSERT_TRUE(dot_width < overflow_width);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_HEX(0x83909D), 48).full, egui_view_counter_badge_mix_disabled(sample).full);
}

static void test_counter_badge_static_preview_consumes_input_and_clears_pressed_state(void)
{
    setup_preview_badge();
    layout_badge(EGUI_VIEW_OF(&preview_badge_widget), 12, 18, 18, 16);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_badge_widget), EGUI_MOTION_EVENT_ACTION_DOWN, 20, 24));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_badge_widget)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_badge_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_compact_mode(EGUI_VIEW_OF(&preview_badge_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_counter_badge_get_count(EGUI_VIEW_OF(&preview_badge_widget)));
}

void test_counter_badge_run(void)
{
    EGUI_TEST_SUITE_BEGIN(counter_badge);
    EGUI_TEST_RUN(test_counter_badge_init_uses_defaults);
    EGUI_TEST_RUN(test_counter_badge_setters_clear_pressed_state_and_update_palette);
    EGUI_TEST_RUN(test_counter_badge_helpers_compute_regions_and_modes);
    EGUI_TEST_RUN(test_counter_badge_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
