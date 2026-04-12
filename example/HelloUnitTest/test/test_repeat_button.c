#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_repeat_button.h"

#include "../../HelloCustomWidgets/input/repeat_button/egui_view_repeat_button.h"
#include "../../HelloCustomWidgets/input/repeat_button/egui_view_repeat_button.c"

static egui_view_repeat_button_t test_widget;
static egui_view_repeat_button_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void on_repeat_button_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_widget(void)
{
    egui_timer_init();
    egui_view_repeat_button_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 120, 40);
    egui_view_repeat_button_set_text(EGUI_VIEW_OF(&test_widget), "Increase");
    egui_view_repeat_button_set_icon(EGUI_VIEW_OF(&test_widget), EGUI_ICON_MS_ADD);
    egui_view_repeat_button_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_repeat_button_set_icon_font(EGUI_VIEW_OF(&test_widget), EGUI_FONT_ICON_MS_16);
    egui_view_repeat_button_set_icon_text_gap(EGUI_VIEW_OF(&test_widget), 6);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_widget), on_repeat_button_click);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_widget), 1);
#endif
    g_click_count = 0;
}

static void setup_preview_widget(void)
{
    egui_timer_init();
    egui_view_repeat_button_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 84, 32);
    egui_view_repeat_button_apply_compact_style(EGUI_VIEW_OF(&preview_widget));
    egui_view_repeat_button_set_text(EGUI_VIEW_OF(&preview_widget), "Fast");
    egui_view_repeat_button_set_icon(EGUI_VIEW_OF(&preview_widget), EGUI_ICON_MS_REFRESH);
    egui_view_repeat_button_set_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_repeat_button_set_icon_font(EGUI_VIEW_OF(&preview_widget), EGUI_FONT_ICON_MS_16);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_widget), on_repeat_button_click);
    egui_view_repeat_button_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    g_click_count = 0;
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

static void layout_widget(void)
{
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 18, 120, 40);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 14, 22, 84, 32);
}

static void attach_view(egui_view_t *view)
{
    egui_view_dispatch_attach_to_window(view);
}

static void detach_view(egui_view_t *view)
{
    egui_view_dispatch_detach_from_window(view);
}

static int send_touch_action(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->dispatch_touch_event(view, &event);
}

static int send_key_action(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_action(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_action(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void assert_timer_stopped(egui_view_repeat_button_t *widget)
{
    EGUI_TEST_ASSERT_FALSE(widget->timer_started);
    EGUI_TEST_ASSERT_FALSE(egui_timer_check_timer_start(&widget->repeat_timer));
}

static void assert_pressed_state_cleared(egui_view_repeat_button_t *widget)
{
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(widget->touch_active);
    EGUI_TEST_ASSERT_FALSE(widget->key_active);
    assert_timer_stopped(widget);
}

static void start_touch_hold(egui_dim_t *center_x, egui_dim_t *center_y)
{
    layout_widget();
    attach_view(EGUI_VIEW_OF(&test_widget));
    get_view_center(EGUI_VIEW_OF(&test_widget), center_x, center_y);
    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, *center_x, *center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_widget.touch_active);
    EGUI_TEST_ASSERT_TRUE(test_widget.timer_started);
    EGUI_TEST_ASSERT_TRUE(egui_timer_check_timer_start(&test_widget.repeat_timer));
}

static void seed_preview_timer_state(uint8_t touch_active, uint8_t key_active)
{
    preview_widget.touch_active = touch_active;
    preview_widget.key_active = key_active;
    preview_widget.timer_started = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&preview_widget), 1);
    egui_timer_start_timer(&preview_widget.repeat_timer, 100, 0);
    EGUI_TEST_ASSERT_TRUE(egui_timer_check_timer_start(&preview_widget.repeat_timer));
}

static void test_repeat_button_init_uses_default_timing_and_style(void)
{
    egui_timer_init();
    egui_view_repeat_button_init(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_EQUAL_INT(360, test_widget.initial_delay_ms);
    EGUI_TEST_ASSERT_EQUAL_INT(90, test_widget.repeat_interval_ms);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->background == EGUI_BG_OF(&repeat_button_standard_background));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, test_widget.base.base.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(6, test_widget.base.icon_text_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_widget)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_widget)->padding.right);
    EGUI_TEST_ASSERT_FALSE(test_widget.touch_active);
    EGUI_TEST_ASSERT_FALSE(test_widget.key_active);
    assert_timer_stopped(&test_widget);
}

static void test_repeat_button_setters_and_style_helpers_clear_pressed_state(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_widget();
    start_touch_hold(&center_x, &center_y);
    egui_view_repeat_button_apply_compact_style(EGUI_VIEW_OF(&test_widget));
    assert_pressed_state_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->background == EGUI_BG_OF(&repeat_button_compact_background));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, test_widget.base.base.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_widget.base.icon_text_gap);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_click_count);
    egui_view_repeat_button_apply_disabled_style(EGUI_VIEW_OF(&test_widget));
    assert_pressed_state_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->background == EGUI_BG_OF(&repeat_button_disabled_background));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x708090).full, test_widget.base.base.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_widget.base.icon_text_gap);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_click_count);
    egui_view_repeat_button_set_text(EGUI_VIEW_OF(&test_widget), "Pause");
    assert_pressed_state_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(strcmp(test_widget.base.base.text, "Pause") == 0);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, g_click_count);
    egui_view_repeat_button_set_icon(EGUI_VIEW_OF(&test_widget), EGUI_ICON_MS_REFRESH);
    assert_pressed_state_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(strcmp(test_widget.base.icon, EGUI_ICON_MS_REFRESH) == 0);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(5, g_click_count);
    egui_view_repeat_button_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_state_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.base.base.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(6, g_click_count);
    egui_view_repeat_button_set_icon_font(EGUI_VIEW_OF(&test_widget), EGUI_FONT_ICON_MS_20);
    assert_pressed_state_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.base.icon_font == EGUI_FONT_ICON_MS_20);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(7, g_click_count);
    egui_view_repeat_button_set_icon_text_gap(EGUI_VIEW_OF(&test_widget), 9);
    assert_pressed_state_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(9, test_widget.base.icon_text_gap);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(8, g_click_count);
    egui_view_repeat_button_set_repeat_timing(EGUI_VIEW_OF(&test_widget), 10, 20);
    assert_pressed_state_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(80, test_widget.initial_delay_ms);
    EGUI_TEST_ASSERT_EQUAL_INT(40, test_widget.repeat_interval_ms);

    egui_view_repeat_button_set_repeat_timing(EGUI_VIEW_OF(&test_widget), 0, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(360, test_widget.initial_delay_ms);
    EGUI_TEST_ASSERT_EQUAL_INT(90, test_widget.repeat_interval_ms);
}

static void test_repeat_button_touch_down_clicks_immediately_and_repeats(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_widget();
    start_touch_hold(&center_x, &center_y);

    egui_view_repeat_button_tick(&test_widget.repeat_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_click_count);
    egui_view_repeat_button_tick(&test_widget.repeat_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_click_count);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_click_count);
    assert_pressed_state_cleared(&test_widget);
}

static void test_repeat_button_touch_move_outside_stops_timer_and_move_back_restarts_without_extra_click(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;

    setup_widget();
    start_touch_hold(&inside_x, &inside_y);
    outside_x = EGUI_VIEW_OF(&test_widget)->region_screen.location.x + EGUI_VIEW_OF(&test_widget)->region_screen.size.width + 12;

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_widget.touch_active);
    assert_timer_stopped(&test_widget);

    egui_view_repeat_button_tick(&test_widget.repeat_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_widget.touch_active);
    EGUI_TEST_ASSERT_TRUE(test_widget.timer_started);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);

    egui_view_repeat_button_tick(&test_widget.repeat_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_click_count);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_click_count);
    assert_pressed_state_cleared(&test_widget);
}

static void test_repeat_button_key_repeat_works_for_space_and_enter(void)
{
    setup_widget();
    attach_view(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_widget.key_active);
    EGUI_TEST_ASSERT_TRUE(test_widget.timer_started);

    egui_view_repeat_button_tick(&test_widget.repeat_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_click_count);

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_click_count);
    assert_pressed_state_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_click_count);
    assert_pressed_state_cleared(&test_widget);
}

static void test_repeat_button_unhandled_key_clears_pressed_state_and_stops_timer(void)
{
    setup_widget();
    attach_view(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);
    EGUI_TEST_ASSERT_TRUE(test_widget.key_active);
    EGUI_TEST_ASSERT_TRUE(test_widget.timer_started);

    EGUI_TEST_ASSERT_FALSE(send_key_action(EGUI_VIEW_OF(&test_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);
    assert_pressed_state_cleared(&test_widget);
}

static void test_repeat_button_disabled_guard_prevents_click_and_clears_state(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_widget();
    start_touch_hold(&center_x, &center_y);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);

    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);
    assert_pressed_state_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_click_count);
    assert_pressed_state_cleared(&test_widget);
}

static void test_repeat_button_static_preview_consumes_input_and_clears_timer_state(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_widget();
    layout_preview_widget();
    get_view_center(EGUI_VIEW_OF(&preview_widget), &center_x, &center_y);

    seed_preview_timer_state(1, 0);
    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(preview_widget.touch_active);
    EGUI_TEST_ASSERT_FALSE(preview_widget.key_active);
    assert_timer_stopped(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    seed_preview_timer_state(0, 1);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(preview_widget.touch_active);
    EGUI_TEST_ASSERT_FALSE(preview_widget.key_active);
    assert_timer_stopped(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

static void test_repeat_button_attach_and_detach_restore_repeat_timer(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_widget();
    start_touch_hold(&center_x, &center_y);
    detach_view(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_widget.touch_active);
    assert_timer_stopped(&test_widget);

    attach_view(EGUI_VIEW_OF(&test_widget));
    EGUI_TEST_ASSERT_TRUE(test_widget.timer_started);
    EGUI_TEST_ASSERT_TRUE(egui_timer_check_timer_start(&test_widget.repeat_timer));

    egui_view_repeat_button_tick(&test_widget.repeat_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_click_count);

    EGUI_TEST_ASSERT_TRUE(send_touch_action(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    assert_pressed_state_cleared(&test_widget);
}

void test_repeat_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(repeat_button);
    EGUI_TEST_RUN(test_repeat_button_init_uses_default_timing_and_style);
    EGUI_TEST_RUN(test_repeat_button_setters_and_style_helpers_clear_pressed_state);
    EGUI_TEST_RUN(test_repeat_button_touch_down_clicks_immediately_and_repeats);
    EGUI_TEST_RUN(test_repeat_button_touch_move_outside_stops_timer_and_move_back_restarts_without_extra_click);
    EGUI_TEST_RUN(test_repeat_button_key_repeat_works_for_space_and_enter);
    EGUI_TEST_RUN(test_repeat_button_unhandled_key_clears_pressed_state_and_stops_timer);
    EGUI_TEST_RUN(test_repeat_button_disabled_guard_prevents_click_and_clears_state);
    EGUI_TEST_RUN(test_repeat_button_static_preview_consumes_input_and_clears_timer_state);
    EGUI_TEST_RUN(test_repeat_button_attach_and_detach_restore_repeat_timer);
    EGUI_TEST_SUITE_END();
}
