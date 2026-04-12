#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_rich_text_block.h"

#include "../../HelloCustomWidgets/display/rich_text_block/egui_view_rich_text_block.h"
#include "../../HelloCustomWidgets/display/rich_text_block/egui_view_rich_text_block.c"

static egui_view_rich_text_block_t test_widget;
static egui_view_rich_text_block_t preview_widget;
static egui_view_api_t preview_api;

static const egui_view_rich_text_block_paragraph_t g_paragraphs[] = {
        {"Lead sentence for the summary block.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS},
        {"Body copy wraps across the available width and keeps the reading order stable.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Caption line", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const egui_view_rich_text_block_paragraph_t g_accent_preview[] = {
        {"Approve before publishing.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT},
        {"Read-only preview", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const egui_view_rich_text_block_paragraph_t g_overflow_paragraphs[] = {
        {"One", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Two", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS},
        {"Three", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT},
        {"Four", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
        {"Five", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
};

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

static void setup_widget(void)
{
    egui_view_rich_text_block_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 176, 86);
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&test_widget), g_paragraphs, EGUI_ARRAY_SIZE(g_paragraphs));
}

static void setup_preview_widget(void)
{
    egui_view_rich_text_block_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 84, 42);
    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&preview_widget), g_accent_preview, EGUI_ARRAY_SIZE(g_accent_preview));
    egui_view_rich_text_block_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_rich_text_block_set_read_only_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_rich_text_block_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
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
    handled |= view->api->on_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->on_key_event(view, &event);
    return handled;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void assert_pressed_cleared(egui_view_t *view)
{
    EGUI_TEST_ASSERT_FALSE(view->is_pressed);
}

static void test_rich_text_block_init_uses_defaults(void)
{
    egui_view_rich_text_block_init(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_text_block_get_paragraph_count(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_NULL(test_widget.emphasis_font);
    EGUI_TEST_ASSERT_NULL(test_widget.caption_font);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_text_block_get_compact_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rich_text_block_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, test_widget.paragraph_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_widget.line_space);
}

static void test_rich_text_block_setters_clamp_content_and_clear_pressed_state(void)
{
    setup_widget();

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&test_widget), g_overflow_paragraphs, EGUI_ARRAY_SIZE(g_overflow_paragraphs));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_TEXT_BLOCK_MAX_PARAGRAPHS, egui_view_rich_text_block_get_paragraph_count(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY, egui_view_rich_text_block_clamp_style(9));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_TEXT_BLOCK_MAX_PARAGRAPHS, egui_view_rich_text_block_clamp_count(9));

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_widget));

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    EGUI_TEST_ASSERT_TRUE(egui_view_rich_text_block_resolve_font(&test_widget, EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS) ==
                          (const egui_font_t *)&egui_res_font_montserrat_12_4);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_widget));

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    EGUI_TEST_ASSERT_TRUE(egui_view_rich_text_block_resolve_font(&test_widget, EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION) ==
                          (const egui_font_t *)&egui_res_font_montserrat_8_4);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_widget));

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_text_block_get_compact_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_widget.paragraph_gap);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_widget));

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_text_block_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_widget));

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                          EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_widget.accent_color.full);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(EGUI_COLOR_HEX(0x123456), EGUI_COLOR_DARK_GREY, 64).full,
                               egui_view_rich_text_block_mix_disabled(EGUI_COLOR_HEX(0x123456)).full);
}

static void test_rich_text_block_layout_helpers_measure_paragraphs(void)
{
    egui_view_rich_text_block_paragraph_layout_t body_layout;
    egui_view_rich_text_block_paragraph_layout_t accent_layout;
    egui_dim_t content_height;

    setup_widget();
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 16, 176, 86);

    egui_view_rich_text_block_prepare_layout(&test_widget, EGUI_VIEW_OF(&test_widget), &g_paragraphs[0], 0, &body_layout);
    EGUI_TEST_ASSERT_TRUE(body_layout.text_region.size.height > 0);
    EGUI_TEST_ASSERT_FALSE(body_layout.has_box);
    EGUI_TEST_ASSERT_TRUE(body_layout.font == (const egui_font_t *)&egui_res_font_montserrat_12_4);

    egui_view_rich_text_block_prepare_layout(&test_widget, EGUI_VIEW_OF(&test_widget), &g_accent_preview[0], body_layout.text_region.size.height + test_widget.paragraph_gap,
                                             &accent_layout);
    EGUI_TEST_ASSERT_TRUE(accent_layout.has_box);
    EGUI_TEST_ASSERT_TRUE(accent_layout.box_region.size.height > accent_layout.text_region.size.height);
    EGUI_TEST_ASSERT_TRUE(accent_layout.text_region.location.x > accent_layout.box_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(test_widget.accent_color.full,
                               egui_view_rich_text_block_resolve_color(&test_widget, EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_widget.muted_text_color.full,
                               egui_view_rich_text_block_resolve_color(&test_widget, EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION).full);

    content_height = egui_view_rich_text_block_measure_content_height(&test_widget, EGUI_VIEW_OF(&test_widget));
    EGUI_TEST_ASSERT_TRUE(content_height > body_layout.text_region.size.height);
}

static void test_rich_text_block_static_preview_consumes_input_and_preserves_state(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 84, 42);
    get_view_center(EGUI_VIEW_OF(&preview_widget), &center_x, &center_y);

    EGUI_VIEW_OF(&preview_widget)->is_pressed = 1;
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_widget));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_text_block_get_compact_mode(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_text_block_get_read_only_mode(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ARRAY_SIZE(g_accent_preview), egui_view_rich_text_block_get_paragraph_count(EGUI_VIEW_OF(&preview_widget)));

    EGUI_VIEW_OF(&preview_widget)->is_pressed = 1;
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_widget));
    EGUI_TEST_ASSERT_TRUE(preview_widget.paragraphs == g_accent_preview);
}

void test_rich_text_block_run(void)
{
    EGUI_TEST_SUITE_BEGIN(rich_text_block);
    EGUI_TEST_RUN(test_rich_text_block_init_uses_defaults);
    EGUI_TEST_RUN(test_rich_text_block_setters_clamp_content_and_clear_pressed_state);
    EGUI_TEST_RUN(test_rich_text_block_layout_helpers_measure_paragraphs);
    EGUI_TEST_RUN(test_rich_text_block_static_preview_consumes_input_and_preserves_state);
    EGUI_TEST_SUITE_END();
}
