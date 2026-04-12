#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_flyout.h"

#include "../../HelloCustomWidgets/feedback/flyout/egui_view_flyout.h"
#include "../../HelloCustomWidgets/feedback/flyout/egui_view_flyout.c"

static egui_view_flyout_t test_widget;
static egui_view_flyout_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_action_count;
static uint8_t g_action_snapshot;
static uint8_t g_action_part;

static const egui_view_flyout_snapshot_t g_snapshots[] = {
        {"Review", "Flyout", "Review draft", "Keep actions anchored near the current affordance.", "Publish", "Later", "Bottom placement",
         EGUI_VIEW_FLYOUT_TONE_ACCENT, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, -18},
        {"Search", "Top placement", "Find in workspace", "The panel can sit above the target when space is tight.", "Search", "Close", "Top placement",
         EGUI_VIEW_FLYOUT_TONE_SUCCESS, EGUI_VIEW_FLYOUT_PLACEMENT_TOP, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_SECONDARY, 18},
        {"Sync", "Warning", "Reconnect before sending", "Pending changes stay local until sync completes.", "Open sync", "Dismiss", "Warning tone",
         EGUI_VIEW_FLYOUT_TONE_WARNING, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, 0},
        {"Pinned", "Closed", "Tap target to reopen", "", "", "", "", EGUI_VIEW_FLYOUT_TONE_NEUTRAL, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 0, 0, 0,
         EGUI_VIEW_FLYOUT_PART_TARGET, 0},
};

static const egui_view_flyout_snapshot_t g_overflow_snapshots[] = {
        {"A", "", "A", "", "A", "B", "", EGUI_VIEW_FLYOUT_TONE_ACCENT, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, 0},
        {"B", "", "B", "", "A", "B", "", EGUI_VIEW_FLYOUT_TONE_SUCCESS, EGUI_VIEW_FLYOUT_PLACEMENT_TOP, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_SECONDARY, 0},
        {"C", "", "C", "", "A", "B", "", EGUI_VIEW_FLYOUT_TONE_WARNING, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, 0},
        {"D", "", "D", "", "A", "B", "", EGUI_VIEW_FLYOUT_TONE_NEUTRAL, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, 0},
        {"E", "", "E", "", "A", "B", "", EGUI_VIEW_FLYOUT_TONE_ACCENT, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, 0},
        {"F", "", "F", "", "A", "B", "", EGUI_VIEW_FLYOUT_TONE_SUCCESS, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, 0},
        {"G", "", "G", "", "A", "B", "", EGUI_VIEW_FLYOUT_TONE_WARNING, EGUI_VIEW_FLYOUT_PLACEMENT_BOTTOM, 1, 1, 1, EGUI_VIEW_FLYOUT_PART_PRIMARY, 0},
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
    g_action_part = EGUI_VIEW_FLYOUT_PART_NONE;
}

static void setup_widget(const egui_view_flyout_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_flyout_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 196, 132);
    egui_view_flyout_set_snapshots(EGUI_VIEW_OF(&test_widget), snapshots, snapshot_count);
    egui_view_flyout_set_on_action_listener(EGUI_VIEW_OF(&test_widget), on_action);
    reset_action_state();
}

static void setup_preview_widget(void)
{
    egui_view_flyout_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 80);
    egui_view_flyout_set_snapshots(EGUI_VIEW_OF(&preview_widget), &g_snapshots[0], 1);
    egui_view_flyout_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_flyout_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
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
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 20, 196, 132);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 14, 18, 104, 80);
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

static uint8_t get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_flyout_get_part_region(view, part, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void seed_pressed_state(egui_view_flyout_t *widget, uint8_t part, uint8_t visual_pressed)
{
    widget->pressed_part = part;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_flyout_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_flyout_setters_clamp_and_clear_pressed_state(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    egui_view_flyout_set_snapshots(EGUI_VIEW_OF(&test_widget), g_overflow_snapshots, EGUI_ARRAY_SIZE(g_overflow_snapshots));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_MAX_SNAPSHOTS, test_widget.snapshot_count);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_TARGET, 1);
    egui_view_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_SECONDARY, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    egui_view_flyout_set_open(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_flyout_get_open(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_flyout_set_open(EGUI_VIEW_OF(&test_widget), 1);
    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    egui_view_flyout_set_current_part(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_FLYOUT_PART_TARGET);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_TARGET, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    egui_view_flyout_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    egui_view_flyout_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    egui_view_flyout_set_compact_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(test_widget.compact_mode);
    assert_pressed_cleared(&test_widget);

    egui_view_flyout_set_compact_mode(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    egui_view_flyout_set_disabled_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(test_widget.disabled_mode);
    assert_pressed_cleared(&test_widget);

    egui_view_flyout_set_disabled_mode(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    egui_view_flyout_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                 EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                 EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192), EGUI_COLOR_HEX(0xA0A1A2), EGUI_COLOR_HEX(0xB0B1B2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xB0B1B2).full, test_widget.target_border_color.full);
    assert_pressed_cleared(&test_widget);
}

static void test_flyout_default_part_and_snapshot_guards(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_PRIMARY, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));

    egui_view_flyout_set_current_part(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_FLYOUT_PART_SECONDARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_SECONDARY, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));

    egui_view_flyout_set_current_part(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_FLYOUT_PART_NONE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_SECONDARY, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_SECONDARY, 1);
    egui_view_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_TARGET, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_flyout_set_current_part(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_FLYOUT_PART_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_TARGET, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_TARGET, 1);
    egui_view_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_flyout_set_snapshots(EGUI_VIEW_OF(&test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_NONE, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));
}

static void test_flyout_touch_semantics_and_action_dismiss(void)
{
    egui_dim_t x;
    egui_dim_t y;
    egui_dim_t outside_x;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();

    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_FLYOUT_PART_PRIMARY, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_PRIMARY, g_action_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_flyout_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_TARGET, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    reset_action_state();
    egui_view_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_FLYOUT_PART_TARGET, &x, &y));
    outside_x = EGUI_VIEW_OF(&test_widget)->region_screen.location.x + EGUI_VIEW_OF(&test_widget)->region_screen.size.width + 12;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flyout_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_flyout_get_open(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);
}

static void test_flyout_key_navigation_and_activation(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_SECONDARY, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_TARGET, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_PRIMARY, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_SECONDARY, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_SECONDARY, g_action_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_flyout_get_open(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    reset_action_state();
    egui_view_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_SECONDARY, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_TARGET, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLYOUT_PART_PRIMARY, egui_view_flyout_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_flyout_get_open(EGUI_VIEW_OF(&test_widget)));
}

static void test_flyout_disabled_and_view_disabled_guards(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_FLYOUT_PART_TARGET, &x, &y));

    test_widget.disabled_mode = 1;
    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_FALSE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));

    test_widget.disabled_mode = 0;
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, EGUI_VIEW_FLYOUT_PART_TARGET, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_FALSE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
}

static void test_flyout_static_preview_consumes_input_and_keeps_open_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&preview_widget), EGUI_VIEW_FLYOUT_PART_TARGET, &x, &y));

    seed_pressed_state(&preview_widget, EGUI_VIEW_FLYOUT_PART_TARGET, 1);
    preview_widget.open_state = 1;
    preview_widget.current_part = EGUI_VIEW_FLYOUT_PART_PRIMARY;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flyout_get_open(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&preview_widget);

    seed_pressed_state(&preview_widget, EGUI_VIEW_FLYOUT_PART_PRIMARY, 1);
    preview_widget.open_state = 1;
    preview_widget.current_part = EGUI_VIEW_FLYOUT_PART_PRIMARY;
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flyout_get_open(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&preview_widget);
}

void test_flyout_run(void)
{
    EGUI_TEST_SUITE_BEGIN(flyout);
    EGUI_TEST_RUN(test_flyout_setters_clamp_and_clear_pressed_state);
    EGUI_TEST_RUN(test_flyout_default_part_and_snapshot_guards);
    EGUI_TEST_RUN(test_flyout_touch_semantics_and_action_dismiss);
    EGUI_TEST_RUN(test_flyout_key_navigation_and_activation);
    EGUI_TEST_RUN(test_flyout_disabled_and_view_disabled_guards);
    EGUI_TEST_RUN(test_flyout_static_preview_consumes_input_and_keeps_open_state);
}
