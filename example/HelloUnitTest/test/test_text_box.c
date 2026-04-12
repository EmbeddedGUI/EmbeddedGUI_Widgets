#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_text_box.h"

#include "../../HelloCustomWidgets/input/text_box/egui_view_text_box.h"
#include "../../HelloCustomWidgets/input/text_box/egui_view_text_box.c"

static egui_view_textinput_t test_text_box;
static egui_view_textinput_t preview_text_box;
static egui_view_api_t preview_api;
static char submitted_text[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
static uint8_t submit_count;

static void on_submit(egui_view_t *self, const char *text)
{
    EGUI_UNUSED(self);

    strncpy(submitted_text, text, sizeof(submitted_text) - 1);
    submitted_text[sizeof(submitted_text) - 1] = '\0';
    submit_count++;
}

static void setup_text_box(const char *text)
{
    egui_view_textinput_init(EGUI_VIEW_OF(&test_text_box));
    egui_view_set_size(EGUI_VIEW_OF(&test_text_box), 196, 40);
    hcw_text_box_set_font(EGUI_VIEW_OF(&test_text_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    hcw_text_box_apply_standard_style(EGUI_VIEW_OF(&test_text_box));
    hcw_text_box_set_text(EGUI_VIEW_OF(&test_text_box), text);
    egui_view_textinput_set_on_submit(EGUI_VIEW_OF(&test_text_box), on_submit);
    submitted_text[0] = '\0';
    submit_count = 0;
}

static void setup_preview_text_box(const char *text)
{
    egui_view_textinput_init(EGUI_VIEW_OF(&preview_text_box));
    egui_view_set_size(EGUI_VIEW_OF(&preview_text_box), 104, 32);
    hcw_text_box_set_font(EGUI_VIEW_OF(&preview_text_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    hcw_text_box_apply_compact_style(EGUI_VIEW_OF(&preview_text_box));
    hcw_text_box_set_text(EGUI_VIEW_OF(&preview_text_box), text);
    hcw_text_box_override_static_preview_api(EGUI_VIEW_OF(&preview_text_box), &preview_api);
}

static int send_key_event(uint8_t type, uint8_t key_code, uint8_t is_shift)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    event.is_shift = is_shift ? 1 : 0;
    return EGUI_VIEW_OF(&test_text_box)->api->on_key_event(EGUI_VIEW_OF(&test_text_box), &event);
}

static int send_preview_key_event(uint8_t type, uint8_t key_code, uint8_t is_shift)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    event.is_shift = is_shift ? 1 : 0;
    return EGUI_VIEW_OF(&preview_text_box)->api->on_key_event(EGUI_VIEW_OF(&preview_text_box), &event);
}

static int send_preview_touch_event(uint8_t type)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    return EGUI_VIEW_OF(&preview_text_box)->api->on_touch_event(EGUI_VIEW_OF(&preview_text_box), &event);
}

static int press_key(uint8_t key_code, uint8_t is_shift)
{
    int handled = 0;

    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, key_code, is_shift);
    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_UP, key_code, is_shift);
    return handled;
}

static void test_text_box_style_helpers_apply_expected_state(void)
{
    egui_view_textinput_init(EGUI_VIEW_OF(&test_text_box));

    hcw_text_box_apply_standard_style(EGUI_VIEW_OF(&test_text_box));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_text_box)->background != NULL);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_text_box)->is_enable);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(10, EGUI_VIEW_OF(&test_text_box)->padding.left);
#endif
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x1A2734).full, test_text_box.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_text_box.cursor_color.full);

    hcw_text_box_apply_compact_style(EGUI_VIEW_OF(&test_text_box));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_text_box)->is_enable);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(8, EGUI_VIEW_OF(&test_text_box)->padding.left);
#endif
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x183235).full, test_text_box.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, test_text_box.cursor_color.full);

    hcw_text_box_apply_read_only_style(EGUI_VIEW_OF(&test_text_box));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_text_box)->is_enable);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(8, EGUI_VIEW_OF(&test_text_box)->padding.left);
#endif
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x7A8796).full, test_text_box.text_color.full);
}

static void test_text_box_insert_delete_and_submit_via_keyboard(void)
{
    setup_text_box("Node 01");
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_BACKSPACE, 0));
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_2, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("Node 02", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_text_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_ENTER, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(1, submit_count);
    EGUI_TEST_ASSERT_TRUE(strcmp("Node 02", submitted_text) == 0);
}

static void test_text_box_navigation_keys_move_cursor(void)
{
    setup_text_box("ABCD");
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_LEFT, 0));
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_LEFT, 0));
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_DELETE, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("ABD", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_text_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_HOME, 0));
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_Z, 1));
    EGUI_TEST_ASSERT_TRUE(strcmp("ZABD", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_text_box))) == 0);
}

static void test_text_box_setters_and_max_length(void)
{
    setup_text_box("");
    hcw_text_box_set_max_length(EGUI_VIEW_OF(&test_text_box), 4);
    hcw_text_box_set_placeholder(EGUI_VIEW_OF(&test_text_box), "Name");
    hcw_text_box_set_text(EGUI_VIEW_OF(&test_text_box), "abcdef");
    EGUI_TEST_ASSERT_TRUE(strcmp("abcd", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_text_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp("Name", test_text_box.placeholder) == 0);
}

static void test_text_box_read_only_style_blocks_key_input(void)
{
    setup_text_box("Locked");
    hcw_text_box_apply_read_only_style(EGUI_VIEW_OF(&test_text_box));
    EGUI_TEST_ASSERT_FALSE(press_key(EGUI_KEY_CODE_A, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("Locked", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_text_box))) == 0);
}

static void test_text_box_static_preview_consumes_input(void)
{
    setup_preview_text_box("Queued");
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN));
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_UP));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_A, 0));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_A, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("Queued", egui_view_textinput_get_text(EGUI_VIEW_OF(&preview_text_box))) == 0);
}

void test_text_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(text_box);
    EGUI_TEST_RUN(test_text_box_style_helpers_apply_expected_state);
    EGUI_TEST_RUN(test_text_box_insert_delete_and_submit_via_keyboard);
    EGUI_TEST_RUN(test_text_box_navigation_keys_move_cursor);
    EGUI_TEST_RUN(test_text_box_setters_and_max_length);
    EGUI_TEST_RUN(test_text_box_read_only_style_blocks_key_input);
    EGUI_TEST_RUN(test_text_box_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
