#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_canvas.h"

#include "../../HelloCustomWidgets/layout/canvas/egui_view_canvas.h"
#include "../../HelloCustomWidgets/layout/canvas/egui_view_canvas.c"

static egui_view_group_t test_canvas_group;
static egui_view_group_t preview_canvas_group;
static egui_view_label_t test_cards[3];
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void init_card(egui_view_label_t *card, const char *text, egui_dim_t width, egui_dim_t height)
{
    egui_view_label_init(EGUI_VIEW_OF(card));
    egui_view_set_size(EGUI_VIEW_OF(card), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(card), text);
}

static void setup_canvas(egui_view_group_t *canvas, egui_dim_t width, egui_dim_t height)
{
    egui_view_group_init(EGUI_VIEW_OF(canvas));
    egui_view_set_size(EGUI_VIEW_OF(canvas), width, height);
    hcw_canvas_apply_standard_style(EGUI_VIEW_OF(canvas));

    init_card(&test_cards[0], "A", 40, 16);
    init_card(&test_cards[1], "B", 36, 16);
    init_card(&test_cards[2], "C", 44, 14);

    egui_view_group_add_child(EGUI_VIEW_OF(canvas), EGUI_VIEW_OF(&test_cards[0]));
    egui_view_group_add_child(EGUI_VIEW_OF(canvas), EGUI_VIEW_OF(&test_cards[1]));
    egui_view_group_add_child(EGUI_VIEW_OF(canvas), EGUI_VIEW_OF(&test_cards[2]));
}

static void setup_test_canvas(void)
{
    setup_canvas(&test_canvas_group, 120, 72);
    g_click_count = 0;
}

static void setup_preview_canvas(void)
{
    setup_canvas(&preview_canvas_group, 84, 40);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_canvas_group), on_preview_click);
    hcw_canvas_override_static_preview_api(EGUI_VIEW_OF(&preview_canvas_group), &preview_api);
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

static void layout_canvas(egui_view_t *view, egui_dim_t width, egui_dim_t height)
{
    layout_view(view, 12, 18, width, height);
    hcw_canvas_layout_childs(view);
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

static void test_canvas_style_helpers_and_child_origin_clear_pressed_state(void)
{
    setup_test_canvas();

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_canvas_group)->background == NULL);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_canvas_group), 1);
    hcw_canvas_apply_compact_style(EGUI_VIEW_OF(&test_canvas_group));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_canvas_group)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_canvas_group)->background == NULL);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_canvas_group), 1);
    hcw_canvas_apply_standard_style(EGUI_VIEW_OF(&test_canvas_group));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_canvas_group)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_canvas_group)->background == NULL);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_cards[0]), 1);
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[0]), 18, 12);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_cards[0])->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(18, EGUI_VIEW_OF(&test_cards[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_cards[0])->region.location.y);
}

static void test_canvas_layout_childs_places_cards_for_standard_and_compact_modes(void)
{
    setup_test_canvas();

    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[0]), 0, 0);
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[1]), 48, 8);
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[2]), 16, 28);
    layout_canvas(EGUI_VIEW_OF(&test_canvas_group), 120, 72);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_cards[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_cards[0])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(48, EGUI_VIEW_OF(&test_cards[1])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(8, EGUI_VIEW_OF(&test_cards[1])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(16, EGUI_VIEW_OF(&test_cards[2])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(28, EGUI_VIEW_OF(&test_cards[2])->region.location.y);

    hcw_canvas_apply_compact_style(EGUI_VIEW_OF(&test_canvas_group));
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[0]), 6, 2);
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[1]), 54, 10);
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[2]), 24, 32);
    layout_canvas(EGUI_VIEW_OF(&test_canvas_group), 120, 72);
    EGUI_TEST_ASSERT_EQUAL_INT(6, EGUI_VIEW_OF(&test_cards[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_cards[0])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(54, EGUI_VIEW_OF(&test_cards[1])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(10, EGUI_VIEW_OF(&test_cards[1])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(24, EGUI_VIEW_OF(&test_cards[2])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(32, EGUI_VIEW_OF(&test_cards[2])->region.location.y);
}

static void test_canvas_layout_childs_clears_pressed_state(void)
{
    setup_test_canvas();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_canvas_group), 1);
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[0]), 12, 4);
    layout_canvas(EGUI_VIEW_OF(&test_canvas_group), 120, 72);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_canvas_group)->is_pressed);
}

static void test_canvas_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_canvas();
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[0]), 0, 0);
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[1]), 30, 8);
    hcw_canvas_set_child_origin(EGUI_VIEW_OF(&test_cards[2]), 12, 18);
    layout_canvas(EGUI_VIEW_OF(&preview_canvas_group), 84, 40);
    get_view_center(EGUI_VIEW_OF(&preview_canvas_group), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_canvas_group), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_canvas_group), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_canvas_group), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_canvas_group)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_canvas_group), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_canvas_group), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_canvas_group), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_canvas_group)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_canvas_run(void)
{
    EGUI_TEST_SUITE_BEGIN(canvas);
    EGUI_TEST_RUN(test_canvas_style_helpers_and_child_origin_clear_pressed_state);
    EGUI_TEST_RUN(test_canvas_layout_childs_places_cards_for_standard_and_compact_modes);
    EGUI_TEST_RUN(test_canvas_layout_childs_clears_pressed_state);
    EGUI_TEST_RUN(test_canvas_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
