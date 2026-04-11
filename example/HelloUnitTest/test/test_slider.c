#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_slider.h"

#include "../../HelloCustomWidgets/input/slider/egui_view_slider.h"
#include "../../HelloCustomWidgets/input/slider/egui_view_slider.c"

static egui_view_slider_t test_slider;
static egui_view_slider_t preview_slider;
static egui_view_api_t test_slider_api;
static egui_view_api_t preview_slider_api;

static void setup_slider(uint8_t value)
{
    egui_view_slider_init(EGUI_VIEW_OF(&test_slider));
    egui_view_set_size(EGUI_VIEW_OF(&test_slider), 120, 36);
    hcw_slider_apply_standard_style(EGUI_VIEW_OF(&test_slider));
    hcw_slider_override_interaction_api(EGUI_VIEW_OF(&test_slider), &test_slider_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_slider), 1);
#endif
    hcw_slider_set_value(EGUI_VIEW_OF(&test_slider), value);
}

static void setup_preview_slider(uint8_t value)
{
    egui_view_slider_init(EGUI_VIEW_OF(&preview_slider));
    egui_view_set_size(EGUI_VIEW_OF(&preview_slider), 104, 28);
    hcw_slider_apply_compact_style(EGUI_VIEW_OF(&preview_slider));
    hcw_slider_set_value(EGUI_VIEW_OF(&preview_slider), value);
    hcw_slider_override_static_preview_api(EGUI_VIEW_OF(&preview_slider), &preview_slider_api);
}

static void layout_slider(egui_view_slider_t *slider, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(EGUI_VIEW_OF(slider), &region);
    egui_region_copy(&EGUI_VIEW_OF(slider)->region_screen, &region);
}

static int send_touch(egui_view_slider_t *slider, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(slider)->api->on_touch_event(EGUI_VIEW_OF(slider), &event);
}

static int send_key(egui_view_slider_t *slider, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= EGUI_VIEW_OF(slider)->api->dispatch_key_event(EGUI_VIEW_OF(slider), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= EGUI_VIEW_OF(slider)->api->dispatch_key_event(EGUI_VIEW_OF(slider), &event);
    return handled;
}

static int send_preview_key_action(uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return EGUI_VIEW_OF(&preview_slider)->api->on_key_event(EGUI_VIEW_OF(&preview_slider), &event);
}

static void test_slider_style_helpers_update_palette_and_clear_pressed_state(void)
{
    setup_slider(42);

    test_slider.is_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_slider), 1);
    hcw_slider_apply_compact_style(EGUI_VIEW_OF(&test_slider));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_slider.is_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD9E5DE).full, test_slider.track_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, test_slider.active_color.full);

    test_slider.is_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_slider), 1);
    hcw_slider_apply_read_only_style(EGUI_VIEW_OF(&test_slider));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_slider.is_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xE4E9EE).full, test_slider.track_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xAFB8C3).full, test_slider.active_color.full);
}

static void test_slider_set_value_clamps_and_clears_drag_state(void)
{
    setup_slider(24);

    test_slider.is_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_slider), 1);
    hcw_slider_set_value(EGUI_VIEW_OF(&test_slider), 140);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_slider.is_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(100, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
}

static void test_slider_keyboard_navigation_steps_value(void)
{
    setup_slider(50);

    EGUI_TEST_ASSERT_TRUE(send_key(&test_slider, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(55, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_TRUE(send_key(&test_slider, EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(67, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_TRUE(send_key(&test_slider, EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_TRUE(send_key(&test_slider, EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(100, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_TRUE(send_key(&test_slider, EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(95, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
}

static void test_slider_touch_drag_updates_value(void)
{
    egui_view_t *view;
    egui_dim_t left_x;
    egui_dim_t right_x;
    egui_dim_t center_y;

    setup_slider(10);
    layout_slider(&test_slider, 10, 20, 120, 36);
    view = EGUI_VIEW_OF(&test_slider);
    left_x = view->region_screen.location.x + 4;
    right_x = view->region_screen.location.x + view->region_screen.size.width - 4;
    center_y = view->region_screen.location.y + view->region_screen.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_slider, EGUI_MOTION_EVENT_ACTION_DOWN, left_x, center_y));
    EGUI_TEST_ASSERT_TRUE(test_slider.is_dragging);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_slider, EGUI_MOTION_EVENT_ACTION_MOVE, right_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_slider, EGUI_MOTION_EVENT_ACTION_UP, right_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_slider.is_dragging);
    EGUI_TEST_ASSERT_TRUE(egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)) >= 95);
}

static void test_slider_touch_cancel_clears_pressed_state(void)
{
    egui_view_t *view;
    egui_dim_t x;
    egui_dim_t y;

    setup_slider(30);
    layout_slider(&test_slider, 10, 20, 120, 36);
    view = EGUI_VIEW_OF(&test_slider);
    x = view->region_screen.location.x + view->region_screen.size.width / 2;
    y = view->region_screen.location.y + view->region_screen.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_slider, EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_slider.is_dragging);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_slider, EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_slider.is_dragging);
}

static void test_slider_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_view_t *view;
    egui_dim_t x;
    egui_dim_t y;

    setup_slider(48);
    layout_slider(&test_slider, 10, 20, 120, 36);
    view = EGUI_VIEW_OF(&test_slider);
    x = view->region_screen.location.x + view->region_screen.size.width / 2;
    y = view->region_screen.location.y + view->region_screen.size.height / 2;

    test_slider.is_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_slider), 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_slider), 0);
    EGUI_TEST_ASSERT_FALSE(send_key(&test_slider, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_slider.is_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(48, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_FALSE(send_touch(&test_slider, EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(send_touch(&test_slider, EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(48, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
}

static void test_slider_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_view_t *view;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_slider(22);
    layout_slider(&preview_slider, 10, 20, 104, 28);
    view = EGUI_VIEW_OF(&preview_slider);
    x = view->region_screen.location.x + view->region_screen.size.width / 2;
    y = view->region_screen.location.y + view->region_screen.size.height / 2;

    preview_slider.is_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&preview_slider), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(&preview_slider, EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_slider.is_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(22, egui_view_slider_get_value(EGUI_VIEW_OF(&preview_slider)));

    preview_slider.is_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&preview_slider), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_slider.is_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(22, egui_view_slider_get_value(EGUI_VIEW_OF(&preview_slider)));
}

void test_slider_run(void)
{
    EGUI_TEST_SUITE_BEGIN(slider);
    EGUI_TEST_RUN(test_slider_style_helpers_update_palette_and_clear_pressed_state);
    EGUI_TEST_RUN(test_slider_set_value_clamps_and_clears_drag_state);
    EGUI_TEST_RUN(test_slider_keyboard_navigation_steps_value);
    EGUI_TEST_RUN(test_slider_touch_drag_updates_value);
    EGUI_TEST_RUN(test_slider_touch_cancel_clears_pressed_state);
    EGUI_TEST_RUN(test_slider_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_slider_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
