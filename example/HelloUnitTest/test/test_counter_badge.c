#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_counter_badge.h"

#include "../../HelloCustomWidgets/display/counter_badge/egui_view_counter_badge.h"
#include "../../HelloCustomWidgets/display/counter_badge/egui_view_counter_badge.c"

typedef struct counter_badge_preview_snapshot counter_badge_preview_snapshot_t;
struct counter_badge_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_font_t *font;
    uint8_t content_style;
    const char *icon;
    const egui_font_t *icon_font;
    egui_color_t badge_color;
    egui_color_t text_color;
    egui_color_t outline_color;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    egui_alpha_t alpha;
    uint16_t count;
    uint8_t max_display;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t dot_mode;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_counter_badge_t test_badge_widget;
static egui_view_counter_badge_t preview_badge_widget;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

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

static void layout_badge(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void setup_counter_badge(void)
{
    egui_view_counter_badge_init(EGUI_VIEW_OF(&test_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_badge_widget), 42, 24);
    g_click_count = 0;
}

static void setup_preview_badge(void)
{
    egui_view_counter_badge_init(EGUI_VIEW_OF(&preview_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_badge_widget), 18, 16);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_badge_widget), on_preview_click);
    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&preview_badge_widget), 4);
    egui_view_counter_badge_set_compact_mode(EGUI_VIEW_OF(&preview_badge_widget), 1);
    egui_view_counter_badge_override_static_preview_api(EGUI_VIEW_OF(&preview_badge_widget), &preview_api);
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

static void capture_preview_snapshot(counter_badge_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_badge_widget)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_badge_widget)->background;
    snapshot->font = preview_badge_widget.font;
    snapshot->content_style = preview_badge_widget.content_style;
    snapshot->icon = preview_badge_widget.icon;
    snapshot->icon_font = preview_badge_widget.icon_font;
    snapshot->badge_color = preview_badge_widget.badge_color;
    snapshot->text_color = preview_badge_widget.text_color;
    snapshot->outline_color = preview_badge_widget.outline_color;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_badge_widget)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_badge_widget)->api;
    snapshot->alpha = EGUI_VIEW_OF(&preview_badge_widget)->alpha;
    snapshot->count = preview_badge_widget.count;
    snapshot->max_display = preview_badge_widget.max_display;
    snapshot->compact_mode = preview_badge_widget.compact_mode;
    snapshot->read_only_mode = preview_badge_widget.read_only_mode;
    snapshot->dot_mode = preview_badge_widget.dot_mode;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_badge_widget));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_badge_widget)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_badge_widget)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_badge_widget)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_badge_widget)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_badge_widget)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_badge_widget)->padding.bottom;
}

static void assert_preview_state_unchanged(const counter_badge_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_badge_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_badge_widget)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_badge_widget.font == snapshot->font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_style, preview_badge_widget.content_style);
    EGUI_TEST_ASSERT_TRUE(preview_badge_widget.icon == snapshot->icon);
    EGUI_TEST_ASSERT_TRUE(preview_badge_widget.icon_font == snapshot->icon_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->badge_color.full, preview_badge_widget.badge_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_badge_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->outline_color.full, preview_badge_widget.outline_color.full);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_badge_widget)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_badge_widget)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_badge_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->count, preview_badge_widget.count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->max_display, preview_badge_widget.max_display);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_badge_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_badge_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->dot_mode, preview_badge_widget.dot_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_badge_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_badge_widget)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_badge_widget)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_badge_widget)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_badge_widget)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_badge_widget)->padding.bottom);
}

static void test_counter_badge_init_uses_defaults(void)
{
    setup_counter_badge();

    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_counter_badge_get_count(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(99, egui_view_counter_badge_get_max_display(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_counter_badge_get_dot_mode(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_counter_badge_get_compact_mode(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_FALSE(egui_view_counter_badge_get_read_only_mode(EGUI_VIEW_OF(&test_badge_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC42B1C).full, test_badge_widget.badge_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, test_badge_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, test_badge_widget.outline_color.full);
}

static void test_counter_badge_setters_clear_pressed_state_and_update_palette(void)
{
    setup_counter_badge();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&test_badge_widget), 128);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(128, egui_view_counter_badge_get_count(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_max_display(EGUI_VIEW_OF(&test_badge_widget), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_counter_badge_get_max_display(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_dot_mode(EGUI_VIEW_OF(&test_badge_widget), 3);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_dot_mode(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_palette(EGUI_VIEW_OF(&test_badge_widget), EGUI_COLOR_HEX(0x13579B), EGUI_COLOR_HEX(0xF8F9FA), EGUI_COLOR_HEX(0x102030));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x13579B).full, test_badge_widget.badge_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF8F9FA).full, test_badge_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x102030).full, test_badge_widget.outline_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_compact_mode(EGUI_VIEW_OF(&test_badge_widget), 2);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_compact_mode(EGUI_VIEW_OF(&test_badge_widget)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge_widget), 1);
    egui_view_counter_badge_set_read_only_mode(EGUI_VIEW_OF(&test_badge_widget), 4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_read_only_mode(EGUI_VIEW_OF(&test_badge_widget)));
}

static void test_counter_badge_helpers_compute_regions_and_modes(void)
{
    egui_region_t region;
    egui_dim_t single_width;
    egui_dim_t overflow_width;
    egui_dim_t dot_width;
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_counter_badge();
    layout_badge(EGUI_VIEW_OF(&test_badge_widget), 10, 20, 42, 24);

    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_badge_region(EGUI_VIEW_OF(&test_badge_widget), &region));
    single_width = region.size.width;

    egui_view_counter_badge_set_count(EGUI_VIEW_OF(&test_badge_widget), 128);
    egui_view_counter_badge_set_max_display(EGUI_VIEW_OF(&test_badge_widget), 99);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_badge_region(EGUI_VIEW_OF(&test_badge_widget), &region));
    overflow_width = region.size.width;
    EGUI_TEST_ASSERT_TRUE(overflow_width > single_width);

    egui_view_counter_badge_set_dot_mode(EGUI_VIEW_OF(&test_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(egui_view_counter_badge_get_badge_region(EGUI_VIEW_OF(&test_badge_widget), &region));
    dot_width = region.size.width;
    EGUI_TEST_ASSERT_TRUE(dot_width < overflow_width);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_HEX(0x83909D), 48).full, egui_view_counter_badge_mix_disabled(sample).full);
}

static void test_counter_badge_static_preview_consumes_input_and_keeps_state(void)
{
    counter_badge_preview_snapshot_t initial_snapshot;

    setup_preview_badge();
    layout_badge(EGUI_VIEW_OF(&preview_badge_widget), 12, 18, 18, 16);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_badge_widget), EGUI_MOTION_EVENT_ACTION_DOWN, 20, 24));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_badge_widget), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_counter_badge_run(void)
{
    EGUI_TEST_SUITE_BEGIN(counter_badge);
    EGUI_TEST_RUN(test_counter_badge_init_uses_defaults);
    EGUI_TEST_RUN(test_counter_badge_setters_clear_pressed_state_and_update_palette);
    EGUI_TEST_RUN(test_counter_badge_helpers_compute_regions_and_modes);
    EGUI_TEST_RUN(test_counter_badge_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
