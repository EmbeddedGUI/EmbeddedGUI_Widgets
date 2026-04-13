#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_dock_panel.h"

#include "../../HelloCustomWidgets/layout/dock_panel/egui_view_dock_panel.h"
#include "../../HelloCustomWidgets/layout/dock_panel/egui_view_dock_panel.c"

static hcw_dock_panel_t test_dock_panel;
static hcw_dock_panel_t preview_dock_panel;
static egui_view_label_t test_items[4];
static egui_view_label_t preview_items[3];
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

static void setup_test_panel(void)
{
    uint8_t index;
    static const egui_dim_t widths[] = {48, 18, 20, 30};
    static const egui_dim_t heights[] = {12, 16, 16, 20};

    hcw_dock_panel_init(EGUI_VIEW_OF(&test_dock_panel));
    egui_view_set_size(EGUI_VIEW_OF(&test_dock_panel), 120, 80);
    hcw_dock_panel_apply_standard_style(EGUI_VIEW_OF(&test_dock_panel));
    hcw_dock_panel_set_last_child_fill(EGUI_VIEW_OF(&test_dock_panel), 1);

    for (index = 0; index < EGUI_ARRAY_SIZE(test_items); index++)
    {
        init_card(&test_items[index], "Card", widths[index], heights[index]);
        egui_view_group_add_child(EGUI_VIEW_OF(&test_dock_panel), EGUI_VIEW_OF(&test_items[index]));
    }
}

static void setup_preview_panel(void)
{
    uint8_t index;
    static const egui_dim_t widths[] = {28, 18, 24};
    static const egui_dim_t heights[] = {8, 12, 12};

    hcw_dock_panel_init(EGUI_VIEW_OF(&preview_dock_panel));
    egui_view_set_size(EGUI_VIEW_OF(&preview_dock_panel), 84, 40);
    hcw_dock_panel_apply_standard_style(EGUI_VIEW_OF(&preview_dock_panel));
    hcw_dock_panel_set_last_child_fill(EGUI_VIEW_OF(&preview_dock_panel), 1);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_dock_panel), on_preview_click);
    hcw_dock_panel_override_static_preview_api(EGUI_VIEW_OF(&preview_dock_panel), &preview_api);

    for (index = 0; index < EGUI_ARRAY_SIZE(preview_items); index++)
    {
        init_card(&preview_items[index], "P", widths[index], heights[index]);
        egui_view_group_add_child(EGUI_VIEW_OF(&preview_dock_panel), EGUI_VIEW_OF(&preview_items[index]));
    }
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

static void layout_panel(egui_view_t *view, egui_dim_t width, egui_dim_t height)
{
    layout_view(view, 12, 18, width, height);
    hcw_dock_panel_layout_childs(view);
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

static void test_dock_panel_style_helpers_and_setters_clear_pressed_state(void)
{
    setup_test_panel();

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_dock_panel)->background == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_dock_panel.last_child_fill);
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_dock_panel.content_inset);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_dock_panel), 1);
    hcw_dock_panel_apply_compact_style(EGUI_VIEW_OF(&test_dock_panel));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_dock_panel)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_dock_panel.content_inset);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_dock_panel), 1);
    hcw_dock_panel_set_last_child_fill(EGUI_VIEW_OF(&test_dock_panel), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_dock_panel)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_dock_panel.last_child_fill);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_items[0]), 1);
    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&test_items[0]), HCW_DOCK_PANEL_DOCK_TOP);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_items[0])->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_DOCK_PANEL_DOCK_TOP, EGUI_VIEW_OF(&test_items[0])->margin.left);
    EGUI_TEST_ASSERT_EQUAL_INT(48, EGUI_VIEW_OF(&test_items[0])->margin.right);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_items[0])->margin.top);
}

static void test_dock_panel_layout_childs_places_top_left_right_and_fill(void)
{
    setup_test_panel();

    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&test_items[0]), HCW_DOCK_PANEL_DOCK_TOP);
    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&test_items[1]), HCW_DOCK_PANEL_DOCK_LEFT);
    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&test_items[2]), HCW_DOCK_PANEL_DOCK_RIGHT);
    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&test_items[3]), HCW_DOCK_PANEL_DOCK_FILL);
    layout_panel(EGUI_VIEW_OF(&test_dock_panel), 120, 80);

    EGUI_TEST_ASSERT_EQUAL_INT(4, EGUI_VIEW_OF(&test_items[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(4, EGUI_VIEW_OF(&test_items[0])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(112, EGUI_VIEW_OF(&test_items[0])->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_items[0])->region.size.height);

    EGUI_TEST_ASSERT_EQUAL_INT(4, EGUI_VIEW_OF(&test_items[1])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(16, EGUI_VIEW_OF(&test_items[1])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(18, EGUI_VIEW_OF(&test_items[1])->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(60, EGUI_VIEW_OF(&test_items[1])->region.size.height);

    EGUI_TEST_ASSERT_EQUAL_INT(96, EGUI_VIEW_OF(&test_items[2])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(16, EGUI_VIEW_OF(&test_items[2])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(20, EGUI_VIEW_OF(&test_items[2])->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(60, EGUI_VIEW_OF(&test_items[2])->region.size.height);

    EGUI_TEST_ASSERT_EQUAL_INT(22, EGUI_VIEW_OF(&test_items[3])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(16, EGUI_VIEW_OF(&test_items[3])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(74, EGUI_VIEW_OF(&test_items[3])->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(60, EGUI_VIEW_OF(&test_items[3])->region.size.height);
}

static void test_dock_panel_layout_childs_places_bottom_without_last_fill(void)
{
    setup_test_panel();

    hcw_dock_panel_apply_compact_style(EGUI_VIEW_OF(&test_dock_panel));
    hcw_dock_panel_set_last_child_fill(EGUI_VIEW_OF(&test_dock_panel), 0);

    egui_view_set_size(EGUI_VIEW_OF(&test_items[0]), 40, 10);
    egui_view_set_size(EGUI_VIEW_OF(&test_items[1]), 44, 8);
    egui_view_set_size(EGUI_VIEW_OF(&test_items[2]), 36, 18);
    egui_view_set_gone(EGUI_VIEW_OF(&test_items[3]), 1);

    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&test_items[0]), HCW_DOCK_PANEL_DOCK_TOP);
    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&test_items[1]), HCW_DOCK_PANEL_DOCK_BOTTOM);
    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&test_items[2]), HCW_DOCK_PANEL_DOCK_FILL);
    layout_panel(EGUI_VIEW_OF(&test_dock_panel), 100, 60);

    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_items[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_items[0])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(96, EGUI_VIEW_OF(&test_items[0])->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(10, EGUI_VIEW_OF(&test_items[0])->region.size.height);

    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_items[1])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(50, EGUI_VIEW_OF(&test_items[1])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(96, EGUI_VIEW_OF(&test_items[1])->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(8, EGUI_VIEW_OF(&test_items[1])->region.size.height);

    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_items[2])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_items[2])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(96, EGUI_VIEW_OF(&test_items[2])->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(38, EGUI_VIEW_OF(&test_items[2])->region.size.height);
}

static void test_dock_panel_static_preview_consumes_input(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_panel();

    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&preview_items[0]), HCW_DOCK_PANEL_DOCK_TOP);
    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&preview_items[1]), HCW_DOCK_PANEL_DOCK_LEFT);
    hcw_dock_panel_set_child_dock(EGUI_VIEW_OF(&preview_items[2]), HCW_DOCK_PANEL_DOCK_FILL);
    layout_panel(EGUI_VIEW_OF(&preview_dock_panel), 84, 40);
    get_view_center(EGUI_VIEW_OF(&preview_dock_panel), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_dock_panel), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_dock_panel), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_dock_panel), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_dock_panel)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_dock_panel), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_dock_panel), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_dock_panel), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_dock_panel)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_dock_panel_run(void)
{
    EGUI_TEST_SUITE_BEGIN(dock_panel);
    EGUI_TEST_RUN(test_dock_panel_style_helpers_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_dock_panel_layout_childs_places_top_left_right_and_fill);
    EGUI_TEST_RUN(test_dock_panel_layout_childs_places_bottom_without_last_fill);
    EGUI_TEST_RUN(test_dock_panel_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
