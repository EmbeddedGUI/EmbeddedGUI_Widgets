#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_path_icon.h"

#include "../../HelloCustomWidgets/display/path_icon/egui_view_path_icon.h"
#include "../../HelloCustomWidgets/display/path_icon/egui_view_path_icon.c"

typedef struct path_icon_preview_snapshot path_icon_preview_snapshot_t;
struct path_icon_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_view_path_icon_data_t *data;
    egui_color_t icon_color;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_path_icon_t test_path_icon;
static egui_view_path_icon_t preview_path_icon;
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

static void setup_path_icon(void)
{
    egui_view_path_icon_init(EGUI_VIEW_OF(&test_path_icon));
    egui_view_set_size(EGUI_VIEW_OF(&test_path_icon), 32, 32);
    g_click_count = 0;
}

static void setup_preview_path_icon(void)
{
    egui_view_path_icon_init(EGUI_VIEW_OF(&preview_path_icon));
    egui_view_set_size(EGUI_VIEW_OF(&preview_path_icon), 30, 30);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_path_icon), on_preview_click);
    egui_view_path_icon_set_data(EGUI_VIEW_OF(&preview_path_icon), egui_view_path_icon_get_heart_data());
    egui_view_path_icon_apply_subtle_style(EGUI_VIEW_OF(&preview_path_icon));
    egui_view_path_icon_override_static_preview_api(EGUI_VIEW_OF(&preview_path_icon), &preview_api);
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

static void capture_preview_snapshot(path_icon_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_path_icon)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_path_icon)->background;
    snapshot->data = preview_path_icon.data;
    snapshot->icon_color = preview_path_icon.icon_color;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_path_icon)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_path_icon)->api;
    snapshot->alpha = EGUI_VIEW_OF(&preview_path_icon)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_path_icon));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_path_icon)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_path_icon)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_path_icon)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_path_icon)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_path_icon)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_path_icon)->padding.bottom;
}

static void assert_preview_state_unchanged(const path_icon_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_path_icon)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_path_icon)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_path_icon.data == snapshot->data);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->icon_color.full, preview_path_icon.icon_color.full);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_path_icon)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_path_icon)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_path_icon)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_path_icon)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_path_icon)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_path_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_path_icon)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_path_icon)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_path_icon)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_path_icon)->padding.bottom);
}

static void test_path_icon_init_uses_default_data_and_standard_palette(void)
{
    setup_path_icon();

    EGUI_TEST_ASSERT_TRUE(egui_view_path_icon_get_data(EGUI_VIEW_OF(&test_path_icon)) == egui_view_path_icon_get_bookmark_data());
    EGUI_TEST_ASSERT_TRUE(egui_view_path_icon_get_bookmark_data() != egui_view_path_icon_get_heart_data());
    EGUI_TEST_ASSERT_TRUE(egui_view_path_icon_get_bookmark_data() != egui_view_path_icon_get_send_data());
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_path_icon.icon_color.full);
}

static void test_path_icon_style_helpers_and_setters_clear_pressed_state(void)
{
    setup_path_icon();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_path_icon), 1);
    egui_view_path_icon_apply_standard_style(EGUI_VIEW_OF(&test_path_icon));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_path_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_path_icon.icon_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_path_icon), 1);
    egui_view_path_icon_apply_subtle_style(EGUI_VIEW_OF(&test_path_icon));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_path_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x6F7C8A).full, test_path_icon.icon_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_path_icon), 1);
    egui_view_path_icon_apply_accent_style(EGUI_VIEW_OF(&test_path_icon));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_path_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xA15C00).full, test_path_icon.icon_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_path_icon), 1);
    egui_view_path_icon_set_data(EGUI_VIEW_OF(&test_path_icon), egui_view_path_icon_get_heart_data());
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_path_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_path_icon_get_data(EGUI_VIEW_OF(&test_path_icon)) == egui_view_path_icon_get_heart_data());

    egui_view_set_pressed(EGUI_VIEW_OF(&test_path_icon), 1);
    egui_view_path_icon_set_palette(EGUI_VIEW_OF(&test_path_icon), EGUI_COLOR_HEX(0x0F7B45));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_path_icon)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F7B45).full, test_path_icon.icon_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_path_icon), 1);
    egui_view_path_icon_set_data(EGUI_VIEW_OF(&test_path_icon), NULL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_path_icon)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_path_icon_get_data(EGUI_VIEW_OF(&test_path_icon)) == egui_view_path_icon_get_bookmark_data());
}

static void test_path_icon_static_preview_consumes_input_and_keeps_state(void)
{
    path_icon_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_path_icon();
    layout_view(EGUI_VIEW_OF(&preview_path_icon), 12, 18, 30, 30);
    get_view_center(EGUI_VIEW_OF(&preview_path_icon), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_path_icon), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_path_icon), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_path_icon), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_path_icon), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_path_icon_run(void)
{
    EGUI_TEST_SUITE_BEGIN(path_icon);
    EGUI_TEST_RUN(test_path_icon_init_uses_default_data_and_standard_palette);
    EGUI_TEST_RUN(test_path_icon_style_helpers_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_path_icon_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
