#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_rich_text_block.h"

#include "../../HelloCustomWidgets/display/rich_text_block/egui_view_rich_text_block.h"
#include "../../HelloCustomWidgets/display/rich_text_block/egui_view_rich_text_block.c"

typedef struct rich_text_block_preview_snapshot rich_text_block_preview_snapshot_t;
struct rich_text_block_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_rich_text_block_paragraph_t *paragraphs;
    const egui_font_t *font;
    const egui_font_t *emphasis_font;
    const egui_font_t *caption_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    egui_alpha_t alpha;
    egui_dim_t line_space;
    egui_dim_t paragraph_gap;
    uint8_t paragraph_count;
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

static egui_view_rich_text_block_t test_widget;
static egui_view_rich_text_block_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static const egui_view_rich_text_block_paragraph_t g_primary_paragraphs[] = {
        {"Lead sentence for the summary block.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS},
        {"Body copy wraps across the available width and keeps the reading order stable.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Caption line", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const egui_view_rich_text_block_paragraph_t g_preview_paragraphs[] = {
        {"Owner review\nrequired.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Read only", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const egui_view_rich_text_block_paragraph_t g_accent_paragraphs[] = {
        {"Approve before publishing.", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT},
        {"Owner review required", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
};

static const egui_view_rich_text_block_paragraph_t g_overflow_paragraphs[] = {
        {"One", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
        {"Two", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS},
        {"Three", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_ACCENT},
        {"Four", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION},
        {"Five", EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY},
};

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
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

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void setup_widget(void)
{
    egui_view_rich_text_block_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 176, 96);
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&test_widget), g_primary_paragraphs, (uint8_t)EGUI_ARRAY_SIZE(g_primary_paragraphs));
    g_click_count = 0;
}

static void setup_preview_widget(void)
{
    egui_view_rich_text_block_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 84, 42);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_widget), on_preview_click);
    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&preview_widget), g_preview_paragraphs, (uint8_t)EGUI_ARRAY_SIZE(g_preview_paragraphs));
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rich_text_block_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_rich_text_block_set_read_only_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_rich_text_block_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    g_click_count = 0;
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

static void capture_preview_snapshot(rich_text_block_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_widget)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_widget)->background;
    snapshot->paragraphs = preview_widget.paragraphs;
    snapshot->font = preview_widget.font;
    snapshot->emphasis_font = preview_widget.emphasis_font;
    snapshot->caption_font = preview_widget.caption_font;
    snapshot->surface_color = preview_widget.surface_color;
    snapshot->border_color = preview_widget.border_color;
    snapshot->text_color = preview_widget.text_color;
    snapshot->muted_text_color = preview_widget.muted_text_color;
    snapshot->accent_color = preview_widget.accent_color;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_widget)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_widget)->api;
    snapshot->alpha = EGUI_VIEW_OF(&preview_widget)->alpha;
    snapshot->line_space = preview_widget.line_space;
    snapshot->paragraph_gap = preview_widget.paragraph_gap;
    snapshot->paragraph_count = preview_widget.paragraph_count;
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

static void assert_preview_state_unchanged(const rich_text_block_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_widget.paragraphs == snapshot->paragraphs);
    EGUI_TEST_ASSERT_TRUE(preview_widget.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_widget.emphasis_font == snapshot->emphasis_font);
    EGUI_TEST_ASSERT_TRUE(preview_widget.caption_font == snapshot->caption_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_widget.accent_color.full);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->line_space, preview_widget.line_space);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->paragraph_gap, preview_widget.paragraph_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->paragraph_count, preview_widget.paragraph_count);
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
    egui_view_rich_text_block_set_paragraphs(EGUI_VIEW_OF(&test_widget), g_overflow_paragraphs, (uint8_t)EGUI_ARRAY_SIZE(g_overflow_paragraphs));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_TEXT_BLOCK_MAX_PARAGRAPHS, egui_view_rich_text_block_get_paragraph_count(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_BODY, egui_view_rich_text_block_clamp_style(9));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RICH_TEXT_BLOCK_MAX_PARAGRAPHS, egui_view_rich_text_block_clamp_count(9));

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_emphasis_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    EGUI_TEST_ASSERT_TRUE(egui_view_rich_text_block_resolve_font(&test_widget, EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_EMPHASIS) ==
                          (const egui_font_t *)&egui_res_font_montserrat_12_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_caption_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    EGUI_TEST_ASSERT_TRUE(egui_view_rich_text_block_resolve_font(&test_widget, EGUI_VIEW_RICH_TEXT_BLOCK_STYLE_CAPTION) ==
                          (const egui_font_t *)&egui_res_font_montserrat_8_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_text_block_get_compact_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_widget.paragraph_gap);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rich_text_block_get_read_only_mode(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);

    EGUI_VIEW_OF(&test_widget)->is_pressed = 1;
    egui_view_rich_text_block_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                          EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_widget.accent_color.full);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);

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
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 16, 176, 96);

    egui_view_rich_text_block_prepare_layout(&test_widget, EGUI_VIEW_OF(&test_widget), &g_primary_paragraphs[0], 0, &body_layout);
    EGUI_TEST_ASSERT_TRUE(body_layout.text_region.size.height > 0);
    EGUI_TEST_ASSERT_FALSE(body_layout.has_box);
    EGUI_TEST_ASSERT_TRUE(body_layout.font == (const egui_font_t *)&egui_res_font_montserrat_12_4);

    egui_view_rich_text_block_prepare_layout(&test_widget, EGUI_VIEW_OF(&test_widget), &g_accent_paragraphs[0], body_layout.text_region.size.height + test_widget.paragraph_gap,
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

static void test_rich_text_block_static_preview_consumes_input_and_keeps_state(void)
{
    rich_text_block_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 84, 42);
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

void test_rich_text_block_run(void)
{
    EGUI_TEST_SUITE_BEGIN(rich_text_block);
    EGUI_TEST_RUN(test_rich_text_block_init_uses_defaults);
    EGUI_TEST_RUN(test_rich_text_block_setters_clamp_content_and_clear_pressed_state);
    EGUI_TEST_RUN(test_rich_text_block_layout_helpers_measure_paragraphs);
    EGUI_TEST_RUN(test_rich_text_block_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
