#include "egui.h"
#include "test/egui_test.h"
#include "test_demo_scaffold.h"

#include "../../HelloCustomWidgets/demo_scaffold.h"
#include "../../HelloCustomWidgets/demo_scaffold.c"

static egui_view_linearlayout_t root_layout;
static egui_view_label_t title_label;
static egui_view_label_t main_view;
static egui_view_linearlayout_t preview_row;
static egui_view_label_t preview_left;
static egui_view_label_t preview_right;
static egui_view_label_t chrome_view;

static void setup_title_only_scaffold(void)
{
    egui_view_linearlayout_init(EGUI_VIEW_OF(&root_layout));
    egui_view_set_size(EGUI_VIEW_OF(&root_layout), 96, 80);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&root_layout), 0);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&root_layout), EGUI_ALIGN_HCENTER);

    egui_view_label_init(EGUI_VIEW_OF(&title_label));
    egui_view_set_size(EGUI_VIEW_OF(&title_label), 96, 18);
    egui_view_set_margin(EGUI_VIEW_OF(&title_label), 4, 6, 2, 4);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label));

    egui_view_label_init(EGUI_VIEW_OF(&main_view));
    egui_view_set_size(EGUI_VIEW_OF(&main_view), 90, 40);
    egui_view_set_margin(EGUI_VIEW_OF(&main_view), 0, 0, 0, 2);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&main_view));

    egui_view_linearlayout_init(EGUI_VIEW_OF(&preview_row));
    egui_view_set_size(EGUI_VIEW_OF(&preview_row), 80, 26);
    egui_view_linearlayout_set_orientation(EGUI_VIEW_OF(&preview_row), 1);
    egui_view_linearlayout_set_align_type(EGUI_VIEW_OF(&preview_row), EGUI_ALIGN_VCENTER);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&preview_row));

    egui_view_label_init(EGUI_VIEW_OF(&preview_left));
    egui_view_set_size(EGUI_VIEW_OF(&preview_left), 40, 20);
    egui_view_group_add_child(EGUI_VIEW_OF(&preview_row), EGUI_VIEW_OF(&preview_left));

    egui_view_label_init(EGUI_VIEW_OF(&preview_right));
    egui_view_set_size(EGUI_VIEW_OF(&preview_right), 44, 20);
    egui_view_set_margin(EGUI_VIEW_OF(&preview_right), 6, 0, 0, 0);
    egui_view_group_add_child(EGUI_VIEW_OF(&preview_row), EGUI_VIEW_OF(&preview_right));

    egui_view_label_init(EGUI_VIEW_OF(&chrome_view));
    egui_view_set_size(EGUI_VIEW_OF(&chrome_view), 20, 12);
    egui_view_group_add_child(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&chrome_view));
}

static void layout_title_only_scaffold(void)
{
    egui_region_t root_region;

    root_region.location.x = 0;
    root_region.location.y = 0;
    root_region.size.width = EGUI_VIEW_OF(&root_layout)->region.size.width;
    root_region.size.height = EGUI_VIEW_OF(&root_layout)->region.size.height;
    egui_view_layout(EGUI_VIEW_OF(&root_layout), &root_region);
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&root_layout));
    egui_view_linearlayout_layout_childs(EGUI_VIEW_OF(&preview_row));
}

static void test_demo_scaffold_title_only_enforces_spacing_and_expands_layout(void)
{
    egui_view_t *chrome_views[] = {
            EGUI_VIEW_OF(&chrome_view),
    };
    egui_dim_t expected_preview_row_width;
    egui_dim_t expected_root_width;
    egui_dim_t preview_gap;
    egui_dim_t main_gap;

    setup_title_only_scaffold();
    hello_custom_widgets_demo_apply_title_only_scaffold(EGUI_VIEW_OF(&root_layout), EGUI_VIEW_OF(&title_label), chrome_views, 1);

    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&chrome_view)->is_gone);
    EGUI_TEST_ASSERT_EQUAL_INT(14, EGUI_VIEW_OF(&main_view)->margin.bottom);
    EGUI_TEST_ASSERT_EQUAL_INT(20, EGUI_VIEW_OF(&preview_right)->margin.left);
    expected_preview_row_width = measure_linear_extent(EGUI_VIEW_OF(&preview_row), 1);
    expected_root_width = EGUI_MIN((egui_dim_t)HELLO_CUSTOM_WIDGETS_CANVAS_WIDTH, expected_preview_row_width + HELLO_CUSTOM_WIDGETS_TITLE_ONLY_SIDE_INSET * 2);
    EGUI_TEST_ASSERT_EQUAL_INT(expected_preview_row_width, EGUI_VIEW_OF(&preview_row)->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected_root_width, EGUI_VIEW_OF(&root_layout)->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(measure_linear_extent(EGUI_VIEW_OF(&root_layout), 0), EGUI_VIEW_OF(&root_layout)->region.size.height);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_OF(&root_layout)->region.size.width, EGUI_VIEW_OF(&title_label)->region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&title_label)->margin.left);
    EGUI_TEST_ASSERT_EQUAL_INT(0, EGUI_VIEW_OF(&title_label)->margin.right);

    layout_title_only_scaffold();

    preview_gap = EGUI_VIEW_OF(&preview_right)->region.location.x - (EGUI_VIEW_OF(&preview_left)->region.location.x + EGUI_VIEW_OF(&preview_left)->region.size.width);
    main_gap = EGUI_VIEW_OF(&preview_row)->region.location.y - (EGUI_VIEW_OF(&main_view)->region.location.y + EGUI_VIEW_OF(&main_view)->region.size.height);

    EGUI_TEST_ASSERT_EQUAL_INT(20, preview_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(14, main_gap);
}

static void test_demo_scaffold_triptych_rects_keep_main_and_preview_regions_separated(void)
{
    egui_region_t canvas_region;
    egui_region_t main_rect;
    egui_region_t left_rect;
    egui_region_t right_rect;

    canvas_region.location.x = 0;
    canvas_region.location.y = 0;
    canvas_region.size.width = HELLO_CUSTOM_WIDGETS_CANVAS_WIDTH;
    canvas_region.size.height = HELLO_CUSTOM_WIDGETS_CANVAS_HEIGHT;

    hello_custom_widgets_demo_get_triptych_rects(&canvas_region, HELLO_CUSTOM_WIDGETS_TITLE_TOP_INSET, &main_rect, &left_rect, &right_rect);

    EGUI_TEST_ASSERT_TRUE(main_rect.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(main_rect.size.height > 0);
    EGUI_TEST_ASSERT_TRUE(left_rect.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(left_rect.size.height > 0);
    EGUI_TEST_ASSERT_EQUAL_INT(left_rect.location.y, right_rect.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(left_rect.size.height, right_rect.size.height);
    EGUI_TEST_ASSERT_TRUE(main_rect.location.y >= HELLO_CUSTOM_WIDGETS_TITLE_TOP_INSET);
    EGUI_TEST_ASSERT_TRUE((main_rect.location.y + main_rect.size.height) <= left_rect.location.y - 20);
    EGUI_TEST_ASSERT_TRUE((left_rect.location.x + left_rect.size.width) <= right_rect.location.x - 24);
}

void test_demo_scaffold_run(void)
{
    EGUI_TEST_SUITE_BEGIN(demo_scaffold);
    EGUI_TEST_RUN(test_demo_scaffold_title_only_enforces_spacing_and_expands_layout);
    EGUI_TEST_RUN(test_demo_scaffold_triptych_rects_keep_main_and_preview_regions_separated);
    EGUI_TEST_SUITE_END();
}
