#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_polyline.h"

#include "../../HelloCustomWidgets/display/polyline/egui_view_polyline.h"
#include "../../HelloCustomWidgets/display/polyline/egui_view_polyline.c"

typedef struct polyline_preview_snapshot polyline_preview_snapshot_t;
struct polyline_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    egui_color_t stroke_color;
    egui_color_t accent_color;
    egui_dim_t stroke_width;
    uint8_t point_count;
    uint8_t points_percent[EGUI_VIEW_POLYLINE_MAX_POINTS * 2];
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_margin_padding_t padding_left;
    egui_dim_margin_padding_t padding_right;
    egui_dim_margin_padding_t padding_top;
    egui_dim_margin_padding_t padding_bottom;
};

static egui_view_polyline_t test_control;
static egui_view_polyline_t preview_control;
static egui_view_api_t preview_api;

static const uint8_t test_polyline_step_points[] = {
        12, 68,
        38, 36,
        62, 58,
        88, 24,
};

static const uint8_t test_polyline_muted_points[] = {
        10, 58,
        34, 42,
        58, 48,
        86, 32,
};

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_polyline(void)
{
    egui_view_polyline_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 144, 64);
}

static void setup_preview_control(void)
{
    egui_view_polyline_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 72, 34);
    egui_view_polyline_set_palette(EGUI_VIEW_OF(&preview_control), HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_TINT);
    egui_view_polyline_set_stroke_width(EGUI_VIEW_OF(&preview_control), 1);
    egui_view_polyline_set_points(EGUI_VIEW_OF(&preview_control), test_polyline_step_points, 4);
    egui_view_polyline_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
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

static void capture_preview_snapshot(polyline_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->stroke_color = preview_control.stroke_color;
    snapshot->accent_color = preview_control.accent_color;
    snapshot->stroke_width = preview_control.stroke_width;
    snapshot->point_count = preview_control.point_count;
    memcpy(snapshot->points_percent, preview_control.points_percent, sizeof(snapshot->points_percent));
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_control)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_control)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_control)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_control)->padding.bottom;
}

static void assert_preview_state_unchanged(const polyline_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->stroke_color.full, preview_control.stroke_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->stroke_width, preview_control.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->point_count, preview_control.point_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, memcmp(snapshot->points_percent, preview_control.points_percent, sizeof(snapshot->points_percent)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_control)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_control)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_control)->padding.bottom);
}

static void assert_point(egui_view_t *view, uint8_t index, uint8_t x, uint8_t y)
{
    uint8_t actual_x = 0;
    uint8_t actual_y = 0;

    egui_view_polyline_get_point(view, index, &actual_x, &actual_y);
    EGUI_TEST_ASSERT_EQUAL_INT(x, actual_x);
    EGUI_TEST_ASSERT_EQUAL_INT(y, actual_y);
}

static void test_polyline_init_defaults(void)
{
    setup_polyline();

    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_polyline_get_stroke_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_polyline_get_point_count(EGUI_VIEW_OF(&test_control)));
    assert_point(EGUI_VIEW_OF(&test_control), 0, 8, 72);
    assert_point(EGUI_VIEW_OF(&test_control), 4, 92, 34);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY.full, test_control.stroke_color.full);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.right);
#endif
}

static void test_polyline_setters_clamp_and_clear_pressed_state(void)
{
    const uint8_t points[] = {
            7, 8,
            120, 130,
            30, 40,
            50, 60,
            70, 80,
            90, 100,
            12, 24,
            36, 48,
            60, 72,
    };

    setup_polyline();
    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_polyline_set_stroke_width(EGUI_VIEW_OF(&test_control), -3);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_polyline_get_stroke_width(EGUI_VIEW_OF(&test_control)));

    egui_view_polyline_set_stroke_width(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_polyline_get_stroke_width(EGUI_VIEW_OF(&test_control)));

    egui_view_polyline_set_points(EGUI_VIEW_OF(&test_control), points, 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_POLYLINE_MAX_POINTS, egui_view_polyline_get_point_count(EGUI_VIEW_OF(&test_control)));
    assert_point(EGUI_VIEW_OF(&test_control), 0, 7, 8);
    assert_point(EGUI_VIEW_OF(&test_control), 1, 100, 100);
    assert_point(EGUI_VIEW_OF(&test_control), 7, 36, 48);

    egui_view_polyline_set_palette(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x111213), EGUI_COLOR_HEX(0x212223));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x111213).full, test_control.stroke_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x212223).full, test_control.accent_color.full);

    egui_view_polyline_set_points(EGUI_VIEW_OF(&test_control), NULL, 4);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_polyline_get_point_count(EGUI_VIEW_OF(&test_control)));
    assert_point(EGUI_VIEW_OF(&test_control), 1, 0, 0);
}

static void test_polyline_styles(void)
{
    setup_polyline();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_polyline_apply_accent_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_polyline_get_stroke_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_polyline_get_point_count(EGUI_VIEW_OF(&test_control)));
    assert_point(EGUI_VIEW_OF(&test_control), 5, 92, 12);

    egui_view_polyline_set_palette(EGUI_VIEW_OF(&test_control), HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_TINT);
    egui_view_polyline_set_stroke_width(EGUI_VIEW_OF(&test_control), 1);
    egui_view_polyline_set_points(EGUI_VIEW_OF(&test_control), test_polyline_step_points, 4);
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_polyline_get_point_count(EGUI_VIEW_OF(&test_control)));
    assert_point(EGUI_VIEW_OF(&test_control), 3, 88, 24);

    egui_view_polyline_set_palette(EGUI_VIEW_OF(&test_control), HCW_COLOR_TEXT_SOFT, HCW_COLOR_BORDER_STRONG);
    egui_view_polyline_set_stroke_width(EGUI_VIEW_OF(&test_control), 1);
    egui_view_polyline_set_points(EGUI_VIEW_OF(&test_control), test_polyline_muted_points, 4);
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_polyline_get_point_count(EGUI_VIEW_OF(&test_control)));
    assert_point(EGUI_VIEW_OF(&test_control), 3, 86, 32);
}

static void test_polyline_static_preview_consumes_input_and_keeps_state(void)
{
    polyline_preview_snapshot_t initial_snapshot;
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

void test_polyline_run(void)
{
    EGUI_TEST_SUITE_BEGIN(polyline);
    EGUI_TEST_RUN(test_polyline_init_defaults);
    EGUI_TEST_RUN(test_polyline_setters_clamp_and_clear_pressed_state);
    EGUI_TEST_RUN(test_polyline_styles);
    EGUI_TEST_RUN(test_polyline_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
