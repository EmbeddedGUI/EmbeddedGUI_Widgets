#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_tab_view.h"

#include "../../HelloCustomWidgets/navigation/tab_view/egui_view_tab_view.h"
#include "../../HelloCustomWidgets/navigation/tab_view/egui_view_tab_view.c"

static egui_view_tab_view_t test_tab_view;
static uint8_t changed_count;
static uint8_t last_snapshot;
static uint8_t last_tab;
static uint8_t last_part;
static uint8_t action_count;
static uint8_t last_action;
static uint8_t last_action_tab;

static const egui_view_tab_view_tab_t unit_tabs_primary[] = {
        {"Home", "Start here", "Workspace", "Primary tab selected", "Close button stays in the active tab", "Ready", "Draft", EGUI_VIEW_TAB_VIEW_TONE_ACCENT, 1,
         1},
        {"Review", "Check changes", "Workspace", "Secondary tab carries review text", "Arrow keys move across visible tabs", "Track", "Sync",
         EGUI_VIEW_TAB_VIEW_TONE_WARNING, 0, 1},
        {"Publish", "Go live", "Workspace", "Footer pill mirrors the active page", "Restore action reopens hidden tabs", "Ship", "Ready",
         EGUI_VIEW_TAB_VIEW_TONE_SUCCESS, 0, 1},
};

static const egui_view_tab_view_tab_t unit_tabs_secondary[] = {
        {"Ops", "Queue", "Monitor", "Second snapshot owns a new track", "Snapshot switch resets closed tabs", "Live", "Focus", EGUI_VIEW_TAB_VIEW_TONE_NEUTRAL,
         0, 1},
        {"Audit", "Review", "Monitor", "Current index comes from snapshot defaults", "Read only mode ignores input", "Audit", "Pinned",
         EGUI_VIEW_TAB_VIEW_TONE_ACCENT, 1, 1},
        {"Archive", "History", "Monitor", "Tab view keeps a content body", "Different from plain tab strip", "Store", "Keep", EGUI_VIEW_TAB_VIEW_TONE_WARNING,
         0, 0},
};

static const egui_view_tab_view_snapshot_t unit_snapshots[] = {
        {"Docs workspace", "Header + tab rail + content body", unit_tabs_primary, 3, 0, 1},
        {"Ops workspace", "Snapshot switching resets runtime state", unit_tabs_secondary, 3, 1, 1},
};

static const egui_view_tab_view_snapshot_t overflow_snapshots[] = {
        {"Docs workspace", "Header + tab rail + content body", unit_tabs_primary, 3, 0, 1},
        {"Ops workspace", "Snapshot switching resets runtime state", unit_tabs_secondary, 3, 1, 1},
        {"Docs workspace", "Header + tab rail + content body", unit_tabs_primary, 3, 2, 1},
        {"Ops workspace", "Snapshot switching resets runtime state", unit_tabs_secondary, 3, 0, 1},
        {"Docs workspace", "Header + tab rail + content body", unit_tabs_primary, 3, 1, 1},
};

static void on_changed(egui_view_t *self, uint8_t snapshot_index, uint8_t tab_index, uint8_t part)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_snapshot = snapshot_index;
    last_tab = tab_index;
    last_part = part;
}

static void on_action(egui_view_t *self, uint8_t action, uint8_t tab_index)
{
    EGUI_UNUSED(self);
    action_count++;
    last_action = action;
    last_action_tab = tab_index;
}

static void reset_listener_state(void)
{
    changed_count = 0;
    last_snapshot = EGUI_VIEW_TAB_VIEW_MAX_SNAPSHOTS;
    last_tab = EGUI_VIEW_TAB_VIEW_TAB_NONE;
    last_part = 0xFF;
    action_count = 0;
    last_action = 0xFF;
    last_action_tab = EGUI_VIEW_TAB_VIEW_TAB_NONE;
}

static void setup_widget(void)
{
    egui_view_tab_view_init(EGUI_VIEW_OF(&test_tab_view));
    egui_view_set_size(EGUI_VIEW_OF(&test_tab_view), 184, 112);
    egui_view_tab_view_set_snapshots(EGUI_VIEW_OF(&test_tab_view), unit_snapshots, 2);
    egui_view_tab_view_set_current_snapshot(EGUI_VIEW_OF(&test_tab_view), 0);
    egui_view_tab_view_set_on_changed_listener(EGUI_VIEW_OF(&test_tab_view), on_changed);
    egui_view_tab_view_set_on_action_listener(EGUI_VIEW_OF(&test_tab_view), on_action);
    reset_listener_state();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_tab_view), 1);
#endif
}

static void layout_widget(void)
{
    egui_region_t region;

    region.location.x = 12;
    region.location.y = 20;
    region.size.width = 184;
    region.size.height = 112;
    egui_view_layout(EGUI_VIEW_OF(&test_tab_view), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_tab_view)->region_screen, &region);
}

static int send_touch_at(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&test_tab_view)->api->on_touch_event(EGUI_VIEW_OF(&test_tab_view), &event);
}

static int send_key(uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= EGUI_VIEW_OF(&test_tab_view)->api->dispatch_key_event(EGUI_VIEW_OF(&test_tab_view), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= EGUI_VIEW_OF(&test_tab_view)->api->dispatch_key_event(EGUI_VIEW_OF(&test_tab_view), &event);
    return handled;
}

static void test_tab_view_set_snapshots_clamps_and_clears_pressed_state(void)
{
    setup_widget();

    test_tab_view.current_snapshot = 1;
    test_tab_view.current_tab = 1;
    test_tab_view.current_part = EGUI_VIEW_TAB_VIEW_PART_CLOSE;
    test_tab_view.closed_mask = 0x03;
    test_tab_view.pressed_tab = 1;
    test_tab_view.pressed_part = EGUI_VIEW_TAB_VIEW_PART_CLOSE;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tab_view), 1);
    egui_view_tab_view_set_snapshots(EGUI_VIEW_OF(&test_tab_view), overflow_snapshots, 5);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_MAX_SNAPSHOTS, test_tab_view.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_view_get_current_snapshot(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tab_view.closed_mask);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);

    test_tab_view.current_snapshot = 1;
    test_tab_view.current_tab = 1;
    test_tab_view.current_part = EGUI_VIEW_TAB_VIEW_PART_CLOSE;
    test_tab_view.closed_mask = 0x01;
    test_tab_view.pressed_tab = 1;
    test_tab_view.pressed_part = EGUI_VIEW_TAB_VIEW_PART_CLOSE;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tab_view), 1);
    egui_view_tab_view_set_snapshots(EGUI_VIEW_OF(&test_tab_view), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tab_view.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_view_get_current_snapshot(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tab_view.closed_mask);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)), test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
}

static void test_tab_view_snapshot_tab_and_part_guards_clear_pressed_state(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_view_get_current_snapshot(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));

    egui_view_tab_view_set_current_tab(EGUI_VIEW_OF(&test_tab_view), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, last_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, last_part);

    test_tab_view.pressed_tab = 2;
    test_tab_view.pressed_part = EGUI_VIEW_TAB_VIEW_PART_TAB;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tab_view), 1);
    egui_view_tab_view_set_current_tab(EGUI_VIEW_OF(&test_tab_view), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);

    egui_view_tab_view_set_current_part(EGUI_VIEW_OF(&test_tab_view), EGUI_VIEW_TAB_VIEW_PART_CLOSE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_CLOSE, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, last_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_CLOSE, last_part);

    test_tab_view.pressed_tab = 2;
    test_tab_view.pressed_part = EGUI_VIEW_TAB_VIEW_PART_CLOSE;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tab_view), 1);
    egui_view_tab_view_set_current_part(EGUI_VIEW_OF(&test_tab_view), EGUI_VIEW_TAB_VIEW_PART_CLOSE);
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);

    test_tab_view.pressed_tab = 2;
    test_tab_view.pressed_part = EGUI_VIEW_TAB_VIEW_PART_TAB;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tab_view), 1);
    egui_view_tab_view_set_current_snapshot(EGUI_VIEW_OF(&test_tab_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_snapshot(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, last_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);

    test_tab_view.pressed_tab = 1;
    test_tab_view.pressed_part = EGUI_VIEW_TAB_VIEW_PART_TAB;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tab_view), 1);
    egui_view_tab_view_set_current_snapshot(EGUI_VIEW_OF(&test_tab_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);

    egui_view_tab_view_set_current_snapshot(EGUI_VIEW_OF(&test_tab_view), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_snapshot(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);
}

static void test_tab_view_touch_selects_tab(void)
{
    egui_region_t region;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_get_tab_region(EGUI_VIEW_OF(&test_tab_view), 1, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 4, region.location.y + 4));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + 4, region.location.y + 4));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, last_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
}

static void test_tab_view_close_and_restore_tabs(void)
{
    egui_region_t close_region;
    egui_region_t add_region;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_get_part_region(EGUI_VIEW_OF(&test_tab_view), EGUI_VIEW_TAB_VIEW_PART_CLOSE, &close_region));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, close_region.location.x + 2, close_region.location.y + 2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_CLOSE, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, close_region.location.x + 2, close_region.location.y + 2));
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_is_tab_closed(EGUI_VIEW_OF(&test_tab_view), 0));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tab_view_get_visible_tab_count(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_ACTION_CLOSE, last_action);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_action_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, last_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_get_part_region(EGUI_VIEW_OF(&test_tab_view), EGUI_VIEW_TAB_VIEW_PART_ADD, &add_region));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, add_region.location.x + 2, add_region.location.y + 2));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)), test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_ADD, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, add_region.location.x + 2, add_region.location.y + 2));
    EGUI_TEST_ASSERT_FALSE(egui_view_tab_view_is_tab_closed(EGUI_VIEW_OF(&test_tab_view), 0));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_tab_view_get_visible_tab_count(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_ACTION_ADD, last_action);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_action_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, last_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
}

static void test_tab_view_snapshot_switch_resets_closed_state(void)
{
    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_close_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_is_tab_closed(EGUI_VIEW_OF(&test_tab_view), 0));

    test_tab_view.pressed_tab = 1;
    test_tab_view.pressed_part = EGUI_VIEW_TAB_VIEW_PART_TAB;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tab_view), 1);
    egui_view_tab_view_set_current_snapshot(EGUI_VIEW_OF(&test_tab_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_snapshot(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_FALSE(egui_view_tab_view_is_tab_closed(EGUI_VIEW_OF(&test_tab_view), 0));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_tab_view_get_visible_tab_count(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
}

static void test_tab_view_close_until_last_visible_then_restore_reopens_first_tab(void)
{
    setup_widget();
    layout_widget();

    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_close_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_close_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_FALSE(egui_view_tab_view_close_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_visible_tab_count(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));

    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_restore_tabs(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_tab_view_get_visible_tab_count(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
}

static void test_tab_view_keyboard_navigation_and_actions(void)
{
    setup_widget();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_CLOSE, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_is_tab_closed(EGUI_VIEW_OF(&test_tab_view), 0));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_CLOSE, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_ADD, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(egui_view_tab_view_is_tab_closed(EGUI_VIEW_OF(&test_tab_view), 0));
}

static void test_tab_view_escape_returns_focus_to_tab_part(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_CLOSE, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
}

static void test_tab_view_read_only_ignores_input(void)
{
    egui_region_t region;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, 0, 0) == 0);
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_get_tab_region(EGUI_VIEW_OF(&test_tab_view), 1, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 4, region.location.y + 4));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
    egui_view_tab_view_set_read_only_mode(EGUI_VIEW_OF(&test_tab_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 4, region.location.y + 4));
    EGUI_TEST_ASSERT_FALSE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + 4, region.location.y + 4));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
}

static void test_tab_view_compact_mode_hides_close_part_region(void)
{
    egui_region_t region;
    egui_region_t tab_region;

    setup_widget();
    layout_widget();
    egui_view_tab_view_set_current_part(EGUI_VIEW_OF(&test_tab_view), EGUI_VIEW_TAB_VIEW_PART_CLOSE);
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_get_part_region(EGUI_VIEW_OF(&test_tab_view), EGUI_VIEW_TAB_VIEW_PART_CLOSE, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 2, region.location.y + 2));
    egui_view_tab_view_set_compact_mode(EGUI_VIEW_OF(&test_tab_view), 1);
    layout_widget();

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, egui_view_tab_view_get_current_part(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(egui_view_tab_view_get_part_region(EGUI_VIEW_OF(&test_tab_view), EGUI_VIEW_TAB_VIEW_PART_CLOSE, &region));

    reset_listener_state();
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_get_tab_region(EGUI_VIEW_OF(&test_tab_view), 1, &tab_region));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, tab_region.location.x + 4, tab_region.location.y + 4));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, tab_region.location.x + 4, tab_region.location.y + 4));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, last_part);
}

static void test_tab_view_set_current_tab_ignores_closed_target(void)
{
    setup_widget();
    layout_widget();

    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_close_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    egui_view_tab_view_set_current_tab(EGUI_VIEW_OF(&test_tab_view), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
}

static void test_tab_view_view_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_region_t region;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(egui_view_tab_view_get_tab_region(EGUI_VIEW_OF(&test_tab_view), 1, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 4, region.location.y + 4));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_tab_view), 0);
    test_tab_view.pressed_tab = 1;
    test_tab_view.pressed_part = EGUI_VIEW_TAB_VIEW_PART_TAB;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tab_view), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 4, region.location.y + 4));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + 4, region.location.y + 4));

    test_tab_view.pressed_tab = 1;
    test_tab_view.pressed_part = EGUI_VIEW_TAB_VIEW_PART_TAB;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_tab_view), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_TAB_NONE, test_tab_view.pressed_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, test_tab_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tab_view)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_tab_view), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 4, region.location.y + 4));
    EGUI_TEST_ASSERT_TRUE(send_touch_at(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + 4, region.location.y + 4));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tab_view_get_current_tab(EGUI_VIEW_OF(&test_tab_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_tab);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TAB_VIEW_PART_TAB, last_part);
}

void test_tab_view_run(void)
{
    EGUI_TEST_SUITE_BEGIN(tab_view);
    EGUI_TEST_RUN(test_tab_view_set_snapshots_clamps_and_clears_pressed_state);
    EGUI_TEST_RUN(test_tab_view_snapshot_tab_and_part_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_tab_view_touch_selects_tab);
    EGUI_TEST_RUN(test_tab_view_close_and_restore_tabs);
    EGUI_TEST_RUN(test_tab_view_snapshot_switch_resets_closed_state);
    EGUI_TEST_RUN(test_tab_view_close_until_last_visible_then_restore_reopens_first_tab);
    EGUI_TEST_RUN(test_tab_view_keyboard_navigation_and_actions);
    EGUI_TEST_RUN(test_tab_view_escape_returns_focus_to_tab_part);
    EGUI_TEST_RUN(test_tab_view_read_only_ignores_input);
    EGUI_TEST_RUN(test_tab_view_compact_mode_hides_close_part_region);
    EGUI_TEST_RUN(test_tab_view_set_current_tab_ignores_closed_target);
    EGUI_TEST_RUN(test_tab_view_view_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
