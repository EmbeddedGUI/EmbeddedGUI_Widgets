#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_resize_grip.h"

#include "../../HelloCustomWidgets/layout/resize_grip/egui_view_resize_grip.h"
#include "../../HelloCustomWidgets/layout/resize_grip/egui_view_resize_grip.c"

typedef struct resize_grip_preview_snapshot resize_grip_preview_snapshot_t;
struct resize_grip_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t dot_color;
    egui_color_t accent_color;
    egui_dim_t grip_size;
    egui_dim_t dot_size;
    egui_dim_t dot_gap;
    egui_dim_t corner_radius;
    egui_alpha_t alpha;
    uint8_t corner;
    uint8_t compact_mode;
    uint8_t disabled_mode;
    uint8_t read_only_mode;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_margin_padding_t padding_left;
    egui_dim_margin_padding_t padding_right;
    egui_dim_margin_padding_t padding_top;
    egui_dim_margin_padding_t padding_bottom;
};

static egui_view_resize_grip_t test_control;
static egui_view_resize_grip_t preview_control;
static egui_view_api_t preview_api;

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_resize_grip(void)
{
    egui_view_resize_grip_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 64, 64);
}

static void setup_preview_control(void)
{
    egui_view_resize_grip_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 42, 42);
    egui_view_resize_grip_apply_compact_style(EGUI_VIEW_OF(&preview_control));
    egui_view_resize_grip_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
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

static void capture_preview_snapshot(resize_grip_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->surface_color = preview_control.surface_color;
    snapshot->border_color = preview_control.border_color;
    snapshot->dot_color = preview_control.dot_color;
    snapshot->accent_color = preview_control.accent_color;
    snapshot->grip_size = preview_control.grip_size;
    snapshot->dot_size = preview_control.dot_size;
    snapshot->dot_gap = preview_control.dot_gap;
    snapshot->corner_radius = preview_control.corner_radius;
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->corner = preview_control.corner;
    snapshot->compact_mode = preview_control.compact_mode;
    snapshot->disabled_mode = preview_control.disabled_mode;
    snapshot->read_only_mode = preview_control.read_only_mode;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_control)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_control)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_control)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_control)->padding.bottom;
}

static void assert_preview_state_unchanged(const resize_grip_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->dot_color.full, preview_control.dot_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->grip_size, preview_control.grip_size);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->dot_size, preview_control.dot_size);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->dot_gap, preview_control.dot_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->corner_radius, preview_control.corner_radius);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->corner, preview_control.corner);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_control.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->disabled_mode, preview_control.disabled_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_control.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_control)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_control)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_control)->padding.bottom);
}

static void test_resize_grip_init_defaults(void)
{
    setup_resize_grip();

    EGUI_TEST_ASSERT_EQUAL_INT(34, egui_view_resize_grip_get_grip_size(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_resize_grip_get_dot_size(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_resize_grip_get_dot_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_resize_grip_get_corner_radius(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_RIGHT, egui_view_resize_grip_get_corner(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_resize_grip_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_resize_grip_get_disabled_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_resize_grip_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_SURFACE.full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_control.dot_color.full);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.right);
#endif
}

static void test_resize_grip_metrics_corner_and_region(void)
{
    egui_region_t grip_region;

    setup_resize_grip();
    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_resize_grip_set_metrics(EGUI_VIEW_OF(&test_control), -3, 1, 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_resize_grip_get_grip_size(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_resize_grip_get_dot_size(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_resize_grip_get_dot_gap(EGUI_VIEW_OF(&test_control)));

    egui_view_resize_grip_set_metrics(EGUI_VIEW_OF(&test_control), 99, 99, 99);
    EGUI_TEST_ASSERT_EQUAL_INT(64, egui_view_resize_grip_get_grip_size(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_resize_grip_get_dot_size(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_resize_grip_get_dot_gap(EGUI_VIEW_OF(&test_control)));

    egui_view_resize_grip_set_metrics(EGUI_VIEW_OF(&test_control), 28, 4, 5);
    egui_view_resize_grip_set_corner_radius(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(14, egui_view_resize_grip_get_corner_radius(EGUI_VIEW_OF(&test_control)));
    egui_view_resize_grip_set_corner_radius(EGUI_VIEW_OF(&test_control), -1);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_resize_grip_get_corner_radius(EGUI_VIEW_OF(&test_control)));

    layout_view(EGUI_VIEW_OF(&test_control), 10, 20, 100, 80);
    egui_view_resize_grip_get_grip_region(EGUI_VIEW_OF(&test_control), &grip_region);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(70, grip_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(50, grip_region.location.y);
#else
    EGUI_TEST_ASSERT_EQUAL_INT(72, grip_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(52, grip_region.location.y);
#endif
    EGUI_TEST_ASSERT_EQUAL_INT(28, grip_region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(28, grip_region.size.height);

    egui_view_resize_grip_set_corner(EGUI_VIEW_OF(&test_control), EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_LEFT);
    egui_view_resize_grip_get_grip_region(EGUI_VIEW_OF(&test_control), &grip_region);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(2, grip_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(50, grip_region.location.y);
#else
    EGUI_TEST_ASSERT_EQUAL_INT(0, grip_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(52, grip_region.location.y);
#endif

    egui_view_resize_grip_set_corner(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RESIZE_GRIP_CORNER_BOTTOM_RIGHT, egui_view_resize_grip_get_corner(EGUI_VIEW_OF(&test_control)));
}

static void test_resize_grip_styles_and_palette(void)
{
    setup_resize_grip();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_resize_grip_apply_accent_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_resize_grip_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY.full, test_control.dot_color.full);

    egui_view_resize_grip_apply_compact_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_resize_grip_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_resize_grip_get_disabled_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_resize_grip_get_grip_size(EGUI_VIEW_OF(&test_control)));

    egui_view_resize_grip_apply_disabled_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_resize_grip_get_disabled_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_resize_grip_get_read_only_mode(EGUI_VIEW_OF(&test_control)));

    egui_view_resize_grip_apply_read_only_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_resize_grip_get_disabled_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_resize_grip_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_control.dot_color.full);

    egui_view_resize_grip_set_palette(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213),
                                      EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x212223).full, test_control.dot_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x313233).full, test_control.accent_color.full);
}

static void test_resize_grip_static_preview_consumes_input_and_keeps_state(void)
{
    resize_grip_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_control();
    layout_view(EGUI_VIEW_OF(&preview_control), 12, 18, 42, 42);
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

void test_resize_grip_run(void)
{
    EGUI_TEST_SUITE_BEGIN(resize_grip);
    EGUI_TEST_RUN(test_resize_grip_init_defaults);
    EGUI_TEST_RUN(test_resize_grip_metrics_corner_and_region);
    EGUI_TEST_RUN(test_resize_grip_styles_and_palette);
    EGUI_TEST_RUN(test_resize_grip_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
