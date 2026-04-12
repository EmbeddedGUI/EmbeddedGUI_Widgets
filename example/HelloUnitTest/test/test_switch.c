#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_switch.h"

#include "../../HelloCustomWidgets/input/switch/egui_view_switch.h"
#include "../../HelloCustomWidgets/input/switch/egui_view_switch.c"

static egui_view_switch_t test_switch;
static egui_view_switch_t preview_switch;
static egui_view_api_t test_switch_api;
static egui_view_api_t preview_switch_api;
static uint8_t g_checked_count;
static uint8_t g_last_checked;

static void on_checked(egui_view_t *self, int is_checked)
{
    EGUI_UNUSED(self);
    g_checked_count++;
    g_last_checked = (uint8_t)is_checked;
}

static void reset_listener_state(void)
{
    g_checked_count = 0;
    g_last_checked = 0xFF;
}

static void setup_switch(uint8_t checked)
{
    egui_view_switch_init(EGUI_VIEW_OF(&test_switch));
    egui_view_set_size(EGUI_VIEW_OF(&test_switch), 112, 44);
    hcw_switch_apply_standard_style(EGUI_VIEW_OF(&test_switch));
    hcw_switch_set_state_icons(EGUI_VIEW_OF(&test_switch), EGUI_ICON_MS_DONE, EGUI_ICON_MS_CROSS);
    hcw_switch_set_icon_font(EGUI_VIEW_OF(&test_switch), EGUI_FONT_ICON_MS_20);
    hcw_switch_set_checked(EGUI_VIEW_OF(&test_switch), checked);
    egui_view_switch_set_on_checked_listener(EGUI_VIEW_OF(&test_switch), on_checked);
    hcw_switch_override_interaction_api(EGUI_VIEW_OF(&test_switch), &test_switch_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_switch), 1);
#endif
    reset_listener_state();
}

static void setup_preview_switch(uint8_t checked)
{
    egui_view_switch_init(EGUI_VIEW_OF(&preview_switch));
    egui_view_set_size(EGUI_VIEW_OF(&preview_switch), 76, 32);
    hcw_switch_apply_compact_style(EGUI_VIEW_OF(&preview_switch));
    hcw_switch_set_state_icons(EGUI_VIEW_OF(&preview_switch), EGUI_ICON_MS_DONE, EGUI_ICON_MS_CROSS);
    hcw_switch_set_icon_font(EGUI_VIEW_OF(&preview_switch), EGUI_FONT_ICON_MS_16);
    hcw_switch_set_checked(EGUI_VIEW_OF(&preview_switch), checked);
    hcw_switch_override_static_preview_api(EGUI_VIEW_OF(&preview_switch), &preview_switch_api);
}

static void layout_switch(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static int send_touch(egui_view_switch_t *control, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(control)->api->on_touch_event(EGUI_VIEW_OF(control), &event);
}

static int send_key(egui_view_switch_t *control, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= EGUI_VIEW_OF(control)->api->dispatch_key_event(EGUI_VIEW_OF(control), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= EGUI_VIEW_OF(control)->api->dispatch_key_event(EGUI_VIEW_OF(control), &event);
    return handled;
}

static int send_preview_key_action(egui_view_switch_t *control, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return EGUI_VIEW_OF(control)->api->on_key_event(EGUI_VIEW_OF(control), &event);
}

static void get_switch_center(egui_view_switch_t *control, egui_dim_t *x, egui_dim_t *y)
{
    *x = EGUI_VIEW_OF(control)->region_screen.location.x + EGUI_VIEW_OF(control)->region_screen.size.width / 2;
    *y = EGUI_VIEW_OF(control)->region_screen.location.y + EGUI_VIEW_OF(control)->region_screen.size.height / 2;
}

static void test_switch_style_helpers_update_palette_and_clear_pressed_state(void)
{
    egui_view_switch_t *local;

    setup_switch(1);
    local = &test_switch;

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_switch_apply_compact_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, local->bk_color_on.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD9E5DE).full, local->bk_color_off.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, local->switch_color_on.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_switch_apply_read_only_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xAFB8C3).full, local->bk_color_on.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xE4E9EE).full, local->bk_color_off.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF7F9FB).full, local->switch_color_on.full);
}

static void test_switch_setters_clear_pressed_state_and_update_data(void)
{
    egui_view_switch_t *local;

    setup_switch(0);
    local = &test_switch;

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_switch_set_checked(EGUI_VIEW_OF(local), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_checked_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_checked);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_switch_set_state_icons(EGUI_VIEW_OF(local), EGUI_ICON_MS_DONE, EGUI_ICON_MS_CROSS);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp(local->icon_on, EGUI_ICON_MS_DONE) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp(local->icon_off, EGUI_ICON_MS_CROSS) == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    hcw_switch_set_icon_font(EGUI_VIEW_OF(local), EGUI_FONT_ICON_MS_24);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->icon_font == EGUI_FONT_ICON_MS_24);

    hcw_switch_set_checked(EGUI_VIEW_OF(local), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_checked_count);
}

static void test_switch_touch_same_target_release_toggles_once(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;

    setup_switch(0);
    layout_switch(EGUI_VIEW_OF(&test_switch), 10, 20, 112, 44);
    get_switch_center(&test_switch, &inside_x, &inside_y);
    outside_x = EGUI_VIEW_OF(&test_switch)->region_screen.location.x + EGUI_VIEW_OF(&test_switch)->region_screen.size.width + 12;

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_switch, EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_switch)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_switch, EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_switch)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_switch, EGUI_MOTION_EVENT_ACTION_UP, outside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(test_switch.is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_checked_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(&test_switch, EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_switch, EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_switch, EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_switch)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_switch, EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_switch)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_switch.is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_checked_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_checked);
}

static void test_switch_keyboard_space_and_enter_toggle(void)
{
    setup_switch(0);

    EGUI_TEST_ASSERT_TRUE(send_key(&test_switch, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(test_switch.is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_checked_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_last_checked);

    EGUI_TEST_ASSERT_TRUE(send_key(&test_switch, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(test_switch.is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_checked_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_last_checked);
}

static void test_switch_disabled_input_does_not_toggle(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_switch(1);
    layout_switch(EGUI_VIEW_OF(&test_switch), 10, 20, 112, 44);
    get_switch_center(&test_switch, &inside_x, &inside_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_switch), 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_switch), 0);
    EGUI_TEST_ASSERT_FALSE(send_key(&test_switch, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_switch)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_switch.is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_checked_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_switch), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(&test_switch, EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_switch)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_switch.is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_checked_count);
}

static void test_switch_unhandled_key_clears_pressed_state(void)
{
    setup_switch(0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_switch), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(&test_switch, EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_switch)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(test_switch.is_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_checked_count);
}

static void test_switch_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_preview_switch(1);
    layout_switch(EGUI_VIEW_OF(&preview_switch), 10, 20, 76, 32);
    get_switch_center(&preview_switch, &inside_x, &inside_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_switch), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(&preview_switch, EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_switch)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(preview_switch.is_checked);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_switch), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(&preview_switch, EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(&preview_switch, EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_switch)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(preview_switch.is_checked);
}

void test_switch_run(void)
{
    EGUI_TEST_SUITE_BEGIN(switch);
    EGUI_TEST_RUN(test_switch_style_helpers_update_palette_and_clear_pressed_state);
    EGUI_TEST_RUN(test_switch_setters_clear_pressed_state_and_update_data);
    EGUI_TEST_RUN(test_switch_touch_same_target_release_toggles_once);
    EGUI_TEST_RUN(test_switch_keyboard_space_and_enter_toggle);
    EGUI_TEST_RUN(test_switch_disabled_input_does_not_toggle);
    EGUI_TEST_RUN(test_switch_unhandled_key_clears_pressed_state);
    EGUI_TEST_RUN(test_switch_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
