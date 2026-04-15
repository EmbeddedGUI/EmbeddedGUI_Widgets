#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_arc.h"

#include "../../HelloCustomWidgets/display/arc/egui_view_arc.h"
#include "../../HelloCustomWidgets/display/arc/egui_view_arc.c"

typedef struct arc_preview_snapshot arc_preview_snapshot_t;
struct arc_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    egui_color_t track_color;
    egui_color_t active_color;
    uint8_t value;
    egui_dim_t stroke_width;
    int16_t start_angle;
    int16_t sweep_angle;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_arc_t test_arc;
static egui_view_arc_t preview_arc;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_arc(void)
{
    egui_view_arc_init(EGUI_VIEW_OF(&test_arc));
    egui_view_set_size(EGUI_VIEW_OF(&test_arc), 64, 64);
    egui_view_arc_apply_standard_style(EGUI_VIEW_OF(&test_arc));
    g_click_count = 0;
}

static void setup_preview_arc(void)
{
    egui_view_arc_init(EGUI_VIEW_OF(&preview_arc));
    egui_view_set_size(EGUI_VIEW_OF(&preview_arc), 42, 42);
    egui_view_arc_apply_subtle_style(EGUI_VIEW_OF(&preview_arc));
    egui_view_arc_set_value(EGUI_VIEW_OF(&preview_arc), 24);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_arc), on_preview_click);
    egui_view_arc_override_static_preview_api(EGUI_VIEW_OF(&preview_arc), &preview_api);
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

static int send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(arc_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_arc)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_arc)->background;
    snapshot->track_color = preview_arc.track_color;
    snapshot->active_color = preview_arc.active_color;
    snapshot->value = preview_arc.value;
    snapshot->stroke_width = preview_arc.stroke_width;
    snapshot->start_angle = preview_arc.start_angle;
    snapshot->sweep_angle = preview_arc.sweep_angle;
    snapshot->alpha = EGUI_VIEW_OF(&preview_arc)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_arc));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_arc)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_arc)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_arc)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_arc)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_arc)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_arc)->padding.bottom;
}

static void assert_preview_state_unchanged(const arc_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_arc)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_arc)->background == snapshot->background);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->track_color.full, preview_arc.track_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->active_color.full, preview_arc.active_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->value, preview_arc.value);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->stroke_width, preview_arc.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->start_angle, preview_arc.start_angle);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->sweep_angle, preview_arc.sweep_angle);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_arc)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_arc)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_arc)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_arc)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_arc)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_arc)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_arc)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_arc)->padding.bottom);
}

static void test_arc_style_helpers_apply_expected_palette_and_geometry(void)
{
    setup_arc();

    EGUI_TEST_ASSERT_EQUAL_INT(8, test_arc.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(140, test_arc.start_angle);
    EGUI_TEST_ASSERT_EQUAL_INT(260, test_arc.sweep_angle);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD9E2EA).full, test_arc.track_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_arc.active_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_arc), 1);
    egui_view_arc_apply_subtle_style(EGUI_VIEW_OF(&test_arc));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_arc)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(6, test_arc.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xE5EBF0).full, test_arc.track_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x8A9AA9).full, test_arc.active_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_arc), 1);
    egui_view_arc_apply_attention_style(EGUI_VIEW_OF(&test_arc));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_arc)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(8, test_arc.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xEFD8D4).full, test_arc.track_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC42B1C).full, test_arc.active_color.full);
}

static void test_arc_setters_clamp_and_clear_pressed_state(void)
{
    setup_arc();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_arc), 1);
    egui_view_arc_set_value(EGUI_VIEW_OF(&test_arc), 130);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_arc)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(100, test_arc.value);
    EGUI_TEST_ASSERT_EQUAL_INT(100, egui_view_arc_get_value(EGUI_VIEW_OF(&test_arc)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_arc), 1);
    egui_view_arc_set_angles(EGUI_VIEW_OF(&test_arc), -30, 420);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_arc)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(330, test_arc.start_angle);
    EGUI_TEST_ASSERT_EQUAL_INT(359, test_arc.sweep_angle);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_arc), 1);
    egui_view_arc_set_stroke_width(EGUI_VIEW_OF(&test_arc), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_arc)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_arc.stroke_width);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_arc), 1);
    egui_view_arc_set_palette(EGUI_VIEW_OF(&test_arc), EGUI_COLOR_HEX(0x223344), EGUI_COLOR_HEX(0x556677));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_arc)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x223344).full, test_arc.track_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x556677).full, test_arc.active_color.full);
}

static void test_arc_static_preview_consumes_input_and_keeps_state(void)
{
    arc_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_arc();
    layout_view(EGUI_VIEW_OF(&preview_arc), 12, 18, 42, 42);
    get_view_center(EGUI_VIEW_OF(&preview_arc), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_arc), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_arc), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_arc), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_arc), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_arc_run(void)
{
    EGUI_TEST_SUITE_BEGIN(arc);
    EGUI_TEST_RUN(test_arc_style_helpers_apply_expected_palette_and_geometry);
    EGUI_TEST_RUN(test_arc_setters_clamp_and_clear_pressed_state);
    EGUI_TEST_RUN(test_arc_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
