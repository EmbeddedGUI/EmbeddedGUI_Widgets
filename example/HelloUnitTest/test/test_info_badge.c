#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_info_badge.h"

#include "../../HelloCustomWidgets/display/info_badge/egui_view_info_badge.h"
#include "../../HelloCustomWidgets/display/info_badge/egui_view_info_badge.c"

extern const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_notification_badge_t);

static egui_view_notification_badge_t test_badge;
static egui_view_notification_badge_t preview_badge;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

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
    hcw_info_badge_apply_attention_style(EGUI_VIEW_OF(&preview_badge));
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_badge), on_preview_click);
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
    hcw_info_badge_set_palette(EGUI_VIEW_OF(&test_badge), EGUI_COLOR_HEX(0x13579B), EGUI_COLOR_HEX(0xF8F9FA));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x13579B).full, test_badge.badge_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF8F9FA).full, test_badge.text_color.full);
}

static void test_info_badge_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_badge();
    layout_view(EGUI_VIEW_OF(&preview_badge), 12, 18, 12, 12);
    get_view_center(EGUI_VIEW_OF(&preview_badge), &center_x, &center_y);

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_badge)->api->on_draw != EGUI_VIEW_API_TABLE_NAME(egui_view_notification_badge_t).on_draw);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_badge), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_badge), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_badge), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_badge), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_badge)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_info_badge_run(void)
{
    EGUI_TEST_SUITE_BEGIN(info_badge);
    EGUI_TEST_RUN(test_info_badge_style_helpers_apply_expected_modes_and_palette);
    EGUI_TEST_RUN(test_info_badge_setters_clear_pressed_state_and_switch_modes);
    EGUI_TEST_RUN(test_info_badge_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
