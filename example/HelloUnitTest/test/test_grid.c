#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_grid.h"

#include "../../HelloCustomWidgets/layout/grid/egui_view_grid.h"
#include "../../HelloCustomWidgets/layout/grid/egui_view_grid.c"

static egui_view_gridlayout_t test_grid;
static egui_view_gridlayout_t preview_grid;
static egui_view_label_t test_cells[4];
static egui_view_label_t preview_cells[4];
static egui_view_api_t preview_api;
static uint8_t g_click_count;

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_click_count++;
}

static void init_cell(egui_view_label_t *cell, const char *text)
{
    egui_view_label_init(EGUI_VIEW_OF(cell));
    egui_view_set_size(EGUI_VIEW_OF(cell), 40, 20);
    egui_view_label_set_text(EGUI_VIEW_OF(cell), text);
}

static void setup_grid(egui_view_gridlayout_t *grid, egui_view_label_t *cells, uint8_t cell_count, egui_dim_t width, egui_dim_t height)
{
    uint8_t index;

    egui_view_gridlayout_init(EGUI_VIEW_OF(grid));
    egui_view_set_size(EGUI_VIEW_OF(grid), width, height);
    hcw_grid_apply_standard_style(EGUI_VIEW_OF(grid));

    for (index = 0; index < cell_count; index++)
    {
        init_cell(&cells[index], "Cell");
        egui_view_group_add_child(EGUI_VIEW_OF(grid), EGUI_VIEW_OF(&cells[index]));
    }
}

static void setup_test_grid(void)
{
    setup_grid(&test_grid, test_cells, EGUI_ARRAY_SIZE(test_cells), 120, 80);
    g_click_count = 0;
}

static void setup_preview_grid(void)
{
    setup_grid(&preview_grid, preview_cells, EGUI_ARRAY_SIZE(preview_cells), 84, 60);
    hcw_grid_apply_dense_style(EGUI_VIEW_OF(&preview_grid));
    hcw_grid_set_align_type(EGUI_VIEW_OF(&preview_grid), EGUI_ALIGN_LEFT);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_grid), on_preview_click);
    hcw_grid_override_static_preview_api(EGUI_VIEW_OF(&preview_grid), &preview_api);
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

static void layout_grid(egui_view_t *view, egui_dim_t width, egui_dim_t height)
{
    layout_view(view, 12, 18, width, height);
    hcw_grid_layout_childs(view);
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
    EGUI_TEST_ASSERT_EQUAL_INT(3, preview_grid.col_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_LEFT, preview_grid.align_type);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_grid)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
    assert_regions_equal(preview_cells, EGUI_ARRAY_SIZE(preview_cells), regions);
}

static void test_grid_style_helpers_and_setters_clear_pressed_state(void)
{
    setup_test_grid();

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_grid)->background == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_grid.col_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_HCENTER, test_grid.align_type);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_grid), 1);
    hcw_grid_apply_dense_style(EGUI_VIEW_OF(&test_grid));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_grid)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_grid.col_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_grid), 1);
    hcw_grid_apply_stack_style(EGUI_VIEW_OF(&test_grid));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_grid)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_grid.col_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_grid), 1);
    hcw_grid_set_columns(EGUI_VIEW_OF(&test_grid), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_grid)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_grid.col_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_grid), 1);
    hcw_grid_set_columns(EGUI_VIEW_OF(&test_grid), 9);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_grid)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_grid.col_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_grid), 1);
    hcw_grid_set_align_type(EGUI_VIEW_OF(&test_grid), EGUI_ALIGN_CENTER);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_grid)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_CENTER, test_grid.align_type);
}

static void test_grid_layout_childs_places_cells_for_standard_dense_and_stack_modes(void)
{
    setup_test_grid();

    layout_grid(EGUI_VIEW_OF(&test_grid), 120, 80);
    EGUI_TEST_ASSERT_EQUAL_INT(10, EGUI_VIEW_OF(&test_cells[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_cells[0])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(70, EGUI_VIEW_OF(&test_cells[1])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_cells[1])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(10, EGUI_VIEW_OF(&test_cells[2])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(20, EGUI_VIEW_OF(&test_cells[2])->region.location.y);

    hcw_grid_apply_dense_style(EGUI_VIEW_OF(&test_grid));
    layout_grid(EGUI_VIEW_OF(&test_grid), 120, 80);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&test_cells[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(40, EGUI_VIEW_OF(&test_cells[1])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(80, EGUI_VIEW_OF(&test_cells[2])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(20, EGUI_VIEW_OF(&test_cells[3])->region.location.y);

    hcw_grid_apply_stack_style(EGUI_VIEW_OF(&test_grid));
    layout_grid(EGUI_VIEW_OF(&test_grid), 120, 80);
    EGUI_TEST_ASSERT_EQUAL_INT(40, EGUI_VIEW_OF(&test_cells[0])->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(20, EGUI_VIEW_OF(&test_cells[1])->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(60, EGUI_VIEW_OF(&test_cells[3])->region.location.y);
}

static void test_grid_layout_childs_clears_pressed_state(void)
{
    setup_test_grid();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_grid), 1);
    layout_grid(EGUI_VIEW_OF(&test_grid), 120, 80);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_grid)->is_pressed);
}

static void test_grid_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;
    egui_region_t initial_regions[EGUI_ARRAY_SIZE(preview_cells)];

    setup_preview_grid();
    layout_grid(EGUI_VIEW_OF(&preview_grid), 84, 60);
    capture_regions(preview_cells, EGUI_ARRAY_SIZE(preview_cells), initial_regions);
    get_view_center(EGUI_VIEW_OF(&preview_grid), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_grid), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_grid), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(initial_regions);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_grid), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_grid), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    assert_preview_state_unchanged(initial_regions);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_grid), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_grid), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(initial_regions);
}

void test_grid_run(void)
{
    EGUI_TEST_SUITE_BEGIN(grid);
    EGUI_TEST_RUN(test_grid_style_helpers_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_grid_layout_childs_places_cells_for_standard_dense_and_stack_modes);
    EGUI_TEST_RUN(test_grid_layout_childs_clears_pressed_state);
    EGUI_TEST_RUN(test_grid_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
