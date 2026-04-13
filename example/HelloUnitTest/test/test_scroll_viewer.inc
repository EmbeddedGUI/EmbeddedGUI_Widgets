#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_scroll_viewer.h"

#include "../../HelloCustomWidgets/layout/scroll_viewer/egui_view_scroll_viewer.h"
#include "../../HelloCustomWidgets/layout/scroll_viewer/egui_view_scroll_viewer.c"

static egui_view_scroll_viewer_t scroll_viewer_test_widget;
static egui_view_scroll_viewer_t scroll_viewer_preview_widget;
static egui_view_api_t scroll_viewer_preview_api;
static uint8_t scroll_viewer_change_count;
static uint8_t scroll_viewer_change_snapshot;
static egui_dim_t scroll_viewer_change_vertical;
static egui_dim_t scroll_viewer_change_horizontal;
static uint8_t scroll_viewer_change_part;

static const egui_view_scroll_viewer_block_t scroll_viewer_blocks_a[] = {
        {"REL", "Release pane", "Artifacts", 0, 0, 112, 36, EGUI_VIEW_SCROLL_VIEWER_TONE_ACCENT, 1},
        {"OPS", "Ops summary", "Hand-off", 14, 52, 126, 40, EGUI_VIEW_SCROLL_VIEWER_TONE_SUCCESS, 0},
        {"QA", "Checklist", "Validation", 8, 110, 138, 38, EGUI_VIEW_SCROLL_VIEWER_TONE_WARNING, 0},
};

static const egui_view_scroll_viewer_block_t scroll_viewer_blocks_b[] = {
        {"OPS", "Diagnostics", "Primary lane", 0, 0, 120, 34, EGUI_VIEW_SCROLL_VIEWER_TONE_ACCENT, 0},
        {"QA", "Drill log", "Escalations", 22, 54, 136, 42, EGUI_VIEW_SCROLL_VIEWER_TONE_WARNING, 1},
        {"DOC", "Appendix", "Footer notes", 8, 114, 132, 36, EGUI_VIEW_SCROLL_VIEWER_TONE_NEUTRAL, 0},
};

static const egui_view_scroll_viewer_snapshot_t scroll_viewer_snapshots[] = {
        {"REL", "Release pane", "Summary", "Helper", "Footer", scroll_viewer_blocks_a, 3, 180, 220, 132, 144, 24, 6, 1},
        {"OPS", "Diagnostics", "Summary", "Helper", "Footer", scroll_viewer_blocks_b, 3, 196, 248, 132, 144, 88, 18, 1},
};

static const egui_view_scroll_viewer_snapshot_t scroll_viewer_overflow_snapshots[] = {
        {"A", "A", "A", "A", "A", scroll_viewer_blocks_a, 3, 200, 260, 132, 144, 40, 12, 1},
        {"B", "B", "B", "B", "B", scroll_viewer_blocks_a, 3, 180, 220, 132, 144, 24, 6, 1},
        {"C", "C", "C", "C", "C", scroll_viewer_blocks_a, 3, 180, 220, 132, 144, 24, 6, 1},
        {"D", "D", "D", "D", "D", scroll_viewer_blocks_a, 3, 180, 220, 132, 144, 24, 6, 1},
        {"E", "E", "E", "E", "E", scroll_viewer_blocks_a, 3, 180, 220, 132, 144, 24, 6, 1},
        {"F", "F", "F", "F", "F", scroll_viewer_blocks_a, 3, 180, 220, 132, 144, 24, 6, 1},
        {"G", "G", "G", "G", "G", scroll_viewer_blocks_a, 3, 180, 220, 132, 144, 24, 6, 1},
};

static const egui_view_scroll_viewer_snapshot_t scroll_viewer_preview_snapshot = {
        "UI", "Preview", "", "", "Preview", scroll_viewer_blocks_a, 3, 154, 160, 102, 82, 18, 8, 1};

static void scroll_viewer_on_change(egui_view_t *self, uint8_t snapshot_index, egui_dim_t vertical_offset, egui_dim_t horizontal_offset, uint8_t part)
{
    EGUI_UNUSED(self);
    scroll_viewer_change_count++;
    scroll_viewer_change_snapshot = snapshot_index;
    scroll_viewer_change_vertical = vertical_offset;
    scroll_viewer_change_horizontal = horizontal_offset;
    scroll_viewer_change_part = part;
}

static void scroll_viewer_reset_change_state(void)
{
    scroll_viewer_change_count = 0;
    scroll_viewer_change_snapshot = 0xFF;
    scroll_viewer_change_vertical = -1;
    scroll_viewer_change_horizontal = -1;
    scroll_viewer_change_part = EGUI_VIEW_SCROLL_VIEWER_PART_NONE;
}

static void scroll_viewer_setup_widget(const egui_view_scroll_viewer_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_scroll_viewer_init(EGUI_VIEW_OF(&scroll_viewer_test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_viewer_test_widget), 196, 160);
    egui_view_scroll_viewer_set_snapshots(EGUI_VIEW_OF(&scroll_viewer_test_widget), snapshots, snapshot_count);
    egui_view_scroll_viewer_set_font(EGUI_VIEW_OF(&scroll_viewer_test_widget), NULL);
    egui_view_scroll_viewer_set_meta_font(EGUI_VIEW_OF(&scroll_viewer_test_widget), NULL);
    egui_view_scroll_viewer_set_on_view_changed_listener(EGUI_VIEW_OF(&scroll_viewer_test_widget), scroll_viewer_on_change);
    scroll_viewer_reset_change_state();
}

static void scroll_viewer_setup_preview_widget(void)
{
    egui_view_scroll_viewer_init(EGUI_VIEW_OF(&scroll_viewer_preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&scroll_viewer_preview_widget), 104, 88);
    egui_view_scroll_viewer_set_snapshots(EGUI_VIEW_OF(&scroll_viewer_preview_widget), &scroll_viewer_preview_snapshot, 1);
    egui_view_scroll_viewer_set_font(EGUI_VIEW_OF(&scroll_viewer_preview_widget), NULL);
    egui_view_scroll_viewer_set_meta_font(EGUI_VIEW_OF(&scroll_viewer_preview_widget), NULL);
    egui_view_scroll_viewer_set_compact_mode(EGUI_VIEW_OF(&scroll_viewer_preview_widget), 1);
    egui_view_scroll_viewer_override_static_preview_api(EGUI_VIEW_OF(&scroll_viewer_preview_widget), &scroll_viewer_preview_api);
    scroll_viewer_reset_change_state();
}

static void scroll_viewer_layout_view(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void scroll_viewer_layout_widget(void)
{
    scroll_viewer_layout_view(EGUI_VIEW_OF(&scroll_viewer_test_widget), 10, 20, 196, 160);
}

static void scroll_viewer_layout_preview_widget(void)
{
    scroll_viewer_layout_view(EGUI_VIEW_OF(&scroll_viewer_preview_widget), 12, 18, 104, 88);
}

static int scroll_viewer_send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int scroll_viewer_send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static int scroll_viewer_send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return scroll_viewer_send_touch_to_view(EGUI_VIEW_OF(&scroll_viewer_test_widget), type, x, y);
}

static int scroll_viewer_send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return scroll_viewer_send_touch_to_view(EGUI_VIEW_OF(&scroll_viewer_preview_widget), type, x, y);
}

static int scroll_viewer_send_key_action(uint8_t type, uint8_t key_code)
{
    return scroll_viewer_send_key_to_view(EGUI_VIEW_OF(&scroll_viewer_test_widget), type, key_code);
}

static int scroll_viewer_send_preview_key_action(uint8_t type, uint8_t key_code)
{
    return scroll_viewer_send_key_to_view(EGUI_VIEW_OF(&scroll_viewer_preview_widget), type, key_code);
}

static int scroll_viewer_send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= scroll_viewer_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= scroll_viewer_send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int scroll_viewer_send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= scroll_viewer_send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= scroll_viewer_send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t scroll_viewer_get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_scroll_viewer_get_part_region(view, part, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static uint8_t scroll_viewer_get_track_press_point(egui_view_t *view, uint8_t above_thumb, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t track_region;
    egui_region_t thumb_region;

    if (!egui_view_scroll_viewer_get_part_region(view, EGUI_VIEW_SCROLL_VIEWER_PART_TRACK, &track_region) ||
        !egui_view_scroll_viewer_get_part_region(view, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, &thumb_region))
    {
        return 0;
    }

    *x = track_region.location.x + track_region.size.width / 2;
    if (above_thumb)
    {
        *y = thumb_region.location.y - 3;
        if (*y <= track_region.location.y)
        {
            *y = track_region.location.y + 2;
        }
    }
    else
    {
        *y = thumb_region.location.y + thumb_region.size.height + 3;
        if (*y >= track_region.location.y + track_region.size.height)
        {
            *y = track_region.location.y + track_region.size.height - 2;
        }
    }
    return 1;
}

static void scroll_viewer_get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 4;
    *y = view->region_screen.location.y - 4;
}

static void scroll_viewer_seed_pressed_state(egui_view_scroll_viewer_t *widget, uint8_t part, uint8_t visual_pressed)
{
    widget->pressed_part = part;
    widget->thumb_dragging = part == EGUI_VIEW_SCROLL_VIEWER_PART_THUMB ? 1 : 0;
    widget->track_direction = part == EGUI_VIEW_SCROLL_VIEWER_PART_TRACK ? 1 : 0;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void scroll_viewer_assert_pressed_cleared(egui_view_scroll_viewer_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, widget->thumb_dragging);
    EGUI_TEST_ASSERT_EQUAL_INT(0, widget->track_direction);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_scroll_viewer_set_snapshots_clamp_and_defaults(void)
{
    scroll_viewer_setup_widget(scroll_viewer_overflow_snapshots, EGUI_ARRAY_SIZE(scroll_viewer_overflow_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_MAX_SNAPSHOTS, scroll_viewer_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_viewer_get_current_snapshot(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(40, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_scroll_viewer_get_horizontal_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_scroll_viewer_get_scrollbar_visibility(EGUI_VIEW_OF(&scroll_viewer_test_widget)));

    scroll_viewer_seed_pressed_state(&scroll_viewer_test_widget, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, 1);
    scroll_viewer_test_widget.current_snapshot = 4;
    egui_view_scroll_viewer_set_snapshots(EGUI_VIEW_OF(&scroll_viewer_test_widget), scroll_viewer_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_viewer_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_viewer_get_current_snapshot(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_scroll_viewer_get_horizontal_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);

    egui_view_scroll_viewer_set_snapshots(EGUI_VIEW_OF(&scroll_viewer_test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_viewer_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_viewer_get_current_snapshot(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_viewer_get_horizontal_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_scroll_viewer_get_scrollbar_visibility(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
}

static void test_scroll_viewer_setters_clear_pressed_and_update_state(void)
{
    scroll_viewer_setup_widget(scroll_viewer_snapshots, EGUI_ARRAY_SIZE(scroll_viewer_snapshots));

    scroll_viewer_seed_pressed_state(&scroll_viewer_test_widget, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, 1);
    egui_view_scroll_viewer_set_font(EGUI_VIEW_OF(&scroll_viewer_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);

    scroll_viewer_seed_pressed_state(&scroll_viewer_test_widget, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, 1);
    egui_view_scroll_viewer_set_meta_font(EGUI_VIEW_OF(&scroll_viewer_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);

    scroll_viewer_seed_pressed_state(&scroll_viewer_test_widget, EGUI_VIEW_SCROLL_VIEWER_PART_TRACK, 1);
    egui_view_scroll_viewer_set_compact_mode(EGUI_VIEW_OF(&scroll_viewer_test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_viewer_test_widget.compact_mode);
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);
    egui_view_scroll_viewer_set_compact_mode(EGUI_VIEW_OF(&scroll_viewer_test_widget), 0);

    scroll_viewer_seed_pressed_state(&scroll_viewer_test_widget, EGUI_VIEW_SCROLL_VIEWER_PART_TRACK, 1);
    egui_view_scroll_viewer_set_read_only_mode(EGUI_VIEW_OF(&scroll_viewer_test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_viewer_test_widget.read_only_mode);
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);
    egui_view_scroll_viewer_set_read_only_mode(EGUI_VIEW_OF(&scroll_viewer_test_widget), 0);

    scroll_viewer_seed_pressed_state(&scroll_viewer_test_widget, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, 1);
    egui_view_scroll_viewer_set_palette(EGUI_VIEW_OF(&scroll_viewer_test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122),
                                        EGUI_COLOR_HEX(0x303132), EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162),
                                        EGUI_COLOR_HEX(0x707172));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, scroll_viewer_test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, scroll_viewer_test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, scroll_viewer_test_widget.viewport_color.full);
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);

    egui_view_scroll_viewer_set_current_snapshot(EGUI_VIEW_OF(&scroll_viewer_test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_scroll_viewer_get_current_snapshot(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(88, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_scroll_viewer_get_horizontal_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));

    egui_view_scroll_viewer_set_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget), 999);
    EGUI_TEST_ASSERT_EQUAL_INT(104, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    egui_view_scroll_viewer_set_horizontal_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget), 999);
    EGUI_TEST_ASSERT_EQUAL_INT(64, egui_view_scroll_viewer_get_horizontal_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));

    egui_view_scroll_viewer_set_scrollbar_visibility(EGUI_VIEW_OF(&scroll_viewer_test_widget), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_viewer_get_scrollbar_visibility(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE, scroll_viewer_test_widget.current_part);
}

static void test_scroll_viewer_regions_scroll_and_listener(void)
{
    egui_region_t track_region;
    egui_dim_t x;
    egui_dim_t y;

    scroll_viewer_setup_widget(scroll_viewer_snapshots, EGUI_ARRAY_SIZE(scroll_viewer_snapshots));
    scroll_viewer_layout_widget();

    EGUI_TEST_ASSERT_TRUE(scroll_viewer_get_part_center(EGUI_VIEW_OF(&scroll_viewer_test_widget), EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE, &x, &y));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_get_part_center(EGUI_VIEW_OF(&scroll_viewer_test_widget), EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, &x, &y));
    EGUI_TEST_ASSERT_TRUE(
            egui_view_scroll_viewer_get_part_region(EGUI_VIEW_OF(&scroll_viewer_test_widget), EGUI_VIEW_SCROLL_VIEWER_PART_TRACK, &track_region));

    scroll_viewer_reset_change_state();
    egui_view_scroll_viewer_set_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget), 48);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_viewer_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_viewer_change_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(48, scroll_viewer_change_vertical);
    EGUI_TEST_ASSERT_EQUAL_INT(6, scroll_viewer_change_horizontal);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, scroll_viewer_change_part);

    scroll_viewer_reset_change_state();
    egui_view_scroll_viewer_set_horizontal_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget), 12);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_viewer_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(12, scroll_viewer_change_horizontal);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE, scroll_viewer_change_part);

    scroll_viewer_reset_change_state();
    egui_view_scroll_viewer_scroll_page(EGUI_VIEW_OF(&scroll_viewer_test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_change_count > 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_TRACK, scroll_viewer_change_part);

    scroll_viewer_reset_change_state();
    egui_view_scroll_viewer_set_on_view_changed_listener(EGUI_VIEW_OF(&scroll_viewer_test_widget), NULL);
    egui_view_scroll_viewer_scroll_line(EGUI_VIEW_OF(&scroll_viewer_test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_viewer_change_count);
}

static void test_scroll_viewer_touch_same_target_release_and_drag_behavior(void)
{
    egui_dim_t track_x;
    egui_dim_t track_y;
    egui_dim_t thumb_x;
    egui_dim_t thumb_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;
    egui_dim_t initial_offset;

    scroll_viewer_setup_widget(scroll_viewer_snapshots, EGUI_ARRAY_SIZE(scroll_viewer_snapshots));
    scroll_viewer_layout_widget();
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_get_track_press_point(EGUI_VIEW_OF(&scroll_viewer_test_widget), 1, &track_x, &track_y));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_get_part_center(EGUI_VIEW_OF(&scroll_viewer_test_widget), EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, &thumb_x, &thumb_y));
    scroll_viewer_get_view_outside_point(EGUI_VIEW_OF(&scroll_viewer_test_widget), &outside_x, &outside_y);

    initial_offset = egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, track_x, track_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_TRACK, scroll_viewer_test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&scroll_viewer_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&scroll_viewer_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_offset, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_viewer_change_count);
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);

    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, track_x, track_y));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_UP, track_x, track_y));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)) < initial_offset);
    EGUI_TEST_ASSERT_EQUAL_INT(1, scroll_viewer_change_count);
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);

    scroll_viewer_reset_change_state();
    initial_offset = egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, thumb_x, thumb_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, scroll_viewer_test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, thumb_x, thumb_y + 24));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)) >= initial_offset);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_UP, thumb_x, thumb_y + 24));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_change_count > 0);
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);
}

static void test_scroll_viewer_key_navigation_scroll_and_tab(void)
{
    scroll_viewer_setup_widget(scroll_viewer_snapshots, EGUI_ARRAY_SIZE(scroll_viewer_snapshots));
    scroll_viewer_layout_widget();

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE, scroll_viewer_test_widget.current_part);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, scroll_viewer_test_widget.current_part);

    scroll_viewer_reset_change_state();
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, scroll_viewer_test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_change_count > 0);
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);

    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(76, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_scroll_viewer_get_horizontal_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(22, egui_view_scroll_viewer_get_horizontal_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)));
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_key(EGUI_KEY_CODE_MINUS));
    EGUI_TEST_ASSERT_TRUE(egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_test_widget)) < 76);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE, scroll_viewer_test_widget.current_part);
}

static void test_scroll_viewer_read_only_and_disabled_guards(void)
{
    egui_dim_t surface_x;
    egui_dim_t surface_y;

    scroll_viewer_setup_widget(scroll_viewer_snapshots, EGUI_ARRAY_SIZE(scroll_viewer_snapshots));
    scroll_viewer_layout_widget();
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_get_part_center(EGUI_VIEW_OF(&scroll_viewer_test_widget), EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE, &surface_x, &surface_y));

    scroll_viewer_seed_pressed_state(&scroll_viewer_test_widget, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, 1);
    egui_view_scroll_viewer_set_read_only_mode(EGUI_VIEW_OF(&scroll_viewer_test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, surface_x, surface_y));
    EGUI_TEST_ASSERT_FALSE(scroll_viewer_send_key(EGUI_KEY_CODE_ENTER));
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);
    egui_view_scroll_viewer_set_read_only_mode(EGUI_VIEW_OF(&scroll_viewer_test_widget), 0);

    scroll_viewer_seed_pressed_state(&scroll_viewer_test_widget, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&scroll_viewer_test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(scroll_viewer_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, surface_x, surface_y));
    EGUI_TEST_ASSERT_FALSE(scroll_viewer_send_key(EGUI_KEY_CODE_ENTER));
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_test_widget);
    egui_view_set_enable(EGUI_VIEW_OF(&scroll_viewer_test_widget), 1);
}

static void test_scroll_viewer_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t surface_x;
    egui_dim_t surface_y;

    scroll_viewer_setup_preview_widget();
    scroll_viewer_layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_get_part_center(EGUI_VIEW_OF(&scroll_viewer_preview_widget), EGUI_VIEW_SCROLL_VIEWER_PART_SURFACE, &surface_x, &surface_y));

    scroll_viewer_seed_pressed_state(&scroll_viewer_preview_widget, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, 1);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, surface_x, surface_y));
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_viewer_change_count);

    scroll_viewer_seed_pressed_state(&scroll_viewer_preview_widget, EGUI_VIEW_SCROLL_VIEWER_PART_THUMB, 1);
    EGUI_TEST_ASSERT_TRUE(scroll_viewer_send_preview_key(EGUI_KEY_CODE_ENTER));
    scroll_viewer_assert_pressed_cleared(&scroll_viewer_preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_scroll_viewer_get_vertical_offset(EGUI_VIEW_OF(&scroll_viewer_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, scroll_viewer_change_count);
}

void test_scroll_viewer_run(void)
{
    EGUI_TEST_SUITE_BEGIN(scroll_viewer);
    EGUI_TEST_RUN(test_scroll_viewer_set_snapshots_clamp_and_defaults);
    EGUI_TEST_RUN(test_scroll_viewer_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_scroll_viewer_regions_scroll_and_listener);
    EGUI_TEST_RUN(test_scroll_viewer_touch_same_target_release_and_drag_behavior);
    EGUI_TEST_RUN(test_scroll_viewer_key_navigation_scroll_and_tab);
    EGUI_TEST_RUN(test_scroll_viewer_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_scroll_viewer_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
