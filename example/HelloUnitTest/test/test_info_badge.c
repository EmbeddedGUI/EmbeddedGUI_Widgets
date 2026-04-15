#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_info_badge.h"

#include "../../HelloCustomWidgets/display/info_badge/egui_view_info_badge.h"
#include "../../HelloCustomWidgets/display/info_badge/egui_view_info_badge.c"

extern const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_notification_badge_t);

typedef struct info_badge_preview_snapshot info_badge_preview_snapshot_t;
struct info_badge_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    uint16_t count;
    uint8_t max_display;
    egui_color_t badge_color;
    egui_color_t text_color;
    const egui_font_t *font;
    uint8_t content_style;
    const char *icon;
    const egui_font_t *icon_font;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_notification_badge_t test_badge;
static egui_view_notification_badge_t preview_badge;
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

static void setup_badge(void)
{
    egui_view_notification_badge_init(EGUI_VIEW_OF(&test_badge));
    egui_view_set_size(EGUI_VIEW_OF(&test_badge), 34, 20);
    hcw_info_badge_apply_count_style(EGUI_VIEW_OF(&test_badge));
    g_click_count = 0;
}

static void setup_preview_badge(void)
{
    egui_view_notification_badge_init(EGUI_VIEW_OF(&preview_badge));
    egui_view_set_size(EGUI_VIEW_OF(&preview_badge), 12, 12);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_badge), on_preview_click);
    egui_view_notification_badge_set_icon_font(EGUI_VIEW_OF(&preview_badge), EGUI_FONT_ICON_MS_16);
    hcw_info_badge_apply_attention_style(EGUI_VIEW_OF(&preview_badge));
    hcw_info_badge_set_palette(EGUI_VIEW_OF(&preview_badge), EGUI_COLOR_HEX(0xC42B1C), EGUI_COLOR_WHITE);
    hcw_info_badge_override_static_preview_api(EGUI_VIEW_OF(&preview_badge), &preview_api);
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

static void capture_preview_snapshot(info_badge_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_badge)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_badge)->background;
    snapshot->count = preview_badge.count;
    snapshot->max_display = preview_badge.max_display;
    snapshot->badge_color = preview_badge.badge_color;
    snapshot->text_color = preview_badge.text_color;
    snapshot->font = preview_badge.font;
    snapshot->content_style = preview_badge.content_style;
    snapshot->icon = preview_badge.icon;
    snapshot->icon_font = preview_badge.icon_font;
    snapshot->alpha = EGUI_VIEW_OF(&preview_badge)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_badge));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_badge)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_badge)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_badge)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_badge)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_badge)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_badge)->padding.bottom;
}

static void assert_preview_state_unchanged(const info_badge_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_badge)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_badge)->background == snapshot->background);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->count, preview_badge.count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->max_display, preview_badge.max_display);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->badge_color.full, preview_badge.badge_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_badge.text_color.full);
    EGUI_TEST_ASSERT_TRUE(preview_badge.font == snapshot->font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_style, preview_badge.content_style);
    assert_optional_string_equal(snapshot->icon, preview_badge.icon);
    EGUI_TEST_ASSERT_TRUE(preview_badge.icon_font == snapshot->icon_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_badge)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_badge)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_badge)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_badge)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_badge)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_badge)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_badge)->padding.bottom);
}

static void test_info_badge_style_helpers_apply_expected_modes_and_palette(void)
{
    setup_badge();

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_badge)->api->on_draw != EGUI_VIEW_API_TABLE_NAME(egui_view_notification_badge_t).on_draw);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_COUNT, test_badge.content_style);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC42B1C).full, test_badge.badge_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, test_badge.text_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge), 1);
    hcw_info_badge_apply_icon_style(EGUI_VIEW_OF(&test_badge));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_ICON, test_badge.content_style);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_badge.icon, EGUI_ICON_MS_INFO));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_badge.badge_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge), 1);
    hcw_info_badge_apply_attention_style(EGUI_VIEW_OF(&test_badge));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_ICON, test_badge.content_style);
    EGUI_TEST_ASSERT_NULL(test_badge.icon);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC42B1C).full, test_badge.badge_color.full);
}

static void test_info_badge_setters_clear_pressed_state_and_switch_modes(void)
{
    setup_badge();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge), 1);
    hcw_info_badge_set_count(EGUI_VIEW_OF(&test_badge), 27);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_COUNT, test_badge.content_style);
    EGUI_TEST_ASSERT_EQUAL_INT(27, test_badge.count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge), 1);
    hcw_info_badge_set_icon(EGUI_VIEW_OF(&test_badge), EGUI_ICON_MS_WARNING);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_ICON, test_badge.content_style);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_badge.icon, EGUI_ICON_MS_WARNING));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge), 1);
    hcw_info_badge_set_icon(EGUI_VIEW_OF(&test_badge), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NOTIFICATION_BADGE_CONTENT_STYLE_ICON, test_badge.content_style);
    EGUI_TEST_ASSERT_NULL(test_badge.icon);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_badge), 1);
    hcw_info_badge_set_palette(EGUI_VIEW_OF(&test_badge), EGUI_COLOR_HEX(0x13579B), EGUI_COLOR_HEX(0xF8F9FA));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x13579B).full, test_badge.badge_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF8F9FA).full, test_badge.text_color.full);
}

static void test_info_badge_static_preview_consumes_input_and_keeps_state(void)
{
    info_badge_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_badge();
    layout_view(EGUI_VIEW_OF(&preview_badge), 12, 18, 12, 12);
    get_view_center(EGUI_VIEW_OF(&preview_badge), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_badge)->api->on_draw != EGUI_VIEW_API_TABLE_NAME(egui_view_notification_badge_t).on_draw);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_badge), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_badge), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_info_badge_run(void)
{
    EGUI_TEST_SUITE_BEGIN(info_badge);
    EGUI_TEST_RUN(test_info_badge_style_helpers_apply_expected_modes_and_palette);
    EGUI_TEST_RUN(test_info_badge_setters_clear_pressed_state_and_switch_modes);
    EGUI_TEST_RUN(test_info_badge_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
