#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_glyphs.h"

#include "../../HelloCustomWidgets/display/glyphs/egui_view_glyphs.h"
#include "../../HelloCustomWidgets/display/glyphs/egui_view_glyphs.c"

typedef struct glyphs_preview_snapshot glyphs_preview_snapshot_t;
struct glyphs_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    const char *unicode_string;
    const egui_font_t *font;
    egui_color_t fill_color;
    egui_color_t accent_color;
    uint8_t font_rendering_em_size;
    uint8_t origin_x_percent;
    uint8_t origin_y_percent;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_margin_padding_t padding_left;
    egui_dim_margin_padding_t padding_right;
    egui_dim_margin_padding_t padding_top;
    egui_dim_margin_padding_t padding_bottom;
};

static egui_view_glyphs_t test_control;
static egui_view_glyphs_t preview_control;
static egui_view_api_t preview_api;

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_glyphs(void)
{
    egui_view_glyphs_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 150, 62);
}

static void setup_preview_control(void)
{
    egui_view_glyphs_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 72, 34);
    egui_view_glyphs_set_unicode_string(EGUI_VIEW_OF(&preview_control), "ID-42");
    egui_view_glyphs_set_font(EGUI_VIEW_OF(&preview_control), (const egui_font_t *)&egui_res_font_montserrat_10_4, 10);
    egui_view_glyphs_set_fill(EGUI_VIEW_OF(&preview_control), HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_SOFT);
    egui_view_glyphs_set_origin(EGUI_VIEW_OF(&preview_control), 8, 24);
    egui_view_glyphs_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
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
    return view->api->dispatch_touch_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= view->api->dispatch_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->dispatch_key_event(view, &event);
    return handled;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(glyphs_preview_snapshot_t *snapshot)
{
    uint8_t origin_x = 0;
    uint8_t origin_y = 0;

    egui_view_glyphs_get_origin(EGUI_VIEW_OF(&preview_control), &origin_x, &origin_y);
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->unicode_string = preview_control.unicode_string;
    snapshot->font = preview_control.font;
    snapshot->fill_color = preview_control.fill_color;
    snapshot->accent_color = preview_control.accent_color;
    snapshot->font_rendering_em_size = preview_control.font_rendering_em_size;
    snapshot->origin_x_percent = origin_x;
    snapshot->origin_y_percent = origin_y;
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_control)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_control)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_control)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_control)->padding.bottom;
}

static void assert_preview_state_unchanged(const glyphs_preview_snapshot_t *snapshot)
{
    uint8_t origin_x = 0;
    uint8_t origin_y = 0;

    egui_view_glyphs_get_origin(EGUI_VIEW_OF(&preview_control), &origin_x, &origin_y);
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_TRUE(preview_control.unicode_string == snapshot->unicode_string);
    EGUI_TEST_ASSERT_TRUE(preview_control.font == snapshot->font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->fill_color.full, preview_control.fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->font_rendering_em_size, preview_control.font_rendering_em_size);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->origin_x_percent, origin_x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->origin_y_percent, origin_y);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_control)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_control)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_control)->padding.bottom);
}

static void assert_origin(egui_view_t *view, uint8_t x, uint8_t y)
{
    uint8_t origin_x = 0;
    uint8_t origin_y = 0;

    egui_view_glyphs_get_origin(view, &origin_x, &origin_y);
    EGUI_TEST_ASSERT_EQUAL_INT(x, origin_x);
    EGUI_TEST_ASSERT_EQUAL_INT(y, origin_y);
}

static void test_glyphs_init_defaults(void)
{
    setup_glyphs();

    EGUI_TEST_ASSERT_TRUE(strcmp("Glyphs", egui_view_glyphs_get_unicode_string(EGUI_VIEW_OF(&test_control))) == 0);
    EGUI_TEST_ASSERT_TRUE(egui_view_glyphs_get_font(EGUI_VIEW_OF(&test_control)) == (const egui_font_t *)&egui_res_font_montserrat_16_4);
    EGUI_TEST_ASSERT_EQUAL_INT(16, egui_view_glyphs_get_font_rendering_em_size(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT.full, test_control.fill_color.full);
    assert_origin(EGUI_VIEW_OF(&test_control), 8, 18);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.right);
#endif
}

static void test_glyphs_setters_clamp_and_clear_pressed_state(void)
{
    setup_glyphs();
    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_glyphs_set_origin(EGUI_VIEW_OF(&test_control), 120, 130);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    assert_origin(EGUI_VIEW_OF(&test_control), 100, 100);

    egui_view_glyphs_set_unicode_string(EGUI_VIEW_OF(&test_control), "Run 07");
    EGUI_TEST_ASSERT_TRUE(strcmp("Run 07", egui_view_glyphs_get_unicode_string(EGUI_VIEW_OF(&test_control))) == 0);

    egui_view_glyphs_set_unicode_string(EGUI_VIEW_OF(&test_control), NULL);
    EGUI_TEST_ASSERT_TRUE(strcmp("Glyphs", egui_view_glyphs_get_unicode_string(EGUI_VIEW_OF(&test_control))) == 0);

    egui_view_glyphs_set_font(EGUI_VIEW_OF(&test_control), NULL, 9);
    EGUI_TEST_ASSERT_TRUE(egui_view_glyphs_get_font(EGUI_VIEW_OF(&test_control)) == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_EQUAL_INT(9, egui_view_glyphs_get_font_rendering_em_size(EGUI_VIEW_OF(&test_control)));

    egui_view_glyphs_set_fill(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x111213), EGUI_COLOR_HEX(0x212223));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x111213).full, test_control.fill_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x212223).full, test_control.accent_color.full);
}

static void test_glyphs_styles(void)
{
    setup_glyphs();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_glyphs_set_unicode_string(EGUI_VIEW_OF(&test_control), "A1 B2 C3");
    egui_view_glyphs_set_font(EGUI_VIEW_OF(&test_control), (const egui_font_t *)&egui_res_font_montserrat_14_4, 14);
    egui_view_glyphs_set_fill(EGUI_VIEW_OF(&test_control), HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_TINT);
    egui_view_glyphs_set_origin(EGUI_VIEW_OF(&test_control), 10, 20);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(strcmp("A1 B2 C3", egui_view_glyphs_get_unicode_string(EGUI_VIEW_OF(&test_control))) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(14, egui_view_glyphs_get_font_rendering_em_size(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY.full, test_control.fill_color.full);
    assert_origin(EGUI_VIEW_OF(&test_control), 10, 20);

    egui_view_glyphs_set_unicode_string(EGUI_VIEW_OF(&test_control), "ID-42");
    egui_view_glyphs_set_font(EGUI_VIEW_OF(&test_control), (const egui_font_t *)&egui_res_font_montserrat_10_4, 10);
    egui_view_glyphs_set_fill(EGUI_VIEW_OF(&test_control), HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_SOFT);
    egui_view_glyphs_set_origin(EGUI_VIEW_OF(&test_control), 8, 24);
    EGUI_TEST_ASSERT_TRUE(strcmp("ID-42", egui_view_glyphs_get_unicode_string(EGUI_VIEW_OF(&test_control))) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_glyphs_get_font_rendering_em_size(EGUI_VIEW_OF(&test_control)));
    assert_origin(EGUI_VIEW_OF(&test_control), 8, 24);

    egui_view_glyphs_set_unicode_string(EGUI_VIEW_OF(&test_control), "Locked");
    egui_view_glyphs_set_fill(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x65717E), HCW_COLOR_BORDER);
    EGUI_TEST_ASSERT_TRUE(strcmp("Locked", egui_view_glyphs_get_unicode_string(EGUI_VIEW_OF(&test_control))) == 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x65717E).full, test_control.fill_color.full);
}

static void test_glyphs_static_preview_consumes_input_and_keeps_state(void)
{
    glyphs_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_control();
    layout_view(EGUI_VIEW_OF(&preview_control), 12, 18, 72, 34);
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

void test_glyphs_run(void)
{
    EGUI_TEST_SUITE_BEGIN(glyphs);
    EGUI_TEST_RUN(test_glyphs_init_defaults);
    EGUI_TEST_RUN(test_glyphs_setters_clamp_and_clear_pressed_state);
    EGUI_TEST_RUN(test_glyphs_styles);
    EGUI_TEST_RUN(test_glyphs_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
