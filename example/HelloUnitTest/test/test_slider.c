#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_slider.h"

#include "../../HelloCustomWidgets/input/slider/egui_view_slider.h"
#include "../../HelloCustomWidgets/input/slider/egui_view_slider.c"

typedef struct slider_preview_snapshot slider_preview_snapshot_t;
struct slider_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    egui_view_on_value_changed_listener_t on_value_changed;
    uint8_t value;
    uint8_t is_dragging;
    egui_color_t track_color;
    egui_color_t active_color;
    egui_color_t thumb_color;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_slider_t test_slider;
static egui_view_slider_t preview_slider;
static egui_view_api_t test_slider_api;
static egui_view_api_t preview_slider_api;
static uint8_t changed_count;
static uint8_t changed_value;

static void on_value_changed(egui_view_t *self, uint8_t value)
{
    EGUI_UNUSED(self);
    changed_count++;
    changed_value = value;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

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
    egui_view_slider_set_on_value_changed_listener(EGUI_VIEW_OF(&preview_slider), on_value_changed);
    hcw_slider_override_static_preview_api(EGUI_VIEW_OF(&preview_slider), &preview_slider_api);
    changed_count = 0;
    changed_value = 0xFF;
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

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static void capture_preview_snapshot(slider_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_slider)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_slider)->background;
    snapshot->on_value_changed = preview_slider.on_value_changed;
    snapshot->value = preview_slider.value;
    snapshot->is_dragging = preview_slider.is_dragging;
    snapshot->track_color = preview_slider.track_color;
    snapshot->active_color = preview_slider.active_color;
    snapshot->thumb_color = preview_slider.thumb_color;
    snapshot->alpha = EGUI_VIEW_OF(&preview_slider)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_slider));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_slider)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_slider)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_slider)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_slider)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_slider)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_slider)->padding.bottom;
}

static void assert_preview_state_unchanged(const slider_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_slider)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_slider)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_slider.on_value_changed == snapshot->on_value_changed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->value, preview_slider.value);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_dragging, preview_slider.is_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->track_color.full, preview_slider.track_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->active_color.full, preview_slider.active_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->thumb_color.full, preview_slider.thumb_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_slider)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_slider)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_slider)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_slider)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_slider)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_slider)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_slider)->padding.bottom);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, changed_value);
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

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_slider), EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(55, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_slider), EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(67, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_slider), EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_slider), EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(100, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_slider), EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(95, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
}

static void test_slider_touch_drag_updates_value(void)
{
    egui_view_t *view;
    egui_dim_t left_x;
    egui_dim_t right_x;
    egui_dim_t center_y;

    setup_slider(10);
    layout_view(EGUI_VIEW_OF(&test_slider), 10, 20, 120, 36);
    view = EGUI_VIEW_OF(&test_slider);
    left_x = view->region_screen.location.x + 4;
    right_x = view->region_screen.location.x + view->region_screen.size.width - 4;
    center_y = view->region_screen.location.y + view->region_screen.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_slider), EGUI_MOTION_EVENT_ACTION_DOWN, left_x, center_y));
    EGUI_TEST_ASSERT_TRUE(test_slider.is_dragging);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_slider), EGUI_MOTION_EVENT_ACTION_MOVE, right_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_slider), EGUI_MOTION_EVENT_ACTION_UP, right_x, center_y));
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
    layout_view(EGUI_VIEW_OF(&test_slider), 10, 20, 120, 36);
    view = EGUI_VIEW_OF(&test_slider);
    x = view->region_screen.location.x + view->region_screen.size.width / 2;
    y = view->region_screen.location.y + view->region_screen.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_slider), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_slider.is_dragging);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_slider), EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_slider.is_dragging);
}

static void test_slider_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_view_t *view;
    egui_dim_t x;
    egui_dim_t y;

    setup_slider(48);
    layout_view(EGUI_VIEW_OF(&test_slider), 10, 20, 120, 36);
    view = EGUI_VIEW_OF(&test_slider);
    x = view->region_screen.location.x + view->region_screen.size.width / 2;
    y = view->region_screen.location.y + view->region_screen.size.height / 2;

    test_slider.is_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_slider), 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_slider), 0);
    EGUI_TEST_ASSERT_FALSE(send_key_to_view(EGUI_VIEW_OF(&test_slider), EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_slider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_slider.is_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(48, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_slider), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_slider), EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(48, egui_view_slider_get_value(EGUI_VIEW_OF(&test_slider)));
}

static void test_slider_static_preview_consumes_input_and_keeps_state(void)
{
    slider_preview_snapshot_t initial_snapshot;
    egui_view_t *view;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_slider(22);
    layout_view(EGUI_VIEW_OF(&preview_slider), 10, 20, 104, 28);
    view = EGUI_VIEW_OF(&preview_slider);
    x = view->region_screen.location.x + view->region_screen.size.width / 2;
    y = view->region_screen.location.y + view->region_screen.size.height / 2;
    capture_preview_snapshot(&initial_snapshot);

    preview_slider.is_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&preview_slider), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_slider), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    preview_slider.is_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&preview_slider), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_slider), EGUI_KEY_CODE_RIGHT));
    assert_preview_state_unchanged(&initial_snapshot);
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
    EGUI_TEST_RUN(test_slider_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
