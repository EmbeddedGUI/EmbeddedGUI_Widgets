#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_text_block.h"

#include "../../HelloCustomWidgets/display/text_block/egui_view_text_block.h"
#include "../../HelloCustomWidgets/display/text_block/egui_view_text_block.c"

typedef struct text_block_preview_snapshot text_block_preview_snapshot_t;
struct text_block_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    const char *text;
    const egui_font_t *font;
    egui_color_t standard_color;
    egui_color_t subtle_color;
    egui_color_t accent_color;
    egui_color_t resolved_text_color;
    egui_alpha_t configured_text_alpha;
    egui_alpha_t resolved_text_alpha;
    egui_dim_t line_space;
    egui_dim_t max_lines;
    egui_dim_t content_line_count;
    egui_dim_t layout_width;
    uint8_t align_type;
    uint8_t is_auto_wrap_enabled;
    uint8_t is_scroll_enabled;
    uint8_t is_border_enabled;
    uint8_t style;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_text_block_t test_widget;
static egui_view_text_block_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

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

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_widget(void)
{
    egui_view_text_block_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 120, 48);
    g_click_count = 0;
}

static void setup_preview_widget(void)
{
    egui_view_text_block_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 88, 34);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_widget), on_preview_click);
    egui_view_text_block_set_text(EGUI_VIEW_OF(&preview_widget), "Compact copy\nfor tight rows.");
    egui_view_text_block_set_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_text_block_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_text_block_apply_subtle_style(EGUI_VIEW_OF(&preview_widget));
    egui_view_textblock_set_max_lines(EGUI_VIEW_OF(&preview_widget), 2);
    egui_view_text_block_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    g_click_count = 0;
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

static int send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(text_block_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_widget)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_widget)->background;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_widget)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_widget)->api;
    snapshot->text = preview_widget.base.text;
    snapshot->font = preview_widget.base.font;
    snapshot->standard_color = preview_widget.standard_color;
    snapshot->subtle_color = preview_widget.subtle_color;
    snapshot->accent_color = preview_widget.accent_color;
    snapshot->resolved_text_color = preview_widget.base.color;
    snapshot->configured_text_alpha = preview_widget.text_alpha;
    snapshot->resolved_text_alpha = preview_widget.base.alpha;
    snapshot->line_space = preview_widget.base.line_space;
    snapshot->max_lines = preview_widget.base.max_lines;
    snapshot->content_line_count = preview_widget.base.content_line_count;
    snapshot->layout_width = preview_widget.base.layout_width;
    snapshot->align_type = preview_widget.base.align_type;
    snapshot->is_auto_wrap_enabled = preview_widget.base.is_auto_wrap_enabled;
    snapshot->is_scroll_enabled = preview_widget.base.is_scroll_enabled;
    snapshot->is_border_enabled = preview_widget.base.is_border_enabled;
    snapshot->style = preview_widget.style;
    snapshot->compact_mode = preview_widget.compact_mode;
    snapshot->read_only_mode = preview_widget.read_only_mode;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_widget));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_widget)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_widget)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_widget)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_widget)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_widget)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_widget)->padding.bottom;
}

static void assert_preview_state_unchanged(const text_block_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->api == snapshot->api);
    assert_optional_string_equal(snapshot->text, preview_widget.base.text);
    EGUI_TEST_ASSERT_TRUE(preview_widget.base.font == snapshot->font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->standard_color.full, preview_widget.standard_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->subtle_color.full, preview_widget.subtle_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_widget.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->resolved_text_color.full, preview_widget.base.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->configured_text_alpha, preview_widget.text_alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->resolved_text_alpha, preview_widget.base.alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->line_space, preview_widget.base.line_space);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->max_lines, preview_widget.base.max_lines);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_line_count, preview_widget.base.content_line_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->layout_width, preview_widget.base.layout_width);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->align_type, preview_widget.base.align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_auto_wrap_enabled, preview_widget.base.is_auto_wrap_enabled);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_scroll_enabled, preview_widget.base.is_scroll_enabled);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_border_enabled, preview_widget.base.is_border_enabled);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->style, preview_widget.style);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_widget)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_widget)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_widget)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_widget)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_widget)->padding.bottom);
}

static void test_text_block_init_uses_display_defaults(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(test_widget.base.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(strcmp(test_widget.base.text, "") == 0);
    EGUI_TEST_ASSERT_TRUE(test_widget.base.is_auto_wrap_enabled);
    EGUI_TEST_ASSERT_FALSE(test_widget.base.is_scroll_enabled);
    EGUI_TEST_ASSERT_FALSE(test_widget.base.is_border_enabled);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, test_widget.base.align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_widget.base.line_space);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x22303F).full, test_widget.base.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALPHA_100, test_widget.base.alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEXT_BLOCK_STYLE_STANDARD, test_widget.style);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_text_block_get_compact_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_text_block_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));
}

static void test_text_block_style_helpers_and_modes_clear_pressed_state(void)
{
    setup_widget();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_widget), 1);
    egui_view_text_block_apply_subtle_style(EGUI_VIEW_OF(&test_widget));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEXT_BLOCK_STYLE_SUBTLE, test_widget.style);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x6B7A89).full, test_widget.base.color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_widget), 1);
    egui_view_text_block_apply_accent_style(EGUI_VIEW_OF(&test_widget));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TEXT_BLOCK_STYLE_ACCENT, test_widget.style);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_widget.base.color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_widget), 1);
    egui_view_text_block_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x111213), EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233), 88);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x313233).full, test_widget.base.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(88, test_widget.base.alpha);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_widget), 1);
    egui_view_text_block_set_text(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp(test_widget.base.text, "") == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_widget), 1);
    egui_view_text_block_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(test_widget.base.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_widget), 1);
    egui_view_text_block_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_text_block_get_compact_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_widget.base.line_space);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_widget), 1);
    egui_view_text_block_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_text_block_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_view_text_block_mix_disabled(EGUI_COLOR_HEX(0x313233)).full, test_widget.base.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(60, test_widget.base.alpha);
}

static void test_text_block_set_font_and_style_preserve_display_only_defaults(void)
{
    setup_widget();

    egui_view_text_block_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_textblock_set_max_lines(EGUI_VIEW_OF(&test_widget), 2);
    egui_view_text_block_set_text(EGUI_VIEW_OF(&test_widget), "One paragraph that still wraps.");
    egui_view_text_block_apply_standard_style(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_TRUE(test_widget.base.font == (const egui_font_t *)&egui_res_font_montserrat_8_4);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_widget.base.max_lines);
    EGUI_TEST_ASSERT_FALSE(test_widget.base.is_scroll_enabled);
    EGUI_TEST_ASSERT_FALSE(test_widget.base.is_border_enabled);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x22303F).full, test_widget.base.color.full);
}

static void test_text_block_static_preview_consumes_input_and_keeps_state(void)
{
    text_block_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 88, 34);
    get_view_center(EGUI_VIEW_OF(&preview_widget), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_text_block_run(void)
{
    EGUI_TEST_SUITE_BEGIN(text_block);
    EGUI_TEST_RUN(test_text_block_init_uses_display_defaults);
    EGUI_TEST_RUN(test_text_block_style_helpers_and_modes_clear_pressed_state);
    EGUI_TEST_RUN(test_text_block_set_font_and_style_preserve_display_only_defaults);
    EGUI_TEST_RUN(test_text_block_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
