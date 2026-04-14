#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_activity_ring.h"

#include "../../HelloCustomWidgets/feedback/activity_ring/egui_view_activity_ring.h"
#include "../../HelloCustomWidgets/feedback/activity_ring/egui_view_activity_ring.c"

static egui_view_activity_ring_t test_activity_ring;
static egui_view_activity_ring_t preview_activity_ring;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_activity_ring(void)
{
    egui_timer_init();
    hcw_activity_ring_init(EGUI_VIEW_OF(&test_activity_ring));
    egui_view_set_size(EGUI_VIEW_OF(&test_activity_ring), 88, 88);
    hcw_activity_ring_apply_standard_style(EGUI_VIEW_OF(&test_activity_ring));
    g_click_count = 0;
}

static void setup_preview_activity_ring(void)
{
    egui_timer_init();
    hcw_activity_ring_init(EGUI_VIEW_OF(&preview_activity_ring));
    egui_view_set_size(EGUI_VIEW_OF(&preview_activity_ring), 48, 48);
    hcw_activity_ring_apply_compact_style(EGUI_VIEW_OF(&preview_activity_ring));
    hcw_activity_ring_set_value(EGUI_VIEW_OF(&preview_activity_ring), 38);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_activity_ring), on_preview_click);
    hcw_activity_ring_override_static_preview_api(EGUI_VIEW_OF(&preview_activity_ring), &preview_api);
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

static void test_activity_ring_style_helpers_apply_expected_geometry_and_palette(void)
{
    setup_activity_ring();

    EGUI_TEST_ASSERT_EQUAL_INT(1, test_activity_ring.ring_count);
    EGUI_TEST_ASSERT_EQUAL_INT(8, test_activity_ring.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_activity_ring.ring_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(-90, test_activity_ring.start_angle);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_activity_ring.show_round_cap);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_activity_ring.ring_colors[0].full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD8E1EA).full, test_activity_ring.ring_bg_colors[0].full);
    EGUI_TEST_ASSERT_FALSE(hcw_activity_ring_get_indeterminate_mode(EGUI_VIEW_OF(&test_activity_ring)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_activity_ring), 1);
    hcw_activity_ring_apply_compact_style(EGUI_VIEW_OF(&test_activity_ring));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_activity_ring)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(6, test_activity_ring.stroke_width);
    EGUI_TEST_ASSERT_FALSE(hcw_activity_ring_get_indeterminate_mode(EGUI_VIEW_OF(&test_activity_ring)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_activity_ring), 1);
    hcw_activity_ring_apply_paused_style(EGUI_VIEW_OF(&test_activity_ring));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_activity_ring)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(6, test_activity_ring.stroke_width);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xB95A00).full, test_activity_ring.ring_colors[0].full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xE7DDCA).full, test_activity_ring.ring_bg_colors[0].full);
    EGUI_TEST_ASSERT_FALSE(hcw_activity_ring_get_indeterminate_mode(EGUI_VIEW_OF(&test_activity_ring)));
}

static void test_activity_ring_value_setter_clamps_and_exits_indeterminate_mode(void)
{
    hcw_activity_ring_state_t *state;

    setup_activity_ring();
    hcw_activity_ring_apply_indeterminate_style(EGUI_VIEW_OF(&test_activity_ring));
    state = hcw_activity_ring_find_state(EGUI_VIEW_OF(&test_activity_ring));

    EGUI_TEST_ASSERT_TRUE(state != NULL);
    EGUI_TEST_ASSERT_TRUE(hcw_activity_ring_get_indeterminate_mode(EGUI_VIEW_OF(&test_activity_ring)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, state->timer_started);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_activity_ring), 1);
    hcw_activity_ring_set_value(EGUI_VIEW_OF(&test_activity_ring), 64);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_activity_ring)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(hcw_activity_ring_get_indeterminate_mode(EGUI_VIEW_OF(&test_activity_ring)));
    EGUI_TEST_ASSERT_EQUAL_INT(64, egui_view_activity_ring_get_value(EGUI_VIEW_OF(&test_activity_ring), 0));
    EGUI_TEST_ASSERT_EQUAL_INT(-90, test_activity_ring.start_angle);
    EGUI_TEST_ASSERT_EQUAL_INT(64, state->determinate_value);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_activity_ring), 1);
    hcw_activity_ring_set_value(EGUI_VIEW_OF(&test_activity_ring), 180);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_activity_ring)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(100, egui_view_activity_ring_get_value(EGUI_VIEW_OF(&test_activity_ring), 0));
    EGUI_TEST_ASSERT_EQUAL_INT(100, state->determinate_value);
}

static void test_activity_ring_indeterminate_attach_tick_and_detach_lifecycle(void)
{
    hcw_activity_ring_state_t *state;
    int16_t start_angle_before_tick;
    uint8_t value_before_tick;

    setup_activity_ring();
    hcw_activity_ring_apply_indeterminate_style(EGUI_VIEW_OF(&test_activity_ring));
    state = hcw_activity_ring_find_state(EGUI_VIEW_OF(&test_activity_ring));

    EGUI_TEST_ASSERT_TRUE(state != NULL);
    EGUI_TEST_ASSERT_TRUE(hcw_activity_ring_get_indeterminate_mode(EGUI_VIEW_OF(&test_activity_ring)));
    EGUI_TEST_ASSERT_EQUAL_INT(hcw_activity_ring_get_indeterminate_value(0), egui_view_activity_ring_get_value(EGUI_VIEW_OF(&test_activity_ring), 0));
    EGUI_TEST_ASSERT_EQUAL_INT(0, state->timer_started);

    attach_view(EGUI_VIEW_OF(&test_activity_ring));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_activity_ring)->is_attached_to_window);
    EGUI_TEST_ASSERT_EQUAL_INT(1, state->timer_started);

    start_angle_before_tick = test_activity_ring.start_angle;
    value_before_tick = egui_view_activity_ring_get_value(EGUI_VIEW_OF(&test_activity_ring), 0);
    hcw_activity_ring_tick(&state->anim_timer);
    EGUI_TEST_ASSERT_EQUAL_INT(1, state->phase);
    EGUI_TEST_ASSERT_TRUE(test_activity_ring.start_angle != start_angle_before_tick || egui_view_activity_ring_get_value(EGUI_VIEW_OF(&test_activity_ring), 0) != value_before_tick);

    detach_view(EGUI_VIEW_OF(&test_activity_ring));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_activity_ring)->is_attached_to_window);
    EGUI_TEST_ASSERT_EQUAL_INT(0, state->timer_started);
}

static void test_activity_ring_palette_setter_clears_pressed_state(void)
{
    egui_color_t ring_color = EGUI_COLOR_HEX(0x111111);
    egui_color_t ring_bg_color = EGUI_COLOR_HEX(0x222222);

    setup_activity_ring();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_activity_ring), 1);
    hcw_activity_ring_set_palette(EGUI_VIEW_OF(&test_activity_ring), ring_color, ring_bg_color);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_activity_ring)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(ring_color.full, test_activity_ring.ring_colors[0].full);
    EGUI_TEST_ASSERT_EQUAL_INT(ring_bg_color.full, test_activity_ring.ring_bg_colors[0].full);
}

static void test_activity_ring_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_activity_ring();
    layout_view(EGUI_VIEW_OF(&preview_activity_ring), 12, 18, 48, 48);
    get_view_center(EGUI_VIEW_OF(&preview_activity_ring), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_activity_ring), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_activity_ring), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_activity_ring), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_activity_ring)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_activity_ring), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_activity_ring), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_activity_ring), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_activity_ring)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_activity_ring_run(void)
{
    EGUI_TEST_SUITE_BEGIN(activity_ring);
    EGUI_TEST_RUN(test_activity_ring_style_helpers_apply_expected_geometry_and_palette);
    EGUI_TEST_RUN(test_activity_ring_value_setter_clamps_and_exits_indeterminate_mode);
    EGUI_TEST_RUN(test_activity_ring_indeterminate_attach_tick_and_detach_lifecycle);
    EGUI_TEST_RUN(test_activity_ring_palette_setter_clears_pressed_state);
    EGUI_TEST_RUN(test_activity_ring_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
