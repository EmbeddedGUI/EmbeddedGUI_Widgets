#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_grid_view.h"

#include "../../HelloCustomWidgets/layout/grid_view/egui_view_grid_view.h"
#include "../../HelloCustomWidgets/layout/grid_view/egui_view_grid_view.c"

static hcw_grid_view_t grid_view_test_widget;
static hcw_grid_view_t grid_view_preview_widget;
static egui_view_api_t grid_view_preview_api;
static uint8_t grid_view_action_count;
static uint8_t grid_view_action_snapshot;
static uint8_t grid_view_action_item;

static const hcw_grid_view_item_t grid_view_items_0[] = {
        {"AST", "Hero", "1", HCW_GRID_VIEW_TONE_ACCENT, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"REL", "Ship", "2", HCW_GRID_VIEW_TONE_SUCCESS, 1, HCW_GRID_VIEW_WIDTH_PROMINENT},
        {"QA", "Test", "3", HCW_GRID_VIEW_TONE_WARNING, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"DOC", "Read", "4", HCW_GRID_VIEW_TONE_NEUTRAL, 1, HCW_GRID_VIEW_WIDTH_PROMINENT},
        {"NAV", "Wire", "5", HCW_GRID_VIEW_TONE_ACCENT, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"OBS", "Live", "6", HCW_GRID_VIEW_TONE_SUCCESS, 1, HCW_GRID_VIEW_WIDTH_PROMINENT},
};

static const hcw_grid_view_item_t grid_view_items_1[] = {
        {"A", "Tags", "1", HCW_GRID_VIEW_TONE_NEUTRAL, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"B", "Flow", "2", HCW_GRID_VIEW_TONE_ACCENT, 1, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"C", "Rows", "3", HCW_GRID_VIEW_TONE_WARNING, 1, HCW_GRID_VIEW_WIDTH_PROMINENT},
        {"D", "Wrap", "4", HCW_GRID_VIEW_TONE_SUCCESS, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
};

static const hcw_grid_view_snapshot_t grid_view_snapshots[] = {
        {"AST", "Assets gallery", "", "", grid_view_items_0, 6, 1, HCW_GRID_VIEW_LAYOUT_WRAP},
        {"TEAM", "Team board", "", "", grid_view_items_1, 4, 2, HCW_GRID_VIEW_LAYOUT_STRIP},
};

static const hcw_grid_view_item_t grid_view_overflow_items[] = {
        {"A", "One", "1", HCW_GRID_VIEW_TONE_ACCENT, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"B", "Two", "2", HCW_GRID_VIEW_TONE_SUCCESS, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"C", "Tri", "3", HCW_GRID_VIEW_TONE_WARNING, 0, HCW_GRID_VIEW_WIDTH_PROMINENT},
        {"D", "For", "4", HCW_GRID_VIEW_TONE_NEUTRAL, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"E", "Fiv", "5", HCW_GRID_VIEW_TONE_ACCENT, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"F", "Six", "6", HCW_GRID_VIEW_TONE_SUCCESS, 0, HCW_GRID_VIEW_WIDTH_PROMINENT},
        {"G", "Sev", "7", HCW_GRID_VIEW_TONE_WARNING, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"H", "Egt", "8", HCW_GRID_VIEW_TONE_NEUTRAL, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"I", "Nin", "9", HCW_GRID_VIEW_TONE_ACCENT, 0, HCW_GRID_VIEW_WIDTH_PROMINENT},
};

static const hcw_grid_view_snapshot_t grid_view_overflow_snapshots[] = {
        {"A", "A", "A", "A", grid_view_overflow_items, 9, 7, HCW_GRID_VIEW_LAYOUT_STACK},
        {"B", "B", "B", "B", grid_view_items_0, 6, 1, HCW_GRID_VIEW_LAYOUT_WRAP},
        {"C", "C", "C", "C", grid_view_items_0, 6, 1, HCW_GRID_VIEW_LAYOUT_WRAP},
        {"D", "D", "D", "D", grid_view_items_0, 6, 1, HCW_GRID_VIEW_LAYOUT_WRAP},
        {"E", "E", "E", "E", grid_view_items_0, 6, 1, HCW_GRID_VIEW_LAYOUT_WRAP},
        {"F", "F", "F", "F", grid_view_items_0, 6, 1, HCW_GRID_VIEW_LAYOUT_WRAP},
        {"G", "G", "G", "G", grid_view_items_0, 6, 1, HCW_GRID_VIEW_LAYOUT_WRAP},
};

static const hcw_grid_view_item_t grid_view_preview_items[] = {
        {"P1", "Ship", "2", HCW_GRID_VIEW_TONE_ACCENT, 1, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"P2", "QA", "5", HCW_GRID_VIEW_TONE_WARNING, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
        {"P3", "Docs", "3", HCW_GRID_VIEW_TONE_NEUTRAL, 0, HCW_GRID_VIEW_WIDTH_BALANCED},
        {"P4", "Live", "1", HCW_GRID_VIEW_TONE_SUCCESS, 0, HCW_GRID_VIEW_WIDTH_COMPACT},
};

static const hcw_grid_view_snapshot_t grid_view_preview_snapshot = {
        "AST", "Compact grid", "", "", grid_view_preview_items, 4, 0, HCW_GRID_VIEW_LAYOUT_STRIP};

static void grid_view_on_action(egui_view_t *self, uint8_t snapshot_index, uint8_t item_index)
{
    EGUI_UNUSED(self);
    grid_view_action_count++;
    grid_view_action_snapshot = snapshot_index;
    grid_view_action_item = item_index;
}

static void grid_view_reset_action_state(void)
{
    grid_view_action_count = 0;
    grid_view_action_snapshot = 0xFF;
    grid_view_action_item = HCW_GRID_VIEW_ITEM_NONE;
}

static void grid_view_setup_widget(const hcw_grid_view_snapshot_t *snapshots, uint8_t snapshot_count)
{
    hcw_grid_view_init(EGUI_VIEW_OF(&grid_view_test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&grid_view_test_widget), 196, 148);
    hcw_grid_view_set_snapshots(EGUI_VIEW_OF(&grid_view_test_widget), snapshots, snapshot_count);
    hcw_grid_view_set_font(EGUI_VIEW_OF(&grid_view_test_widget), NULL);
    hcw_grid_view_set_meta_font(EGUI_VIEW_OF(&grid_view_test_widget), NULL);
    hcw_grid_view_set_on_action_listener(EGUI_VIEW_OF(&grid_view_test_widget), grid_view_on_action);
    grid_view_reset_action_state();
}

static void grid_view_setup_preview_widget(void)
{
    hcw_grid_view_init(EGUI_VIEW_OF(&grid_view_preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&grid_view_preview_widget), 104, 86);
    hcw_grid_view_set_snapshots(EGUI_VIEW_OF(&grid_view_preview_widget), &grid_view_preview_snapshot, 1);
    hcw_grid_view_set_font(EGUI_VIEW_OF(&grid_view_preview_widget), NULL);
    hcw_grid_view_set_meta_font(EGUI_VIEW_OF(&grid_view_preview_widget), NULL);
    hcw_grid_view_set_compact_mode(EGUI_VIEW_OF(&grid_view_preview_widget), 1);
    hcw_grid_view_override_static_preview_api(EGUI_VIEW_OF(&grid_view_preview_widget), &grid_view_preview_api);
    grid_view_reset_action_state();
}

static void grid_view_layout_view(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void grid_view_layout_widget(void)
{
    grid_view_layout_view(EGUI_VIEW_OF(&grid_view_test_widget), 10, 20, 196, 148);
}

static void grid_view_layout_preview_widget(void)
{
    grid_view_layout_view(EGUI_VIEW_OF(&grid_view_preview_widget), 12, 18, 104, 86);
}

static int grid_view_send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int grid_view_send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static int grid_view_send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return grid_view_send_touch_to_view(EGUI_VIEW_OF(&grid_view_test_widget), type, x, y);
}

static int grid_view_send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return grid_view_send_touch_to_view(EGUI_VIEW_OF(&grid_view_preview_widget), type, x, y);
}

static int grid_view_send_key_action(uint8_t type, uint8_t key_code)
{
    return grid_view_send_key_to_view(EGUI_VIEW_OF(&grid_view_test_widget), type, key_code);
}

static int grid_view_send_preview_key_action(uint8_t type, uint8_t key_code)
{
    return grid_view_send_key_to_view(EGUI_VIEW_OF(&grid_view_preview_widget), type, key_code);
}

static int grid_view_send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= grid_view_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= grid_view_send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int grid_view_send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= grid_view_send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= grid_view_send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t grid_view_get_item_center(egui_view_t *view, uint8_t item_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!hcw_grid_view_get_item_region(view, item_index, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void grid_view_get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 4;
    *y = view->region_screen.location.y - 4;
}

static void grid_view_seed_pressed_state(hcw_grid_view_t *widget, uint8_t item_index, uint8_t visual_pressed)
{
    widget->pressed_item = item_index;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void grid_view_assert_pressed_cleared(hcw_grid_view_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_GRID_VIEW_ITEM_NONE, widget->pressed_item);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_grid_view_set_snapshots_clamp_and_defaults(void)
{
    grid_view_setup_widget(grid_view_overflow_snapshots, EGUI_ARRAY_SIZE(grid_view_overflow_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(HCW_GRID_VIEW_MAX_SNAPSHOTS, grid_view_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_grid_view_get_current_snapshot(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(7, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_GRID_VIEW_LAYOUT_STACK, hcw_grid_view_get_layout_mode(EGUI_VIEW_OF(&grid_view_test_widget)));

    grid_view_seed_pressed_state(&grid_view_test_widget, 3, 1);
    grid_view_test_widget.current_snapshot = 5;
    hcw_grid_view_set_snapshots(EGUI_VIEW_OF(&grid_view_test_widget), grid_view_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, grid_view_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_grid_view_get_current_snapshot(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_GRID_VIEW_LAYOUT_WRAP, hcw_grid_view_get_layout_mode(EGUI_VIEW_OF(&grid_view_test_widget)));
    grid_view_assert_pressed_cleared(&grid_view_test_widget);

    hcw_grid_view_set_current_item(EGUI_VIEW_OF(&grid_view_test_widget), HCW_GRID_VIEW_ITEM_NONE);
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));

    hcw_grid_view_set_snapshots(EGUI_VIEW_OF(&grid_view_test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, grid_view_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_grid_view_get_current_snapshot(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_GRID_VIEW_ITEM_NONE, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_GRID_VIEW_LAYOUT_WRAP, hcw_grid_view_get_layout_mode(EGUI_VIEW_OF(&grid_view_test_widget)));
}

static void test_grid_view_setters_clear_pressed_and_update_state(void)
{
    grid_view_setup_widget(grid_view_snapshots, EGUI_ARRAY_SIZE(grid_view_snapshots));

    grid_view_seed_pressed_state(&grid_view_test_widget, 1, 1);
    hcw_grid_view_set_font(EGUI_VIEW_OF(&grid_view_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(grid_view_test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    grid_view_assert_pressed_cleared(&grid_view_test_widget);

    grid_view_seed_pressed_state(&grid_view_test_widget, 1, 1);
    hcw_grid_view_set_meta_font(EGUI_VIEW_OF(&grid_view_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(grid_view_test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    grid_view_assert_pressed_cleared(&grid_view_test_widget);

    grid_view_seed_pressed_state(&grid_view_test_widget, 1, 1);
    hcw_grid_view_set_compact_mode(EGUI_VIEW_OF(&grid_view_test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, grid_view_test_widget.compact_mode);
    grid_view_assert_pressed_cleared(&grid_view_test_widget);
    hcw_grid_view_set_compact_mode(EGUI_VIEW_OF(&grid_view_test_widget), 0);

    grid_view_seed_pressed_state(&grid_view_test_widget, 1, 1);
    hcw_grid_view_set_read_only_mode(EGUI_VIEW_OF(&grid_view_test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, grid_view_test_widget.read_only_mode);
    grid_view_assert_pressed_cleared(&grid_view_test_widget);
    hcw_grid_view_set_read_only_mode(EGUI_VIEW_OF(&grid_view_test_widget), 0);

    grid_view_seed_pressed_state(&grid_view_test_widget, 1, 1);
    hcw_grid_view_set_layout_mode(EGUI_VIEW_OF(&grid_view_test_widget), HCW_GRID_VIEW_LAYOUT_STACK);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_GRID_VIEW_LAYOUT_STACK, hcw_grid_view_get_layout_mode(EGUI_VIEW_OF(&grid_view_test_widget)));
    grid_view_assert_pressed_cleared(&grid_view_test_widget);

    grid_view_seed_pressed_state(&grid_view_test_widget, 1, 1);
    hcw_grid_view_set_palette(EGUI_VIEW_OF(&grid_view_test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                              EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                              EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, grid_view_test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, grid_view_test_widget.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, grid_view_test_widget.border_color.full);
    grid_view_assert_pressed_cleared(&grid_view_test_widget);

    hcw_grid_view_set_current_snapshot(EGUI_VIEW_OF(&grid_view_test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_grid_view_get_current_snapshot(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_GRID_VIEW_LAYOUT_STRIP, hcw_grid_view_get_layout_mode(EGUI_VIEW_OF(&grid_view_test_widget)));

    grid_view_seed_pressed_state(&grid_view_test_widget, 2, 1);
    hcw_grid_view_set_current_item(EGUI_VIEW_OF(&grid_view_test_widget), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    grid_view_assert_pressed_cleared(&grid_view_test_widget);
}

static void test_grid_view_regions_activate_and_listener(void)
{
    egui_dim_t x;
    egui_dim_t y;

    grid_view_setup_widget(grid_view_snapshots, EGUI_ARRAY_SIZE(grid_view_snapshots));
    grid_view_layout_widget();

    EGUI_TEST_ASSERT_TRUE(grid_view_get_item_center(EGUI_VIEW_OF(&grid_view_test_widget), 2, &x, &y));
    hcw_grid_view_set_current_item(EGUI_VIEW_OF(&grid_view_test_widget), 2);
    EGUI_TEST_ASSERT_TRUE(hcw_grid_view_activate_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, grid_view_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, grid_view_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, grid_view_action_item);

    grid_view_reset_action_state();
    hcw_grid_view_set_on_action_listener(EGUI_VIEW_OF(&grid_view_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(hcw_grid_view_activate_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, grid_view_action_count);
}

static void test_grid_view_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    grid_view_setup_widget(grid_view_snapshots, EGUI_ARRAY_SIZE(grid_view_snapshots));
    grid_view_layout_widget();
    EGUI_TEST_ASSERT_TRUE(grid_view_get_item_center(EGUI_VIEW_OF(&grid_view_test_widget), 0, &item_x, &item_y));
    grid_view_get_view_outside_point(EGUI_VIEW_OF(&grid_view_test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, grid_view_test_widget.pressed_item);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&grid_view_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&grid_view_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, grid_view_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    grid_view_assert_pressed_cleared(&grid_view_test_widget);

    EGUI_TEST_ASSERT_TRUE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&grid_view_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_UP, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, grid_view_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    grid_view_assert_pressed_cleared(&grid_view_test_widget);

    grid_view_reset_action_state();
    EGUI_TEST_ASSERT_TRUE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, grid_view_action_count);
    grid_view_assert_pressed_cleared(&grid_view_test_widget);
}

static void test_grid_view_key_navigation_activate_and_tab(void)
{
    grid_view_setup_widget(grid_view_snapshots, EGUI_ARRAY_SIZE(grid_view_snapshots));
    grid_view_layout_widget();

    EGUI_TEST_ASSERT_TRUE(grid_view_send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));

    hcw_grid_view_set_current_item(EGUI_VIEW_OF(&grid_view_test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(grid_view_send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(3, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));

    EGUI_TEST_ASSERT_TRUE(grid_view_send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(5, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_TRUE(grid_view_send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(3, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_TRUE(grid_view_send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(4, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));

    hcw_grid_view_set_current_item(EGUI_VIEW_OF(&grid_view_test_widget), 5);
    EGUI_TEST_ASSERT_TRUE(grid_view_send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_grid_view_get_current_snapshot(EGUI_VIEW_OF(&grid_view_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_test_widget)));

    EGUI_TEST_ASSERT_TRUE(grid_view_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(2, grid_view_test_widget.pressed_item);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&grid_view_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(grid_view_send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, grid_view_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, grid_view_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, grid_view_action_item);
    grid_view_assert_pressed_cleared(&grid_view_test_widget);
}

static void test_grid_view_read_only_and_disabled_guards(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;

    grid_view_setup_widget(grid_view_snapshots, EGUI_ARRAY_SIZE(grid_view_snapshots));
    grid_view_layout_widget();
    EGUI_TEST_ASSERT_TRUE(grid_view_get_item_center(EGUI_VIEW_OF(&grid_view_test_widget), 1, &item_x, &item_y));

    grid_view_seed_pressed_state(&grid_view_test_widget, 1, 1);
    hcw_grid_view_set_read_only_mode(EGUI_VIEW_OF(&grid_view_test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_FALSE(grid_view_send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(hcw_grid_view_handle_navigation_key(EGUI_VIEW_OF(&grid_view_test_widget), EGUI_KEY_CODE_LEFT));
    grid_view_assert_pressed_cleared(&grid_view_test_widget);
    hcw_grid_view_set_read_only_mode(EGUI_VIEW_OF(&grid_view_test_widget), 0);

    grid_view_seed_pressed_state(&grid_view_test_widget, 1, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&grid_view_test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(grid_view_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_FALSE(grid_view_send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(hcw_grid_view_handle_navigation_key(EGUI_VIEW_OF(&grid_view_test_widget), EGUI_KEY_CODE_LEFT));
    grid_view_assert_pressed_cleared(&grid_view_test_widget);
    egui_view_set_enable(EGUI_VIEW_OF(&grid_view_test_widget), 1);
}

static void test_grid_view_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;

    grid_view_setup_preview_widget();
    grid_view_layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(grid_view_get_item_center(EGUI_VIEW_OF(&grid_view_preview_widget), 0, &item_x, &item_y));

    grid_view_seed_pressed_state(&grid_view_preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(grid_view_send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    grid_view_assert_pressed_cleared(&grid_view_preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, grid_view_action_count);

    grid_view_seed_pressed_state(&grid_view_preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(grid_view_send_preview_key(EGUI_KEY_CODE_ENTER));
    grid_view_assert_pressed_cleared(&grid_view_preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, grid_view_action_count);

    grid_view_seed_pressed_state(&grid_view_preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(hcw_grid_view_handle_navigation_key(EGUI_VIEW_OF(&grid_view_preview_widget), EGUI_KEY_CODE_ENTER));
    grid_view_assert_pressed_cleared(&grid_view_preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_grid_view_get_current_item(EGUI_VIEW_OF(&grid_view_preview_widget)));
}

void test_grid_view_run(void)
{
    EGUI_TEST_SUITE_BEGIN(grid_view);
    EGUI_TEST_RUN(test_grid_view_set_snapshots_clamp_and_defaults);
    EGUI_TEST_RUN(test_grid_view_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_grid_view_regions_activate_and_listener);
    EGUI_TEST_RUN(test_grid_view_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_grid_view_key_navigation_activate_and_tab);
    EGUI_TEST_RUN(test_grid_view_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_grid_view_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
