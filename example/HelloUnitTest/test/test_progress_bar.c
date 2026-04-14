#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_progress_bar.h"

#include "../../HelloCustomWidgets/feedback/progress_bar/egui_view_progress_bar.h"
#include "../../HelloCustomWidgets/feedback/progress_bar/egui_view_progress_bar.c"

static egui_view_progress_bar_t test_progress_bar;
static egui_view_progress_bar_t preview_progress_bar;
static egui_view_api_t preview_api;
static uint8_t g_last_progress;
static uint8_t g_progress_notify_count;
static uint8_t g_click_count;

static void on_progress_changed(egui_view_t *self, uint8_t progress)
{
    EGUI_UNUSED(self);
    g_last_progress = progress;
    g_progress_notify_count++;
}

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_progress_bar(void)
{
    egui_timer_init();
    hcw_progress_bar_init(EGUI_VIEW_OF(&test_progress_bar));
    egui_view_set_size(EGUI_VIEW_OF(&test_progress_bar), 196, 18);
    hcw_progress_bar_apply_standard_style(EGUI_VIEW_OF(&test_progress_bar));
    egui_view_progress_bar_set_on_progress_listener(EGUI_VIEW_OF(&test_progress_bar), on_progress_changed);
    g_last_progress = test_progress_bar.process;
    g_progress_notify_count = 0;
    g_click_count = 0;
}

static void setup_preview_progress_bar(void)
{
    egui_timer_init();
    hcw_progress_bar_init(EGUI_VIEW_OF(&preview_progress_bar));
    egui_view_set_size(EGUI_VIEW_OF(&preview_progress_bar), 96, 12);
    hcw_progress_bar_apply_paused_style(EGUI_VIEW_OF(&preview_progress_bar));
    hcw_progress_bar_set_value(EGUI_VIEW_OF(&preview_progress_bar), 46);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_progress_bar), on_preview_click);
    hcw_progress_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_progress_bar), &preview_api);
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

static void test_progress_bar_style_helpers_apply_expected_palette(void)
{
    setup_progress_bar();

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_progress_bar)->background == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD8E1EA).full, test_progress_bar.bk_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_progress_bar.progress_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_progress_bar.is_show_control);
    EGUI_TEST_ASSERT_FALSE(hcw_progress_bar_get_indeterminate_mode(EGUI_VIEW_OF(&test_progress_bar)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_progress_bar), 1);
    hcw_progress_bar_apply_paused_style(EGUI_VIEW_OF(&test_progress_bar));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_progress_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xE7DDCA).full, test_progress_bar.bk_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xB95A00).full, test_progress_bar.progress_color.full);
    EGUI_TEST_ASSERT_FALSE(hcw_progress_bar_get_indeterminate_mode(EGUI_VIEW_OF(&test_progress_bar)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_progress_bar), 1);
    hcw_progress_bar_apply_error_style(EGUI_VIEW_OF(&test_progress_bar));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_progress_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xEED6DA).full, test_progress_bar.bk_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC42B1C).full, test_progress_bar.progress_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_progress_bar), 1);
    hcw_progress_bar_apply_indeterminate_style(EGUI_VIEW_OF(&test_progress_bar));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_progress_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(hcw_progress_bar_get_indeterminate_mode(EGUI_VIEW_OF(&test_progress_bar)));
}

static void test_progress_bar_set_value_clamps_and_notifies_listener(void)
{
    hcw_progress_bar_state_t *state;

    setup_progress_bar();
    hcw_progress_bar_apply_indeterminate_style(EGUI_VIEW_OF(&test_progress_bar));
    state = hcw_progress_bar_find_state(EGUI_VIEW_OF(&test_progress_bar));

    EGUI_TEST_ASSERT_TRUE(state != NULL);
    EGUI_TEST_ASSERT_TRUE(hcw_progress_bar_get_indeterminate_mode(EGUI_VIEW_OF(&test_progress_bar)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_progress_bar), 1);
    hcw_progress_bar_set_value(EGUI_VIEW_OF(&test_progress_bar), 64);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_progress_bar)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(hcw_progress_bar_get_indeterminate_mode(EGUI_VIEW_OF(&test_progress_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(64, test_progress_bar.process);
    EGUI_TEST_ASSERT_EQUAL_INT(64, g_last_progress);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_progress_notify_count);
    EGUI_TEST_ASSERT_EQUAL_INT(64, state->determinate_value);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_progress_bar), 1);
    hcw_progress_bar_set_value(EGUI_VIEW_OF(&test_progress_bar), 180);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_progress_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(100, test_progress_bar.process);
    EGUI_TEST_ASSERT_EQUAL_INT(100, g_last_progress);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_progress_notify_count);

    hcw_progress_bar_set_value(EGUI_VIEW_OF(&test_progress_bar), 100);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_progress_notify_count);
}

static void test_progress_bar_indeterminate_attach_tick_and_detach_lifecycle(void)
{
    hcw_progress_bar_state_t *state;
    uint8_t phase_before_tick;

    setup_progress_bar();
    hcw_progress_bar_apply_indeterminate_style(EGUI_VIEW_OF(&test_progress_bar));
    state = hcw_progress_bar_find_state(EGUI_VIEW_OF(&test_progress_bar));

    EGUI_TEST_ASSERT_TRUE(state != NULL);
    EGUI_TEST_ASSERT_TRUE(hcw_progress_bar_get_indeterminate_mode(EGUI_VIEW_OF(&test_progress_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, state->timer_started);

    attach_view(EGUI_VIEW_OF(&test_progress_bar));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_progress_bar)->is_attached_to_window);
    EGUI_TEST_ASSERT_EQUAL_INT(1, state->timer_started);

    phase_before_tick = state->phase;
    hcw_progress_bar_tick(&state->anim_timer);
    EGUI_TEST_ASSERT_EQUAL_INT((phase_before_tick + 1) % HCW_PROGRESS_BAR_PHASE_COUNT, state->phase);

    detach_view(EGUI_VIEW_OF(&test_progress_bar));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_progress_bar)->is_attached_to_window);
    EGUI_TEST_ASSERT_EQUAL_INT(0, state->timer_started);
}

static void test_progress_bar_palette_setter_clears_pressed_state(void)
{
    egui_color_t track = EGUI_COLOR_HEX(0x111111);
    egui_color_t fill = EGUI_COLOR_HEX(0x222222);
    egui_color_t control = EGUI_COLOR_HEX(0x333333);

    setup_progress_bar();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_progress_bar), 1);
    hcw_progress_bar_set_palette(EGUI_VIEW_OF(&test_progress_bar), track, fill, control);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_progress_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(track.full, test_progress_bar.bk_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(fill.full, test_progress_bar.progress_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(control.full, test_progress_bar.control_color.full);
}

static void test_progress_bar_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_progress_bar();
    layout_view(EGUI_VIEW_OF(&preview_progress_bar), 12, 18, 96, 12);
    get_view_center(EGUI_VIEW_OF(&preview_progress_bar), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_progress_bar), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_progress_bar), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_progress_bar), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_progress_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_progress_bar), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_progress_bar), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_progress_bar), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_progress_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_progress_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(progress_bar);
    EGUI_TEST_RUN(test_progress_bar_style_helpers_apply_expected_palette);
    EGUI_TEST_RUN(test_progress_bar_set_value_clamps_and_notifies_listener);
    EGUI_TEST_RUN(test_progress_bar_indeterminate_attach_tick_and_detach_lifecycle);
    EGUI_TEST_RUN(test_progress_bar_palette_setter_clears_pressed_state);
    EGUI_TEST_RUN(test_progress_bar_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
