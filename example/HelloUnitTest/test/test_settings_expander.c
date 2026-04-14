#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_settings_expander.h"

#include "../../HelloCustomWidgets/layout/settings_expander/egui_view_settings_expander.h"
#include "../../HelloCustomWidgets/layout/settings_expander/egui_view_settings_expander.c"

static egui_view_settings_expander_t test_widget;
static egui_view_settings_expander_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_action_count;
static uint8_t g_action_snapshot;
static uint8_t g_action_part;

static const egui_view_settings_expander_row_t g_rows_backup[] = {
        {"WF", "Wi-Fi only", NULL, EGUI_VIEW_SETTINGS_EXPANDER_TONE_ACCENT, 1, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_SWITCH_ON},
        {"BT", "Battery saver", "Pause", EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL, 0, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_VALUE},
        {"HM", "History", "30d", EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL, 0, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_VALUE},
};

static const egui_view_settings_expander_row_t g_rows_sharing[] = {
        {"AB", "Audience", "Team", EGUI_VIEW_SETTINGS_EXPANDER_TONE_SUCCESS, 1, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_VALUE},
        {"AD", "Advanced", NULL, EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL, 0, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_CHEVRON},
};

static const egui_view_settings_expander_row_t g_rows_quiet[] = {
        {"QS", "Quiet hours", "Manual", EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL, 0, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_VALUE},
        {"SN", "Summary", NULL, EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL, 0, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_SWITCH_ON},
};

static const egui_view_settings_expander_row_t g_rows_overflow[] = {
        {"A", "Alpha", "1", EGUI_VIEW_SETTINGS_EXPANDER_TONE_ACCENT, 0, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_VALUE},
        {"B", "Beta", "2", EGUI_VIEW_SETTINGS_EXPANDER_TONE_SUCCESS, 1, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_SWITCH_ON},
        {"C", "Gamma", "3", EGUI_VIEW_SETTINGS_EXPANDER_TONE_WARNING, 0, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_VALUE},
        {"D", "Delta", NULL, EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL, 0, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_CHEVRON},
        {"E", "Echo", "5", EGUI_VIEW_SETTINGS_EXPANDER_TONE_ACCENT, 0, EGUI_VIEW_SETTINGS_EXPANDER_TRAILING_VALUE},
};

static const egui_view_settings_expander_snapshot_t g_snapshots[] = {
        {"SYNC", "BK", "Backup options", "Keep sync rules under one setting.", "On", "Metered uploads wait for Wi-Fi.", g_rows_backup, 3,
         EGUI_VIEW_SETTINGS_EXPANDER_TONE_ACCENT, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE},
        {"SHARE", "SH", "Sharing scope", "Audience and advanced rules stay grouped.", "Team", "Review external access before publish.", g_rows_sharing, 2,
         EGUI_VIEW_SETTINGS_EXPANDER_TONE_SUCCESS, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1},
        {"ALERTS", "NT", "Quiet hours", "Header can collapse when details are not needed.", "Manual", "", g_rows_quiet, 2,
         EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL, 0, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE},
        {"UPDATE", "UP", "Rollout cadence", "Warning tone keeps pilot controls visible.", "Pilot", "Manual approval stays required.", g_rows_overflow, 5,
         EGUI_VIEW_SETTINGS_EXPANDER_TONE_WARNING, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 3},
};

static const egui_view_settings_expander_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "A", "A", "A", g_rows_overflow, 5, EGUI_VIEW_SETTINGS_EXPANDER_TONE_ACCENT, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE},
        {"B", "B", "B", "B", "B", "B", g_rows_overflow, 5, EGUI_VIEW_SETTINGS_EXPANDER_TONE_SUCCESS, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1},
        {"C", "C", "C", "C", "C", "C", g_rows_overflow, 5, EGUI_VIEW_SETTINGS_EXPANDER_TONE_WARNING, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 2},
        {"D", "D", "D", "D", "D", "D", g_rows_overflow, 5, EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 3},
        {"E", "E", "E", "E", "E", "E", g_rows_overflow, 5, EGUI_VIEW_SETTINGS_EXPANDER_TONE_ACCENT, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE},
        {"F", "F", "F", "F", "F", "F", g_rows_overflow, 5, EGUI_VIEW_SETTINGS_EXPANDER_TONE_SUCCESS, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1},
        {"G", "G", "G", "G", "G", "G", g_rows_overflow, 5, EGUI_VIEW_SETTINGS_EXPANDER_TONE_WARNING, 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 2},
};

static void on_action(egui_view_t *self, uint8_t snapshot_index, uint8_t part)
{
    EGUI_UNUSED(self);
    g_action_count++;
    g_action_snapshot = snapshot_index;
    g_action_part = part;
}

static void reset_action_state(void)
{
    g_action_count = 0;
    g_action_snapshot = 0xFF;
    g_action_part = EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE;
}

static void setup_widget(const egui_view_settings_expander_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_settings_expander_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 196, 146);
    egui_view_settings_expander_set_snapshots(EGUI_VIEW_OF(&test_widget), snapshots, snapshot_count);
    egui_view_settings_expander_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_settings_expander_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_settings_expander_set_on_action_listener(EGUI_VIEW_OF(&test_widget), on_action);
    reset_action_state();
}

static void setup_preview_widget(void)
{
    egui_view_settings_expander_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 88);
    egui_view_settings_expander_set_snapshots(EGUI_VIEW_OF(&preview_widget), &g_snapshots[0], 1);
    egui_view_settings_expander_set_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_settings_expander_set_meta_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_settings_expander_set_on_action_listener(EGUI_VIEW_OF(&preview_widget), on_action);
    egui_view_settings_expander_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_settings_expander_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
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
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 20, 196, 146);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 104, 88);
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

static uint8_t get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_settings_expander_get_part_region(view, part, &region))
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

static void seed_pressed_state(egui_view_settings_expander_t *widget, uint8_t part, uint8_t visual_pressed)
{
    widget->pressed_part = part;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_settings_expander_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_settings_expander_set_snapshots_and_current_part_clamp(void)
{
    setup_widget(g_overflow_snapshots, EGUI_ARRAY_SIZE(g_overflow_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_MAX_SNAPSHOTS, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1, 1);
    test_widget.current_snapshot = 5;
    egui_view_settings_expander_set_snapshots(EGUI_VIEW_OF(&test_widget), g_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_settings_expander_set_snapshots(EGUI_VIEW_OF(&test_widget), g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    egui_view_settings_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    egui_view_settings_expander_set_current_part(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    egui_view_settings_expander_set_snapshots(EGUI_VIEW_OF(&test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
}

static void test_settings_expander_setters_clear_pressed_and_update_state(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    seed_pressed_state(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, 1);
    egui_view_settings_expander_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1, 0);
    egui_view_settings_expander_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 2, 1);
    egui_view_settings_expander_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.compact_mode);
    assert_pressed_cleared(&test_widget);
    egui_view_settings_expander_set_compact_mode(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.compact_mode);

    seed_pressed_state(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 2, 1);
    egui_view_settings_expander_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    assert_pressed_cleared(&test_widget);
    egui_view_settings_expander_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.read_only_mode);

    seed_pressed_state(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, 1);
    egui_view_settings_expander_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                            EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162),
                                            EGUI_COLOR_HEX(0x707172), EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_widget.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, test_widget.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, test_widget.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, test_widget.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x909192).full, test_widget.neutral_color.full);
    assert_pressed_cleared(&test_widget);

    egui_view_settings_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    seed_pressed_state(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1, 1);
    egui_view_settings_expander_set_expanded(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, 1);
    egui_view_settings_expander_set_expanded(EGUI_VIEW_OF(&test_widget), 1);
    egui_view_settings_expander_set_current_part(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);
}

static void test_settings_expander_metrics_hit_testing_and_regions(void)
{
    egui_view_settings_expander_metrics_t metrics;
    egui_dim_t x;
    egui_dim_t y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    egui_view_settings_expander_get_metrics(&test_widget, EGUI_VIEW_OF(&test_widget), &g_snapshots[0], &metrics);

    EGUI_TEST_ASSERT_TRUE(metrics.header_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.body_region.size.height > 0);
    EGUI_TEST_ASSERT_EQUAL_INT(3, metrics.visible_row_count);
    EGUI_TEST_ASSERT_TRUE(metrics.footer_region.size.height > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.row_regions[1].location.y > metrics.row_regions[0].location.y);

    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER,
                               egui_view_settings_expander_resolve_hit(&test_widget, EGUI_VIEW_OF(&test_widget), x, y));

    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE,
                               egui_view_settings_expander_resolve_hit(&test_widget, EGUI_VIEW_OF(&test_widget), x, y));

    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1, &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1,
                               egui_view_settings_expander_resolve_hit(&test_widget, EGUI_VIEW_OF(&test_widget), x, y));

    egui_view_settings_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 3);
    layout_widget();
    egui_view_settings_expander_get_metrics(&test_widget, EGUI_VIEW_OF(&test_widget), &g_snapshots[3], &metrics);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS, metrics.row_count);
    EGUI_TEST_ASSERT_TRUE(metrics.visible_row_count <= EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS);

    egui_view_settings_expander_set_compact_mode(EGUI_VIEW_OF(&test_widget), 1);
    layout_widget();
    egui_view_settings_expander_get_metrics(&test_widget, EGUI_VIEW_OF(&test_widget), &g_snapshots[3], &metrics);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.footer_region.size.height);

    egui_view_settings_expander_set_compact_mode(EGUI_VIEW_OF(&test_widget), 0);
    egui_view_settings_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 2);
    layout_widget();
    EGUI_TEST_ASSERT_FALSE(egui_view_settings_expander_get_part_region(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, &metrics.region));
    EGUI_TEST_ASSERT_TRUE(egui_view_settings_expander_get_part_region(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, &metrics.region));
}

static void test_settings_expander_activate_current_part_and_listener(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    egui_view_settings_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(egui_view_settings_expander_activate_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1, g_action_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));

    reset_action_state();
    egui_view_settings_expander_set_current_part(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER);
    EGUI_TEST_ASSERT_TRUE(egui_view_settings_expander_activate_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(egui_view_settings_expander_activate_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    egui_view_settings_expander_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_settings_expander_activate_current_part(EGUI_VIEW_OF(&test_widget)));
}

static void test_settings_expander_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t row0_x;
    egui_dim_t row0_y;
    egui_dim_t row1_x;
    egui_dim_t row1_y;
    egui_dim_t header_x;
    egui_dim_t header_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, &row0_x, &row0_y));
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 1, &row1_x, &row1_y));
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, &header_x, &header_y));
    get_view_outside_point(EGUI_VIEW_OF(&test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row0_x, row0_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, row1_x, row1_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, row1_x, row1_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row0_x, row0_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, row0_x, row0_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, row0_x, row0_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, g_action_part);
    assert_pressed_cleared(&test_widget);

    reset_action_state();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header_x, header_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header_x, header_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, header_x, header_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_widget);
}

static void test_settings_expander_key_navigation_and_activation(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 2, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, g_action_part);
    assert_pressed_cleared(&test_widget);

    reset_action_state();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_FALSE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ESCAPE));
    egui_view_settings_expander_set_expanded(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
}

static void test_settings_expander_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t row_x;
    egui_dim_t row_y;
    uint8_t initial_snapshot;
    uint8_t initial_part;
    uint8_t initial_expanded;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    egui_view_settings_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    layout_widget();
    initial_snapshot = egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget));
    initial_part = egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget));
    initial_expanded = egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget));
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), initial_part, &row_x, &row_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row_x, row_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    egui_view_settings_expander_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row_x, row_y));

    seed_pressed_state(&test_widget, initial_part, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_HOME));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_part, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_expanded, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_settings_expander_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_expanded, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_part, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_part, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_part, g_action_part);

    reset_action_state();
    egui_view_settings_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), initial_snapshot);
    egui_view_settings_expander_set_expanded(EGUI_VIEW_OF(&test_widget), initial_expanded);
    egui_view_settings_expander_set_current_part(EGUI_VIEW_OF(&test_widget), initial_part);
    initial_snapshot = egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget));
    initial_part = egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget));
    initial_expanded = egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget));
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, initial_part, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row_x, row_y));
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, initial_part, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_END));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_part, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_expanded, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 1);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_part, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_part, g_action_part);
}

static void test_settings_expander_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t row0_x;
    egui_dim_t row0_y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&preview_widget), EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, &row0_x, &row0_y));

    preview_widget.current_part = EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE;
    preview_widget.expanded_state = 1;
    seed_pressed_state(&preview_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, row0_x, row0_y));
    assert_pressed_cleared(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    preview_widget.current_part = EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER;
    preview_widget.expanded_state = 1;
    seed_pressed_state(&preview_widget, EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_get_expanded(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER, egui_view_settings_expander_get_current_part(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_settings_expander_internal_helpers(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    egui_view_settings_expander_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x111111), EGUI_COLOR_HEX(0x222222), EGUI_COLOR_HEX(0x333333),
                                            EGUI_COLOR_HEX(0x444444), EGUI_COLOR_HEX(0x555555), EGUI_COLOR_HEX(0x666666),
                                            EGUI_COLOR_HEX(0x777777), EGUI_COLOR_HEX(0x888888), EGUI_COLOR_HEX(0x999999));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_MAX_SNAPSHOTS, egui_view_settings_expander_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS, egui_view_settings_expander_clamp_row_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_settings_expander_text_len("Review"));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_settings_expander_has_text(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_settings_expander_has_text("A"));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full,
                               egui_view_settings_expander_tone_color(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x777777).full,
                               egui_view_settings_expander_tone_color(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x888888).full,
                               egui_view_settings_expander_tone_color(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x999999).full,
                               egui_view_settings_expander_tone_color(&test_widget, EGUI_VIEW_SETTINGS_EXPANDER_TONE_NEUTRAL).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x666666).full, egui_view_settings_expander_tone_color(&test_widget, 99).full);
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_settings_expander_pill_width("", 1, 24, 60));
    EGUI_TEST_ASSERT_EQUAL_INT(36, egui_view_settings_expander_pill_width("AB", 0, 26, 64));
    EGUI_TEST_ASSERT_EQUAL_INT(32, egui_view_settings_expander_pill_width("Long label", 1, 20, 32));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_MAX_ROWS,
                               egui_view_settings_expander_part_to_row_index(EGUI_VIEW_SETTINGS_EXPANDER_PART_HEADER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 2, egui_view_settings_expander_row_part(2));
    EGUI_TEST_ASSERT_TRUE(egui_view_settings_expander_part_exists(&g_snapshots[0], 1, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 2));
    EGUI_TEST_ASSERT_FALSE(egui_view_settings_expander_part_exists(&g_snapshots[2], 0, EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_ROW_BASE + 3,
                               egui_view_settings_expander_find_last_row(&g_snapshots[3], 1));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SETTINGS_EXPANDER_PART_NONE, egui_view_settings_expander_find_last_row(&g_snapshots[2], 0));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, egui_view_settings_expander_mix_disabled(sample).full);
}

void test_settings_expander_run(void)
{
    EGUI_TEST_SUITE_BEGIN(settings_expander);
    EGUI_TEST_RUN(test_settings_expander_set_snapshots_and_current_part_clamp);
    EGUI_TEST_RUN(test_settings_expander_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_settings_expander_metrics_hit_testing_and_regions);
    EGUI_TEST_RUN(test_settings_expander_activate_current_part_and_listener);
    EGUI_TEST_RUN(test_settings_expander_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_settings_expander_key_navigation_and_activation);
    EGUI_TEST_RUN(test_settings_expander_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_settings_expander_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_settings_expander_internal_helpers);
    EGUI_TEST_SUITE_END();
}
