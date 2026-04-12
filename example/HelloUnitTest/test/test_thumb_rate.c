#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_thumb_rate.h"

#include "../../HelloCustomWidgets/input/thumb_rate/egui_view_thumb_rate.h"
#include "../../HelloCustomWidgets/input/thumb_rate/egui_view_thumb_rate.c"

static egui_view_thumb_rate_t test_thumb_rate;
static egui_view_thumb_rate_t preview_thumb_rate;
static egui_view_api_t preview_api;
static uint8_t g_changed_state = 0xFF;
static uint8_t g_changed_part = EGUI_VIEW_THUMB_RATE_PART_NONE;
static uint8_t g_changed_count = 0;

static void reset_changed_state(void)
{
    g_changed_state = 0xFF;
    g_changed_part = EGUI_VIEW_THUMB_RATE_PART_NONE;
    g_changed_count = 0;
}

static void on_thumb_rate_changed(egui_view_t *self, uint8_t state, uint8_t part)
{
    EGUI_UNUSED(self);
    g_changed_state = state;
    g_changed_part = part;
    g_changed_count++;
}

static void setup_thumb_rate(uint8_t state)
{
    egui_view_thumb_rate_init(EGUI_VIEW_OF(&test_thumb_rate));
    egui_view_set_size(EGUI_VIEW_OF(&test_thumb_rate), 172, 52);
    egui_view_thumb_rate_apply_standard_style(EGUI_VIEW_OF(&test_thumb_rate));
    egui_view_thumb_rate_set_labels(EGUI_VIEW_OF(&test_thumb_rate), "Like", "Dislike");
    egui_view_thumb_rate_set_on_changed_listener(EGUI_VIEW_OF(&test_thumb_rate), on_thumb_rate_changed);
    egui_view_thumb_rate_set_state(EGUI_VIEW_OF(&test_thumb_rate), state);
    egui_view_thumb_rate_set_current_part(EGUI_VIEW_OF(&test_thumb_rate),
                                          state == EGUI_VIEW_THUMB_RATE_STATE_DISLIKED ? EGUI_VIEW_THUMB_RATE_PART_DISLIKE : EGUI_VIEW_THUMB_RATE_PART_LIKE);
    reset_changed_state();
}

static void setup_preview_thumb_rate(uint8_t state)
{
    egui_view_thumb_rate_init(EGUI_VIEW_OF(&preview_thumb_rate));
    egui_view_set_size(EGUI_VIEW_OF(&preview_thumb_rate), 96, 36);
    egui_view_thumb_rate_apply_compact_style(EGUI_VIEW_OF(&preview_thumb_rate));
    egui_view_thumb_rate_set_labels(EGUI_VIEW_OF(&preview_thumb_rate), "Like", "Dislike");
    egui_view_thumb_rate_set_state(EGUI_VIEW_OF(&preview_thumb_rate), state);
    egui_view_thumb_rate_set_current_part(EGUI_VIEW_OF(&preview_thumb_rate),
                                          state == EGUI_VIEW_THUMB_RATE_STATE_DISLIKED ? EGUI_VIEW_THUMB_RATE_PART_DISLIKE : EGUI_VIEW_THUMB_RATE_PART_LIKE);
    egui_view_thumb_rate_override_static_preview_api(EGUI_VIEW_OF(&preview_thumb_rate), &preview_api);
}

static void layout_thumb_rate(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
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

static int send_key(egui_view_t *view, uint8_t key_code)
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

static void get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(egui_view_thumb_rate_get_part_region(view, part, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static void assert_changed_state(uint8_t count, uint8_t state, uint8_t part)
{
    EGUI_TEST_ASSERT_EQUAL_INT(count, g_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(state, g_changed_state);
    EGUI_TEST_ASSERT_EQUAL_INT(part, g_changed_part);
}

static void test_thumb_rate_style_helpers_update_flags_and_clear_pressed_state(void)
{
    setup_thumb_rate(EGUI_VIEW_THUMB_RATE_STATE_LIKED);

    EGUI_TEST_ASSERT_EQUAL_INT(0, test_thumb_rate.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_thumb_rate.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_thumb_rate.like_color.full);

    EGUI_VIEW_OF(&test_thumb_rate)->is_pressed = 1;
    test_thumb_rate.pressed_part = EGUI_VIEW_THUMB_RATE_PART_LIKE;
    egui_view_thumb_rate_apply_compact_style(EGUI_VIEW_OF(&test_thumb_rate));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_thumb_rate)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_NONE, test_thumb_rate.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_thumb_rate.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_thumb_rate.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F766E).full, test_thumb_rate.like_color.full);

    EGUI_VIEW_OF(&test_thumb_rate)->is_pressed = 1;
    test_thumb_rate.pressed_part = EGUI_VIEW_THUMB_RATE_PART_DISLIKE;
    egui_view_thumb_rate_apply_read_only_style(EGUI_VIEW_OF(&test_thumb_rate));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_thumb_rate)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_NONE, test_thumb_rate.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_thumb_rate.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_thumb_rate.read_only_mode);
}

static void test_thumb_rate_setters_clear_pressed_state_and_normalize(void)
{
    setup_thumb_rate(EGUI_VIEW_THUMB_RATE_STATE_NONE);

    EGUI_VIEW_OF(&test_thumb_rate)->is_pressed = 1;
    test_thumb_rate.pressed_part = EGUI_VIEW_THUMB_RATE_PART_LIKE;
    egui_view_thumb_rate_set_labels(EGUI_VIEW_OF(&test_thumb_rate), "Helpful", "Not useful");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_thumb_rate)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_NONE, test_thumb_rate.pressed_part);
    EGUI_TEST_ASSERT_TRUE(strcmp(test_thumb_rate.like_label, "Helpful") == 0);

    EGUI_VIEW_OF(&test_thumb_rate)->is_pressed = 1;
    test_thumb_rate.pressed_part = EGUI_VIEW_THUMB_RATE_PART_DISLIKE;
    egui_view_thumb_rate_set_palette(EGUI_VIEW_OF(&test_thumb_rate), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                     EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_thumb_rate)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_NONE, test_thumb_rate.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_thumb_rate.like_color.full);

    EGUI_VIEW_OF(&test_thumb_rate)->is_pressed = 1;
    test_thumb_rate.pressed_part = EGUI_VIEW_THUMB_RATE_PART_LIKE;
    egui_view_thumb_rate_set_state(EGUI_VIEW_OF(&test_thumb_rate), 9);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_thumb_rate)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_NONE, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));

    egui_view_thumb_rate_set_current_part(EGUI_VIEW_OF(&test_thumb_rate), EGUI_VIEW_THUMB_RATE_PART_DISLIKE);
    egui_view_thumb_rate_set_current_part(EGUI_VIEW_OF(&test_thumb_rate), 8);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_DISLIKE, egui_view_thumb_rate_get_current_part(EGUI_VIEW_OF(&test_thumb_rate)));
}

static void test_thumb_rate_touch_click_commits_and_same_part_click_toggles_off(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_thumb_rate(EGUI_VIEW_THUMB_RATE_STATE_NONE);
    layout_thumb_rate(EGUI_VIEW_OF(&test_thumb_rate), 12, 18, 172, 52);
    get_part_center(EGUI_VIEW_OF(&test_thumb_rate), EGUI_VIEW_THUMB_RATE_PART_LIKE, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_LIKED, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));
    assert_changed_state(1, EGUI_VIEW_THUMB_RATE_STATE_LIKED, EGUI_VIEW_THUMB_RATE_PART_LIKE);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_NONE, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));
    assert_changed_state(2, EGUI_VIEW_THUMB_RATE_STATE_NONE, EGUI_VIEW_THUMB_RATE_PART_LIKE);
}

static void test_thumb_rate_same_target_release_requires_return_to_origin(void)
{
    egui_dim_t like_x;
    egui_dim_t like_y;
    egui_dim_t dislike_x;
    egui_dim_t dislike_y;

    setup_thumb_rate(EGUI_VIEW_THUMB_RATE_STATE_NONE);
    layout_thumb_rate(EGUI_VIEW_OF(&test_thumb_rate), 12, 18, 172, 52);
    get_part_center(EGUI_VIEW_OF(&test_thumb_rate), EGUI_VIEW_THUMB_RATE_PART_LIKE, &like_x, &like_y);
    get_part_center(EGUI_VIEW_OF(&test_thumb_rate), EGUI_VIEW_THUMB_RATE_PART_DISLIKE, &dislike_x, &dislike_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_DOWN, like_x, like_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_MOVE, dislike_x, dislike_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_UP, dislike_x, dislike_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_NONE, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_DOWN, like_x, like_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_MOVE, dislike_x, dislike_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_MOVE, like_x, like_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_UP, like_x, like_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_LIKED, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));
    assert_changed_state(1, EGUI_VIEW_THUMB_RATE_STATE_LIKED, EGUI_VIEW_THUMB_RATE_PART_LIKE);
}

static void test_thumb_rate_keyboard_navigation_and_commit(void)
{
    setup_thumb_rate(EGUI_VIEW_THUMB_RATE_STATE_NONE);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_thumb_rate), EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_DISLIKE, egui_view_thumb_rate_get_current_part(EGUI_VIEW_OF(&test_thumb_rate)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_thumb_rate), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_DISLIKED, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));
    assert_changed_state(1, EGUI_VIEW_THUMB_RATE_STATE_DISLIKED, EGUI_VIEW_THUMB_RATE_PART_DISLIKE);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_thumb_rate), EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_LIKE, egui_view_thumb_rate_get_current_part(EGUI_VIEW_OF(&test_thumb_rate)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_thumb_rate), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_LIKED, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));
    assert_changed_state(2, EGUI_VIEW_THUMB_RATE_STATE_LIKED, EGUI_VIEW_THUMB_RATE_PART_LIKE);
}

static void test_thumb_rate_escape_clears_state(void)
{
    setup_thumb_rate(EGUI_VIEW_THUMB_RATE_STATE_DISLIKED);
    egui_view_thumb_rate_set_current_part(EGUI_VIEW_OF(&test_thumb_rate), EGUI_VIEW_THUMB_RATE_PART_DISLIKE);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_thumb_rate), EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_NONE, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));
    assert_changed_state(1, EGUI_VIEW_THUMB_RATE_STATE_NONE, EGUI_VIEW_THUMB_RATE_PART_DISLIKE);
}

static void test_thumb_rate_disabled_and_read_only_clear_pressed_state_without_commit(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_thumb_rate(EGUI_VIEW_THUMB_RATE_STATE_NONE);
    layout_thumb_rate(EGUI_VIEW_OF(&test_thumb_rate), 12, 18, 172, 52);
    get_part_center(EGUI_VIEW_OF(&test_thumb_rate), EGUI_VIEW_THUMB_RATE_PART_LIKE, &x, &y);

    egui_view_thumb_rate_set_read_only_mode(EGUI_VIEW_OF(&test_thumb_rate), 1);
    EGUI_VIEW_OF(&test_thumb_rate)->is_pressed = 1;
    test_thumb_rate.pressed_part = EGUI_VIEW_THUMB_RATE_PART_LIKE;
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_thumb_rate)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_NONE, test_thumb_rate.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_thumb_rate), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_NONE, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);

    egui_view_thumb_rate_set_read_only_mode(EGUI_VIEW_OF(&test_thumb_rate), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_thumb_rate), 0);
    EGUI_VIEW_OF(&test_thumb_rate)->is_pressed = 1;
    test_thumb_rate.pressed_part = EGUI_VIEW_THUMB_RATE_PART_LIKE;
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_VIEW_OF(&test_thumb_rate), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_thumb_rate)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_NONE, test_thumb_rate.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&test_thumb_rate), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_NONE, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&test_thumb_rate)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_thumb_rate_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_thumb_rate(EGUI_VIEW_THUMB_RATE_STATE_LIKED);
    layout_thumb_rate(EGUI_VIEW_OF(&preview_thumb_rate), 12, 18, 96, 36);
    get_part_center(EGUI_VIEW_OF(&preview_thumb_rate), EGUI_VIEW_THUMB_RATE_PART_LIKE, &x, &y);

    EGUI_VIEW_OF(&preview_thumb_rate)->is_pressed = 1;
    preview_thumb_rate.pressed_part = EGUI_VIEW_THUMB_RATE_PART_LIKE;
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_thumb_rate), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_thumb_rate)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_NONE, preview_thumb_rate.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_LIKED, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&preview_thumb_rate)));

    EGUI_VIEW_OF(&preview_thumb_rate)->is_pressed = 1;
    preview_thumb_rate.pressed_part = EGUI_VIEW_THUMB_RATE_PART_DISLIKE;
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_thumb_rate), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_thumb_rate)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_PART_NONE, preview_thumb_rate.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_THUMB_RATE_STATE_LIKED, egui_view_thumb_rate_get_state(EGUI_VIEW_OF(&preview_thumb_rate)));
}

void test_thumb_rate_run(void)
{
    EGUI_TEST_SUITE_BEGIN(thumb_rate);
    EGUI_TEST_RUN(test_thumb_rate_style_helpers_update_flags_and_clear_pressed_state);
    EGUI_TEST_RUN(test_thumb_rate_setters_clear_pressed_state_and_normalize);
    EGUI_TEST_RUN(test_thumb_rate_touch_click_commits_and_same_part_click_toggles_off);
    EGUI_TEST_RUN(test_thumb_rate_same_target_release_requires_return_to_origin);
    EGUI_TEST_RUN(test_thumb_rate_keyboard_navigation_and_commit);
    EGUI_TEST_RUN(test_thumb_rate_escape_clears_state);
    EGUI_TEST_RUN(test_thumb_rate_disabled_and_read_only_clear_pressed_state_without_commit);
    EGUI_TEST_RUN(test_thumb_rate_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
