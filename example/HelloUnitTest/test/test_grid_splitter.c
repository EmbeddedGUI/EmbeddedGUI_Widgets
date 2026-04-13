#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_grid_splitter.h"

#include "../../HelloCustomWidgets/layout/grid_splitter/egui_view_grid_splitter.h"
#include "../../HelloCustomWidgets/layout/grid_splitter/egui_view_grid_splitter.c"

static egui_view_grid_splitter_t test_widget;
static egui_view_grid_splitter_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_ratio_change_count;
static uint8_t g_ratio_snapshot;
static uint8_t g_ratio_value;

static const egui_view_grid_splitter_snapshot_t g_snapshots[] = {
        {"LAYOUT", "Canvas split", "A", "Left", "12", "Pinned rail", "Right", "100%", "Detail pane", "42/58", 42,
         EGUI_VIEW_GRID_SPLITTER_EMPHASIS_RIGHT},
        {"REVIEW", "Audit split", "B", "Queue", "18", "Review rail", "Panel", "Ready", "Inspector pane", "58/42", 58,
         EGUI_VIEW_GRID_SPLITTER_EMPHASIS_LEFT},
};

static const egui_view_grid_splitter_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "", "A", "A", "A", "A", "A", "A", "", 10, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_NONE},
        {"B", "B", "", "B", "B", "B", "B", "B", "B", "", 30, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_NONE},
        {"C", "C", "", "C", "C", "C", "C", "C", "C", "", 40, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_NONE},
        {"D", "D", "", "D", "D", "D", "D", "D", "D", "", 50, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_NONE},
        {"E", "E", "", "E", "E", "E", "E", "E", "E", "", 60, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_NONE},
        {"F", "F", "", "F", "F", "F", "F", "F", "F", "", 70, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_NONE},
        {"G", "G", "", "G", "G", "G", "G", "G", "G", "", 95, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_NONE},
};

static const egui_view_grid_splitter_snapshot_t g_preview_snapshot = {
        "UI", "Compact split", "", "List", "6", "Left rail", "Preview", "1:1", "Static pane", "", 46, EGUI_VIEW_GRID_SPLITTER_EMPHASIS_RIGHT};

static void on_ratio_changed(egui_view_t *self, uint8_t snapshot_index, uint8_t split_ratio)
{
    EGUI_UNUSED(self);
    g_ratio_change_count++;
    g_ratio_snapshot = snapshot_index;
    g_ratio_value = split_ratio;
}

static void reset_ratio_state(void)
{
    g_ratio_change_count = 0;
    g_ratio_snapshot = 0xFF;
    g_ratio_value = 0xFF;
}

static void setup_widget(const egui_view_grid_splitter_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_grid_splitter_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 196, 118);
    egui_view_grid_splitter_set_snapshots(EGUI_VIEW_OF(&test_widget), snapshots, snapshot_count);
    egui_view_grid_splitter_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_grid_splitter_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_grid_splitter_set_on_ratio_changed_listener(EGUI_VIEW_OF(&test_widget), on_ratio_changed);
    reset_ratio_state();
}

static void setup_preview_widget(void)
{
    egui_view_grid_splitter_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 74);
    egui_view_grid_splitter_set_snapshots(EGUI_VIEW_OF(&preview_widget), &g_preview_snapshot, 1);
    egui_view_grid_splitter_set_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_grid_splitter_set_meta_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_grid_splitter_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_grid_splitter_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    reset_ratio_state();
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
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 20, 196, 118);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 104, 74);
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
    return view->api->on_key_event(view, &event);
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

static uint8_t get_handle_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_grid_splitter_get_handle_region(view, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void seed_pressed_state(egui_view_grid_splitter_t *widget, uint8_t visual_pressed, uint8_t dragging)
{
    widget->pressed_handle = 1;
    widget->handle_dragging = dragging ? 1 : 0;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_grid_splitter_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(0, widget->pressed_handle);
    EGUI_TEST_ASSERT_EQUAL_INT(0, widget->handle_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_grid_splitter_set_snapshots_clamp_and_defaults(void)
{
    setup_widget(g_overflow_snapshots, EGUI_ARRAY_SIZE(g_overflow_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_GRID_SPLITTER_MAX_SNAPSHOTS, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_grid_splitter_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MIN, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, 1, 1);
    test_widget.current_snapshot = 5;
    test_widget.split_ratio = 77;
    egui_view_grid_splitter_set_snapshots(EGUI_VIEW_OF(&test_widget), g_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_grid_splitter_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(42, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_grid_splitter_set_snapshots(EGUI_VIEW_OF(&test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_grid_splitter_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_DEFAULT, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));
}

static void test_grid_splitter_setters_clear_pressed_and_update_state(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_grid_splitter_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_grid_splitter_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_grid_splitter_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.compact_mode);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_grid_splitter_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    assert_pressed_cleared(&test_widget);
    egui_view_grid_splitter_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_grid_splitter_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                        EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_widget.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_widget.border_color.full);
    assert_pressed_cleared(&test_widget);

    reset_ratio_state();
    egui_view_grid_splitter_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_grid_splitter_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(58, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_ratio_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_ratio_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(58, g_ratio_value);

    reset_ratio_state();
    seed_pressed_state(&test_widget, 1, 1);
    egui_view_grid_splitter_set_split_ratio(EGUI_VIEW_OF(&test_widget), 72);
    EGUI_TEST_ASSERT_EQUAL_INT(72, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_ratio_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_ratio_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(72, g_ratio_value);
    assert_pressed_cleared(&test_widget);
}

static void test_grid_splitter_handle_region_drag_and_listener(void)
{
    egui_dim_t handle_x;
    egui_dim_t handle_y;
    egui_dim_t drag_x;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_handle_center(EGUI_VIEW_OF(&test_widget), &handle_x, &handle_y));

    drag_x = EGUI_VIEW_OF(&test_widget)->region_screen.location.x + EGUI_VIEW_OF(&test_widget)->region_screen.size.width - 18;
    reset_ratio_state();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, handle_x, handle_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.pressed_handle);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.handle_dragging);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, drag_x, handle_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)) >= 70);
    EGUI_TEST_ASSERT_TRUE(g_ratio_change_count >= 1);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_ratio_snapshot);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, drag_x, handle_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)) >= 70);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(get_handle_center(EGUI_VIEW_OF(&test_widget), &handle_x, &handle_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, handle_x, handle_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, handle_x, handle_y));
    assert_pressed_cleared(&test_widget);
}

static void test_grid_splitter_key_navigation_and_reset(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(47, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MAX, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_GRID_SPLITTER_SPLIT_RATIO_MIN, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));

    reset_ratio_state();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_grid_splitter_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(58, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_ratio_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_ratio_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(58, g_ratio_value);

    egui_view_grid_splitter_set_split_ratio(EGUI_VIEW_OF(&test_widget), 72);
    reset_ratio_state();
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.pressed_handle);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(58, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_ratio_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_ratio_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(58, g_ratio_value);
    assert_pressed_cleared(&test_widget);
}

static void test_grid_splitter_read_only_and_disabled_guards(void)
{
    egui_dim_t handle_x;
    egui_dim_t handle_y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_handle_center(EGUI_VIEW_OF(&test_widget), &handle_x, &handle_y));

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_grid_splitter_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, handle_x, handle_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&test_widget);
    egui_view_grid_splitter_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, handle_x, handle_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&test_widget);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 1);
}

static void test_grid_splitter_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t handle_x;
    egui_dim_t handle_y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(get_handle_center(EGUI_VIEW_OF(&preview_widget), &handle_x, &handle_y));

    seed_pressed_state(&preview_widget, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, handle_x, handle_y));
    EGUI_TEST_ASSERT_EQUAL_INT(46, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_ratio_change_count);
    assert_pressed_cleared(&preview_widget);

    seed_pressed_state(&preview_widget, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(46, egui_view_grid_splitter_get_split_ratio(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_ratio_change_count);
    assert_pressed_cleared(&preview_widget);
}

void test_grid_splitter_run(void)
{
    EGUI_TEST_SUITE_BEGIN(grid_splitter);
    EGUI_TEST_RUN(test_grid_splitter_set_snapshots_clamp_and_defaults);
    EGUI_TEST_RUN(test_grid_splitter_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_grid_splitter_handle_region_drag_and_listener);
    EGUI_TEST_RUN(test_grid_splitter_key_navigation_and_reset);
    EGUI_TEST_RUN(test_grid_splitter_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_grid_splitter_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
