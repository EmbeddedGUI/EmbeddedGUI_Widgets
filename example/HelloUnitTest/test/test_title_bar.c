#include <string.h>

#include "egui.h"
#include "resource/egui_icon_material_symbols.h"
#include "test/egui_test.h"
#include "test_title_bar.h"

#include "../../HelloCustomWidgets/navigation/title_bar/egui_view_title_bar.h"
#include "../../HelloCustomWidgets/navigation/title_bar/egui_view_title_bar.c"

static egui_view_title_bar_t test_title_bar;
static egui_view_title_bar_t preview_title_bar;
static egui_view_api_t preview_api;
static uint8_t g_action_part = EGUI_VIEW_TITLE_BAR_PART_NONE;
static uint8_t g_action_snapshot = 0xFF;
static uint8_t g_action_count = 0;

static const egui_view_title_bar_snapshot_t g_primary_snapshots[] = {
        {EGUI_ICON_MS_HOME, "Workspace", "Atlas", "Weekly review", "Live", "Share", "More", 1, 1, 1},
        {EGUI_ICON_MS_SETTINGS, "Files", "Review", "Compact state", "Pane", "Open", "", 0, 1, 0},
        {EGUI_ICON_MS_INFO, "Audit", "Read back", "Frozen shell", "Ready", "Done", "Reset", 1, 0, 1},
};

static const egui_view_title_bar_snapshot_t g_preview_snapshot[] = {
        {EGUI_ICON_MS_HOME, NULL, "Atlas", NULL, NULL, "Sync", NULL, 1, 0, 1},
};

static void on_action(egui_view_t *self, uint8_t part, uint8_t snapshot_index)
{
    EGUI_UNUSED(self);
    g_action_part = part;
    g_action_snapshot = snapshot_index;
    g_action_count++;
}

static void reset_action_state(void)
{
    g_action_part = EGUI_VIEW_TITLE_BAR_PART_NONE;
    g_action_snapshot = 0xFF;
    g_action_count = 0;
}

static void setup_title_bar(const egui_view_title_bar_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_title_bar_init(EGUI_VIEW_OF(&test_title_bar));
    egui_view_set_size(EGUI_VIEW_OF(&test_title_bar), 196, 96);
    egui_view_title_bar_set_snapshots(EGUI_VIEW_OF(&test_title_bar), snapshots, snapshot_count);
    egui_view_title_bar_set_font(EGUI_VIEW_OF(&test_title_bar), NULL);
    egui_view_title_bar_set_meta_font(EGUI_VIEW_OF(&test_title_bar), NULL);
    egui_view_title_bar_set_icon_font(EGUI_VIEW_OF(&test_title_bar), EGUI_FONT_ICON_MS_20);
    egui_view_title_bar_set_on_action_listener(EGUI_VIEW_OF(&test_title_bar), on_action);
    reset_action_state();
}

static void setup_preview_title_bar(void)
{
    egui_view_title_bar_init(EGUI_VIEW_OF(&preview_title_bar));
    egui_view_set_size(EGUI_VIEW_OF(&preview_title_bar), 104, 72);
    egui_view_title_bar_set_snapshots(EGUI_VIEW_OF(&preview_title_bar), g_preview_snapshot, 1);
    egui_view_title_bar_set_font(EGUI_VIEW_OF(&preview_title_bar), NULL);
    egui_view_title_bar_set_meta_font(EGUI_VIEW_OF(&preview_title_bar), NULL);
    egui_view_title_bar_set_icon_font(EGUI_VIEW_OF(&preview_title_bar), EGUI_FONT_ICON_MS_16);
    egui_view_title_bar_set_compact_mode(EGUI_VIEW_OF(&preview_title_bar), 1);
    egui_view_title_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_title_bar), &preview_api);
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

static void layout_title_bar(void)
{
    layout_view(EGUI_VIEW_OF(&test_title_bar), 10, 20, 196, 96);
}

static void layout_preview_title_bar(void)
{
    layout_view(EGUI_VIEW_OF(&preview_title_bar), 12, 18, 104, 72);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_title_bar), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_title_bar), type, x, y);
}

static int send_key(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_title_bar), type, key_code);
}

static int send_preview_key(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_title_bar), type, key_code);
}

static uint8_t get_part_center_for_view(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_title_bar_get_part_region(view, part, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void seed_pressed_state(egui_view_title_bar_t *bar, uint8_t part, uint8_t visual_pressed)
{
    egui_view_set_pressed(EGUI_VIEW_OF(bar), 0);
    bar->pressed_part = part;
    if (visual_pressed)
    {
        egui_view_set_pressed(EGUI_VIEW_OF(bar), 1);
    }
}

static void assert_pressed_cleared(egui_view_title_bar_t *bar)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_NONE, bar->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(bar)->is_pressed);
}

static void test_title_bar_set_snapshots_and_current_part_clamp(void)
{
    setup_title_bar(g_primary_snapshots, 3);

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_title_bar_get_current_snapshot(EGUI_VIEW_OF(&test_title_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));

    egui_view_title_bar_set_current_snapshot(EGUI_VIEW_OF(&test_title_bar), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_title_bar_get_current_snapshot(EGUI_VIEW_OF(&test_title_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));

    egui_view_title_bar_set_current_part(EGUI_VIEW_OF(&test_title_bar), EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));

    egui_view_title_bar_set_snapshots(EGUI_VIEW_OF(&test_title_bar), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_title_bar_get_current_snapshot(EGUI_VIEW_OF(&test_title_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_NONE, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));
}

static void test_title_bar_setters_clear_pressed_state(void)
{
    setup_title_bar(g_primary_snapshots, 3);

    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_BACK, 1);
    egui_view_title_bar_set_font(EGUI_VIEW_OF(&test_title_bar), NULL);
    assert_pressed_cleared(&test_title_bar);

    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, 0);
    egui_view_title_bar_set_meta_font(EGUI_VIEW_OF(&test_title_bar), NULL);
    assert_pressed_cleared(&test_title_bar);

    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION, 1);
    egui_view_title_bar_set_icon_font(EGUI_VIEW_OF(&test_title_bar), EGUI_FONT_ICON_MS_16);
    assert_pressed_cleared(&test_title_bar);

    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION, 1);
    egui_view_title_bar_set_palette(EGUI_VIEW_OF(&test_title_bar), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD4DCE4), EGUI_COLOR_HEX(0x1C2935),
                                    EGUI_COLOR_HEX(0x6D7B89), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xF4F7FA), EGUI_COLOR_HEX(0xD7DFE7),
                                    EGUI_COLOR_HEX(0xDCE4EB));
    assert_pressed_cleared(&test_title_bar);

    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION, 1);
    egui_view_title_bar_set_current_snapshot(EGUI_VIEW_OF(&test_title_bar), 1);
    assert_pressed_cleared(&test_title_bar);

    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, 1);
    egui_view_title_bar_set_current_part(EGUI_VIEW_OF(&test_title_bar), EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION);
    assert_pressed_cleared(&test_title_bar);

    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION, 1);
    egui_view_title_bar_set_compact_mode(EGUI_VIEW_OF(&test_title_bar), 1);
    assert_pressed_cleared(&test_title_bar);

    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION, 0);
    egui_view_title_bar_set_read_only_mode(EGUI_VIEW_OF(&test_title_bar), 1);
    assert_pressed_cleared(&test_title_bar);
}

static void test_title_bar_metrics_and_hit_testing(void)
{
    egui_view_title_bar_metrics_t metrics;
    egui_dim_t x;
    egui_dim_t y;

    setup_title_bar(g_primary_snapshots, 3);
    layout_title_bar();
    egui_view_title_bar_get_metrics(&test_title_bar, EGUI_VIEW_OF(&test_title_bar), &g_primary_snapshots[0], &metrics);

    EGUI_TEST_ASSERT_TRUE(metrics.show_leading_header);
    EGUI_TEST_ASSERT_TRUE(metrics.show_trailing_header);
    EGUI_TEST_ASSERT_TRUE(metrics.show_primary_action);
    EGUI_TEST_ASSERT_TRUE(metrics.show_secondary_action);
    EGUI_TEST_ASSERT_TRUE(metrics.title_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.subtitle_region.location.y > metrics.title_region.location.y);

    EGUI_TEST_ASSERT_TRUE(get_part_center_for_view(EGUI_VIEW_OF(&test_title_bar), EGUI_VIEW_TITLE_BAR_PART_BACK, &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_resolve_hit(&test_title_bar, EGUI_VIEW_OF(&test_title_bar), x, y));

    EGUI_TEST_ASSERT_TRUE(get_part_center_for_view(EGUI_VIEW_OF(&test_title_bar), EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, egui_view_title_bar_resolve_hit(&test_title_bar, EGUI_VIEW_OF(&test_title_bar), x, y));

    EGUI_TEST_ASSERT_TRUE(get_part_center_for_view(EGUI_VIEW_OF(&test_title_bar), EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION, &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION, egui_view_title_bar_resolve_hit(&test_title_bar, EGUI_VIEW_OF(&test_title_bar), x, y));

    EGUI_TEST_ASSERT_TRUE(get_part_center_for_view(EGUI_VIEW_OF(&test_title_bar), EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION, &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION, egui_view_title_bar_resolve_hit(&test_title_bar, EGUI_VIEW_OF(&test_title_bar), x, y));

    egui_view_title_bar_set_compact_mode(EGUI_VIEW_OF(&test_title_bar), 1);
    layout_title_bar();
    egui_view_title_bar_get_metrics(&test_title_bar, EGUI_VIEW_OF(&test_title_bar), &g_primary_snapshots[0], &metrics);
    EGUI_TEST_ASSERT_FALSE(metrics.show_leading_header);
    EGUI_TEST_ASSERT_FALSE(metrics.show_trailing_header);
    EGUI_TEST_ASSERT_FALSE(metrics.show_subtitle);
}

static void test_title_bar_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t x_back;
    egui_dim_t y_back;
    egui_dim_t x_pane;
    egui_dim_t y_pane;

    setup_title_bar(g_primary_snapshots, 3);
    layout_title_bar();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_view(EGUI_VIEW_OF(&test_title_bar), EGUI_VIEW_TITLE_BAR_PART_BACK, &x_back, &y_back));
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_view(EGUI_VIEW_OF(&test_title_bar), EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, &x_pane, &y_pane));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x_back, y_back));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, test_title_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_title_bar)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x_pane, y_pane));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, test_title_bar.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_title_bar)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x_pane, y_pane));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_title_bar);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x_back, y_back));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x_pane, y_pane));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x_back, y_back));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_title_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x_back, y_back));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, g_action_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    assert_pressed_cleared(&test_title_bar);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x_back, y_back));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, test_title_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, x_back, y_back));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    assert_pressed_cleared(&test_title_bar);
}

static void test_title_bar_key_navigation_and_activation(void)
{
    setup_title_bar(g_primary_snapshots, 3);

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_PRIMARY_ACTION, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_SECONDARY_ACTION, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, test_title_bar.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, g_action_part);
    assert_pressed_cleared(&test_title_bar);
}

static void test_title_bar_focus_change_resolves_first_active_part(void)
{
    setup_title_bar(g_primary_snapshots, 3);
    test_title_bar.current_part = EGUI_VIEW_TITLE_BAR_PART_NONE;
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    EGUI_VIEW_OF(&test_title_bar)->api->on_focus_changed(EGUI_VIEW_OF(&test_title_bar), 1);
#endif
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));
}

static void test_title_bar_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x_back;
    egui_dim_t y_back;

    setup_title_bar(g_primary_snapshots, 3);
    layout_title_bar();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_view(EGUI_VIEW_OF(&test_title_bar), EGUI_VIEW_TITLE_BAR_PART_BACK, &x_back, &y_back));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x_back, y_back));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_title_bar)->is_pressed);
    egui_view_title_bar_set_read_only_mode(EGUI_VIEW_OF(&test_title_bar), 1);
    assert_pressed_cleared(&test_title_bar);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x_back, y_back));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_title_bar_set_read_only_mode(EGUI_VIEW_OF(&test_title_bar), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&test_title_bar)));

    egui_view_set_enable(EGUI_VIEW_OF(&test_title_bar), 0);
    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x_back, y_back));
    assert_pressed_cleared(&test_title_bar);

    seed_pressed_state(&test_title_bar, EGUI_VIEW_TITLE_BAR_PART_PANE_TOGGLE, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_LEFT));
    assert_pressed_cleared(&test_title_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_title_bar_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t x_back;
    egui_dim_t y_back;

    setup_preview_title_bar();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_title_bar_get_current_snapshot(EGUI_VIEW_OF(&preview_title_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&preview_title_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_title_bar.compact_mode);
    layout_preview_title_bar();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_view(EGUI_VIEW_OF(&preview_title_bar), EGUI_VIEW_TITLE_BAR_PART_BACK, &x_back, &y_back));

    seed_pressed_state(&preview_title_bar, EGUI_VIEW_TITLE_BAR_PART_BACK, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x_back, y_back));
    assert_pressed_cleared(&preview_title_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_title_bar_get_current_snapshot(EGUI_VIEW_OF(&preview_title_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&preview_title_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    seed_pressed_state(&preview_title_bar, EGUI_VIEW_TITLE_BAR_PART_BACK, 0);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_END));
    assert_pressed_cleared(&preview_title_bar);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_title_bar_get_current_snapshot(EGUI_VIEW_OF(&preview_title_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TITLE_BAR_PART_BACK, egui_view_title_bar_get_current_part(EGUI_VIEW_OF(&preview_title_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

void test_title_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(title_bar);
    EGUI_TEST_RUN(test_title_bar_set_snapshots_and_current_part_clamp);
    EGUI_TEST_RUN(test_title_bar_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_title_bar_metrics_and_hit_testing);
    EGUI_TEST_RUN(test_title_bar_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_title_bar_key_navigation_and_activation);
    EGUI_TEST_RUN(test_title_bar_focus_change_resolves_first_active_part);
    EGUI_TEST_RUN(test_title_bar_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_title_bar_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
