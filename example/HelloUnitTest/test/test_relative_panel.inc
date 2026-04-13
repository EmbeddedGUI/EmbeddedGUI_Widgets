#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_relative_panel.h"

#include "../../HelloCustomWidgets/layout/relative_panel/egui_view_relative_panel.h"
#include "../../HelloCustomWidgets/layout/relative_panel/egui_view_relative_panel.c"

static egui_view_relative_panel_t relative_panel_test_widget;
static egui_view_relative_panel_t relative_panel_preview_widget;
static egui_view_api_t relative_panel_preview_api;
static uint8_t relative_panel_action_count;
static uint8_t relative_panel_action_snapshot;
static uint8_t relative_panel_action_item;

static const egui_view_relative_panel_item_t relative_panel_items_a[] = {
        {"SUM", "Summary", "Top-left", "Align left + top", 0, 0, 96, 40, EGUI_VIEW_RELATIVE_PANEL_TONE_ACCENT, 1,
         EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE},
        {"STAT", "Rail", "Right of summary", "Right of Summary", 116, 6, 74, 54, EGUI_VIEW_RELATIVE_PANEL_TONE_SUCCESS, 0, 0},
        {"DET", "Detail", "Below summary", "Below Summary", 10, 64, 128, 48, EGUI_VIEW_RELATIVE_PANEL_TONE_WARNING, 0, 0},
        {"AUX", "Assist", "Align right", "Align right to Detail", 102, 120, 90, 26, EGUI_VIEW_RELATIVE_PANEL_TONE_NEUTRAL, 0, 2},
};

static const egui_view_relative_panel_item_t relative_panel_items_b[] = {
        {"HUB", "Hero", "Anchor", "Align left + top", 0, 0, 104, 38, EGUI_VIEW_RELATIVE_PANEL_TONE_ACCENT, 1,
         EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE},
        {"OPS", "Status", "Right rail", "Right of Hero", 126, 8, 66, 48, EGUI_VIEW_RELATIVE_PANEL_TONE_SUCCESS, 0, 0},
        {"DOC", "Shelf", "Below hero", "Below Hero", 18, 62, 118, 50, EGUI_VIEW_RELATIVE_PANEL_TONE_NEUTRAL, 0, 0},
        {"QA", "Pin", "Below rail", "Below Status rail", 142, 82, 50, 40, EGUI_VIEW_RELATIVE_PANEL_TONE_WARNING, 0, 1},
};

static const egui_view_relative_panel_snapshot_t relative_panel_snapshots[] = {
        {"OPS", "Anchored summary", "Summary", "Footer", relative_panel_items_a, 4, 1, 192, 152},
        {"REL", "Status rail", "Summary", "Footer", relative_panel_items_b, 4, 2, 192, 152},
};

static const egui_view_relative_panel_snapshot_t relative_panel_overflow_snapshots[] = {
        {"A", "A", "A", "A", relative_panel_items_a, 4, 3, 192, 152},
        {"B", "B", "B", "B", relative_panel_items_a, 4, 1, 192, 152},
        {"C", "C", "C", "C", relative_panel_items_a, 4, 1, 192, 152},
        {"D", "D", "D", "D", relative_panel_items_a, 4, 1, 192, 152},
        {"E", "E", "E", "E", relative_panel_items_a, 4, 1, 192, 152},
        {"F", "F", "F", "F", relative_panel_items_a, 4, 1, 192, 152},
        {"G", "G", "G", "G", relative_panel_items_a, 4, 1, 192, 152},
};

static const egui_view_relative_panel_snapshot_t relative_panel_preview_snapshot = {
        "UI", "Compact preview", "", "Preview", relative_panel_items_a, 4, 0, 192, 152};

static void relative_panel_on_action(egui_view_t *self, uint8_t snapshot_index, uint8_t item_index)
{
    EGUI_UNUSED(self);
    relative_panel_action_count++;
    relative_panel_action_snapshot = snapshot_index;
    relative_panel_action_item = item_index;
}

static void relative_panel_reset_action_state(void)
{
    relative_panel_action_count = 0;
    relative_panel_action_snapshot = 0xFF;
    relative_panel_action_item = EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE;
}

static void relative_panel_setup_widget(const egui_view_relative_panel_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_relative_panel_init(EGUI_VIEW_OF(&relative_panel_test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&relative_panel_test_widget), 196, 160);
    egui_view_relative_panel_set_snapshots(EGUI_VIEW_OF(&relative_panel_test_widget), snapshots, snapshot_count);
    egui_view_relative_panel_set_font(EGUI_VIEW_OF(&relative_panel_test_widget), NULL);
    egui_view_relative_panel_set_meta_font(EGUI_VIEW_OF(&relative_panel_test_widget), NULL);
    egui_view_relative_panel_set_on_action_listener(EGUI_VIEW_OF(&relative_panel_test_widget), relative_panel_on_action);
    relative_panel_reset_action_state();
}

static void relative_panel_setup_preview_widget(void)
{
    egui_view_relative_panel_init(EGUI_VIEW_OF(&relative_panel_preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&relative_panel_preview_widget), 104, 88);
    egui_view_relative_panel_set_snapshots(EGUI_VIEW_OF(&relative_panel_preview_widget), &relative_panel_preview_snapshot, 1);
    egui_view_relative_panel_set_font(EGUI_VIEW_OF(&relative_panel_preview_widget), NULL);
    egui_view_relative_panel_set_meta_font(EGUI_VIEW_OF(&relative_panel_preview_widget), NULL);
    egui_view_relative_panel_set_compact_mode(EGUI_VIEW_OF(&relative_panel_preview_widget), 1);
    egui_view_relative_panel_override_static_preview_api(EGUI_VIEW_OF(&relative_panel_preview_widget), &relative_panel_preview_api);
    relative_panel_reset_action_state();
}

static void relative_panel_layout_view(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(view, &region);
    egui_region_copy(&view->region_screen, &region);
}

static void relative_panel_layout_widget(void)
{
    relative_panel_layout_view(EGUI_VIEW_OF(&relative_panel_test_widget), 10, 20, 196, 160);
}

static void relative_panel_layout_preview_widget(void)
{
    relative_panel_layout_view(EGUI_VIEW_OF(&relative_panel_preview_widget), 12, 18, 104, 88);
}

static int relative_panel_send_touch_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int relative_panel_send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static int relative_panel_send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return relative_panel_send_touch_to_view(EGUI_VIEW_OF(&relative_panel_test_widget), type, x, y);
}

static int relative_panel_send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return relative_panel_send_touch_to_view(EGUI_VIEW_OF(&relative_panel_preview_widget), type, x, y);
}

static int relative_panel_send_key_action(uint8_t type, uint8_t key_code)
{
    return relative_panel_send_key_to_view(EGUI_VIEW_OF(&relative_panel_test_widget), type, key_code);
}

static int relative_panel_send_preview_key_action(uint8_t type, uint8_t key_code)
{
    return relative_panel_send_key_to_view(EGUI_VIEW_OF(&relative_panel_preview_widget), type, key_code);
}

static int relative_panel_send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= relative_panel_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= relative_panel_send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int relative_panel_send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= relative_panel_send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= relative_panel_send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t relative_panel_get_item_center(egui_view_t *view, uint8_t item_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_relative_panel_get_item_region(view, item_index, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void relative_panel_get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 4;
    *y = view->region_screen.location.y - 4;
}

static void relative_panel_seed_pressed_state(egui_view_relative_panel_t *widget, uint8_t item_index, uint8_t rule_pressed, uint8_t visual_pressed)
{
    widget->pressed_item = item_index;
    widget->pressed_rule = rule_pressed ? 1 : 0;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void relative_panel_assert_pressed_cleared(egui_view_relative_panel_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE, widget->pressed_item);
    EGUI_TEST_ASSERT_EQUAL_INT(0, widget->pressed_rule);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_relative_panel_set_snapshots_clamp_and_defaults(void)
{
    relative_panel_setup_widget(relative_panel_overflow_snapshots, EGUI_ARRAY_SIZE(relative_panel_overflow_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RELATIVE_PANEL_MAX_SNAPSHOTS, relative_panel_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_relative_panel_get_current_snapshot(EGUI_VIEW_OF(&relative_panel_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));

    relative_panel_seed_pressed_state(&relative_panel_test_widget, 1, 1, 1);
    relative_panel_test_widget.current_snapshot = 4;
    egui_view_relative_panel_set_snapshots(EGUI_VIEW_OF(&relative_panel_test_widget), relative_panel_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, relative_panel_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_relative_panel_get_current_snapshot(EGUI_VIEW_OF(&relative_panel_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);

    egui_view_relative_panel_set_snapshots(EGUI_VIEW_OF(&relative_panel_test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, relative_panel_test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_relative_panel_get_current_snapshot(EGUI_VIEW_OF(&relative_panel_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RELATIVE_PANEL_ITEM_NONE, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));
}

static void test_relative_panel_setters_clear_pressed_and_update_state(void)
{
    relative_panel_setup_widget(relative_panel_snapshots, EGUI_ARRAY_SIZE(relative_panel_snapshots));

    relative_panel_seed_pressed_state(&relative_panel_test_widget, 1, 0, 1);
    egui_view_relative_panel_set_font(EGUI_VIEW_OF(&relative_panel_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(relative_panel_test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);

    relative_panel_seed_pressed_state(&relative_panel_test_widget, 1, 0, 1);
    egui_view_relative_panel_set_meta_font(EGUI_VIEW_OF(&relative_panel_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(relative_panel_test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);

    relative_panel_seed_pressed_state(&relative_panel_test_widget, 1, 0, 1);
    egui_view_relative_panel_set_compact_mode(EGUI_VIEW_OF(&relative_panel_test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, relative_panel_test_widget.compact_mode);
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);
    egui_view_relative_panel_set_compact_mode(EGUI_VIEW_OF(&relative_panel_test_widget), 0);

    relative_panel_seed_pressed_state(&relative_panel_test_widget, 1, 0, 1);
    egui_view_relative_panel_set_read_only_mode(EGUI_VIEW_OF(&relative_panel_test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, relative_panel_test_widget.read_only_mode);
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);
    egui_view_relative_panel_set_read_only_mode(EGUI_VIEW_OF(&relative_panel_test_widget), 0);

    relative_panel_seed_pressed_state(&relative_panel_test_widget, 1, 1, 1);
    egui_view_relative_panel_set_palette(EGUI_VIEW_OF(&relative_panel_test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122),
                                         EGUI_COLOR_HEX(0x303132), EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162),
                                         EGUI_COLOR_HEX(0x707172));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, relative_panel_test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, relative_panel_test_widget.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, relative_panel_test_widget.border_color.full);
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);

    egui_view_relative_panel_set_current_snapshot(EGUI_VIEW_OF(&relative_panel_test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_relative_panel_get_current_snapshot(EGUI_VIEW_OF(&relative_panel_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));

    relative_panel_seed_pressed_state(&relative_panel_test_widget, 2, 0, 1);
    egui_view_relative_panel_set_current_item(EGUI_VIEW_OF(&relative_panel_test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);
}

static void test_relative_panel_regions_activate_and_listener(void)
{
    egui_dim_t x;
    egui_dim_t y;

    relative_panel_setup_widget(relative_panel_snapshots, EGUI_ARRAY_SIZE(relative_panel_snapshots));
    relative_panel_layout_widget();

    EGUI_TEST_ASSERT_TRUE(relative_panel_get_item_center(EGUI_VIEW_OF(&relative_panel_test_widget), 2, &x, &y));
    egui_view_relative_panel_set_current_item(EGUI_VIEW_OF(&relative_panel_test_widget), 2);
    EGUI_TEST_ASSERT_TRUE(egui_view_relative_panel_activate_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, relative_panel_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, relative_panel_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, relative_panel_action_item);

    relative_panel_reset_action_state();
    egui_view_relative_panel_set_on_action_listener(EGUI_VIEW_OF(&relative_panel_test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_relative_panel_activate_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, relative_panel_action_count);
}

static void test_relative_panel_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    relative_panel_setup_widget(relative_panel_snapshots, EGUI_ARRAY_SIZE(relative_panel_snapshots));
    relative_panel_layout_widget();
    EGUI_TEST_ASSERT_TRUE(relative_panel_get_item_center(EGUI_VIEW_OF(&relative_panel_test_widget), 0, &item_x, &item_y));
    relative_panel_get_view_outside_point(EGUI_VIEW_OF(&relative_panel_test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, relative_panel_test_widget.pressed_item);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&relative_panel_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&relative_panel_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, relative_panel_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);

    EGUI_TEST_ASSERT_TRUE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&relative_panel_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_UP, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, relative_panel_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);

    relative_panel_reset_action_state();
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, relative_panel_action_count);
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);
}

static void test_relative_panel_key_navigation_rule_focus_and_snapshot_cycle(void)
{
    relative_panel_setup_widget(relative_panel_snapshots, EGUI_ARRAY_SIZE(relative_panel_snapshots));
    relative_panel_layout_widget();

    EGUI_TEST_ASSERT_TRUE(relative_panel_send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));

    EGUI_TEST_ASSERT_TRUE(relative_panel_send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));

    EGUI_TEST_ASSERT_TRUE(relative_panel_send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));

    EGUI_TEST_ASSERT_TRUE(relative_panel_send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, relative_panel_test_widget.focus_on_rule);

    EGUI_TEST_ASSERT_TRUE(relative_panel_send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, relative_panel_test_widget.pressed_rule);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&relative_panel_test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_relative_panel_get_current_snapshot(EGUI_VIEW_OF(&relative_panel_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, relative_panel_test_widget.focus_on_rule);
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);
}

static void test_relative_panel_read_only_and_disabled_guards(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;

    relative_panel_setup_widget(relative_panel_snapshots, EGUI_ARRAY_SIZE(relative_panel_snapshots));
    relative_panel_layout_widget();
    EGUI_TEST_ASSERT_TRUE(relative_panel_get_item_center(EGUI_VIEW_OF(&relative_panel_test_widget), 1, &item_x, &item_y));

    relative_panel_seed_pressed_state(&relative_panel_test_widget, 1, 1, 1);
    egui_view_relative_panel_set_read_only_mode(EGUI_VIEW_OF(&relative_panel_test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_FALSE(relative_panel_send_key(EGUI_KEY_CODE_ENTER));
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);
    egui_view_relative_panel_set_read_only_mode(EGUI_VIEW_OF(&relative_panel_test_widget), 0);

    relative_panel_seed_pressed_state(&relative_panel_test_widget, 1, 1, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&relative_panel_test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(relative_panel_send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_FALSE(relative_panel_send_key(EGUI_KEY_CODE_ENTER));
    relative_panel_assert_pressed_cleared(&relative_panel_test_widget);
    egui_view_set_enable(EGUI_VIEW_OF(&relative_panel_test_widget), 1);
}

static void test_relative_panel_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;

    relative_panel_setup_preview_widget();
    relative_panel_layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(relative_panel_get_item_center(EGUI_VIEW_OF(&relative_panel_preview_widget), 0, &item_x, &item_y));

    relative_panel_seed_pressed_state(&relative_panel_preview_widget, 0, 1, 1);
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    relative_panel_assert_pressed_cleared(&relative_panel_preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, relative_panel_action_count);

    relative_panel_seed_pressed_state(&relative_panel_preview_widget, 0, 1, 1);
    EGUI_TEST_ASSERT_TRUE(relative_panel_send_preview_key(EGUI_KEY_CODE_ENTER));
    relative_panel_assert_pressed_cleared(&relative_panel_preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_relative_panel_get_current_item(EGUI_VIEW_OF(&relative_panel_preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, relative_panel_action_count);
}

void test_relative_panel_run(void)
{
    EGUI_TEST_SUITE_BEGIN(relative_panel);
    EGUI_TEST_RUN(test_relative_panel_set_snapshots_clamp_and_defaults);
    EGUI_TEST_RUN(test_relative_panel_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_relative_panel_regions_activate_and_listener);
    EGUI_TEST_RUN(test_relative_panel_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_relative_panel_key_navigation_rule_focus_and_snapshot_cycle);
    EGUI_TEST_RUN(test_relative_panel_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_relative_panel_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
