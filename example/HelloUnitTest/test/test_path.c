#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_path.h"

#include "../../HelloCustomWidgets/display/path/egui_view_path.h"
#include "../../HelloCustomWidgets/display/path/egui_view_path.c"

typedef struct path_preview_snapshot path_preview_snapshot_t;
struct path_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    const egui_view_path_data_t *data;
    egui_color_t fill_color;
    egui_color_t stroke_color;
    egui_color_t accent_color;
    egui_dim_t stroke_width;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_margin_padding_t padding_left;
    egui_dim_margin_padding_t padding_right;
    egui_dim_margin_padding_t padding_top;
    egui_dim_margin_padding_t padding_bottom;
};

static egui_view_path_t test_control;
static egui_view_path_t preview_control;
static egui_view_api_t preview_api;

static void apply_test_path_line_style(egui_view_t *view)
{
    egui_view_path_set_palette(view, HCW_COLOR_PRIMARY_TINT, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_SOFT);
    egui_view_path_set_stroke_width(view, 1);
    egui_view_path_set_data(view, egui_view_path_get_line_data());
}

static void apply_test_path_muted_style(egui_view_t *view)
{
    egui_view_path_set_palette(view, HCW_COLOR_BORDER, HCW_COLOR_TEXT_SOFT, HCW_COLOR_BORDER_STRONG);
    egui_view_path_set_stroke_width(view, 1);
    egui_view_path_set_data(view, egui_view_path_get_bookmark_data());
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_path(void)
{
    egui_view_path_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 144, 64);
}

static void setup_preview_control(void)
{
    egui_view_path_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 72, 34);
    apply_test_path_line_style(EGUI_VIEW_OF(&preview_control));
    egui_view_path_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
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

static void capture_preview_snapshot(path_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->data = preview_control.data;
    snapshot->fill_color = preview_control.fill_color;
    snapshot->stroke_color = preview_control.stroke_color;
    snapshot->accent_color = preview_control.accent_color;
    snapshot->stroke_width = preview_control.stroke_width;
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_control)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_control)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_control)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_control)->padding.bottom;
}

static void assert_preview_state_unchanged(const path_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_TRUE(preview_control.data == snapshot->data);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->fill_color.full, preview_control.fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->stroke_color.full, preview_control.stroke_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->stroke_width, preview_control.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_control)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_control)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_control)->padding.bottom);
}

static void test_path_init_defaults(void)
{
    setup_path();

    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_path_get_stroke_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_TRUE(egui_view_path_get_data(EGUI_VIEW_OF(&test_control)) == egui_view_path_get_shield_data());
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_TINT.full, test_control.fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY.full, test_control.stroke_color.full);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.right);
#endif
}

static void test_path_setters_clamp_and_clear_pressed_state(void)
{
    static const egui_view_path_data_t invalid_data = { 0, 0, 0, NULL };

    setup_path();
    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_path_set_stroke_width(EGUI_VIEW_OF(&test_control), -3);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_path_get_stroke_width(EGUI_VIEW_OF(&test_control)));

    egui_view_path_set_stroke_width(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_path_get_stroke_width(EGUI_VIEW_OF(&test_control)));

    egui_view_path_set_data(EGUI_VIEW_OF(&test_control), egui_view_path_get_curve_data());
    EGUI_TEST_ASSERT_TRUE(egui_view_path_get_data(EGUI_VIEW_OF(&test_control)) == egui_view_path_get_curve_data());

    egui_view_path_set_data(EGUI_VIEW_OF(&test_control), &invalid_data);
    EGUI_TEST_ASSERT_TRUE(egui_view_path_get_data(EGUI_VIEW_OF(&test_control)) == egui_view_path_get_shield_data());

    egui_view_path_set_palette(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x111213), EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x111213).full, test_control.fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x212223).full, test_control.stroke_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x313233).full, test_control.accent_color.full);
}

static void test_path_styles(void)
{
    setup_path();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_path_apply_accent_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_path_get_stroke_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_TRUE(egui_view_path_get_data(EGUI_VIEW_OF(&test_control)) == egui_view_path_get_curve_data());

    apply_test_path_line_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_path_get_stroke_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_TRUE(egui_view_path_get_data(EGUI_VIEW_OF(&test_control)) == egui_view_path_get_line_data());
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_TINT.full, test_control.fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY.full, test_control.stroke_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_SOFT.full, test_control.accent_color.full);

    apply_test_path_muted_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_TRUE(egui_view_path_get_data(EGUI_VIEW_OF(&test_control)) == egui_view_path_get_bookmark_data());
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_BORDER.full, test_control.fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_control.stroke_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_BORDER_STRONG.full, test_control.accent_color.full);
}

static void test_path_static_preview_consumes_input_and_keeps_state(void)
{
    path_preview_snapshot_t initial_snapshot;
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

void test_path_run(void)
{
    EGUI_TEST_SUITE_BEGIN(path);
    EGUI_TEST_RUN(test_path_init_defaults);
    EGUI_TEST_RUN(test_path_setters_clamp_and_clear_pressed_state);
    EGUI_TEST_RUN(test_path_styles);
    EGUI_TEST_RUN(test_path_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
