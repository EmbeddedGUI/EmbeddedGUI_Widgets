#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_bullet_decorator.h"

#include "../../HelloCustomWidgets/layout/bullet_decorator/egui_view_bullet_decorator.h"
#include "../../HelloCustomWidgets/layout/bullet_decorator/egui_view_bullet_decorator.c"

typedef struct bullet_decorator_preview_snapshot bullet_decorator_preview_snapshot_t;
struct bullet_decorator_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    const egui_font_t *text_font;
    const egui_font_t *bullet_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t bullet_color;
    egui_color_t text_color;
    egui_color_t accent_color;
    char content_text[EGUI_VIEW_BULLET_DECORATOR_MAX_TEXT_LEN + 1];
    char bullet_text[EGUI_VIEW_BULLET_DECORATOR_MAX_BULLET_TEXT_LEN + 1];
    egui_dim_t bullet_slot_width;
    egui_dim_t bullet_gap;
    egui_dim_t bullet_size;
    egui_alpha_t alpha;
    uint8_t bullet_kind;
    uint8_t content_align_type;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_margin_padding_t padding_left;
    egui_dim_margin_padding_t padding_right;
    egui_dim_margin_padding_t padding_top;
    egui_dim_margin_padding_t padding_bottom;
};

static egui_view_bullet_decorator_t test_control;
static egui_view_bullet_decorator_t preview_control;
static egui_view_api_t preview_api;

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void assert_string_equal(const char *expected, const char *actual)
{
    EGUI_TEST_ASSERT_TRUE(expected != NULL);
    EGUI_TEST_ASSERT_TRUE(actual != NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(expected, actual));
}

static void setup_bullet_decorator(void)
{
    egui_view_bullet_decorator_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 150, 40);
}

static void setup_preview_control(void)
{
    egui_view_bullet_decorator_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 92, 30);
    egui_view_bullet_decorator_apply_compact_style(EGUI_VIEW_OF(&preview_control));
    egui_view_bullet_decorator_set_content_text(EGUI_VIEW_OF(&preview_control), "Compact");
    egui_view_bullet_decorator_set_bullet_kind(EGUI_VIEW_OF(&preview_control), EGUI_VIEW_BULLET_DECORATOR_BULLET_DOT);
    egui_view_bullet_decorator_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
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

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
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

static void capture_preview_snapshot(bullet_decorator_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->text_font = preview_control.text_font;
    snapshot->bullet_font = preview_control.bullet_font;
    snapshot->surface_color = preview_control.surface_color;
    snapshot->border_color = preview_control.border_color;
    snapshot->bullet_color = preview_control.bullet_color;
    snapshot->text_color = preview_control.text_color;
    snapshot->accent_color = preview_control.accent_color;
    egui_view_bullet_decorator_copy_text(snapshot->content_text, sizeof(snapshot->content_text), preview_control.content_text);
    egui_view_bullet_decorator_copy_text(snapshot->bullet_text, sizeof(snapshot->bullet_text), preview_control.bullet_text);
    snapshot->bullet_slot_width = preview_control.bullet_slot_width;
    snapshot->bullet_gap = preview_control.bullet_gap;
    snapshot->bullet_size = preview_control.bullet_size;
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->bullet_kind = preview_control.bullet_kind;
    snapshot->content_align_type = preview_control.content_align_type;
    snapshot->compact_mode = preview_control.compact_mode;
    snapshot->read_only_mode = preview_control.read_only_mode;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_control)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_control)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_control)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_control)->padding.bottom;
}

static void assert_preview_state_unchanged(const bullet_decorator_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_TRUE(preview_control.text_font == snapshot->text_font);
    EGUI_TEST_ASSERT_TRUE(preview_control.bullet_font == snapshot->bullet_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->bullet_color.full, preview_control.bullet_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_control.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    assert_string_equal(snapshot->content_text, preview_control.content_text);
    assert_string_equal(snapshot->bullet_text, preview_control.bullet_text);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->bullet_slot_width, preview_control.bullet_slot_width);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->bullet_gap, preview_control.bullet_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->bullet_size, preview_control.bullet_size);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->bullet_kind, preview_control.bullet_kind);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_align_type, preview_control.content_align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_control.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_control.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_control)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_control)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_control)->padding.bottom);
}

static void test_bullet_decorator_init_defaults(void)
{
    setup_bullet_decorator();

    assert_string_equal("Decorated content", egui_view_bullet_decorator_get_content_text(EGUI_VIEW_OF(&test_control)));
    assert_string_equal("1.", egui_view_bullet_decorator_get_bullet_text(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_TRUE(test_control.text_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_control.bullet_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_BULLET_DECORATOR_BULLET_DOT, egui_view_bullet_decorator_get_bullet_kind(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_bullet_decorator_get_bullet_slot_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_bullet_decorator_get_bullet_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_bullet_decorator_get_bullet_size(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, egui_view_bullet_decorator_get_content_align_type(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_bullet_decorator_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_bullet_decorator_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_control.bullet_color.full);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.right);
#endif
}

static void test_bullet_decorator_setters_regions_and_clamps(void)
{
    char long_text[] = "This decorated content text is intentionally longer than storage";
    egui_region_t bullet_region;
    egui_region_t content_region;

    setup_bullet_decorator();
    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_bullet_decorator_set_content_text(EGUI_VIEW_OF(&test_control), long_text);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_BULLET_DECORATOR_MAX_TEXT_LEN, egui_view_bullet_decorator_text_len(test_control.content_text));

    egui_view_bullet_decorator_set_bullet_text(EGUI_VIEW_OF(&test_control), "123456789");
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_BULLET_DECORATOR_MAX_BULLET_TEXT_LEN, egui_view_bullet_decorator_text_len(test_control.bullet_text));

    egui_view_bullet_decorator_set_bullet_kind(EGUI_VIEW_OF(&test_control), EGUI_VIEW_BULLET_DECORATOR_BULLET_SQUARE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_BULLET_DECORATOR_BULLET_SQUARE, egui_view_bullet_decorator_get_bullet_kind(EGUI_VIEW_OF(&test_control)));
    egui_view_bullet_decorator_set_bullet_kind(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_BULLET_DECORATOR_BULLET_DOT, egui_view_bullet_decorator_get_bullet_kind(EGUI_VIEW_OF(&test_control)));

    egui_view_bullet_decorator_set_metrics(EGUI_VIEW_OF(&test_control), -4, 99, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_bullet_decorator_get_bullet_slot_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_bullet_decorator_get_bullet_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_bullet_decorator_get_bullet_size(EGUI_VIEW_OF(&test_control)));

    egui_view_bullet_decorator_set_metrics(EGUI_VIEW_OF(&test_control), 80, -2, 99);
    EGUI_TEST_ASSERT_EQUAL_INT(44, egui_view_bullet_decorator_get_bullet_slot_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_bullet_decorator_get_bullet_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(14, egui_view_bullet_decorator_get_bullet_size(EGUI_VIEW_OF(&test_control)));

    egui_view_bullet_decorator_set_metrics(EGUI_VIEW_OF(&test_control), 24, 6, 8);
    layout_view(EGUI_VIEW_OF(&test_control), 10, 20, 150, 40);
    egui_view_bullet_decorator_get_regions(EGUI_VIEW_OF(&test_control), &bullet_region, &content_region);
    EGUI_TEST_ASSERT_EQUAL_INT(24, bullet_region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(content_region.location.x, bullet_region.location.x + bullet_region.size.width + 6);
    EGUI_TEST_ASSERT_EQUAL_INT(bullet_region.size.height, content_region.size.height);
}

static void test_bullet_decorator_styles_palette_and_fonts(void)
{
    setup_bullet_decorator();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_bullet_decorator_apply_accent_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_bullet_decorator_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_bullet_decorator_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(26, egui_view_bullet_decorator_get_bullet_slot_width(EGUI_VIEW_OF(&test_control)));

    egui_view_bullet_decorator_apply_compact_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_bullet_decorator_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_bullet_decorator_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, test_control.bullet_color.full);

    egui_view_bullet_decorator_apply_read_only_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_bullet_decorator_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_bullet_decorator_get_read_only_mode(EGUI_VIEW_OF(&test_control)));

    egui_view_bullet_decorator_set_fonts(EGUI_VIEW_OF(&test_control), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                         (const egui_font_t *)&egui_res_font_montserrat_8_4);
    EGUI_TEST_ASSERT_TRUE(test_control.text_font == (const egui_font_t *)&egui_res_font_montserrat_10_4);
    EGUI_TEST_ASSERT_TRUE(test_control.bullet_font == (const egui_font_t *)&egui_res_font_montserrat_8_4);

    egui_view_bullet_decorator_set_palette(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213),
                                           EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233), EGUI_COLOR_HEX(0x414243));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x212223).full, test_control.bullet_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x414243).full, test_control.accent_color.full);

    egui_view_bullet_decorator_set_content_align_type(EGUI_VIEW_OF(&test_control), EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, egui_view_bullet_decorator_get_content_align_type(EGUI_VIEW_OF(&test_control)));
}

static void test_bullet_decorator_static_preview_consumes_input_and_keeps_state(void)
{
    bullet_decorator_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_control();
    layout_view(EGUI_VIEW_OF(&preview_control), 12, 18, 92, 30);
    get_view_center(EGUI_VIEW_OF(&preview_control), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_control), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_control), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_control), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_control), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_control), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

static void test_bullet_decorator_text_helpers(void)
{
    char label[16];
    char value[24];

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_bullet_decorator_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(17, egui_view_bullet_decorator_text_len("Decorated content"));
    egui_view_bullet_decorator_copy_elided(label, sizeof(label), "Synchronization", 8);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp("Synch...", label));
    egui_view_bullet_decorator_fit_text_to_width(NULL, "Bullet content target", value, sizeof(value), 24, 4);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp("Bul...", value));
}

void test_bullet_decorator_run(void)
{
    EGUI_TEST_SUITE_BEGIN(bullet_decorator);
    EGUI_TEST_RUN(test_bullet_decorator_init_defaults);
    EGUI_TEST_RUN(test_bullet_decorator_setters_regions_and_clamps);
    EGUI_TEST_RUN(test_bullet_decorator_styles_palette_and_fonts);
    EGUI_TEST_RUN(test_bullet_decorator_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_bullet_decorator_text_helpers);
    EGUI_TEST_SUITE_END();
}
