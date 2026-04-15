#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_toggle_split_button.h"

#include "../../HelloCustomWidgets/input/toggle_split_button/egui_view_toggle_split_button.h"
#include "../../HelloCustomWidgets/input/toggle_split_button/egui_view_toggle_split_button.c"

static egui_view_toggle_split_button_t test_button;
static egui_view_toggle_split_button_t preview_button;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static uint8_t last_snapshot;
static uint8_t last_checked;
static uint8_t last_part;

typedef struct toggle_split_button_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_toggle_split_button_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_toggle_split_button_changed_listener_t on_changed;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t danger_color;
    egui_color_t neutral_color;
    uint8_t checked_states[EGUI_VIEW_TOGGLE_SPLIT_BUTTON_MAX_SNAPSHOTS];
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
} toggle_split_button_preview_snapshot_t;

static const egui_view_toggle_split_button_snapshot_t test_snapshots[] = {
        {"Alert", "AL", "Alerts", "Primary on", EGUI_VIEW_TOGGLE_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY},
        {"Sync", "SY", "Sync", "Menu focus", EGUI_VIEW_TOGGLE_SPLIT_BUTTON_TONE_SUCCESS, 1, 1, 1, EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU},
        {"Record", "RC", "Record", "Third item", EGUI_VIEW_TOGGLE_SPLIT_BUTTON_TONE_DANGER, 0, 1, 1, EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY},
};

static const egui_view_toggle_split_button_snapshot_t preview_snapshot = {
        "Quick", "QK", "Quick", "Compact preset", EGUI_VIEW_TOGGLE_SPLIT_BUTTON_TONE_ACCENT, 1, 1, 1, EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY};

static void reset_changed_state(void)
{
    changed_count = 0;
    last_snapshot = 0xFF;
    last_checked = 0xFF;
    last_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE;
}

static void on_button_changed(egui_view_t *self, uint8_t snapshot_index, uint8_t checked, uint8_t part)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_snapshot = snapshot_index;
    last_checked = checked;
    last_part = part;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_button(void)
{
    egui_view_toggle_split_button_init(EGUI_VIEW_OF(&test_button));
    egui_view_set_size(EGUI_VIEW_OF(&test_button), 116, 88);
    egui_view_toggle_split_button_set_snapshots(EGUI_VIEW_OF(&test_button), test_snapshots, EGUI_ARRAY_SIZE(test_snapshots));
    egui_view_toggle_split_button_set_on_changed_listener(EGUI_VIEW_OF(&test_button), on_button_changed);
    reset_changed_state();
}

static void setup_preview_button(void)
{
    egui_view_toggle_split_button_init(EGUI_VIEW_OF(&preview_button));
    egui_view_set_size(EGUI_VIEW_OF(&preview_button), 104, 44);
    egui_view_toggle_split_button_set_snapshots(EGUI_VIEW_OF(&preview_button), &preview_snapshot, 1);
    egui_view_toggle_split_button_set_compact_mode(EGUI_VIEW_OF(&preview_button), 1);
    egui_view_toggle_split_button_set_on_changed_listener(EGUI_VIEW_OF(&preview_button), on_button_changed);
    egui_view_toggle_split_button_override_static_preview_api(EGUI_VIEW_OF(&preview_button), &preview_api);
    reset_changed_state();
}

static void capture_preview_snapshot(toggle_split_button_preview_snapshot_t *snapshot)
{
    uint8_t index;

    snapshot->region_screen = EGUI_VIEW_OF(&preview_button)->region_screen;
    snapshot->snapshots = preview_button.snapshots;
    snapshot->font = preview_button.font;
    snapshot->meta_font = preview_button.meta_font;
    snapshot->on_changed = preview_button.on_changed;
    snapshot->surface_color = preview_button.surface_color;
    snapshot->border_color = preview_button.border_color;
    snapshot->text_color = preview_button.text_color;
    snapshot->muted_text_color = preview_button.muted_text_color;
    snapshot->accent_color = preview_button.accent_color;
    snapshot->success_color = preview_button.success_color;
    snapshot->warning_color = preview_button.warning_color;
    snapshot->danger_color = preview_button.danger_color;
    snapshot->neutral_color = preview_button.neutral_color;
    for (index = 0; index < EGUI_VIEW_TOGGLE_SPLIT_BUTTON_MAX_SNAPSHOTS; index++)
    {
        snapshot->checked_states[index] = preview_button.checked_states[index];
    }
    snapshot->snapshot_count = preview_button.snapshot_count;
    snapshot->current_snapshot = preview_button.current_snapshot;
    snapshot->current_part = preview_button.current_part;
    snapshot->compact_mode = preview_button.compact_mode;
    snapshot->read_only_mode = preview_button.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_button)->alpha;
}

static void assert_preview_state_unchanged(const toggle_split_button_preview_snapshot_t *snapshot)
{
    uint8_t index;

    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_button)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_button.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_button.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_button.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_button.on_changed == snapshot->on_changed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_button.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_button.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_button.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_button.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_button.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_button.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_button.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->danger_color.full, preview_button.danger_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_button.neutral_color.full);
    for (index = 0; index < EGUI_VIEW_TOGGLE_SPLIT_BUTTON_MAX_SNAPSHOTS; index++)
    {
        EGUI_TEST_ASSERT_EQUAL_INT(snapshot->checked_states[index], preview_button.checked_states[index]);
    }
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_button.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_button.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, preview_button.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_button.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_button.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_button)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, last_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, last_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_NONE, preview_button.pressed_part);
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

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(EGUI_VIEW_OF(&test_button), EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(EGUI_VIEW_OF(&test_button), EGUI_KEY_EVENT_ACTION_UP, key_code);
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

static int send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(EGUI_VIEW_OF(&preview_button), EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(EGUI_VIEW_OF(&preview_button), EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static void test_toggle_split_button_setters_clear_pressed_state(void)
{
    setup_button();

    test_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_toggle_split_button_set_snapshots(EGUI_VIEW_OF(&test_button), test_snapshots, EGUI_ARRAY_SIZE(test_snapshots));
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
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU, last_part);
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_handle_navigation_key(EGUI_VIEW_OF(&test_button), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, egui_view_toggle_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, last_part);
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
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, last_part);
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
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU, last_part);
}

static void test_toggle_split_button_keyboard_activation(void)
{
    setup_button();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_checked(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, last_part);

    egui_view_toggle_split_button_set_current_part(EGUI_VIEW_OF(&test_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU);
    reset_changed_state();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU, last_part);
}

static void test_toggle_split_button_plus_minus_cycle_snapshot(void)
{
    setup_button();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, last_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU, last_part);

    reset_changed_state();
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_MINUS));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_toggle_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_checked);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU, last_part);
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

static void test_toggle_split_button_static_preview_consumes_input_and_keeps_state(void)
{
    egui_region_t region;
    toggle_split_button_preview_snapshot_t initial_snapshot;

    setup_preview_button();
    layout_preview_button();
    capture_preview_snapshot(&initial_snapshot);
    EGUI_TEST_ASSERT_TRUE(egui_view_toggle_split_button_get_part_region(EGUI_VIEW_OF(&preview_button), EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY, &region));

    EGUI_VIEW_OF(&preview_button)->is_pressed = true;
    preview_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_PRIMARY;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, region.location.x + 8, region.location.y + region.size.height / 2));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_button)->is_pressed = true;
    preview_button.pressed_part = EGUI_VIEW_TOGGLE_SPLIT_BUTTON_PART_MENU;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_RIGHT));
    assert_preview_state_unchanged(&initial_snapshot);
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
    EGUI_TEST_RUN(test_toggle_split_button_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
