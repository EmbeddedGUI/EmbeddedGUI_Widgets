#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_split_button.h"

#include "../../HelloCustomWidgets/input/split_button/egui_view_split_button.h"
#include "../../HelloCustomWidgets/input/split_button/egui_view_split_button.c"

static egui_view_split_button_t test_button;
static egui_view_split_button_t preview_button;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static uint8_t last_part;

static const egui_view_split_button_snapshot_t g_snapshots[] = {
        {"Save draft", "SV", "Save", "Run save or open more", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 1, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"Share handoff", "SH", "Share", "Send fast or choose route", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_MENU},
        {"Export file", "EX", "Export", "Export PDF or pick format", EGUI_VIEW_SPLIT_BUTTON_TONE_WARNING, 0, 0, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"Locked", "LK", "Lock", "No actions available", EGUI_VIEW_SPLIT_BUTTON_TONE_NEUTRAL, 0, 0, 0, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
};

static const egui_view_split_button_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "A", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"B", "B", "B", "B", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"C", "C", "C", "C", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"D", "D", "D", "D", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"E", "E", "E", "E", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"F", "F", "F", "F", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
        {"G", "G", "G", "G", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 0, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY},
};

typedef struct split_button_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_split_button_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_split_button_part_changed_listener_t on_part_changed;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t danger_color;
    egui_color_t neutral_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t disabled_mode;
    uint8_t alpha;
} split_button_preview_snapshot_t;

static const egui_view_split_button_snapshot_t preview_snapshot = {
        "Quick", "SV", "Save", "Tight split action", EGUI_VIEW_SPLIT_BUTTON_TONE_ACCENT, 1, 1, 1, EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY};

static void reset_changed_state(void)
{
    changed_count = 0;
    last_part = EGUI_VIEW_SPLIT_BUTTON_PART_NONE;
}

static void assert_region_equal(const egui_region_t *lhs, const egui_region_t *rhs)
{
    EGUI_TEST_ASSERT_EQUAL_INT(lhs->location.x, rhs->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(lhs->location.y, rhs->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(lhs->size.width, rhs->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(lhs->size.height, rhs->size.height);
}

static void on_part_changed(egui_view_t *self, uint8_t part)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_part = part;
}

static void setup_button(void)
{
    egui_view_split_button_init(EGUI_VIEW_OF(&test_button));
    egui_view_set_size(EGUI_VIEW_OF(&test_button), 116, 88);
    egui_view_split_button_set_snapshots(EGUI_VIEW_OF(&test_button), g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    egui_view_split_button_set_on_part_changed_listener(EGUI_VIEW_OF(&test_button), on_part_changed);
    reset_changed_state();
}

static void setup_preview_button(void)
{
    egui_view_split_button_init(EGUI_VIEW_OF(&preview_button));
    egui_view_set_size(EGUI_VIEW_OF(&preview_button), 104, 44);
    egui_view_split_button_set_snapshots(EGUI_VIEW_OF(&preview_button), &preview_snapshot, 1);
    egui_view_split_button_set_compact_mode(EGUI_VIEW_OF(&preview_button), 1);
    egui_view_split_button_override_static_preview_api(EGUI_VIEW_OF(&preview_button), &preview_api);
    reset_changed_state();
}

static void capture_preview_snapshot(split_button_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_button)->region_screen;
    snapshot->snapshots = preview_button.snapshots;
    snapshot->font = preview_button.font;
    snapshot->meta_font = preview_button.meta_font;
    snapshot->on_part_changed = preview_button.on_part_changed;
    snapshot->surface_color = preview_button.surface_color;
    snapshot->border_color = preview_button.border_color;
    snapshot->text_color = preview_button.text_color;
    snapshot->muted_text_color = preview_button.muted_text_color;
    snapshot->accent_color = preview_button.accent_color;
    snapshot->success_color = preview_button.success_color;
    snapshot->warning_color = preview_button.warning_color;
    snapshot->danger_color = preview_button.danger_color;
    snapshot->neutral_color = preview_button.neutral_color;
    snapshot->snapshot_count = preview_button.snapshot_count;
    snapshot->current_snapshot = preview_button.current_snapshot;
    snapshot->current_part = preview_button.current_part;
    snapshot->compact_mode = preview_button.compact_mode;
    snapshot->disabled_mode = preview_button.disabled_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_button)->alpha;
}

static void assert_preview_state_unchanged(const split_button_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_button)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_button.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_button.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_button.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_button.on_part_changed == snapshot->on_part_changed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_button.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_button.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_button.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_button.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_button.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_button.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_button.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->danger_color.full, preview_button.danger_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_button.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_button.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_button.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, preview_button.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_button.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->disabled_mode, preview_button.disabled_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_button)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, last_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, preview_button.pressed_part);
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

static void test_split_button_setters_clear_pressed_state(void)
{
    setup_button();

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_split_button_set_snapshots(EGUI_VIEW_OF(&test_button), g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_split_button_set_current_part(EGUI_VIEW_OF(&test_button), EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_split_button_set_compact_mode(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    egui_view_split_button_set_disabled_mode(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
}

static void test_split_button_set_snapshots_clamp_and_resolve_default_part(void)
{
    setup_button();

    egui_view_split_button_set_snapshots(EGUI_VIEW_OF(&test_button), g_overflow_snapshots, EGUI_ARRAY_SIZE(g_overflow_snapshots));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_MAX_SNAPSHOTS, test_button.snapshot_count);

    test_button.current_snapshot = 5;
    egui_view_split_button_set_snapshots(EGUI_VIEW_OF(&test_button), g_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_button.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));

    egui_view_split_button_set_snapshots(EGUI_VIEW_OF(&test_button), g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));

    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));

    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
}

static void test_split_button_snapshot_and_part_guards(void)
{
    setup_button();

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));

    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_split_button_get_current_snapshot(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_split_button_set_current_part(EGUI_VIEW_OF(&test_button), EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, last_part);

    egui_view_split_button_set_current_part(EGUI_VIEW_OF(&test_button), EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);

    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 2);
    reset_changed_state();
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    egui_view_split_button_set_current_part(EGUI_VIEW_OF(&test_button), EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_split_button_font_modes_and_palette(void)
{
    setup_button();

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_split_button_set_font(EGUI_VIEW_OF(&test_button), NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_MENU;
    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_split_button_set_meta_font(EGUI_VIEW_OF(&test_button), NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(test_button.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_button.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_split_button_set_compact_mode(EGUI_VIEW_OF(&test_button), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_button.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_MENU;
    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_split_button_set_disabled_mode(EGUI_VIEW_OF(&test_button), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_button.disabled_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    EGUI_VIEW_OF(&test_button)->is_pressed = true;
    egui_view_split_button_set_palette(EGUI_VIEW_OF(&test_button), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                       EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                       EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_button.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_button.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_button.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_button.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_button.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, test_button.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, test_button.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, test_button.danger_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x909192).full, test_button.neutral_color.full);
}

static void test_split_button_touch_switches_part_and_notifies(void)
{
    egui_view_split_button_metrics_t metrics;
    const egui_view_split_button_snapshot_t *snapshot;
    egui_dim_t primary_x;
    egui_dim_t primary_y;
    egui_dim_t menu_x;
    egui_dim_t menu_y;

    setup_button();
    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 1);
    reset_changed_state();
    layout_button();
    snapshot = egui_view_split_button_get_snapshot(&test_button);
    egui_view_split_button_get_metrics(&test_button, EGUI_VIEW_OF(&test_button), snapshot, &metrics);

    primary_x = metrics.primary_region.location.x + metrics.primary_region.size.width / 2;
    primary_y = metrics.primary_region.location.y + metrics.primary_region.size.height / 2;
    menu_x = metrics.menu_region.location.x + metrics.menu_region.size.width / 2;
    menu_y = metrics.menu_region.location.y + metrics.menu_region.size.height / 2;

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY,
                               egui_view_split_button_hit_part(&test_button, EGUI_VIEW_OF(&test_button), primary_x, primary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_hit_part(&test_button, EGUI_VIEW_OF(&test_button), menu_x, menu_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, last_part);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, menu_x, menu_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, test_button.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, menu_x, menu_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, last_part);
}

static void test_split_button_same_target_release_requires_return_to_origin(void)
{
    egui_view_split_button_metrics_t metrics;
    const egui_view_split_button_snapshot_t *snapshot;
    egui_dim_t primary_x;
    egui_dim_t primary_y;
    egui_dim_t menu_x;
    egui_dim_t menu_y;

    setup_button();
    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 1);
    reset_changed_state();
    layout_button();
    snapshot = egui_view_split_button_get_snapshot(&test_button);
    egui_view_split_button_get_metrics(&test_button, EGUI_VIEW_OF(&test_button), snapshot, &metrics);
    primary_x = metrics.primary_region.location.x + metrics.primary_region.size.width / 2;
    primary_y = metrics.primary_region.location.y + metrics.primary_region.size.height / 2;
    menu_x = metrics.menu_region.location.x + metrics.menu_region.size.width / 2;
    menu_y = metrics.menu_region.location.y + metrics.menu_region.size.height / 2;

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, menu_x, menu_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, menu_x, menu_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, menu_x, menu_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, last_part);
}

static void test_split_button_touch_cancel_clears_pressed_state(void)
{
    egui_view_split_button_metrics_t metrics;
    const egui_view_split_button_snapshot_t *snapshot;
    egui_dim_t primary_x;
    egui_dim_t primary_y;

    setup_button();
    layout_button();
    snapshot = egui_view_split_button_get_snapshot(&test_button);
    egui_view_split_button_get_metrics(&test_button, EGUI_VIEW_OF(&test_button), snapshot, &metrics);

    primary_x = metrics.primary_region.location.x + metrics.primary_region.size.width / 2;
    primary_y = metrics.primary_region.location.y + metrics.primary_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, primary_x, primary_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, primary_x, primary_y));
}

static void test_split_button_compact_mode_clears_pressed_and_ignores_input(void)
{
    egui_view_split_button_metrics_t metrics;
    const egui_view_split_button_snapshot_t *snapshot;
    egui_dim_t primary_x;
    egui_dim_t primary_y;

    setup_button();
    layout_button();
    snapshot = egui_view_split_button_get_snapshot(&test_button);
    egui_view_split_button_get_metrics(&test_button, EGUI_VIEW_OF(&test_button), snapshot, &metrics);
    primary_x = metrics.primary_region.location.x + metrics.primary_region.size.width / 2;
    primary_y = metrics.primary_region.location.y + metrics.primary_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);

    egui_view_split_button_set_compact_mode(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_button.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_split_button_set_compact_mode(EGUI_VIEW_OF(&test_button), 0);
    layout_button();
    snapshot = egui_view_split_button_get_snapshot(&test_button);
    egui_view_split_button_get_metrics(&test_button, EGUI_VIEW_OF(&test_button), snapshot, &metrics);
    primary_x = metrics.primary_region.location.x + metrics.primary_region.size.width / 2;
    primary_y = metrics.primary_region.location.y + metrics.primary_region.size.height / 2;
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_split_button_keyboard_navigation_and_fallback(void)
{
    setup_button();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, last_part);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, changed_count);

    egui_view_split_button_set_current_snapshot(EGUI_VIEW_OF(&test_button), 2);
    reset_changed_state();
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_split_button_disabled_mode_ignores_input_and_clears_pressed_state(void)
{
    egui_view_split_button_metrics_t metrics;
    const egui_view_split_button_snapshot_t *snapshot;
    egui_dim_t primary_x;
    egui_dim_t primary_y;

    setup_button();
    layout_button();
    snapshot = egui_view_split_button_get_snapshot(&test_button);
    egui_view_split_button_get_metrics(&test_button, EGUI_VIEW_OF(&test_button), snapshot, &metrics);
    primary_x = metrics.primary_region.location.x + metrics.primary_region.size.width / 2;
    primary_y = metrics.primary_region.location.y + metrics.primary_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, test_button.pressed_part);

    egui_view_split_button_set_disabled_mode(EGUI_VIEW_OF(&test_button), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_button.disabled_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    test_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_MENU;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_button), true);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_NONE, test_button.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_button)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_split_button_set_disabled_mode(EGUI_VIEW_OF(&test_button), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPLIT_BUTTON_PART_MENU, egui_view_split_button_get_current_part(EGUI_VIEW_OF(&test_button)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
}

static void test_split_button_static_preview_consumes_input_and_keeps_state(void)
{
    egui_view_split_button_metrics_t metrics;
    const egui_view_split_button_snapshot_t *snapshot;
    egui_dim_t primary_x;
    egui_dim_t primary_y;
    split_button_preview_snapshot_t initial_snapshot;

    setup_preview_button();
    layout_preview_button();
    capture_preview_snapshot(&initial_snapshot);
    snapshot = egui_view_split_button_get_snapshot(&preview_button);
    egui_view_split_button_get_metrics(&preview_button, EGUI_VIEW_OF(&preview_button), snapshot, &metrics);
    primary_x = metrics.primary_region.location.x + metrics.primary_region.size.width / 2;
    primary_y = metrics.primary_region.location.y + metrics.primary_region.size.height / 2;

    EGUI_VIEW_OF(&preview_button)->is_pressed = true;
    preview_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_PRIMARY;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_button)->is_pressed = true;
    preview_button.pressed_part = EGUI_VIEW_SPLIT_BUTTON_PART_MENU;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_RIGHT));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_split_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(split_button);
    EGUI_TEST_RUN(test_split_button_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_split_button_set_snapshots_clamp_and_resolve_default_part);
    EGUI_TEST_RUN(test_split_button_snapshot_and_part_guards);
    EGUI_TEST_RUN(test_split_button_font_modes_and_palette);
    EGUI_TEST_RUN(test_split_button_touch_switches_part_and_notifies);
    EGUI_TEST_RUN(test_split_button_same_target_release_requires_return_to_origin);
    EGUI_TEST_RUN(test_split_button_touch_cancel_clears_pressed_state);
    EGUI_TEST_RUN(test_split_button_compact_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_split_button_keyboard_navigation_and_fallback);
    EGUI_TEST_RUN(test_split_button_disabled_mode_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_split_button_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
