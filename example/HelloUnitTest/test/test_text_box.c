#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_text_box.h"

#include "../../HelloCustomWidgets/input/text_box/egui_view_text_box.h"
#include "../../HelloCustomWidgets/input/text_box/egui_view_text_box.c"

typedef struct text_box_preview_snapshot text_box_preview_snapshot_t;
struct text_box_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    egui_view_textinput_on_submit_t on_submit;
    const egui_font_t *font;
    const char *placeholder;
    egui_color_t text_color;
    egui_alpha_t text_alpha;
    egui_color_t placeholder_color;
    egui_alpha_t placeholder_alpha;
    egui_color_t cursor_color;
    char text[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
    uint8_t text_len;
    uint8_t cursor_pos;
    uint8_t max_length;
    uint8_t cursor_visible;
    uint8_t align_type;
    egui_dim_t scroll_offset_x;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

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

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void assert_optional_string_equal(const char *expected, const char *actual)
{
    if (expected == NULL || actual == NULL)
    {
        EGUI_TEST_ASSERT_TRUE(expected == actual);
        return;
    }
    EGUI_TEST_ASSERT_TRUE(strcmp(expected, actual) == 0);
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
    layout_view(EGUI_VIEW_OF(&test_text_box), 10, 20, 196, 40);
}

static void setup_preview_text_box(const char *text)
{
    egui_view_textinput_init(EGUI_VIEW_OF(&preview_text_box));
    egui_view_set_size(EGUI_VIEW_OF(&preview_text_box), 104, 32);
    hcw_text_box_set_font(EGUI_VIEW_OF(&preview_text_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    hcw_text_box_apply_compact_style(EGUI_VIEW_OF(&preview_text_box));
    hcw_text_box_set_placeholder(EGUI_VIEW_OF(&preview_text_box), "Compact");
    hcw_text_box_set_text(EGUI_VIEW_OF(&preview_text_box), text);
    hcw_text_box_set_max_length(EGUI_VIEW_OF(&preview_text_box), 16);
    egui_view_textinput_set_on_submit(EGUI_VIEW_OF(&preview_text_box), on_submit);
    hcw_text_box_override_static_preview_api(EGUI_VIEW_OF(&preview_text_box), &preview_api);
    submitted_text[0] = '\0';
    submit_count = 0;
    layout_view(EGUI_VIEW_OF(&preview_text_box), 12, 18, 104, 32);
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

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code, uint8_t is_shift)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    event.is_shift = is_shift ? 1 : 0;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code, uint8_t is_shift)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code, is_shift);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code, is_shift);
    return handled;
}

static int press_key(uint8_t key_code, uint8_t is_shift)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_text_box), key_code, is_shift);
}

static int send_preview_key(uint8_t key_code, uint8_t is_shift)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_text_box), key_code, is_shift);
}

static void capture_preview_snapshot(text_box_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_text_box)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_text_box)->background;
    snapshot->on_submit = preview_text_box.on_submit;
    snapshot->font = preview_text_box.font;
    snapshot->placeholder = preview_text_box.placeholder;
    snapshot->text_color = preview_text_box.text_color;
    snapshot->text_alpha = preview_text_box.text_alpha;
    snapshot->placeholder_color = preview_text_box.placeholder_color;
    snapshot->placeholder_alpha = preview_text_box.placeholder_alpha;
    snapshot->cursor_color = preview_text_box.cursor_color;
    strcpy(snapshot->text, preview_text_box.text);
    snapshot->text_len = preview_text_box.text_len;
    snapshot->cursor_pos = preview_text_box.cursor_pos;
    snapshot->max_length = preview_text_box.max_length;
    snapshot->cursor_visible = preview_text_box.cursor_visible;
    snapshot->align_type = preview_text_box.align_type;
    snapshot->scroll_offset_x = preview_text_box.scroll_offset_x;
    snapshot->alpha = EGUI_VIEW_OF(&preview_text_box)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_text_box));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_text_box)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_text_box)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_text_box)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_text_box)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_text_box)->padding.bottom;
}

static void assert_preview_state_unchanged(const text_box_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_text_box)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_text_box)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_text_box.on_submit == snapshot->on_submit);
    EGUI_TEST_ASSERT_TRUE(preview_text_box.font == snapshot->font);
    assert_optional_string_equal(snapshot->placeholder, preview_text_box.placeholder);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_text_box.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_alpha, preview_text_box.text_alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->placeholder_color.full, preview_text_box.placeholder_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->placeholder_alpha, preview_text_box.placeholder_alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->cursor_color.full, preview_text_box.cursor_color.full);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->text, preview_text_box.text) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_len, preview_text_box.text_len);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->cursor_pos, preview_text_box.cursor_pos);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->max_length, preview_text_box.max_length);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->cursor_visible, preview_text_box.cursor_visible);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->align_type, preview_text_box.align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->scroll_offset_x, preview_text_box.scroll_offset_x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_text_box)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_text_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_text_box)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_text_box)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_text_box)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_text_box)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_text_box)->padding.bottom);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_text_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, submit_count);
    EGUI_TEST_ASSERT_TRUE(strcmp("", submitted_text) == 0);
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

static void test_text_box_static_preview_consumes_input_and_keeps_state(void)
{
    text_box_preview_snapshot_t initial_snapshot;
    egui_region_t preview_region;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_text_box("Queued");
    preview_region = EGUI_VIEW_OF(&preview_text_box)->region_screen;
    center_x = preview_region.location.x + preview_region.size.width / 2;
    center_y = preview_region.location.y + preview_region.size.height / 2;
    capture_preview_snapshot(&initial_snapshot);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_text_box), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER, 0));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_text_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(text_box);
    EGUI_TEST_RUN(test_text_box_style_helpers_apply_expected_state);
    EGUI_TEST_RUN(test_text_box_insert_delete_and_submit_via_keyboard);
    EGUI_TEST_RUN(test_text_box_navigation_keys_move_cursor);
    EGUI_TEST_RUN(test_text_box_setters_and_max_length);
    EGUI_TEST_RUN(test_text_box_read_only_style_blocks_key_input);
    EGUI_TEST_RUN(test_text_box_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
