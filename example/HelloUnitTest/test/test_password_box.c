#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_password_box.h"

#include "../../HelloCustomWidgets/input/password_box/egui_view_password_box.h"
#include "../../HelloCustomWidgets/input/password_box/egui_view_password_box.c"

typedef struct password_box_preview_snapshot password_box_preview_snapshot_t;
struct password_box_preview_snapshot
{
    egui_region_t region_screen;
    egui_view_on_password_box_changed_listener_t on_changed;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    const egui_font_t *icon_font;
    const char *label;
    const char *helper;
    const char *placeholder;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    char text[EGUI_VIEW_PASSWORD_BOX_MAX_TEXT_LEN + 1];
    char masked_text[EGUI_VIEW_PASSWORD_BOX_MAX_TEXT_LEN + 1];
    uint8_t text_len;
    uint8_t cursor_pos;
    uint8_t current_part;
    uint8_t pressed_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t revealed;
    uint8_t cursor_visible;
    egui_dim_t scroll_offset_x;
    egui_alpha_t alpha;
    uint8_t enable;
};

static egui_view_password_box_t test_password_box;
static egui_view_password_box_t preview_password_box;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static uint8_t changed_revealed;
static uint8_t changed_part;
static char changed_text[EGUI_VIEW_PASSWORD_BOX_MAX_TEXT_LEN + 1];

static void on_changed(egui_view_t *self, const char *text, uint8_t revealed, uint8_t part)
{
    EGUI_UNUSED(self);
    changed_count++;
    changed_revealed = revealed;
    changed_part = part;
    strncpy(changed_text, text, EGUI_VIEW_PASSWORD_BOX_MAX_TEXT_LEN);
    changed_text[EGUI_VIEW_PASSWORD_BOX_MAX_TEXT_LEN] = '\0';
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
    egui_view_password_box_set_font(EGUI_VIEW_OF(&preview_password_box), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_password_box_set_meta_font(EGUI_VIEW_OF(&preview_password_box), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_password_box_set_placeholder(EGUI_VIEW_OF(&preview_password_box), "Quick PIN");
    egui_view_password_box_set_palette(EGUI_VIEW_OF(&preview_password_box), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD5DCE4), EGUI_COLOR_HEX(0x1A2734),
                                       EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x0F6CBD));
    egui_view_password_box_set_text(EGUI_VIEW_OF(&preview_password_box), text);
    egui_view_password_box_set_compact_mode(EGUI_VIEW_OF(&preview_password_box), 1);
    egui_view_password_box_set_on_changed_listener(EGUI_VIEW_OF(&preview_password_box), on_changed);
    egui_view_password_box_override_static_preview_api(EGUI_VIEW_OF(&preview_password_box), &preview_api);
    changed_count = 0;
    changed_revealed = 0xFF;
    changed_part = EGUI_VIEW_PASSWORD_BOX_PART_NONE;
    changed_text[0] = '\0';
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

static int send_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_password_box), type, x, y);
}

static int send_preview_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_password_box), type, x, y);
}

static int send_key_event(uint8_t type, uint8_t key_code, uint8_t is_shift)
{
    return dispatch_key_event_to_view(EGUI_VIEW_OF(&test_password_box), type, key_code, is_shift);
}

static int send_preview_key(uint8_t key_code, uint8_t is_shift)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_password_box), key_code, is_shift);
}

static int press_key(uint8_t key_code, uint8_t is_shift)
{
    int handled = 0;

    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, key_code, is_shift);
    handled |= send_key_event(EGUI_KEY_EVENT_ACTION_UP, key_code, is_shift);
    return handled;
}

static void capture_preview_snapshot(password_box_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_password_box)->region_screen;
    snapshot->on_changed = preview_password_box.on_changed;
    snapshot->font = preview_password_box.font;
    snapshot->meta_font = preview_password_box.meta_font;
    snapshot->icon_font = preview_password_box.icon_font;
    snapshot->label = preview_password_box.label;
    snapshot->helper = preview_password_box.helper;
    snapshot->placeholder = preview_password_box.placeholder;
    snapshot->surface_color = preview_password_box.surface_color;
    snapshot->border_color = preview_password_box.border_color;
    snapshot->text_color = preview_password_box.text_color;
    snapshot->muted_text_color = preview_password_box.muted_text_color;
    snapshot->accent_color = preview_password_box.accent_color;
    strcpy(snapshot->text, preview_password_box.text);
    strcpy(snapshot->masked_text, preview_password_box.masked_text);
    snapshot->text_len = preview_password_box.text_len;
    snapshot->cursor_pos = preview_password_box.cursor_pos;
    snapshot->current_part = preview_password_box.current_part;
    snapshot->pressed_part = preview_password_box.pressed_part;
    snapshot->compact_mode = preview_password_box.compact_mode;
    snapshot->read_only_mode = preview_password_box.read_only_mode;
    snapshot->revealed = preview_password_box.revealed;
    snapshot->cursor_visible = preview_password_box.cursor_visible;
    snapshot->scroll_offset_x = preview_password_box.scroll_offset_x;
    snapshot->alpha = EGUI_VIEW_OF(&preview_password_box)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_password_box));
}

static void assert_preview_state_unchanged(const password_box_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_password_box)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_password_box.on_changed == snapshot->on_changed);
    EGUI_TEST_ASSERT_TRUE(preview_password_box.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_password_box.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_password_box.icon_font == snapshot->icon_font);
    assert_optional_string_equal(snapshot->label, preview_password_box.label);
    assert_optional_string_equal(snapshot->helper, preview_password_box.helper);
    assert_optional_string_equal(snapshot->placeholder, preview_password_box.placeholder);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_password_box.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_password_box.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_password_box.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_password_box.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_password_box.accent_color.full);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->text, preview_password_box.text) == 0);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->masked_text, preview_password_box.masked_text) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_len, preview_password_box.text_len);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->cursor_pos, preview_password_box.cursor_pos);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, preview_password_box.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_part, preview_password_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_password_box.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_password_box.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->revealed, preview_password_box.revealed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->cursor_visible, preview_password_box.cursor_visible);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->scroll_offset_x, preview_password_box.scroll_offset_x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_password_box)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_password_box)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_password_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, changed_revealed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PASSWORD_BOX_PART_NONE, changed_part);
    EGUI_TEST_ASSERT_TRUE(strcmp("", changed_text) == 0);
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

static void test_password_box_static_preview_consumes_input_and_keeps_state(void)
{
    password_box_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_password_box("7429");
    layout_preview_password_box(10, 20);
    get_part_center(EGUI_VIEW_OF(&preview_password_box), EGUI_VIEW_PASSWORD_BOX_PART_FIELD, &x, &y);
    capture_preview_snapshot(&initial_snapshot);

    EGUI_VIEW_OF(&preview_password_box)->is_pressed = 1;
    preview_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_FIELD;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_password_box_set_current_part(EGUI_VIEW_OF(&preview_password_box), EGUI_VIEW_PASSWORD_BOX_PART_REVEAL);
    capture_preview_snapshot(&initial_snapshot);
    EGUI_VIEW_OF(&preview_password_box)->is_pressed = 1;
    preview_password_box.pressed_part = EGUI_VIEW_PASSWORD_BOX_PART_REVEAL;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER, 0));
    assert_preview_state_unchanged(&initial_snapshot);
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
    EGUI_TEST_RUN(test_password_box_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
