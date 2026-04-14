#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_spinner.h"

#include "../../HelloCustomWidgets/feedback/spinner/egui_view_spinner.h"
#include "../../HelloCustomWidgets/feedback/spinner/egui_view_spinner.c"

static egui_view_spinner_t test_spinner_widget;
static egui_view_spinner_t preview_spinner_widget;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_spinner(void)
{
    egui_timer_init();
    egui_view_spinner_init(EGUI_VIEW_OF(&test_spinner_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_spinner_widget), 44, 44);
    hcw_spinner_apply_standard_style(EGUI_VIEW_OF(&test_spinner_widget));
    g_click_count = 0;
}

static void setup_preview_spinner(void)
{
    egui_timer_init();
    egui_view_spinner_init(EGUI_VIEW_OF(&preview_spinner_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_spinner_widget), 24, 24);
    hcw_spinner_apply_compact_style(EGUI_VIEW_OF(&preview_spinner_widget));
    hcw_spinner_set_spinning(EGUI_VIEW_OF(&preview_spinner_widget), 0);
    hcw_spinner_set_rotation_angle(EGUI_VIEW_OF(&preview_spinner_widget), 300);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_spinner_widget), on_preview_click);
    hcw_spinner_override_static_preview_api(EGUI_VIEW_OF(&preview_spinner_widget), &preview_api);
    g_click_count = 0;
}

static void attach_view(egui_view_t *view)
{
    egui_view_dispatch_attach_to_window(view);
}

static void detach_view(egui_view_t *view)
{
    egui_view_dispatch_detach_from_window(view);
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

static int send_touch(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int send_key(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void test_spinner_style_helpers_apply_expected_geometry_and_palette(void)
{
    setup_spinner();

    EGUI_TEST_ASSERT_EQUAL_INT(4, test_spinner_widget.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(104, test_spinner_widget.arc_length);
    EGUI_TEST_ASSERT_EQUAL_INT(-90, test_spinner_widget.rotation_angle);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_spinner_widget.color.full);
    EGUI_TEST_ASSERT_TRUE(test_spinner_widget.is_spinning);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_spinner_widget), 1);
    hcw_spinner_apply_compact_style(EGUI_VIEW_OF(&test_spinner_widget));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_spinner_widget.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(84, test_spinner_widget.arc_length);
    EGUI_TEST_ASSERT_TRUE(test_spinner_widget.is_spinning);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_spinner_widget), 1);
    hcw_spinner_apply_muted_style(EGUI_VIEW_OF(&test_spinner_widget));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_spinner_widget.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(76, test_spinner_widget.arc_length);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x8A96A2).full, test_spinner_widget.color.full);
}

static void test_spinner_setters_clear_pressed_state_and_clamp(void)
{
    setup_spinner();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_spinner_widget), 1);
    hcw_spinner_set_palette(EGUI_VIEW_OF(&test_spinner_widget), EGUI_COLOR_HEX(0x13579B));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x13579B).full, test_spinner_widget.color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_spinner_widget), 1);
    hcw_spinner_set_stroke_width(EGUI_VIEW_OF(&test_spinner_widget), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_spinner_widget.stroke_width);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_spinner_widget), 1);
    hcw_spinner_set_arc_length(EGUI_VIEW_OF(&test_spinner_widget), 400);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(300, test_spinner_widget.arc_length);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_spinner_widget), 1);
    hcw_spinner_set_rotation_angle(EGUI_VIEW_OF(&test_spinner_widget), -15);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(345, test_spinner_widget.rotation_angle);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_spinner_widget), 1);
    hcw_spinner_set_rotation_angle(EGUI_VIEW_OF(&test_spinner_widget), 390);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(30, test_spinner_widget.rotation_angle);
}

static void test_spinner_set_spinning_starts_and_stops_animation(void)
{
    setup_spinner();
    attach_view(EGUI_VIEW_OF(&test_spinner_widget));
    EGUI_TEST_ASSERT_TRUE(egui_timer_check_timer_start(&test_spinner_widget.spin_timer));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_spinner_widget), 1);
    hcw_spinner_set_spinning(EGUI_VIEW_OF(&test_spinner_widget), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(test_spinner_widget.is_spinning);
    EGUI_TEST_ASSERT_FALSE(egui_timer_check_timer_start(&test_spinner_widget.spin_timer));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_spinner_widget), 1);
    hcw_spinner_set_spinning(EGUI_VIEW_OF(&test_spinner_widget), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_spinner_widget.is_spinning);
    EGUI_TEST_ASSERT_TRUE(egui_timer_check_timer_start(&test_spinner_widget.spin_timer));

    detach_view(EGUI_VIEW_OF(&test_spinner_widget));
    EGUI_TEST_ASSERT_FALSE(egui_timer_check_timer_start(&test_spinner_widget.spin_timer));
}

static void test_spinner_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_spinner();
    EGUI_TEST_ASSERT_FALSE(preview_spinner_widget.is_spinning);
    EGUI_TEST_ASSERT_EQUAL_INT(300, preview_spinner_widget.rotation_angle);

    attach_view(EGUI_VIEW_OF(&preview_spinner_widget));
    EGUI_TEST_ASSERT_FALSE(egui_timer_check_timer_start(&preview_spinner_widget.spin_timer));

    layout_view(EGUI_VIEW_OF(&preview_spinner_widget), 12, 18, 24, 24);
    get_view_center(EGUI_VIEW_OF(&preview_spinner_widget), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_spinner_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_spinner_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_spinner_widget), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_spinner_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_spinner_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_spinner_widget), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_spinner_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    detach_view(EGUI_VIEW_OF(&preview_spinner_widget));
    EGUI_TEST_ASSERT_FALSE(egui_timer_check_timer_start(&preview_spinner_widget.spin_timer));
}

void test_spinner_run(void)
{
    EGUI_TEST_SUITE_BEGIN(spinner);
    EGUI_TEST_RUN(test_spinner_style_helpers_apply_expected_geometry_and_palette);
    EGUI_TEST_RUN(test_spinner_setters_clear_pressed_state_and_clamp);
    EGUI_TEST_RUN(test_spinner_set_spinning_starts_and_stops_animation);
    EGUI_TEST_RUN(test_spinner_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
