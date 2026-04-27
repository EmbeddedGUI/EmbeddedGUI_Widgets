#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_floater.h"

#include "../../HelloCustomWidgets/layout/floater/egui_view_floater.h"
#include "../../HelloCustomWidgets/layout/floater/egui_view_floater.c"

typedef struct floater_preview_snapshot floater_preview_snapshot_t;
struct floater_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    egui_view_t *child;
    const egui_font_t *text_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t floater_surface_color;
    egui_color_t floater_border_color;
    egui_color_t accent_color;
    char leading_text[EGUI_VIEW_FLOATER_MAX_TEXT_LEN + 1];
    char wrap_text[EGUI_VIEW_FLOATER_MAX_TEXT_LEN + 1];
    char trailing_text[EGUI_VIEW_FLOATER_MAX_TEXT_LEN + 1];
    egui_dim_t floater_width;
    egui_dim_t floater_height;
    egui_dim_t wrap_gap;
    egui_dim_t corner_radius;
    uint8_t align;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_t child_x;
    egui_dim_t child_y;
};

static egui_view_floater_t test_control;
static egui_view_label_t test_child;
static egui_view_floater_t preview_control;
static egui_view_label_t preview_child;
static egui_view_api_t preview_api;
static uint8_t click_count;

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

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void setup_child(egui_view_label_t *child, egui_dim_t width, egui_dim_t height, const char *text)
{
    egui_view_label_init(EGUI_VIEW_OF(child), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(child), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(child), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(child), EGUI_ALIGN_CENTER);
}

static void setup_floater(void)
{
    egui_view_floater_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 160, 80);
}

static void setup_preview_control(void)
{
    egui_view_floater_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 92, 48);
    setup_child(&preview_child, 42, 16, "Fig");
    egui_view_floater_set_child(EGUI_VIEW_OF(&preview_control), EGUI_VIEW_OF(&preview_child));
    egui_view_floater_apply_compact_style(EGUI_VIEW_OF(&preview_control));
    egui_view_floater_set_text(EGUI_VIEW_OF(&preview_control), "Fit", "wrap", "Next");
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_control), on_preview_click);
    egui_view_floater_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
    click_count = 0;
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

static void capture_preview_snapshot(floater_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->child = preview_control.child;
    snapshot->text_font = preview_control.text_font;
    snapshot->surface_color = preview_control.surface_color;
    snapshot->border_color = preview_control.border_color;
    snapshot->text_color = preview_control.text_color;
    snapshot->floater_surface_color = preview_control.floater_surface_color;
    snapshot->floater_border_color = preview_control.floater_border_color;
    snapshot->accent_color = preview_control.accent_color;
    egui_view_floater_copy_text(snapshot->leading_text, sizeof(snapshot->leading_text), preview_control.leading_text);
    egui_view_floater_copy_text(snapshot->wrap_text, sizeof(snapshot->wrap_text), preview_control.wrap_text);
    egui_view_floater_copy_text(snapshot->trailing_text, sizeof(snapshot->trailing_text), preview_control.trailing_text);
    snapshot->floater_width = preview_control.floater_width;
    snapshot->floater_height = preview_control.floater_height;
    snapshot->wrap_gap = preview_control.wrap_gap;
    snapshot->corner_radius = preview_control.corner_radius;
    snapshot->align = preview_control.align;
    snapshot->compact_mode = preview_control.compact_mode;
    snapshot->read_only_mode = preview_control.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->child_x = EGUI_VIEW_OF(&preview_child)->region.location.x;
    snapshot->child_y = EGUI_VIEW_OF(&preview_child)->region.location.y;
}

static void assert_preview_state_unchanged(const floater_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_TRUE(preview_control.child == snapshot->child);
    EGUI_TEST_ASSERT_TRUE(preview_control.text_font == snapshot->text_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_control.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->floater_surface_color.full, preview_control.floater_surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->floater_border_color.full, preview_control.floater_border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    assert_string_equal(snapshot->leading_text, preview_control.leading_text);
    assert_string_equal(snapshot->wrap_text, preview_control.wrap_text);
    assert_string_equal(snapshot->trailing_text, preview_control.trailing_text);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->floater_width, preview_control.floater_width);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->floater_height, preview_control.floater_height);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->wrap_gap, preview_control.wrap_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->corner_radius, preview_control.corner_radius);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->align, preview_control.align);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_control.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_control.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->child_x, EGUI_VIEW_OF(&preview_child)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->child_y, EGUI_VIEW_OF(&preview_child)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_floater_init_defaults(void)
{
    setup_floater();

    EGUI_TEST_ASSERT_TRUE(egui_view_floater_get_child(EGUI_VIEW_OF(&test_control)) == NULL);
    assert_string_equal("Intro text", egui_view_floater_get_leading_text(EGUI_VIEW_OF(&test_control)));
    assert_string_equal("wrapped context", egui_view_floater_get_wrap_text(EGUI_VIEW_OF(&test_control)));
    assert_string_equal("after floater", egui_view_floater_get_trailing_text(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_TRUE(test_control.text_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_EQUAL_INT(68, egui_view_floater_get_floater_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(34, egui_view_floater_get_floater_height(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_floater_get_wrap_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_floater_get_corner_radius(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLOATER_ALIGN_LEFT, egui_view_floater_get_align(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_floater_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_floater_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_control.accent_color.full);
}

static void test_floater_child_regions_and_layout(void)
{
    egui_region_t leading_region;
    egui_region_t floater_region;
    egui_region_t wrap_region;
    egui_region_t trailing_region;

    setup_floater();
    setup_child(&test_child, 40, 16, "Child");
    egui_view_floater_set_child(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_child));
    EGUI_TEST_ASSERT_TRUE(egui_view_floater_get_child(EGUI_VIEW_OF(&test_control)) == EGUI_VIEW_OF(&test_child));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_group_get_child_count(EGUI_VIEW_OF(&test_control)));

    layout_view(EGUI_VIEW_OF(&test_control), 10, 20, 160, 80);
    egui_view_floater_layout_child(EGUI_VIEW_OF(&test_control));
    egui_view_floater_get_regions(EGUI_VIEW_OF(&test_control), &leading_region, &floater_region, &wrap_region, &trailing_region);
    EGUI_TEST_ASSERT_EQUAL_INT(12, leading_region.size.height);
    EGUI_TEST_ASSERT_EQUAL_INT(0, floater_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(17, floater_region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(68, floater_region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(34, floater_region.size.height);
    EGUI_TEST_ASSERT_EQUAL_INT(73, wrap_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(87, wrap_region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(68, trailing_region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(14, EGUI_VIEW_OF(&test_child)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(26, EGUI_VIEW_OF(&test_child)->region.location.y);

    egui_view_floater_set_align(EGUI_VIEW_OF(&test_control), EGUI_VIEW_FLOATER_ALIGN_RIGHT);
    egui_view_floater_get_regions(EGUI_VIEW_OF(&test_control), NULL, &floater_region, &wrap_region, NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(92, floater_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(0, wrap_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(87, wrap_region.size.width);

    egui_view_floater_set_metrics(EGUI_VIEW_OF(&test_control), 56, 26, 4, 6);
    egui_view_floater_set_compact_mode(EGUI_VIEW_OF(&test_control), 1);
    egui_view_floater_set_align(EGUI_VIEW_OF(&test_control), EGUI_VIEW_FLOATER_ALIGN_FULL);
    egui_view_floater_get_regions(EGUI_VIEW_OF(&test_control), &leading_region, &floater_region, &wrap_region, NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(10, leading_region.size.height);
    EGUI_TEST_ASSERT_EQUAL_INT(0, floater_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(160, floater_region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(14, floater_region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(44, wrap_region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(22, wrap_region.size.height);
}

static void test_floater_styles_palette_text_and_clamps(void)
{
    char long_text[] = "This text is intentionally longer than floater storage";

    setup_floater();
    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_floater_set_text(EGUI_VIEW_OF(&test_control), long_text, "wrap", "tail");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLOATER_MAX_TEXT_LEN, (int)strlen(test_control.leading_text));
    assert_string_equal("wrap", test_control.wrap_text);
    assert_string_equal("tail", test_control.trailing_text);

    egui_view_floater_set_metrics(EGUI_VIEW_OF(&test_control), -1, -1, 99, 99);
    EGUI_TEST_ASSERT_EQUAL_INT(42, egui_view_floater_get_floater_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(22, egui_view_floater_get_floater_height(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_floater_get_wrap_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(16, egui_view_floater_get_corner_radius(EGUI_VIEW_OF(&test_control)));

    egui_view_floater_set_metrics(EGUI_VIEW_OF(&test_control), 999, 999, -1, -1);
    EGUI_TEST_ASSERT_EQUAL_INT(118, egui_view_floater_get_floater_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(58, egui_view_floater_get_floater_height(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_floater_get_wrap_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_floater_get_corner_radius(EGUI_VIEW_OF(&test_control)));

    egui_view_floater_set_align(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLOATER_ALIGN_LEFT, egui_view_floater_get_align(EGUI_VIEW_OF(&test_control)));

    egui_view_floater_apply_accent_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLOATER_ALIGN_RIGHT, egui_view_floater_get_align(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_floater_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_floater_get_read_only_mode(EGUI_VIEW_OF(&test_control)));

    egui_view_floater_apply_compact_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLOATER_ALIGN_FULL, egui_view_floater_get_align(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_floater_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_floater_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, test_control.accent_color.full);

    egui_view_floater_apply_read_only_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_floater_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_floater_get_read_only_mode(EGUI_VIEW_OF(&test_control)));

    egui_view_floater_set_font(EGUI_VIEW_OF(&test_control), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    EGUI_TEST_ASSERT_TRUE(test_control.text_font == (const egui_font_t *)&egui_res_font_montserrat_10_4);

    egui_view_floater_set_palette(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213), EGUI_COLOR_HEX(0x212223),
                                  EGUI_COLOR_HEX(0x313233), EGUI_COLOR_HEX(0x414243), EGUI_COLOR_HEX(0x515253));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x313233).full, test_control.floater_surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x515253).full, test_control.accent_color.full);
}

static void test_floater_static_preview_consumes_input_and_keeps_state(void)
{
    floater_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_control();
    layout_view(EGUI_VIEW_OF(&preview_control), 12, 18, 92, 48);
    egui_view_floater_layout_child(EGUI_VIEW_OF(&preview_control));
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

void test_floater_run(void)
{
    EGUI_TEST_SUITE_BEGIN(floater);
    EGUI_TEST_RUN(test_floater_init_defaults);
    EGUI_TEST_RUN(test_floater_child_regions_and_layout);
    EGUI_TEST_RUN(test_floater_styles_palette_text_and_clamps);
    EGUI_TEST_RUN(test_floater_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
