#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_toggle_split_button.h"

#include "../../HelloCustomWidgets/input/toggle_split_button/egui_view_toggle_split_button.h"
#include "../../HelloCustomWidgets/input/toggle_split_button/egui_view_toggle_split_button.c"

static egui_view_toggle_split_button_t test_button;
static egui_view_toggle_split_button_t preview_button;
static egui_view_api_t preview_api;

static const egui_view_toggle_split_button_snapshot_t test_snapshots[] = {
        {"Alert", "AL", "Alerts", "Primary on", EGUI_VIEW_TOGGLE_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY},
        {"Sync", "SY", "Sync", "Menu focus", EGUI_VIEW_TOGGLE_SPLIT_BUTTON_TONE_SUCCESS, 1, 1, 1, EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU},
        {"Record", "RC", "Record", "Third item", EGUI_VIEW_TOGGLE_SPLIT_BUTTON_TONE_DANGER, 0, 1, 1, EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY},
};

static void setup_button(void)
{
    egui_view_toggle_split_button_init(EGUI_VIEW_OF(&test_button));
    egui_view_set_size(EGUI_VIEW_OF(&test_button), 116, 88);
    egui_view_toggle_split_button_set_snapshots(EGUI_VIEW_OF(&test_button), test_snapshots, 3);
}

static void setup_preview_button(void)
{
    egui_view_toggle_split_button_init(EGUI_VIEW_OF(&preview_button));
    egui_view_set_size(EGUI_VIEW_OF(&preview_button), 104, 44);
    egui_view_toggle_split_button_set_snapshots(EGUI_VIEW_OF(&preview_button), test_snapshots, 3);
    egui_view_toggle_split_button_set_current_snapshot(EGUI_VIEW_OF(&preview_button), 1);
    egui_view_toggle_split_button_set_compact_mode(EGUI_VIEW_OF(&preview_button), 1);
    egui_view_toggle_split_button_override_static_preview_api(EGUI_VIEW_OF(&preview_button), &preview_api);
}

static void layout_button(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 116;
    region.size.height = 88;
    egui_view_layout(EGUI_VIEW_OF(&test_button), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_button)->region_screen, &region);
}

static void layout_preview_button(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 104;
    region.size.height = 44;
    egui_view_layout(EGUI_VIEW_OF(&preview_button), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_button)->region_screen, &region);
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&test_button)->api->on_touch_event(EGUI_VIEW_OF(&test_button), &event);
}

static int send_key(uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= EGUI_VIEW_OF(&test_button)->api->on_key_event(EGUI_VIEW_OF(&test_button), &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= EGUI_VIEW_OF(&test_button)->api->on_key_event(EGUI_VIEW_OF(&test_button), &event);
    return handled;
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return EGUI_VIEW_OF(&preview_button)->api->on_touch_event(EGUI_VIEW_OF(&preview_button), &event);
}

static int send_preview_key_action(uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return EGUI_VIEW_OF(&preview_button)->api->on_key_event(EGUI_VIEW_OF(&preview_button), &event);
}

static void test_toggle_split_button_setters_clear_pressed_state(void)
{
    setup_button();

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_toggle_split_button_set_snapshots(EGUI_VIEW_OF(&test_button), test_snapshots, 3);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_toggle_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_toggle_split_button_set_checked(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_toggle_split_button_set_current_part(EGUI_VIEW_OF(&test_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_toggle_split_button_set_compact_mode(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_toggle_split_button_set_read_only_mode(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
}

static void test_toggle_split_button_tab_cycles_parts(void)
{
    setup_button();
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, egui_view_toggle_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_handle_navigation_key(EGUI_VIEW_OF(&test_button), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU, egui_view_toggle_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_handle_navigation_key(EGUI_VIEW_OF(&test_button), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, egui_view_toggle_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_split_button_primary_touch_toggles_checked(void)
{
    egui_region_t region;

    setup_button();
    layout_button();
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_part_region(EGUI_VIEW_OF(&test_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, &region));
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_split_button_touch_cancel_clears_pressed_state(void)
{
    egui_region_t region;

    setup_button();
    layout_button();
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_part_region(EGUI_VIEW_OF(&test_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, region.location.x + 8, region.location.y + region.size.height / 2));
}

static void test_toggle_split_button_menu_touch_cycles_snapshot(void)
{
    egui_region_t region;

    setup_button();
    layout_button();
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_part_region(EGUI_VIEW_OF(&test_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + region.size.width / 2, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU, egui_view_toggle_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_split_button_keyboard_activation(void)
{
    setup_button();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
    egui_view_toggle_split_button_set_current_part(EGUI_VIEW_OF(&test_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_split_button_plus_minus_cycle_snapshot(void)
{
    setup_button();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_MINUS));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_split_button_compact_mode_clears_pressed_and_ignores_input(void)
{
    egui_region_t region;

    setup_button();
    layout_button();
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_part_region(EGUI_VIEW_OF(&test_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);

    egui_view_toggle_split_button_set_compact_mode(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_button.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));

    egui_view_toggle_split_button_set_compact_mode(EGUI_VIEW_OF(&test_button), 0);
    layout_button();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_split_button_snapshot_state_persists(void)
{
    setup_button();
    egui_view_toggle_split_button_set_checked(EGUI_VIEW_OF(&test_button), 1);
    egui_view_toggle_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
    egui_view_toggle_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_split_button_read_only_mode_clears_pressed_and_ignores_input(void)
{
    egui_region_t region;

    setup_button();
    layout_button();
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_part_region(EGUI_VIEW_OF(&test_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);

    egui_view_toggle_split_button_set_read_only_mode(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_button.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));

    egui_view_toggle_split_button_set_read_only_mode(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_split_button_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_region_t region;

    setup_button();
    layout_button();
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_part_region(EGUI_VIEW_OF(&test_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, &region));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);

    egui_view_set_enable(EGUI_VIEW_OF(&test_button), false);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, region.location.x + 8, region.location.y + region.size.height / 2));

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));

    egui_view_set_enable(EGUI_VIEW_OF(&test_button), true);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
}

static void test_toggle_split_button_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_region_t region;
    uint8_t snapshot_before;
    uint8_t checked_before;
    uint8_t part_before;

    setup_preview_button();
    layout_preview_button();
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_part_region(EGUI_VIEW_OF(&preview_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, &region));
    snapshot_before = egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&preview_button));
    checked_before = egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&preview_button));
    part_before = egui_view_toggle_split_button_get_current_part(EGUI_VIEW_OF(&preview_button));

    EGUI_VIEW_OF(&preview_button)->is_pressed = true;
    preview_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, preview_button.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot_before, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&preview_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(checked_before, egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&preview_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(part_before, egui_view_toggle_split_button_get_current_part(EGUI_VIEW_OF(&preview_button)));

    EGUI_VIEW_OF(&preview_button)->is_pressed = true;
    preview_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU;
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, preview_button.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot_before, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&preview_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(checked_before, egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&preview_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(part_before, egui_view_toggle_split_button_get_current_part(EGUI_VIEW_OF(&preview_button)));
}

void test_toggle_split_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(toggle_split_button);
    EGUI_TEST_RUN(test_toggle_split_button_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_toggle_split_button_tab_cycles_parts);
    EGUI_TEST_RUN(test_toggle_split_button_primary_touch_toggles_checked);
    EGUI_TEST_RUN(test_toggle_split_button_touch_cancel_clears_pressed_state);
    EGUI_TEST_RUN(test_toggle_split_button_menu_touch_cycles_snapshot);
    EGUI_TEST_RUN(test_toggle_split_button_keyboard_activation);
    EGUI_TEST_RUN(test_toggle_split_button_plus_minus_cycle_snapshot);
    EGUI_TEST_RUN(test_toggle_split_button_compact_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_toggle_split_button_snapshot_state_persists);
    EGUI_TEST_RUN(test_toggle_split_button_read_only_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_toggle_split_button_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_toggle_split_button_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
