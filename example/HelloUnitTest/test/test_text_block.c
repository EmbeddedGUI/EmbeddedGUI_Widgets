#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_text_block.h"

#include "../../HelloCustomWidgets/display/text_block/egui_view_text_block.h"
#include "../../HelloCustomWidgets/display/text_block/egui_view_text_block.c"

static egui_view_text_block_t test_widget;
static egui_view_text_block_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

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
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 84, 34);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_widget), on_preview_click);
    egui_view_text_block_set_text(EGUI_VIEW_OF(&preview_widget), "Preview\ncopy");
    egui_view_text_block_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_text_block_apply_subtle_style(EGUI_VIEW_OF(&preview_widget));
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
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.style);
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

static void test_text_block_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 84, 34);
    get_view_center(EGUI_VIEW_OF(&preview_widget), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_text_block_get_compact_mode(EGUI_VIEW_OF(&preview_widget)));
}

void test_text_block_run(void)
{
    EGUI_TEST_SUITE_BEGIN(text_block);
    EGUI_TEST_RUN(test_text_block_init_uses_display_defaults);
    EGUI_TEST_RUN(test_text_block_style_helpers_and_modes_clear_pressed_state);
    EGUI_TEST_RUN(test_text_block_set_font_and_style_preserve_display_only_defaults);
    EGUI_TEST_RUN(test_text_block_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
