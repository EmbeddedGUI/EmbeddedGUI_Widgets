#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_divider.h"

#include "../../HelloCustomWidgets/display/divider/egui_view_divider.h"
#include "../../HelloCustomWidgets/display/divider/egui_view_divider.c"

typedef struct divider_preview_snapshot divider_preview_snapshot_t;
struct divider_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    egui_color_t color;
    egui_alpha_t alpha;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_divider_t test_divider;
static egui_view_divider_t preview_divider;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

EGUI_BACKGROUND_COLOR_PARAM_INIT_ROUND_RECTANGLE(bg_test_panel_param, EGUI_COLOR_HEX(0xFFFFFF), EGUI_ALPHA_100, 4);
EGUI_BACKGROUND_PARAM_INIT(bg_test_panel_params, &bg_test_panel_param, NULL, NULL);
EGUI_BACKGROUND_COLOR_STATIC_CONST_INIT(bg_test_panel, &bg_test_panel_params);

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
    egui_view_set_size(EGUI_VIEW_OF(&preview_divider), 36, 2);
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

static void capture_preview_snapshot(divider_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_divider)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_divider)->background;
    snapshot->color = preview_divider.color;
    snapshot->alpha = preview_divider.alpha;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_divider)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_divider)->api;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_divider));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_divider)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_divider)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_divider)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_divider)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_divider)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_divider)->padding.bottom;
}

static void assert_preview_state_unchanged(const divider_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_divider)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_divider)->background == snapshot->background);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->color.full, preview_divider.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, preview_divider.alpha);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_divider)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_divider)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_divider)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_divider)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_divider)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_divider)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_divider)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_divider)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_divider)->padding.bottom);
}

static void test_divider_style_helpers_apply_expected_palette_and_clear_pressed_state(void)
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

static void test_divider_palette_setter_clears_pressed_state_and_resets_background(void)
{
    egui_color_t color = EGUI_COLOR_HEX(0x345678);

    setup_divider();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_divider), 1);
    egui_view_set_background(EGUI_VIEW_OF(&test_divider), EGUI_BG_OF(&bg_test_panel));
    egui_view_set_padding(EGUI_VIEW_OF(&test_divider), 2, 3, 4, 5);
    hcw_divider_set_palette(EGUI_VIEW_OF(&test_divider), color, 55);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_divider)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_divider)->background == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(color.full, test_divider.color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(55, test_divider.alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_divider)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_divider)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_divider)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_divider)->padding.bottom);
}

static void test_divider_static_preview_consumes_input_and_keeps_state(void)
{
    divider_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_divider();
    layout_view(EGUI_VIEW_OF(&preview_divider), 12, 18, 36, 2);
    get_view_center(EGUI_VIEW_OF(&preview_divider), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_divider), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_divider), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_divider), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_divider), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_divider_run(void)
{
    EGUI_TEST_SUITE_BEGIN(divider);
    EGUI_TEST_RUN(test_divider_style_helpers_apply_expected_palette_and_clear_pressed_state);
    EGUI_TEST_RUN(test_divider_palette_setter_clears_pressed_state_and_resets_background);
    EGUI_TEST_RUN(test_divider_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
