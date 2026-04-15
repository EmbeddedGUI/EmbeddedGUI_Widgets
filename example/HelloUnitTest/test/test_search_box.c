#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_search_box.h"

#include "../../HelloCustomWidgets/input/search_box/egui_view_search_box.h"
#include "../../HelloCustomWidgets/input/search_box/egui_view_search_box.c"

typedef struct search_box_preview_snapshot search_box_preview_snapshot_t;
struct search_box_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    egui_view_textinput_on_submit_t on_submit;
    const egui_font_t *font;
    const egui_font_t *icon_font;
    const char *placeholder;
    egui_color_t text_color;
    egui_alpha_t text_alpha;
    egui_color_t placeholder_color;
    egui_alpha_t placeholder_alpha;
    egui_color_t cursor_color;
    egui_color_t icon_color;
    egui_color_t clear_fill_color;
    egui_color_t clear_fill_pressed_color;
    egui_color_t clear_icon_color;
    char text[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
    uint8_t text_len;
    uint8_t cursor_pos;
    uint8_t max_length;
    uint8_t cursor_visible;
    uint8_t align_type;
    egui_dim_t scroll_offset_x;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t clear_pressed;
    uint8_t is_focused;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_search_box_t test_search_box;
static egui_view_search_box_t preview_search_box;
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

static void setup_search_box(const char *text)
{
    egui_view_search_box_init(EGUI_VIEW_OF(&test_search_box));
    egui_view_set_size(EGUI_VIEW_OF(&test_search_box), 196, 40);
    egui_view_search_box_set_font(EGUI_VIEW_OF(&test_search_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_search_box_apply_standard_style(EGUI_VIEW_OF(&test_search_box));
    egui_view_search_box_set_text(EGUI_VIEW_OF(&test_search_box), text);
    egui_view_textinput_set_on_submit(EGUI_VIEW_OF(&test_search_box), on_submit);
    submitted_text[0] = '\0';
    submit_count = 0;
    layout_view(EGUI_VIEW_OF(&test_search_box), 10, 20, 196, 40);
}

static void setup_preview_search_box(const char *text)
{
    egui_view_search_box_init(EGUI_VIEW_OF(&preview_search_box));
    egui_view_set_size(EGUI_VIEW_OF(&preview_search_box), 104, 32);
    egui_view_search_box_set_font(EGUI_VIEW_OF(&preview_search_box), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_search_box_apply_compact_style(EGUI_VIEW_OF(&preview_search_box));
    egui_view_search_box_set_placeholder(EGUI_VIEW_OF(&preview_search_box), "Recent");
    egui_view_search_box_set_text(EGUI_VIEW_OF(&preview_search_box), text);
    egui_view_search_box_set_max_length(EGUI_VIEW_OF(&preview_search_box), 16);
    egui_view_textinput_set_on_submit(EGUI_VIEW_OF(&preview_search_box), on_submit);
    egui_view_search_box_override_static_preview_api(EGUI_VIEW_OF(&preview_search_box), &preview_api);
    submitted_text[0] = '\0';
    submit_count = 0;
    layout_view(EGUI_VIEW_OF(&preview_search_box), 12, 18, 104, 32);
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
    return send_key_to_view(EGUI_VIEW_OF(&test_search_box), key_code, is_shift);
}

static int send_preview_key(uint8_t key_code, uint8_t is_shift)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_search_box), key_code, is_shift);
}

static void capture_preview_snapshot(search_box_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_search_box)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_search_box)->background;
    snapshot->on_submit = preview_search_box.textinput.on_submit;
    snapshot->font = preview_search_box.textinput.font;
    snapshot->icon_font = preview_search_box.icon_font;
    snapshot->placeholder = preview_search_box.textinput.placeholder;
    snapshot->text_color = preview_search_box.textinput.text_color;
    snapshot->text_alpha = preview_search_box.textinput.text_alpha;
    snapshot->placeholder_color = preview_search_box.textinput.placeholder_color;
    snapshot->placeholder_alpha = preview_search_box.textinput.placeholder_alpha;
    snapshot->cursor_color = preview_search_box.textinput.cursor_color;
    snapshot->icon_color = preview_search_box.icon_color;
    snapshot->clear_fill_color = preview_search_box.clear_fill_color;
    snapshot->clear_fill_pressed_color = preview_search_box.clear_fill_pressed_color;
    snapshot->clear_icon_color = preview_search_box.clear_icon_color;
    strcpy(snapshot->text, preview_search_box.textinput.text);
    snapshot->text_len = preview_search_box.textinput.text_len;
    snapshot->cursor_pos = preview_search_box.textinput.cursor_pos;
    snapshot->max_length = preview_search_box.textinput.max_length;
    snapshot->cursor_visible = preview_search_box.textinput.cursor_visible;
    snapshot->align_type = preview_search_box.textinput.align_type;
    snapshot->scroll_offset_x = preview_search_box.textinput.scroll_offset_x;
    snapshot->alpha = EGUI_VIEW_OF(&preview_search_box)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_search_box));
    snapshot->clear_pressed = preview_search_box.clear_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_search_box)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_search_box)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_search_box)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_search_box)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_search_box)->padding.bottom;
}

static void assert_preview_state_unchanged(const search_box_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_search_box)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_search_box)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_search_box.textinput.on_submit == snapshot->on_submit);
    EGUI_TEST_ASSERT_TRUE(preview_search_box.textinput.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_search_box.icon_font == snapshot->icon_font);
    assert_optional_string_equal(snapshot->placeholder, preview_search_box.textinput.placeholder);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_search_box.textinput.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_alpha, preview_search_box.textinput.text_alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->placeholder_color.full, preview_search_box.textinput.placeholder_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->placeholder_alpha, preview_search_box.textinput.placeholder_alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->cursor_color.full, preview_search_box.textinput.cursor_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->icon_color.full, preview_search_box.icon_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->clear_fill_color.full, preview_search_box.clear_fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->clear_fill_pressed_color.full, preview_search_box.clear_fill_pressed_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->clear_icon_color.full, preview_search_box.clear_icon_color.full);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->text, preview_search_box.textinput.text) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_len, preview_search_box.textinput.text_len);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->cursor_pos, preview_search_box.textinput.cursor_pos);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->max_length, preview_search_box.textinput.max_length);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->cursor_visible, preview_search_box.textinput.cursor_visible);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->align_type, preview_search_box.textinput.align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->scroll_offset_x, preview_search_box.textinput.scroll_offset_x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_search_box)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_search_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->clear_pressed, preview_search_box.clear_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_search_box)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_search_box)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_search_box)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_search_box)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_search_box)->padding.bottom);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_search_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, submit_count);
    EGUI_TEST_ASSERT_TRUE(strcmp("", submitted_text) == 0);
}

static void test_search_box_style_helpers_apply_expected_state(void)
{
    egui_view_search_box_init(EGUI_VIEW_OF(&test_search_box));

    egui_view_search_box_apply_standard_style(EGUI_VIEW_OF(&test_search_box));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_search_box)->background != NULL);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_search_box)->is_enable);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(28, EGUI_VIEW_OF(&test_search_box)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(26, EGUI_VIEW_OF(&test_search_box)->padding.right);
#endif
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x5C6B79).full, test_search_box.icon_color.full);

    egui_view_search_box_apply_compact_style(EGUI_VIEW_OF(&test_search_box));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_search_box)->is_enable);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(24, EGUI_VIEW_OF(&test_search_box)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(22, EGUI_VIEW_OF(&test_search_box)->padding.right);
#endif
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x55716D).full, test_search_box.icon_color.full);

    egui_view_search_box_apply_read_only_style(EGUI_VIEW_OF(&test_search_box));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_search_box)->is_enable);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x8A97A5).full, test_search_box.icon_color.full);
}

static void test_search_box_insert_delete_and_submit_via_keyboard(void)
{
    setup_search_box("Roadmap");
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_BACKSPACE, 0));
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_S, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("Roadmas", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_search_box))) == 0);
    EGUI_TEST_ASSERT_TRUE(press_key(EGUI_KEY_CODE_ENTER, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(1, submit_count);
    EGUI_TEST_ASSERT_TRUE(strcmp("Roadmas", submitted_text) == 0);
}

static void test_search_box_clear_button_requires_same_target_release(void)
{
    egui_region_t clear_region;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_search_box("Assets");
    EGUI_TEST_ASSERT_TRUE(egui_view_search_box_get_clear_region(EGUI_VIEW_OF(&test_search_box), &clear_region));

    outside_x = clear_region.location.x - 6;
    outside_y = clear_region.location.y - 6;

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_DOWN,
                                             clear_region.location.x + clear_region.size.width / 2,
                                             clear_region.location.y + clear_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(strcmp("Assets", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_search_box))) == 0);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_DOWN,
                                             clear_region.location.x + clear_region.size.width / 2,
                                             clear_region.location.y + clear_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_MOVE,
                                             clear_region.location.x + clear_region.size.width / 2,
                                             clear_region.location.y + clear_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_search_box), EGUI_MOTION_EVENT_ACTION_UP,
                                             clear_region.location.x + clear_region.size.width / 2,
                                             clear_region.location.y + clear_region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(strcmp("", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_search_box))) == 0);
    EGUI_TEST_ASSERT_FALSE(test_search_box.clear_pressed);
    EGUI_TEST_ASSERT_FALSE(egui_view_search_box_get_clear_region(EGUI_VIEW_OF(&test_search_box), &clear_region));
}

static void test_search_box_read_only_style_blocks_input_and_hides_clear(void)
{
    egui_region_t clear_region;

    setup_search_box("Locked");
    egui_view_search_box_apply_read_only_style(EGUI_VIEW_OF(&test_search_box));
    EGUI_TEST_ASSERT_FALSE(press_key(EGUI_KEY_CODE_A, 0));
    EGUI_TEST_ASSERT_TRUE(strcmp("Locked", egui_view_textinput_get_text(EGUI_VIEW_OF(&test_search_box))) == 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_search_box_get_clear_region(EGUI_VIEW_OF(&test_search_box), &clear_region));
}

static void test_search_box_static_preview_consumes_input_and_keeps_state(void)
{
    search_box_preview_snapshot_t initial_snapshot;
    egui_region_t clear_region;

    setup_preview_search_box("Assets");
    EGUI_TEST_ASSERT_TRUE(egui_view_search_box_get_clear_region(EGUI_VIEW_OF(&preview_search_box), &clear_region));
    capture_preview_snapshot(&initial_snapshot);

    preview_search_box.clear_pressed = 1;
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_search_box), EGUI_MOTION_EVENT_ACTION_DOWN,
                                             clear_region.location.x + clear_region.size.width / 2,
                                             clear_region.location.y + clear_region.size.height / 2));
    assert_preview_state_unchanged(&initial_snapshot);

    capture_preview_snapshot(&initial_snapshot);
    preview_search_box.clear_pressed = 1;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER, 0));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_search_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(search_box);
    EGUI_TEST_RUN(test_search_box_style_helpers_apply_expected_state);
    EGUI_TEST_RUN(test_search_box_insert_delete_and_submit_via_keyboard);
    EGUI_TEST_RUN(test_search_box_clear_button_requires_same_target_release);
    EGUI_TEST_RUN(test_search_box_read_only_style_blocks_input_and_hides_clear);
    EGUI_TEST_RUN(test_search_box_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
