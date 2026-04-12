#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_divider.h"

#include "../../HelloCustomWidgets/display/divider/egui_view_divider.h"
#include "../../HelloCustomWidgets/display/divider/egui_view_divider.c"

static egui_view_divider_t test_divider;
static egui_view_divider_t preview_divider;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_divider(void)
{
    egui_view_divider_init(EGUI_VIEW_OF(&test_divider));
    egui_view_set_size(EGUI_VIEW_OF(&test_divider), 176, 2);
    hcw_divider_apply_standard_style(EGUI_VIEW_OF(&test_divider));
    g_click_count = 0;
}

static void setup_preview_divider(void)
{
    egui_view_divider_init(EGUI_VIEW_OF(&preview_divider));
    egui_view_set_size(EGUI_VIEW_OF(&preview_divider), 84, 1);
    hcw_divider_apply_subtle_style(EGUI_VIEW_OF(&preview_divider));
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_divider), on_preview_click);
    hcw_divider_override_static_preview_api(EGUI_VIEW_OF(&preview_divider), &preview_api);
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

static void test_divider_style_helpers_apply_expected_palette_and_alpha(void)
{
    setup_divider();

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_divider)->background == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC7D1DB).full, test_divider.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALPHA_100, test_divider.alpha);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_divider), 1);
    hcw_divider_apply_subtle_style(EGUI_VIEW_OF(&test_divider));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_divider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xD9E1E8).full, test_divider.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(72, test_divider.alpha);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_divider), 1);
    hcw_divider_apply_accent_style(EGUI_VIEW_OF(&test_divider));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_divider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_divider.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALPHA_100, test_divider.alpha);
}

static void test_divider_palette_setter_clears_pressed_state(void)
{
    egui_color_t color = EGUI_COLOR_HEX(0x345678);

    setup_divider();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_divider), 1);
    hcw_divider_set_palette(EGUI_VIEW_OF(&test_divider), color, 55);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_divider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(color.full, test_divider.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(55, test_divider.alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_divider)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_divider)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_divider)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_divider)->padding.bottom);
}

static void test_divider_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_divider();
    layout_view(EGUI_VIEW_OF(&preview_divider), 12, 18, 84, 1);
    get_view_center(EGUI_VIEW_OF(&preview_divider), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_divider), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_divider), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_divider), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_divider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_divider), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_divider), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_divider), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_divider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_divider_run(void)
{
    EGUI_TEST_SUITE_BEGIN(divider);
    EGUI_TEST_RUN(test_divider_style_helpers_apply_expected_palette_and_alpha);
    EGUI_TEST_RUN(test_divider_palette_setter_clears_pressed_state);
    EGUI_TEST_RUN(test_divider_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
