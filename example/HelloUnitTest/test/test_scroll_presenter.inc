#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_scroll_presenter.h"

#include "../../HelloCustomWidgets/layout/scroll_presenter/egui_view_scroll_presenter.h"
#include "../../HelloCustomWidgets/layout/scroll_presenter/egui_view_scroll_presenter.c"

static egui_view_scroll_presenter_t scroll_presenter_test_widget;
static egui_view_scroll_presenter_t scroll_presenter_preview_widget;
static egui_view_api_t scroll_presenter_preview_api;
static uint8_t scroll_presenter_change_count;
static uint8_t scroll_presenter_change_snapshot;
static egui_dim_t scroll_presenter_change_vertical;
static egui_dim_t scroll_presenter_change_horizontal;
static uint8_t scroll_presenter_change_part;

static const egui_view_scroll_presenter_item_t scroll_presenter_items_a[] = {
        {"MAP", "Origin", "Canvas", 0, 0, 112, 36, EGUI_VIEW_SCROLL_PRESENTER_TONE_ACCENT, 1},
        {"OPS", "Signal", "Cluster", 96, 56, 126, 40, EGUI_VIEW_SCROLL_PRESENTER_TONE_SUCCESS, 0},
        {"QA", "Branch", "Checks", 28, 134, 132, 38, EGUI_VIEW_SCROLL_PRESENTER_TONE_WARNING, 0},
};

static const egui_view_scroll_presenter_item_t scroll_presenter_items_b[] = {
        {"REL", "Timeline", "Stream", 0, 0, 118, 34, EGUI_VIEW_SCROLL_PRESENTER_TONE_ACCENT, 0},
        {"UX", "Review", "Bridge", 124, 18, 120, 38, EGUI_VIEW_SCROLL_PRESENTER_TONE_NEUTRAL, 1},
        {"ENG", "Queue", "Build lane", 54, 102, 134, 40, EGUI_VIEW_SCROLL_PRESENTER_TONE_SUCCESS, 0},
};

static const egui_view_scroll_presenter_snapshot_t scroll_presenter_snapshots[] = {
        {"MAP", "Canvas", "Summary", "Helper", "Footer", scroll_presenter_items_a, 3, 200, 260, 132, 144, 40, 12},
        {"REL", "Timeline", "Summary", "Helper", "Footer", scroll_presenter_items_b, 3, 208, 248, 132, 144, 88, 18},
};

static const egui_view_scroll_presenter_snapshot_t scroll_presenter_overflow_snapshots[] = {
        {"A", "A", "A", "A", "A", scroll_presenter_items_a, 3, 200, 260, 132, 144, 40, 12},
        {"B", "B", "B", "B", "B", scroll_presenter_items_a, 3, 200, 260, 132, 144, 40, 12},
        {"C", "C", "C", "C", "C", scroll_presenter_items_a, 3, 200, 260, 132, 144, 40, 12},
        {"D", "D", "D", "D", "D", scroll_presenter_items_a, 3, 200, 260, 132, 144, 40, 12},
        {"E", "E", "E", "E", "E", scroll_presenter_items_a, 3, 200, 260, 132, 144, 40, 12},
        {"F", "F", "F", "F", "F", scroll_presenter_items_a, 3, 200, 260, 132, 144, 40, 12},
        {"G", "G", "G", "G", "G", scroll_presenter_items_a, 3, 200, 260, 132, 144, 40, 12},
};

static const egui_view_scroll_presenter_snapshot_t scroll_presenter_preview_snapshot = {
        "UI", "Preview", "", "", "Preview", scroll_presenter_items_a, 3, 154, 160, 102, 82, 18, 8};

static void scroll_presenter_on_change(egui_view_t *self, uint8_t snapshot_index, egui_dim_t vertical_offset, egui_dim_t horizontal_offset, uint8_t part)
{
    EGUI_UNUSED(self);
    scroll_presenter_change_count++;
    scroll_presenter_change_snapshot = snapshot_index;
    scroll_presenter_change_vertical = vertical_offset;
    scroll_presenter_change_horizontal = horizontal_offset;
    scroll_presenter_change_part = part;
}

static void scroll_presenter_reset_change_state(void)
{
    scroll_presenter_change_count = 0;
    scroll_presenter_change_snapshot = 0xFF;
    scroll_presenter_change_vertical = -1;
    scroll_presenter_change_horizontal = -1;
    scroll_presenter_change_part = EGUI_VIEW_SCROLL_PRESENTER_PART_NONE;
}

static void scroll_presenter_setup_widget(const egui_view_scroll_presenter_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_scroll_presenter_init(EGUI_VIEW_OF(&scroll_presenter_test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_presenter_test_widget), 196, 160);
    egui_view_scroll_presenter_set_snapshots(EGUI_VIEW_OF(&scroll_presenter_test_widget), snapshots, snapshot_count);
    egui_view_scroll_presenter_set_font(EGUI_VIEW_OF(&scroll_presenter_test_widget), NULL);
    egui_view_scroll_presenter_set_meta_font(EGUI_VIEW_OF(&scroll_presenter_test_widget), NULL);
    egui_view_scroll_presenter_set_on_view_changed_listener(EGUI_VIEW_OF(&scroll_presenter_test_widget), scroll_presenter_on_change);
    scroll_presenter_reset_change_state();
}

static void scroll_presenter_setup_preview_widget(void)
{
    egui_view_scroll_presenter_init(EGUI_VIEW_OF(&scroll_presenter_preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_presenter_preview_widget), 104, 88);
    egui_view_scroll_presenter_set_snapshots(EGUI_VIEW_OF(&scroll_presenter_preview_widget), &scroll_presenter_preview_snapshot, 1);
    egui_view_scroll_presenter_set_font(EGUI_VIEW_OF(&scroll_presenter_preview_widget), NULL);
    egui_view_scroll_presenter_set_meta_font(EGUI_VIEW_OF(&scroll_presenter_preview_widget), NULL);
    egui_view_scroll_presenter_set_compact_mode(EGUI_VIEW_OF(&scroll_presenter_preview_widget), 1);
    egui_view_scroll_presenter_override_static_preview_api(EGUI_VIEW_OF(&scroll_presenter_preview_widget), &scroll_presenter_preview_api);
    scroll_presenter_reset_change_state();
}

static void scroll_presenter_layout_view(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void scroll_presenter_layout_widget(void)
{
    scroll_presenter_layout_view(EGUI_VIEW_OF(&scroll_presenter_test_widget), 10, 20, 196, 160);
}

static void scroll_presenter_layout_preview_widget(void)
{
    scroll_presenter_layout_view(EGUI_VIEW_OF(&scroll_presenter_preview_widget), 12, 18, 104, 88);
}

static int scroll_presenter_send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int scroll_presenter_send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int scroll_presenter_send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return scroll_presenter_send_touch_to_view(EGUI_VIEW_OF(&scroll_presenter_test_widget), type, x, y);
}

static int scroll_presenter_send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return scroll_presenter_send_touch_to_view(EGUI_VIEW_OF(&scroll_presenter_preview_widget), type, x, y);
}

static int scroll_presenter_send_key_action(uint8_t type, uint8_t key_code)
{
    return scroll_presenter_send_key_to_view(EGUI_VIEW_OF(&scroll_presenter_test_widget), type, key_code);
}

static int scroll_presenter_send_preview_key_action(uint8_t type, uint8_t key_code)
{
    return scroll_presenter_send_key_to_view(EGUI_VIEW_OF(&scroll_presenter_preview_widget), type, key_code);
}

static int scroll_presenter_send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= scroll_presenter_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= scroll_presenter_send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int scroll_presenter_send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= scroll_presenter_send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= scroll_presenter_send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t scroll_presenter_get_surface_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_scroll_presenter_get_surface_region(view, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void scroll_presenter_seed_pressed_state(egui_view_scroll_presenter_t *widget, uint8_t visual_pressed)
{
    widget->pressed_part = EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE;
    widget->surface_dragging = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void scroll_presenter_assert_pressed_cleared(egui_view_scroll_presenter_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_PRESENTER_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, widget->surface_dragging);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_scroll_presenter_set_snapshots_clamp_and_defaults(void)
{
    scroll_presenter_setup_widget(scroll_presenter_overflow_snapshots, EGUI_ARRAY_SIZE(scroll_presenter_overflow_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_PRESENTER_MAX_SNAPSHOTS, scroll_presenter_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(40, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));

    scroll_presenter_seed_pressed_state(&scroll_presenter_test_widget, 1);
    scroll_presenter_test_widget.current_snapshot = 4;
    egui_view_scroll_presenter_set_snapshots(EGUI_VIEW_OF(&scroll_presenter_test_widget), scroll_presenter_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_presenter_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(40, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_test_widget);

    egui_view_scroll_presenter_set_snapshots(EGUI_VIEW_OF(&scroll_presenter_test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_presenter_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
}

static void test_scroll_presenter_setters_clear_pressed_and_update_state(void)
{
    scroll_presenter_setup_widget(scroll_presenter_snapshots, EGUI_ARRAY_SIZE(scroll_presenter_snapshots));

    scroll_presenter_seed_pressed_state(&scroll_presenter_test_widget, 1);
    egui_view_scroll_presenter_set_font(EGUI_VIEW_OF(&scroll_presenter_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_test_widget);

    scroll_presenter_seed_pressed_state(&scroll_presenter_test_widget, 1);
    egui_view_scroll_presenter_set_meta_font(EGUI_VIEW_OF(&scroll_presenter_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_test_widget);

    scroll_presenter_seed_pressed_state(&scroll_presenter_test_widget, 1);
    egui_view_scroll_presenter_set_compact_mode(EGUI_VIEW_OF(&scroll_presenter_test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_presenter_test_widget.compact_mode);
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_test_widget);
    egui_view_scroll_presenter_set_compact_mode(EGUI_VIEW_OF(&scroll_presenter_test_widget), 0);

    scroll_presenter_seed_pressed_state(&scroll_presenter_test_widget, 1);
    egui_view_scroll_presenter_set_read_only_mode(EGUI_VIEW_OF(&scroll_presenter_test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_presenter_test_widget.read_only_mode);
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_test_widget);
    egui_view_scroll_presenter_set_read_only_mode(EGUI_VIEW_OF(&scroll_presenter_test_widget), 0);

    scroll_presenter_seed_pressed_state(&scroll_presenter_test_widget, 1);
    egui_view_scroll_presenter_set_palette(EGUI_VIEW_OF(&scroll_presenter_test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122),
                                           EGUI_COLOR_HEX(0x303132), EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162),
                                           EGUI_COLOR_HEX(0x707172));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, scroll_presenter_test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, scroll_presenter_test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, scroll_presenter_test_widget.viewport_color.full);
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_test_widget);

    egui_view_scroll_presenter_set_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_scroll_presenter_get_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(88, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));

    egui_view_scroll_presenter_set_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget), 999);
    EGUI_TEST_ASSERT_EQUAL_INT(104, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    egui_view_scroll_presenter_set_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget), 999);
    EGUI_TEST_ASSERT_EQUAL_INT(76, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
}

static void test_scroll_presenter_regions_scroll_and_listener(void)
{
    egui_region_t surface_region;

    scroll_presenter_setup_widget(scroll_presenter_snapshots, EGUI_ARRAY_SIZE(scroll_presenter_snapshots));
    scroll_presenter_layout_widget();

    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_presenter_get_surface_region(EGUI_VIEW_OF(&scroll_presenter_test_widget), &surface_region));

    scroll_presenter_reset_change_state();
    egui_view_scroll_presenter_set_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget), 60);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_presenter_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_presenter_change_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(60, scroll_presenter_change_vertical);
    EGUI_TEST_ASSERT_EQUAL_INT(12, scroll_presenter_change_horizontal);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE, scroll_presenter_change_part);

    scroll_presenter_reset_change_state();
    egui_view_scroll_presenter_set_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget), 20);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_presenter_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(20, scroll_presenter_change_horizontal);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE, scroll_presenter_change_part);

    scroll_presenter_reset_change_state();
    egui_view_scroll_presenter_scroll_page(EGUI_VIEW_OF(&scroll_presenter_test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_change_count > 0);

    scroll_presenter_reset_change_state();
    egui_view_scroll_presenter_activate_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_test_widget));
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_change_count > 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_scroll_presenter_get_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
}

static void test_scroll_presenter_touch_drag_behavior(void)
{
    egui_dim_t surface_x;
    egui_dim_t surface_y;

    scroll_presenter_setup_widget(scroll_presenter_snapshots, EGUI_ARRAY_SIZE(scroll_presenter_snapshots));
    scroll_presenter_layout_widget();
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_get_surface_center(EGUI_VIEW_OF(&scroll_presenter_test_widget), &surface_x, &surface_y));

    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, surface_x, surface_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE, scroll_presenter_test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, surface_x - 18, surface_y - 22));
    EGUI_TEST_ASSERT_EQUAL_INT(62, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(30, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_touch(EGUI_MOTION_EVENT_ACTION_UP, surface_x - 18, surface_y - 22));
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_change_count > 0);
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_test_widget);
}

static void test_scroll_presenter_key_navigation_and_snapshot_cycle(void)
{
    scroll_presenter_setup_widget(scroll_presenter_snapshots, EGUI_ARRAY_SIZE(scroll_presenter_snapshots));
    scroll_presenter_layout_widget();

    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));

    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(116, egui_view_scroll_presenter_get_max_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(68, egui_view_scroll_presenter_get_max_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(116, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(68, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));

    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)) < 68);
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_key(EGUI_KEY_CODE_MINUS));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)) < 116);
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_scroll_presenter_get_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
}

static void test_scroll_presenter_read_only_and_disabled_guards(void)
{
    egui_dim_t surface_x;
    egui_dim_t surface_y;
    egui_dim_t initial_vertical;
    egui_dim_t initial_horizontal;

    scroll_presenter_setup_widget(scroll_presenter_snapshots, EGUI_ARRAY_SIZE(scroll_presenter_snapshots));
    scroll_presenter_layout_widget();
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_get_surface_center(EGUI_VIEW_OF(&scroll_presenter_test_widget), &surface_x, &surface_y));
    initial_vertical = egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget));
    initial_horizontal = egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget));

    scroll_presenter_seed_pressed_state(&scroll_presenter_test_widget, 1);
    egui_view_scroll_presenter_set_read_only_mode(EGUI_VIEW_OF(&scroll_presenter_test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(scroll_presenter_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, surface_x, surface_y));
    EGUI_TEST_ASSERT_FALSE(scroll_presenter_send_key(EGUI_KEY_CODE_ENTER));
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_vertical, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_horizontal, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_presenter_change_count);
    egui_view_scroll_presenter_set_read_only_mode(EGUI_VIEW_OF(&scroll_presenter_test_widget), 0);

    scroll_presenter_seed_pressed_state(&scroll_presenter_test_widget, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&scroll_presenter_test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(scroll_presenter_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, surface_x, surface_y));
    EGUI_TEST_ASSERT_FALSE(scroll_presenter_send_key(EGUI_KEY_CODE_ENTER));
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_vertical, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_horizontal, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_presenter_change_count);
    egui_view_set_enable(EGUI_VIEW_OF(&scroll_presenter_test_widget), 1);

    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_view_scroll_presenter_get_max_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)),
                               egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_view_scroll_presenter_get_max_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)),
                               egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_test_widget)));
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_change_count > 0);
}

static void test_scroll_presenter_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t surface_x;
    egui_dim_t surface_y;

    scroll_presenter_setup_preview_widget();
    scroll_presenter_layout_preview_widget();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_presenter_preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_presenter_preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE, scroll_presenter_preview_widget.current_part);
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_get_surface_center(EGUI_VIEW_OF(&scroll_presenter_preview_widget), &surface_x, &surface_y));

    scroll_presenter_seed_pressed_state(&scroll_presenter_preview_widget, 1);
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, surface_x, surface_y));
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE, scroll_presenter_preview_widget.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_presenter_change_count);

    scroll_presenter_seed_pressed_state(&scroll_presenter_preview_widget, 1);
    EGUI_TEST_ASSERT_TRUE(scroll_presenter_send_preview_key(EGUI_KEY_CODE_ENTER));
    scroll_presenter_assert_pressed_cleared(&scroll_presenter_preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_presenter_get_current_snapshot(EGUI_VIEW_OF(&scroll_presenter_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_scroll_presenter_get_vertical_offset(EGUI_VIEW_OF(&scroll_presenter_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_scroll_presenter_get_horizontal_offset(EGUI_VIEW_OF(&scroll_presenter_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_PRESENTER_PART_SURFACE, scroll_presenter_preview_widget.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_presenter_change_count);
}

void test_scroll_presenter_run(void)
{
    EGUI_TEST_SUITE_BEGIN(scroll_presenter);
    EGUI_TEST_RUN(test_scroll_presenter_set_snapshots_clamp_and_defaults);
    EGUI_TEST_RUN(test_scroll_presenter_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_scroll_presenter_regions_scroll_and_listener);
    EGUI_TEST_RUN(test_scroll_presenter_touch_drag_behavior);
    EGUI_TEST_RUN(test_scroll_presenter_key_navigation_and_snapshot_cycle);
    EGUI_TEST_RUN(test_scroll_presenter_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_scroll_presenter_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
