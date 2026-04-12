#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_image_icon.h"

#include "../../HelloCustomWidgets/display/image_icon/resource/img/egui_res_image_image_icon_landscape_rgb565_8.c"
#include "../../HelloCustomWidgets/display/image_icon/egui_view_image_icon.h"
#include "../../HelloCustomWidgets/display/image_icon/egui_view_image_icon.c"

static egui_view_image_icon_t test_image_icon;
static egui_view_image_icon_t preview_image_icon;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void setup_image_icon(void)
{
    egui_view_image_icon_init(EGUI_VIEW_OF(&test_image_icon));
    egui_view_set_size(EGUI_VIEW_OF(&test_image_icon), 32, 32);
    g_click_count = 0;
}

static void setup_preview_image_icon(void)
{
    egui_view_image_icon_init(EGUI_VIEW_OF(&preview_image_icon));
    egui_view_set_size(EGUI_VIEW_OF(&preview_image_icon), 24, 24);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_image_icon), on_preview_click);
    egui_view_image_icon_set_image(EGUI_VIEW_OF(&preview_image_icon), egui_view_image_icon_get_fresh_image());
    egui_view_image_icon_override_static_preview_api(EGUI_VIEW_OF(&preview_image_icon), &preview_api);
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

static void test_image_icon_init_uses_resize_mode_and_default_image(void)
{
    setup_image_icon();

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_IMAGE_TYPE_RESIZE, test_image_icon.image_type);
    EGUI_TEST_ASSERT_TRUE(egui_view_image_icon_get_image(EGUI_VIEW_OF(&test_image_icon)) == egui_view_image_icon_get_default_image());
    EGUI_TEST_ASSERT_TRUE(egui_view_image_icon_get_default_image() != egui_view_image_icon_get_warm_image());
    EGUI_TEST_ASSERT_TRUE(egui_view_image_icon_get_default_image() != egui_view_image_icon_get_fresh_image());
}

static void test_image_icon_set_image_updates_source_and_falls_back_to_default(void)
{
    setup_image_icon();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_image_icon), 1);
    egui_view_image_icon_set_image(EGUI_VIEW_OF(&test_image_icon), egui_view_image_icon_get_warm_image());
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_image_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_image_icon_get_image(EGUI_VIEW_OF(&test_image_icon)) == egui_view_image_icon_get_warm_image());

    egui_view_set_pressed(EGUI_VIEW_OF(&test_image_icon), 1);
    egui_view_image_icon_set_image(EGUI_VIEW_OF(&test_image_icon), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_image_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_image_icon_get_image(EGUI_VIEW_OF(&test_image_icon)) == egui_view_image_icon_get_default_image());
}

static void test_image_icon_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_image_icon();
    layout_view(EGUI_VIEW_OF(&preview_image_icon), 12, 18, 24, 24);
    get_view_center(EGUI_VIEW_OF(&preview_image_icon), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_image_icon), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_image_icon), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_image_icon), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_image_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_image_icon), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_image_icon), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_image_icon), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_image_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_image_icon_run(void)
{
    EGUI_TEST_SUITE_BEGIN(image_icon);
    EGUI_TEST_RUN(test_image_icon_init_uses_resize_mode_and_default_image);
    EGUI_TEST_RUN(test_image_icon_set_image_updates_source_and_falls_back_to_default);
    EGUI_TEST_RUN(test_image_icon_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
