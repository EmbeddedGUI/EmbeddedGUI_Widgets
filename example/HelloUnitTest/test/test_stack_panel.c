#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_stack_panel.h"

#include "../../HelloCustomWidgets/layout/stack_panel/egui_view_stack_panel.h"
#include "../../HelloCustomWidgets/layout/stack_panel/egui_view_stack_panel.c"

static egui_view_linearlayout_t test_stack_panel;
static egui_view_linearlayout_t preview_stack_panel;
static egui_view_label_t test_cells[4];
static egui_view_label_t preview_cells[2];
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void init_cell(egui_view_label_t *cell, egui_dim_t width, egui_dim_t height, const char *text)
{
    egui_view_label_init(EGUI_VIEW_OF(cell));
    egui_view_set_size(EGUI_VIEW_OF(cell), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(cell), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(cell), EGUI_ALIGN_CENTER);
}

static void setup_test_stack_panel(void)
{
    uint8_t index;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&test_stack_panel));
    egui_view_set_size(EGUI_VIEW_OF(&test_stack_panel), 120, 64);
    hcw_stack_panel_apply_standard_style(EGUI_VIEW_OF(&test_stack_panel));

    for (index = 0; index < EGUI_ARRAY_SIZE(test_cells); index++)
    {
        init_cell(&test_cells[index], 40, 12, "Cell");
        egui_view_group_add_child(EGUI_VIEW_OF(&test_stack_panel), EGUI_VIEW_OF(&test_cells[index]));
    }

    g_click_count = 0;
}

static void setup_preview_stack_panel(void)
{
    uint8_t index;

    egui_view_linearlayout_init(EGUI_VIEW_OF(&preview_stack_panel));
    egui_view_set_size(EGUI_VIEW_OF(&preview_stack_panel), 84, 36);
    hcw_stack_panel_apply_horizontal_style(EGUI_VIEW_OF(&preview_stack_panel));

    for (index = 0; index < EGUI_ARRAY_SIZE(preview_cells); index++)
    {
        init_cell(&preview_cells[index], 32, 16, "P");
        egui_view_group_add_child(EGUI_VIEW_OF(&preview_stack_panel), EGUI_VIEW_OF(&preview_cells[index]));
    }

    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_stack_panel), on_preview_click);
    hcw_stack_panel_override_static_preview_api(EGUI_VIEW_OF(&preview_stack_panel), &preview_api);
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

static void layout_stack_panel(egui_view_t *view, egui_dim_t width, egui_dim_t height)
{
    layout_view(view, 12, 18, width, height);
    hcw_stack_panel_layout_childs(view);
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

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key(egui_view_t *view, uint8_t key_code)
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

static void capture_regions(egui_view_label_t *cells, uint8_t count, egui_region_t *regions)
{
    uint8_t index;

    for (index = 0; index < count; index++)
    {
        egui_region_copy(&regions[index], &EGUI_VIEW_OF(&cells[index])->region);
    }
}

static void assert_regions_equal(egui_view_label_t *cells, uint8_t count, const egui_region_t *regions)
{
    uint8_t index;

    for (index = 0; index < count; index++)
    {
        EGUI_TEST_ASSERT_EQUAL_INT(regions[index].location.x, EGUI_VIEW_OF(&cells[index])->region.location.x);
        EGUI_TEST_ASSERT_EQUAL_INT(regions[index].location.y, EGUI_VIEW_OF(&cells[index])->region.location.y);
        EGUI_TEST_ASSERT_EQUAL_INT(regions[index].size.width, EGUI_VIEW_OF(&cells[index])->region.size.width);
        EGUI_TEST_ASSERT_EQUAL_INT(regions[index].size.height, EGUI_VIEW_OF(&cells[index])->region.size.height);
    }
}

static void assert_preview_state_unchanged(const egui_region_t *regions)
{
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_stack_panel.is_orientation_horizontal);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_VCENTER, preview_stack_panel.align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_stack_panel.is_auto_width);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_stack_panel.is_auto_height);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_stack_panel)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
    assert_regions_equal(preview_cells, EGUI_ARRAY_SIZE(preview_cells), regions);
}

static void test_stack_panel_style_helpers_and_setters_clear_pressed_state(void)
{
    setup_test_stack_panel();

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_stack_panel)->background == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_stack_panel.is_orientation_horizontal);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_HCENTER, test_stack_panel.align_type);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_stack_panel), 1);
    hcw_stack_panel_apply_horizontal_style(EGUI_VIEW_OF(&test_stack_panel));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_stack_panel)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_stack_panel.is_orientation_horizontal);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_VCENTER, test_stack_panel.align_type);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_stack_panel), 1);
    hcw_stack_panel_apply_compact_style(EGUI_VIEW_OF(&test_stack_panel));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_stack_panel)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_stack_panel.is_orientation_horizontal);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_LEFT, test_stack_panel.align_type);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_stack_panel), 1);
    hcw_stack_panel_set_orientation(EGUI_VIEW_OF(&test_stack_panel), 9);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_stack_panel)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_stack_panel.is_orientation_horizontal);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_stack_panel), 1);
    hcw_stack_panel_set_align_type(EGUI_VIEW_OF(&test_stack_panel), EGUI_ALIGN_RIGHT);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_stack_panel)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_RIGHT, test_stack_panel.align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_stack_panel.is_auto_width);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_stack_panel.is_auto_height);
}

static void test_stack_panel_layout_childs_places_cells_for_standard_horizontal_and_compact_modes(void)
{
    setup_test_stack_panel();

    layout_stack_panel(EGUI_VIEW_OF(&test_stack_panel), 120, 64);
    EGUI_TEST_ASSERT_EQUAL_INT(40, EGUI_VIEW_OF(&test_cells[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_cells[0])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_cells[1])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(36, EGUI_VIEW_OF(&test_cells[3])->region.location.y);

    hcw_stack_panel_apply_horizontal_style(EGUI_VIEW_OF(&test_stack_panel));
    layout_stack_panel(EGUI_VIEW_OF(&test_stack_panel), 176, 32);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_cells[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(10, EGUI_VIEW_OF(&test_cells[0])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(40, EGUI_VIEW_OF(&test_cells[1])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(120, EGUI_VIEW_OF(&test_cells[3])->region.location.x);

    hcw_stack_panel_apply_compact_style(EGUI_VIEW_OF(&test_stack_panel));
    layout_stack_panel(EGUI_VIEW_OF(&test_stack_panel), 120, 64);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_cells[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_cells[0])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_cells[1])->region.location.y);
}

static void test_stack_panel_layout_childs_clears_pressed_state(void)
{
    setup_test_stack_panel();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_stack_panel), 1);
    layout_stack_panel(EGUI_VIEW_OF(&test_stack_panel), 120, 64);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_stack_panel)->is_pressed);
}

static void test_stack_panel_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;
    egui_region_t initial_regions[EGUI_ARRAY_SIZE(preview_cells)];

    setup_preview_stack_panel();
    layout_stack_panel(EGUI_VIEW_OF(&preview_stack_panel), 84, 36);
    capture_regions(preview_cells, EGUI_ARRAY_SIZE(preview_cells), initial_regions);
    get_view_center(EGUI_VIEW_OF(&preview_stack_panel), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_stack_panel), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_stack_panel), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(initial_regions);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_stack_panel), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_stack_panel), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    assert_preview_state_unchanged(initial_regions);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_stack_panel), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_stack_panel), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(initial_regions);
}

void test_stack_panel_run(void)
{
    EGUI_TEST_SUITE_BEGIN(stack_panel);
    EGUI_TEST_RUN(test_stack_panel_style_helpers_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_stack_panel_layout_childs_places_cells_for_standard_horizontal_and_compact_modes);
    EGUI_TEST_RUN(test_stack_panel_layout_childs_clears_pressed_state);
    EGUI_TEST_RUN(test_stack_panel_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
