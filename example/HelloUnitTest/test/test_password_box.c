#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_password_box.h"

#include "../../HelloCustomWidgets/input/password_box/egui_view_password_box.h"
#include "../../HelloCustomWidgets/input/password_box/egui_view_password_box.c"

static egui_view_password_box_t test_password_box;
static egui_view_password_box_t preview_password_box;
static egui_view_api_t preview_api;

static void setup_password_box(const char *text)
{
    egui_view_password_box_init(EGUI_VIEW_OF(&test_password_box));
    egui_view_set_size(EGUI_VIEW_OF(&test_password_box), 196, 70);
    egui_view_password_box_set_text(EGUI_VIEW_OF(&test_password_box), text);
}

static void setup_preview_password_box(const char *text)
{
    egui_view_password_box_init(EGUI_VIEW_OF(&preview_password_box));
    egui_view_set_size(EGUI_VIEW_OF(&preview_password_box), 106, 44);
    egui_view_password_box_set_text(EGUI_VIEW_OF(&preview_password_box), text);
    egui_view_password_box_set_compact_mode(EGUI_VIEW_OF(&preview_password_box), 1);
    egui_view_password_box_override_static_preview_api(EGUI_VIEW_OF(&preview_password_box), &preview_api);
}

static void layout_password_box(egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(EGUI_VIEW_OF(&test_password_box), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_password_box)->region_screen, &region);
}

static void layout_preview_password_box(egui_dim_t x, egui_dim_t y)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = 106;
    region.size.height = 44;
    egui_view_layout(EGUI_VIEW_OF(&preview_password_box), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_password_box)->region_screen, &region);
}

static void get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_get_part_region(view, part, &region));
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
    return EGUI_VIEW_OF(&test_password_box)->api->on_touch_event(EGUI_VIEW_OF(&test_password_box), &event);
}

static int send_preview_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&preview_password_box)->api->on_touch_event(EGUI_VIEW_OF(&preview_password_box), &event);
}

static int send_key_event(uint8_t type, uint8_t key_code, uint8_t is_shift)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    event.is_shift = is_shift ? 1 : 0;
    return EGUI_VIEW_OF(&test_password_box)->api->on_key_event(EGUI_VIEW_OF(&test_password_box), &event);
}

static int send_preview_key_event(uint8_t type, uint8_t key_code, uint8_t is_shift)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    event.is_shift = is_shift ? 1 : 0;
    return EGUI_VIEW_OF(&preview_password_box)->api->on_key_event(EGUI_VIEW_OF(&preview_password_box), &event);
}

static int press_key(uint8_t key_code, uint8_t is_shift)
{
    int handled = 0;

    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, key_code, is_shift);
    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_UP, key_code, is_shift);
    return handled;
}

static void test_password_box_tab_cycles_to_reveal_when_text_exists(void)
{
    setup_password_box("abcd");
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_FIELD, egui_view_password_box_get_current_part(EGUI_VIEW_OF(&test_password_box)));
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_handle_navigation_key(EGUI_VIEW_OF(&test_password_box), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_REVEAL, egui_view_password_box_get_current_part(EGUI_VIEW_OF(&test_password_box)));
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_handle_navigation_key(EGUI_VIEW_OF(&test_password_box), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_FIELD, egui_view_password_box_get_current_part(EGUI_VIEW_OF(&test_password_box)));
}

static void test_password_box_tab_stays_on_field_when_empty_or_read_only(void)
{
    setup_password_box("");
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_handle_navigation_key(EGUI_VIEW_OF(&test_password_box), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_FIELD, egui_view_password_box_get_current_part(EGUI_VIEW_OF(&test_password_box)));

    setup_password_box("abcd");
    egui_view_password_box_set_read_only_mode(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_password_box_handle_navigation_key(EGUI_VIEW_OF(&test_password_box), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_FIELD, egui_view_password_box_get_current_part(EGUI_VIEW_OF(&test_password_box)));
}

static void test_password_box_reveal_toggle_via_keyboard(void)
{
    setup_password_box("secret");
    egui_view_password_box_set_current_part(EGUI_VIEW_OF(&test_password_box), EGUI_VIEW_PASSWORD_BOX_PART_REVEAL);
    EGUI_TEST_ASSERT_FALSE(egui_view_password_box_get_revealed(EGUI_VIEW_OF(&test_password_box)));
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_handle_navigation_key(EGUI_VIEW_OF(&test_password_box), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_get_revealed(EGUI_VIEW_OF(&test_password_box)));
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_handle_navigation_key(EGUI_VIEW_OF(&test_password_box), EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_FALSE(egui_view_password_box_get_revealed(EGUI_VIEW_OF(&test_password_box)));
}

static void test_password_box_setters_clear_pressed_state(void)
{
    setup_password_box("secret");
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    egui_view_password_box_set_text(EGUI_VIEW_OF(&test_password_box), "deploy-key");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_TEST_ASSERT_TRUE(strcmp("deploy-key", egui_view_password_box_get_text(EGUI_VIEW_OF(&test_password_box))) == 0);

    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_REVEAL;
    egui_view_password_box_clear(EGUI_VIEW_OF(&test_password_box));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_TEST_ASSERT_TRUE(strcmp("", egui_view_password_box_get_text(EGUI_VIEW_OF(&test_password_box))) == 0);

    setup_password_box("secret");
    egui_view_password_box_set_current_part(EGUI_VIEW_OF(&test_password_box), EGUI_VIEW_PASSWORD_BOX_PART_REVEAL);
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    egui_view_password_box_set_current_part(EGUI_VIEW_OF(&test_password_box), EGUI_VIEW_PASSWORD_BOX_PART_FIELD);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_FIELD, egui_view_password_box_get_current_part(EGUI_VIEW_OF(&test_password_box)));

    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_REVEAL;
    egui_view_password_box_set_revealed(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_get_revealed(EGUI_VIEW_OF(&test_password_box)));

    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    egui_view_password_box_set_palette(EGUI_VIEW_OF(&test_password_box), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);

    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_REVEAL;
    egui_view_password_box_set_compact_mode(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_password_box.compact_mode);

    egui_view_password_box_set_compact_mode(EGUI_VIEW_OF(&test_password_box), 0);
    egui_view_password_box_set_current_part(EGUI_VIEW_OF(&test_password_box), EGUI_VIEW_PASSWORD_BOX_PART_REVEAL);
    egui_view_password_box_set_revealed(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    egui_view_password_box_set_read_only_mode(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_password_box.read_only_mode);
    EGUI_TEST_ASSERT_FALSE(egui_view_password_box_get_revealed(EGUI_VIEW_OF(&test_password_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_FIELD, egui_view_password_box_get_current_part(EGUI_VIEW_OF(&test_password_box)));
}

static void test_password_box_insert_and_backspace_respect_cursor(void)
{
    setup_password_box("abc");
    egui_view_password_box_set_cursor_pos(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_X, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("axbc", egui_view_password_box_get_text(EGUI_VIEW_OF(&test_password_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_BACKSPACE, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("abc", egui_view_password_box_get_text(EGUI_VIEW_OF(&test_password_box))) == 0);
}

static void test_password_box_delete_forward_and_navigation(void)
{
    setup_password_box("abcd");
    egui_view_password_box_move_cursor_home(EGUI_VIEW_OF(&test_password_box));
    egui_view_password_box_move_cursor_right(EGUI_VIEW_OF(&test_password_box));
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_DELETE, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("acd", egui_view_password_box_get_text(EGUI_VIEW_OF(&test_password_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_handle_navigation_key(EGUI_VIEW_OF(&test_password_box), EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_handle_navigation_key(EGUI_VIEW_OF(&test_password_box), EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_REVEAL, egui_view_password_box_get_current_part(EGUI_VIEW_OF(&test_password_box)));
}

static void test_password_box_touch_reveal_toggle(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_password_box("secret");
    layout_password_box(10, 20, 196, 70);
    get_part_center(EGUI_VIEW_OF(&test_password_box), EGUI_VIEW_PASSWORD_BOX_PART_REVEAL, &x, &y);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_TRUE(egui_view_password_box_get_revealed(EGUI_VIEW_OF(&test_password_box)));
}

static void test_password_box_read_only_ignores_changes(void)
{
    setup_password_box("secret");
    egui_view_password_box_set_read_only_mode(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_TEST_ASSERT_FALSE(press_key(EGUI_KEY_CODE_A, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("secret", egui_view_password_box_get_text(EGUI_VIEW_OF(&test_password_box))) == 0);
    egui_view_password_box_set_revealed(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_password_box_get_revealed(EGUI_VIEW_OF(&test_password_box)));
}

static void test_password_box_compact_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_password_box("secret");
    layout_password_box(10, 20, 196, 70);
    get_part_center(EGUI_VIEW_OF(&test_password_box), EGUI_VIEW_PASSWORD_BOX_PART_FIELD, &x, &y);

    egui_view_password_box_set_read_only_mode(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_REVEAL;
    EGUI_TEST_ASSERT_FALSE(press_key(EGUI_KEY_CODE_ENTER, 0));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);

    egui_view_password_box_set_read_only_mode(EGUI_VIEW_OF(&test_password_box), 0);
    egui_view_password_box_set_compact_mode(EGUI_VIEW_OF(&test_password_box), 1);
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_REVEAL;
    EGUI_TEST_ASSERT_FALSE(press_key(EGUI_KEY_CODE_SPACE, 0));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);

    egui_view_password_box_set_compact_mode(EGUI_VIEW_OF(&test_password_box), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_password_box), 0);
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
    EGUI_VIEW_OF(&test_password_box)->is_pressed = 1;
    test_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_REVEAL;
    EGUI_TEST_ASSERT_FALSE(press_key(EGUI_KEY_CODE_ENTER, 0));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, test_password_box.pressed_part);
}

static void test_password_box_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_password_box("7429");
    layout_preview_password_box(10, 20);
    get_part_center(EGUI_VIEW_OF(&preview_password_box), EGUI_VIEW_PASSWORD_BOX_PART_FIELD, &x, &y);

    EGUI_VIEW_OF(&preview_password_box)->is_pressed = 1;
    preview_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, preview_password_box.pressed_part);
    EGUI_TEST_ASSERT_TRUE(strcmp("7429", egui_view_password_box_get_text(EGUI_VIEW_OF(&preview_password_box))) == 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_password_box_get_revealed(EGUI_VIEW_OF(&preview_password_box)));

    egui_view_password_box_set_current_part(EGUI_VIEW_OF(&preview_password_box), EGUI_VIEW_PASSWORD_BOX_PART_REVEAL);
    EGUI_VIEW_OF(&preview_password_box)->is_pressed = 1;
    preview_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_REVEAL;
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER, 0));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER, 0));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, preview_password_box.pressed_part);
    EGUI_TEST_ASSERT_TRUE(strcmp("7429", egui_view_password_box_get_text(EGUI_VIEW_OF(&preview_password_box))) == 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_password_box_get_revealed(EGUI_VIEW_OF(&preview_password_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_REVEAL, egui_view_password_box_get_current_part(EGUI_VIEW_OF(&preview_password_box)));
}

void test_password_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(password_box);
    EGUI_TEST_RUN(test_password_box_tab_cycles_to_reveal_when_text_exists);
    EGUI_TEST_RUN(test_password_box_tab_stays_on_field_when_empty_or_read_only);
    EGUI_TEST_RUN(test_password_box_reveal_toggle_via_keyboard);
    EGUI_TEST_RUN(test_password_box_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_password_box_insert_and_backspace_respect_cursor);
    EGUI_TEST_RUN(test_password_box_delete_forward_and_navigation);
    EGUI_TEST_RUN(test_password_box_touch_reveal_toggle);
    EGUI_TEST_RUN(test_password_box_read_only_ignores_changes);
    EGUI_TEST_RUN(test_password_box_compact_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_password_box_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
