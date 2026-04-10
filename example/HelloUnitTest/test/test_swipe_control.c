#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_swipe_control.h"

#include "../../HelloCustomWidgets/input/swipe_control/egui_view_swipe_control.h"
#include "../../HelloCustomWidgets/input/swipe_control/egui_view_swipe_control.c"

static egui_view_swipe_control_t test_swipe_control;
static egui_view_swipe_control_t preview_swipe_control;
static egui_view_api_t preview_api;
static uint8_t g_changed_count = 0;
static uint8_t g_changed_reveal = 0xFF;
static uint8_t g_changed_part = EGUI_VIEW_SWIPE_CONTROL_PART_NONE;

static const egui_view_swipe_control_item_t unit_item = {
        "Mail", "Inbox row", "Single row with reveal actions", "Ready", EGUI_COLOR_HEX(0xE9F4FF), EGUI_COLOR_HEX(0x2563EB)};
static const egui_view_swipe_control_action_t unit_start_action = {"Pin", "Keep", EGUI_COLOR_HEX(0x0F766E), EGUI_COLOR_HEX(0xFFFFFF)};
static const egui_view_swipe_control_action_t unit_end_action = {"Delete", "Remove", EGUI_COLOR_HEX(0xDC2626), EGUI_COLOR_HEX(0xFFFFFF)};

static void reset_changed_state(void)
{
    g_changed_count = 0;
    g_changed_reveal = 0xFF;
    g_changed_part = EGUI_VIEW_SWIPE_CONTROL_PART_NONE;
}

static void on_swipe_changed(egui_view_t *self, uint8_t reveal_state, uint8_t current_part)
{
    EGUI_UNUSED(self);
    g_changed_count++;
    g_changed_reveal = reveal_state;
    g_changed_part = current_part;
}

static void setup_swipe_control(void)
{
    egui_view_swipe_control_init(EGUI_VIEW_OF(&test_swipe_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_swipe_control), 160, 92);
    egui_view_swipe_control_set_item(EGUI_VIEW_OF(&test_swipe_control), &unit_item);
    egui_view_swipe_control_set_actions(EGUI_VIEW_OF(&test_swipe_control), &unit_start_action, &unit_end_action);
    egui_view_swipe_control_set_on_changed_listener(EGUI_VIEW_OF(&test_swipe_control), on_swipe_changed);
    egui_view_swipe_control_set_reveal_state(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE);
    reset_changed_state();
}

static void setup_preview_swipe_control(void)
{
    egui_view_swipe_control_init(EGUI_VIEW_OF(&preview_swipe_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_swipe_control), 104, 64);
    egui_view_swipe_control_set_item(EGUI_VIEW_OF(&preview_swipe_control), &unit_item);
    egui_view_swipe_control_set_actions(EGUI_VIEW_OF(&preview_swipe_control), &unit_start_action, &unit_end_action);
    egui_view_swipe_control_set_compact_mode(EGUI_VIEW_OF(&preview_swipe_control), 1);
    egui_view_swipe_control_set_reveal_state(EGUI_VIEW_OF(&preview_swipe_control), EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE);
    egui_view_swipe_control_override_static_preview_api(EGUI_VIEW_OF(&preview_swipe_control), &preview_api);
}

static void layout_swipe_control(egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(EGUI_VIEW_OF(&test_swipe_control), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_swipe_control)->region_screen, &region);
}

static void layout_preview_swipe_control(egui_dim_t x, egui_dim_t y)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = 104;
    region.size.height = 64;
    egui_view_layout(EGUI_VIEW_OF(&preview_swipe_control), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_swipe_control)->region_screen, &region);
}

static void get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(egui_view_swipe_control_get_part_region(view, part, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static int send_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&test_swipe_control)->api->on_touch_event(EGUI_VIEW_OF(&test_swipe_control), &event);
}

static int send_key_event(uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return EGUI_VIEW_OF(&test_swipe_control)->api->on_key_event(EGUI_VIEW_OF(&test_swipe_control), &event);
}

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_preview_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&preview_swipe_control)->api->on_touch_event(EGUI_VIEW_OF(&preview_swipe_control), &event);
}

static int send_preview_key_event(uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return EGUI_VIEW_OF(&preview_swipe_control)->api->on_key_event(EGUI_VIEW_OF(&preview_swipe_control), &event);
}

static void assert_changed_state(uint8_t count, uint8_t reveal_state, uint8_t current_part)
{
    EGUI_TEST_ASSERT_EQUAL_INT(count, g_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(reveal_state, g_changed_reveal);
    EGUI_TEST_ASSERT_EQUAL_INT(current_part, g_changed_part);
}

static void test_swipe_control_default_state(void)
{
    setup_swipe_control();
    EGUI_TEST_ASSERT_TRUE(egui_view_swipe_control_get_item(EGUI_VIEW_OF(&test_swipe_control)) != NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
}

static void test_swipe_control_setters_clear_pressed_state(void)
{
    setup_swipe_control();

    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    test_swipe_control.dragging = 1;
    egui_view_swipe_control_set_title(EGUI_VIEW_OF(&test_swipe_control), "Inbox");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_swipe_control.dragging);

    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    egui_view_swipe_control_set_helper(EGUI_VIEW_OF(&test_swipe_control), "Swipe right");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);

    egui_view_swipe_control_set_reveal_state(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_REVEAL_START);
    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION;
    test_swipe_control.dragging = 1;
    egui_view_swipe_control_set_item(EGUI_VIEW_OF(&test_swipe_control), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_swipe_control.dragging);
    EGUI_TEST_ASSERT_TRUE(egui_view_swipe_control_get_item(EGUI_VIEW_OF(&test_swipe_control)) == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));

    egui_view_swipe_control_set_item(EGUI_VIEW_OF(&test_swipe_control), &unit_item);
    egui_view_swipe_control_set_actions(EGUI_VIEW_OF(&test_swipe_control), &unit_start_action, &unit_end_action);
    egui_view_swipe_control_set_reveal_state(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_REVEAL_START);
    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION;
    egui_view_swipe_control_set_actions(EGUI_VIEW_OF(&test_swipe_control), NULL, &unit_end_action);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_TRUE(test_swipe_control.start_action == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));

    egui_view_swipe_control_set_actions(EGUI_VIEW_OF(&test_swipe_control), &unit_start_action, &unit_end_action);
    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    test_swipe_control.dragging = 1;
    egui_view_swipe_control_set_reveal_state(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_REVEAL_END);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_swipe_control.dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_END, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_END_ACTION, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));

    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_END_ACTION;
    egui_view_swipe_control_set_current_part(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_START, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));

    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION;
    egui_view_swipe_control_set_palette(EGUI_VIEW_OF(&test_swipe_control), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                        EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);

    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_END_ACTION;
    egui_view_swipe_control_set_compact_mode(EGUI_VIEW_OF(&test_swipe_control), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));

    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    egui_view_swipe_control_set_read_only_mode(EGUI_VIEW_OF(&test_swipe_control), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
}

static void test_swipe_control_keyboard_reveal_cycle(void)
{
    setup_swipe_control();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_START, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
    assert_changed_state(1, EGUI_VIEW_SWIPE_CONTROL_REVEAL_START, EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION);

    reset_changed_state();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_END, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_END_ACTION, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
    assert_changed_state(1, EGUI_VIEW_SWIPE_CONTROL_REVEAL_END, EGUI_VIEW_SWIPE_CONTROL_PART_END_ACTION);

    reset_changed_state();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
    assert_changed_state(1, EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE);
}

static void test_swipe_control_tab_cycles_parts(void)
{
    setup_swipe_control();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_END_ACTION, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
}

static void test_swipe_control_touch_swipe_reveals_actions(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_swipe_control();
    layout_swipe_control(10, 20, 160, 92);
    get_part_center(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x + 24, y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x + 24, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_START, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_swipe_control.dragging);
    assert_changed_state(1, EGUI_VIEW_SWIPE_CONTROL_REVEAL_START, EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION);

    reset_changed_state();
    egui_view_swipe_control_set_reveal_state(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE);
    layout_swipe_control(10, 20, 160, 92);
    get_part_center(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x - 24, y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x - 24, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_END, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_END_ACTION, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_swipe_control.dragging);
    assert_changed_state(1, EGUI_VIEW_SWIPE_CONTROL_REVEAL_END, EGUI_VIEW_SWIPE_CONTROL_PART_END_ACTION);
}

static void test_swipe_control_part_region_exposes_visible_action(void)
{
    egui_region_t region;

    setup_swipe_control();
    layout_swipe_control(10, 20, 160, 92);
    EGUI_TEST_ASSERT_FALSE(egui_view_swipe_control_get_part_region(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION, &region));

    egui_view_swipe_control_set_reveal_state(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_REVEAL_START);
    layout_swipe_control(10, 20, 160, 92);
    EGUI_TEST_ASSERT_TRUE(egui_view_swipe_control_get_part_region(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION, &region));
    EGUI_TEST_ASSERT_TRUE(region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(region.size.height > 0);
}

static void test_swipe_control_surface_tap_closes_reveal(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_swipe_control();
    egui_view_swipe_control_set_reveal_state(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_REVEAL_START);
    layout_swipe_control(10, 20, 160, 92);
    get_part_center(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&test_swipe_control)));
    assert_changed_state(1, EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE);
}

static void test_swipe_control_release_requires_same_target_when_not_dragging(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_swipe_control();
    layout_swipe_control(10, 20, 160, 92);
    get_part_center(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, 300, 300));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);

    reset_changed_state();
    egui_view_swipe_control_set_reveal_state(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_REVEAL_START);
    layout_swipe_control(10, 20, 160, 92);
    get_part_center(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_START_ACTION, &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, 300, 300));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_START, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&test_swipe_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_swipe_control_touch_cancel_clears_pressed_state_without_notify(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_swipe_control();
    layout_swipe_control(10, 20, 160, 92);
    get_part_center(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, test_swipe_control.pressed_part);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_swipe_control.dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_swipe_control_compact_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_swipe_control();
    layout_swipe_control(10, 20, 160, 92);
    get_part_center(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, &x, &y);

    egui_view_swipe_control_set_compact_mode(EGUI_VIEW_OF(&test_swipe_control), 1);
    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    test_swipe_control.dragging = 1;
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_swipe_control.dragging);
    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);

    setup_swipe_control();
    layout_swipe_control(10, 20, 160, 92);
    get_part_center(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, &x, &y);
    egui_view_swipe_control_set_read_only_mode(EGUI_VIEW_OF(&test_swipe_control), 1);
    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    test_swipe_control.dragging = 1;
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_swipe_control.dragging);
    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);

    setup_swipe_control();
    layout_swipe_control(10, 20, 160, 92);
    get_part_center(EGUI_VIEW_OF(&test_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, &x, &y);
    egui_view_set_enable(EGUI_VIEW_OF(&test_swipe_control), 0);
    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    test_swipe_control.dragging = 1;
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_swipe_control.dragging);
    EGUI_VIEW_OF(&test_swipe_control)->is_pressed = 1;
    test_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, test_swipe_control.pressed_part);
}

static void test_swipe_control_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_swipe_control();
    layout_preview_swipe_control(10, 20);
    get_part_center(EGUI_VIEW_OF(&preview_swipe_control), EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, &x, &y);

    EGUI_VIEW_OF(&preview_swipe_control)->is_pressed = 1;
    preview_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    preview_swipe_control.dragging = 1;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, preview_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_swipe_control.dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&preview_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&preview_swipe_control)));

    EGUI_VIEW_OF(&preview_swipe_control)->is_pressed = 1;
    preview_swipe_control.pressed_part = EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE;
    preview_swipe_control.dragging = 1;
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_swipe_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_NONE, preview_swipe_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_swipe_control.dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_REVEAL_NONE, egui_view_swipe_control_get_reveal_state(EGUI_VIEW_OF(&preview_swipe_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SWIPE_CONTROL_PART_SURFACE, egui_view_swipe_control_get_current_part(EGUI_VIEW_OF(&preview_swipe_control)));
}

void test_swipe_control_run(void)
{
    EGUI_TEST_SUITE_BEGIN(swipe_control);
    EGUI_TEST_RUN(test_swipe_control_default_state);
    EGUI_TEST_RUN(test_swipe_control_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_swipe_control_keyboard_reveal_cycle);
    EGUI_TEST_RUN(test_swipe_control_tab_cycles_parts);
    EGUI_TEST_RUN(test_swipe_control_touch_swipe_reveals_actions);
    EGUI_TEST_RUN(test_swipe_control_part_region_exposes_visible_action);
    EGUI_TEST_RUN(test_swipe_control_surface_tap_closes_reveal);
    EGUI_TEST_RUN(test_swipe_control_release_requires_same_target_when_not_dragging);
    EGUI_TEST_RUN(test_swipe_control_touch_cancel_clears_pressed_state_without_notify);
    EGUI_TEST_RUN(test_swipe_control_compact_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_swipe_control_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
