#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_data_grid.h"

#include "../../HelloCustomWidgets/layout/data_grid/egui_view_data_grid.h"
#include "../../HelloCustomWidgets/layout/data_grid/egui_view_data_grid.c"

static egui_view_data_grid_t test_widget;
static egui_view_data_grid_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_action_count;
static uint8_t g_action_snapshot;
static uint8_t g_action_row;

static const egui_view_data_grid_column_t g_columns[] = {
        {"CASE", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"OWNER", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"STATE", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"ETA", EGUI_VIEW_DATA_GRID_ALIGN_RIGHT},
};

static const egui_view_data_grid_row_t g_rows_0[] = {
        {{"AL-42", "Rina", "Ready", "Today"}, EGUI_VIEW_DATA_GRID_TONE_ACCENT, 1},
        {{"BK-14", "Omar", "Hold", "Tue"}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 0},
        {{"CK-07", "Lea", "Review", "Fri"}, EGUI_VIEW_DATA_GRID_TONE_NEUTRAL, 0},
        {{"DX-88", "Nia", "Done", "Live"}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 1},
};

static const egui_view_data_grid_row_t g_rows_1[] = {
        {{"QA-10", "Ava", "Queued", "08:30"}, EGUI_VIEW_DATA_GRID_TONE_NEUTRAL, 0},
        {{"QA-14", "Moe", "Run", "09:10"}, EGUI_VIEW_DATA_GRID_TONE_ACCENT, 1},
        {{"QA-18", "Ivy", "Check", "11:00"}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 0},
        {{"QA-21", "Noa", "Done", "12:40"}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 1},
};

static const egui_view_data_grid_snapshot_t g_snapshots[] = {
        {"OPS", "Rollout board", "Selected row stays inside one grid shell.", "4 rows share one selection target.", g_columns, g_rows_0, 4, 4, 1},
        {"QA", "Audit board", "Header and rows still keep the same table shell.", "QA rows still use the same row activation path.", g_columns, g_rows_1, 4, 4, 2},
};

static const egui_view_data_grid_row_t g_overflow_rows[] = {
        {{"A", "A", "A", "1"}, EGUI_VIEW_DATA_GRID_TONE_ACCENT, 0},
        {{"B", "B", "B", "2"}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 0},
        {{"C", "C", "C", "3"}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 0},
        {{"D", "D", "D", "4"}, EGUI_VIEW_DATA_GRID_TONE_NEUTRAL, 0},
        {{"E", "E", "E", "5"}, EGUI_VIEW_DATA_GRID_TONE_ACCENT, 0},
        {{"F", "F", "F", "6"}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 0},
};

static const egui_view_data_grid_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "A", g_columns, g_overflow_rows, 7, 9, 5},
        {"B", "B", "B", "B", g_columns, g_rows_0, 4, 4, 1},
        {"C", "C", "C", "C", g_columns, g_rows_0, 4, 4, 1},
        {"D", "D", "D", "D", g_columns, g_rows_0, 4, 4, 1},
        {"E", "E", "E", "E", g_columns, g_rows_0, 4, 4, 1},
        {"F", "F", "F", "F", g_columns, g_rows_0, 4, 4, 1},
        {"G", "G", "G", "G", g_columns, g_rows_0, 4, 4, 1},
};

static const egui_view_data_grid_column_t g_preview_columns[] = {
        {"CASE", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"STATE", EGUI_VIEW_DATA_GRID_ALIGN_LEFT},
        {"ETA", EGUI_VIEW_DATA_GRID_ALIGN_RIGHT},
};

static const egui_view_data_grid_row_t g_preview_rows[] = {
        {{"A-42", "Ready", "Today", NULL}, EGUI_VIEW_DATA_GRID_TONE_ACCENT, 1},
        {{"B-14", "Hold", "Tue", NULL}, EGUI_VIEW_DATA_GRID_TONE_WARNING, 0},
        {{"C-07", "Done", "Fri", NULL}, EGUI_VIEW_DATA_GRID_TONE_SUCCESS, 0},
};

static const egui_view_data_grid_snapshot_t g_preview_snapshot = {
        "OPS", "Compact grid", "", "", g_preview_columns, g_preview_rows, 3, 3, 0};

static void on_action(egui_view_t *self, uint8_t snapshot_index, uint8_t row_index)
{
    EGUI_UNUSED(self);
    g_action_count++;
    g_action_snapshot = snapshot_index;
    g_action_row = row_index;
}

static void reset_action_state(void)
{
    g_action_count = 0;
    g_action_snapshot = 0xFF;
    g_action_row = EGUI_VIEW_DATA_GRID_ROW_NONE;
}

static void setup_widget(const egui_view_data_grid_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_data_grid_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 196, 126);
    egui_view_data_grid_set_snapshots(EGUI_VIEW_OF(&test_widget), snapshots, snapshot_count);
    egui_view_data_grid_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_data_grid_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_data_grid_set_on_action_listener(EGUI_VIEW_OF(&test_widget), on_action);
    reset_action_state();
}

static void setup_preview_widget(void)
{
    egui_view_data_grid_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 80);
    egui_view_data_grid_set_snapshots(EGUI_VIEW_OF(&preview_widget), &g_preview_snapshot, 1);
    egui_view_data_grid_set_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_data_grid_set_meta_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_data_grid_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_data_grid_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    reset_action_state();
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

static void layout_widget(void)
{
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 20, 196, 126);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 104, 80);
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

static int send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_widget), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_widget), type, x, y);
}

static int send_key_action(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_widget), type, key_code);
}

static int send_preview_key_action(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_widget), type, key_code);
}

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t get_row_center(egui_view_t *view, uint8_t row_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_data_grid_get_row_region(view, row_index, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 4;
    *y = view->region_screen.location.y - 4;
}

static void seed_pressed_state(egui_view_data_grid_t *widget, uint8_t row_index, uint8_t visual_pressed)
{
    widget->pressed_row = row_index;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_data_grid_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATA_GRID_ROW_NONE, widget->pressed_row);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_data_grid_set_snapshots_clamp_and_defaults(void)
{
    setup_widget(g_overflow_snapshots, EGUI_ARRAY_SIZE(g_overflow_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATA_GRID_MAX_SNAPSHOTS, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, 3, 1);
    test_widget.current_snapshot = 5;
    egui_view_data_grid_set_snapshots(EGUI_VIEW_OF(&test_widget), g_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_data_grid_set_current_row(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_DATA_GRID_ROW_NONE);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));

    egui_view_data_grid_set_snapshots(EGUI_VIEW_OF(&test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_DATA_GRID_ROW_NONE, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
}

static void test_data_grid_setters_clear_pressed_and_update_state(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_data_grid_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1, 0);
    egui_view_data_grid_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_data_grid_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.compact_mode);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_data_grid_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    assert_pressed_cleared(&test_widget);
    egui_view_data_grid_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_data_grid_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                    EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                    EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_widget.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_widget.border_color.full);
    assert_pressed_cleared(&test_widget);

    egui_view_data_grid_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, 2, 1);
    egui_view_data_grid_set_current_row(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);
}

static void test_data_grid_regions_activate_and_listener(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();

    EGUI_TEST_ASSERT_TRUE(get_row_center(EGUI_VIEW_OF(&test_widget), 2, &x, &y));
    egui_view_data_grid_set_current_row(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_TRUE(egui_view_data_grid_activate_current_row(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_action_row);

    reset_action_state();
    egui_view_data_grid_set_on_action_listener(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_data_grid_activate_current_row(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_data_grid_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t row_x;
    egui_dim_t row_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_row_center(EGUI_VIEW_OF(&test_widget), 0, &row_x, &row_y));
    get_view_outside_point(EGUI_VIEW_OF(&test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row_x, row_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.pressed_row);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row_x, row_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, row_x, row_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, row_x, row_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    reset_action_state();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row_x, row_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, row_x, row_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_widget);
}

static void test_data_grid_key_navigation_activate_and_tab(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_widget.pressed_row);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_action_row);
    assert_pressed_cleared(&test_widget);
}

static void test_data_grid_read_only_and_disabled_guards(void)
{
    egui_dim_t row_x;
    egui_dim_t row_y;
    uint8_t initial_snapshot;
    uint8_t initial_row;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_row_center(EGUI_VIEW_OF(&test_widget), 1, &row_x, &row_y));
    initial_snapshot = egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&test_widget));
    initial_row = egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget));

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_data_grid_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row_x, row_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_row, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    egui_view_data_grid_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_action_row);

    reset_action_state();
    egui_view_data_grid_set_current_snapshot(EGUI_VIEW_OF(&test_widget), initial_snapshot);
    egui_view_data_grid_set_current_row(EGUI_VIEW_OF(&test_widget), initial_row);
    initial_snapshot = egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&test_widget));
    initial_row = egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget));

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row_x, row_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_row, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 1);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_row);
}

static void test_data_grid_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t row_x;
    egui_dim_t row_y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(get_row_center(EGUI_VIEW_OF(&preview_widget), 0, &row_x, &row_y));

    seed_pressed_state(&preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row_x, row_y));
    assert_pressed_cleared(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    seed_pressed_state(&preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_data_grid_get_current_row(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

void test_data_grid_run(void)
{
    EGUI_TEST_SUITE_BEGIN(data_grid);
    EGUI_TEST_RUN(test_data_grid_set_snapshots_clamp_and_defaults);
    EGUI_TEST_RUN(test_data_grid_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_data_grid_regions_activate_and_listener);
    EGUI_TEST_RUN(test_data_grid_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_data_grid_key_navigation_activate_and_tab);
    EGUI_TEST_RUN(test_data_grid_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_data_grid_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
