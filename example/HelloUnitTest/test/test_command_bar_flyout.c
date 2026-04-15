#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_command_bar_flyout.h"

#include "../../HelloCustomWidgets/input/command_bar_flyout/egui_view_command_bar_flyout.h"
#include "../../HelloCustomWidgets/input/command_bar_flyout/egui_view_command_bar_flyout.c"

static egui_view_command_bar_flyout_t test_flyout;
static egui_view_command_bar_flyout_t preview_flyout;
static egui_view_api_t preview_api;
static uint8_t g_action_count;
static uint8_t g_action_snapshot;
static uint8_t g_action_part;

typedef struct
{
    egui_region_t region_screen;
    const egui_view_command_bar_flyout_snapshot_t *snapshots;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_command_bar_flyout_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t section_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t danger_color;
    egui_color_t neutral_color;
    egui_color_t shadow_color;
    uint8_t snapshot_count;
    uint8_t current_snapshot;
    uint8_t current_part;
    uint8_t open_state;
    uint8_t compact_mode;
    uint8_t disabled_mode;
    egui_alpha_t alpha;
} command_bar_flyout_preview_snapshot_t;

static const egui_view_command_bar_flyout_primary_item_t g_primary_items_0[] = {
        {"SV", "Save", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 1, 1},
        {"SH", "Share", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 0, 1},
        {"LK", "Locked", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 0},
};

static const egui_view_command_bar_flyout_secondary_item_t g_secondary_items_0[] = {
        {"LK", "Copy link", "Ctrl+L", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"DP", "Duplicate", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"DL", "Delete", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER, 0, 1, 1},
};

static const egui_view_command_bar_flyout_primary_item_t g_primary_items_1[] = {
        {"AP", "Approve", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_SUCCESS, 1, 1},
        {"CM", "Comment", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 0, 1},
};

static const egui_view_command_bar_flyout_secondary_item_t g_secondary_items_1[] = {
        {"AS", "Assign", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"ES", "Escalate", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_WARNING, 0, 1, 0},
};

static const egui_view_command_bar_flyout_primary_item_t g_primary_items_disabled[] = {
        {"LK", "Locked", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 0},
        {"RM", "Remove", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER, 0, 0},
};

static const egui_view_command_bar_flyout_secondary_item_t g_secondary_items_disabled[] = {
        {"OP", "Open", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 0, 0},
        {"DL", "Delete", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_DANGER, 0, 0, 1},
};

static const egui_view_command_bar_flyout_snapshot_t g_snapshots[] = {
        {"Edit", "Page actions", "Share surface", "Keep the flyout open", g_primary_items_0, EGUI_ARRAY_SIZE(g_primary_items_0), g_secondary_items_0,
         EGUI_ARRAY_SIZE(g_secondary_items_0), 1, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
        {"Quick", "Quick actions", "Quick surface", "Closed trigger", g_primary_items_1, EGUI_ARRAY_SIZE(g_primary_items_1), g_secondary_items_1,
         EGUI_ARRAY_SIZE(g_secondary_items_1), 0, EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER},
        {"Review", "Review actions", "Review surface", "Secondary focus", g_primary_items_1, EGUI_ARRAY_SIZE(g_primary_items_1), g_secondary_items_1,
         EGUI_ARRAY_SIZE(g_secondary_items_1), 1, EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(0)},
        {"Locked", "Locked actions", "Locked surface", "Disabled focus", g_primary_items_disabled, EGUI_ARRAY_SIZE(g_primary_items_disabled),
         g_secondary_items_disabled, EGUI_ARRAY_SIZE(g_secondary_items_disabled), 1, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
};

static const egui_view_command_bar_flyout_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "A", g_primary_items_0, EGUI_ARRAY_SIZE(g_primary_items_0), g_secondary_items_0, EGUI_ARRAY_SIZE(g_secondary_items_0), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
        {"B", "B", "B", "B", g_primary_items_0, EGUI_ARRAY_SIZE(g_primary_items_0), g_secondary_items_0, EGUI_ARRAY_SIZE(g_secondary_items_0), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
        {"C", "C", "C", "C", g_primary_items_0, EGUI_ARRAY_SIZE(g_primary_items_0), g_secondary_items_0, EGUI_ARRAY_SIZE(g_secondary_items_0), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
        {"D", "D", "D", "D", g_primary_items_0, EGUI_ARRAY_SIZE(g_primary_items_0), g_secondary_items_0, EGUI_ARRAY_SIZE(g_secondary_items_0), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
        {"E", "E", "E", "E", g_primary_items_0, EGUI_ARRAY_SIZE(g_primary_items_0), g_secondary_items_0, EGUI_ARRAY_SIZE(g_secondary_items_0), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
        {"F", "F", "F", "F", g_primary_items_0, EGUI_ARRAY_SIZE(g_primary_items_0), g_secondary_items_0, EGUI_ARRAY_SIZE(g_secondary_items_0), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
        {"G", "G", "G", "G", g_primary_items_0, EGUI_ARRAY_SIZE(g_primary_items_0), g_secondary_items_0, EGUI_ARRAY_SIZE(g_secondary_items_0), 1,
         EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)},
};

static const egui_view_command_bar_flyout_primary_item_t g_preview_primary_items[] = {
        {"SV", "Save", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 1, 1},
        {"SH", "Share", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_ACCENT, 0, 1},
};

static const egui_view_command_bar_flyout_secondary_item_t g_preview_secondary_items[] = {
        {"LK", "Copy", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
        {"MR", "More", "", EGUI_VIEW_COMMAND_BAR_FLYOUT_TONE_NEUTRAL, 0, 1, 0},
};

static const egui_view_command_bar_flyout_snapshot_t g_preview_snapshot = {
        "Quick", "Compact flyout", "Compact surface", "", g_preview_primary_items, EGUI_ARRAY_SIZE(g_preview_primary_items), g_preview_secondary_items,
        EGUI_ARRAY_SIZE(g_preview_secondary_items), 1, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)};

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
    g_action_part = EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_flyout(const egui_view_command_bar_flyout_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_command_bar_flyout_init(EGUI_VIEW_OF(&test_flyout));
    egui_view_set_size(EGUI_VIEW_OF(&test_flyout), 196, 160);
    egui_view_command_bar_flyout_set_snapshots(EGUI_VIEW_OF(&test_flyout), snapshots, snapshot_count);
    egui_view_command_bar_flyout_set_on_action_listener(EGUI_VIEW_OF(&test_flyout), on_action);
    reset_action_state();
}

static void setup_preview_flyout(void)
{
    egui_view_command_bar_flyout_init(EGUI_VIEW_OF(&preview_flyout));
    egui_view_set_size(EGUI_VIEW_OF(&preview_flyout), 104, 72);
    egui_view_command_bar_flyout_set_snapshots(EGUI_VIEW_OF(&preview_flyout), &g_preview_snapshot, 1);
    egui_view_command_bar_flyout_set_compact_mode(EGUI_VIEW_OF(&preview_flyout), 1);
    egui_view_command_bar_flyout_set_on_action_listener(EGUI_VIEW_OF(&preview_flyout), on_action);
    egui_view_command_bar_flyout_override_static_preview_api(EGUI_VIEW_OF(&preview_flyout), &preview_api);
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

static void layout_flyout(void)
{
    layout_view(EGUI_VIEW_OF(&test_flyout), 10, 20, 196, 160);
}

static void layout_preview_flyout(void)
{
    layout_view(EGUI_VIEW_OF(&preview_flyout), 12, 18, 104, 72);
}

static void capture_preview_snapshot(command_bar_flyout_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_flyout)->region_screen;
    snapshot->snapshots = preview_flyout.snapshots;
    snapshot->font = preview_flyout.font;
    snapshot->meta_font = preview_flyout.meta_font;
    snapshot->on_action = preview_flyout.on_action;
    snapshot->surface_color = preview_flyout.surface_color;
    snapshot->section_color = preview_flyout.section_color;
    snapshot->border_color = preview_flyout.border_color;
    snapshot->text_color = preview_flyout.text_color;
    snapshot->muted_text_color = preview_flyout.muted_text_color;
    snapshot->accent_color = preview_flyout.accent_color;
    snapshot->success_color = preview_flyout.success_color;
    snapshot->warning_color = preview_flyout.warning_color;
    snapshot->danger_color = preview_flyout.danger_color;
    snapshot->neutral_color = preview_flyout.neutral_color;
    snapshot->shadow_color = preview_flyout.shadow_color;
    snapshot->snapshot_count = preview_flyout.snapshot_count;
    snapshot->current_snapshot = preview_flyout.current_snapshot;
    snapshot->current_part = preview_flyout.current_part;
    snapshot->open_state = preview_flyout.open_state;
    snapshot->compact_mode = preview_flyout.compact_mode;
    snapshot->disabled_mode = preview_flyout.disabled_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_flyout)->alpha;
}

static void assert_preview_state_unchanged(const command_bar_flyout_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_flyout)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_flyout.snapshots == snapshot->snapshots);
    EGUI_TEST_ASSERT_TRUE(preview_flyout.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_flyout.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_flyout.on_action == snapshot->on_action);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_flyout.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->section_color.full, preview_flyout.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_flyout.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_flyout.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_flyout.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_flyout.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_flyout.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_flyout.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->danger_color.full, preview_flyout.danger_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_flyout.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->shadow_color.full, preview_flyout.shadow_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->snapshot_count, preview_flyout.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_snapshot, preview_flyout.current_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, preview_flyout.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->open_state, preview_flyout.open_state);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_flyout.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->disabled_mode, preview_flyout.disabled_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_flyout)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE, g_action_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_flyout)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE, preview_flyout.pressed_part);
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

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_flyout), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_flyout), type, x, y);
}

static int send_key_action(uint8_t type, uint8_t key_code)
{
    return dispatch_key_event_to_view(EGUI_VIEW_OF(&test_flyout), type, key_code);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_flyout), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_flyout), key_code);
}

static uint8_t get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_command_bar_flyout_get_part_region(view, part, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void seed_pressed_state(egui_view_command_bar_flyout_t *flyout, uint8_t part, uint8_t visual_pressed)
{
    flyout->pressed_part = part;
    egui_view_set_pressed(EGUI_VIEW_OF(flyout), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_command_bar_flyout_t *flyout)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_NONE, flyout->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(flyout)->is_pressed);
}

static void test_command_bar_flyout_setters_clamp_and_clear_pressed_state(void)
{
    setup_flyout(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    egui_view_command_bar_flyout_set_snapshots(EGUI_VIEW_OF(&test_flyout), g_overflow_snapshots, EGUI_ARRAY_SIZE(g_overflow_snapshots));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SNAPSHOTS, test_flyout.snapshot_count);
    assert_pressed_cleared(&test_flyout);

    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, 1);
    egui_view_command_bar_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 0);
    assert_pressed_cleared(&test_flyout);

    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    egui_view_command_bar_flyout_set_open(EGUI_VIEW_OF(&test_flyout), 0);
    assert_pressed_cleared(&test_flyout);

    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    egui_view_command_bar_flyout_set_current_part(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER);
    assert_pressed_cleared(&test_flyout);

    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    egui_view_command_bar_flyout_set_font(EGUI_VIEW_OF(&test_flyout), NULL);
    assert_pressed_cleared(&test_flyout);

    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    egui_view_command_bar_flyout_set_meta_font(EGUI_VIEW_OF(&test_flyout), NULL);
    assert_pressed_cleared(&test_flyout);

    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    egui_view_command_bar_flyout_set_compact_mode(EGUI_VIEW_OF(&test_flyout), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_flyout.compact_mode);
    assert_pressed_cleared(&test_flyout);

    egui_view_command_bar_flyout_set_compact_mode(EGUI_VIEW_OF(&test_flyout), 0);
    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    egui_view_command_bar_flyout_set_disabled_mode(EGUI_VIEW_OF(&test_flyout), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_flyout.disabled_mode);
    assert_pressed_cleared(&test_flyout);

    egui_view_command_bar_flyout_set_disabled_mode(EGUI_VIEW_OF(&test_flyout), 0);
    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    egui_view_command_bar_flyout_set_palette(EGUI_VIEW_OF(&test_flyout), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                             EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                             EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192), EGUI_COLOR_HEX(0xA0A1A2), EGUI_COLOR_HEX(0xB0B1B2));
    assert_pressed_cleared(&test_flyout);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_flyout.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_flyout.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_flyout.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xB0B1B2).full, test_flyout.shadow_color.full);
}

static void test_command_bar_flyout_snapshot_open_and_part_guards(void)
{
    setup_flyout(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_command_bar_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_command_bar_flyout_get_open(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    egui_view_command_bar_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_command_bar_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_command_bar_flyout_get_open(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    egui_view_command_bar_flyout_set_current_part(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    egui_view_command_bar_flyout_set_open(EGUI_VIEW_OF(&test_flyout), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_command_bar_flyout_get_open(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    egui_view_command_bar_flyout_set_current_part(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(1));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(1), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    egui_view_command_bar_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_command_bar_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));

    egui_view_command_bar_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));
}

static void test_command_bar_flyout_metrics_hit_testing_and_helpers(void)
{
    egui_view_command_bar_flyout_metrics_t metrics;
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);
    egui_dim_t x;
    egui_dim_t y;

    setup_flyout(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_flyout();
    egui_view_command_bar_flyout_get_metrics(&test_flyout, EGUI_VIEW_OF(&test_flyout), &g_snapshots[0], &metrics);

    EGUI_TEST_ASSERT_TRUE(metrics.show_panel);
    EGUI_TEST_ASSERT_EQUAL_INT(3, metrics.primary_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, metrics.secondary_count);
    EGUI_TEST_ASSERT_TRUE(metrics.trigger_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.panel_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.rail_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.secondary_regions[2].size.width > 0);

    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER,
                               egui_view_command_bar_flyout_resolve_hit(&test_flyout, EGUI_VIEW_OF(&test_flyout), x, y));

    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0),
                               egui_view_command_bar_flyout_resolve_hit(&test_flyout, EGUI_VIEW_OF(&test_flyout), x, y));

    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(0), &x, &y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(0),
                               egui_view_command_bar_flyout_resolve_hit(&test_flyout, EGUI_VIEW_OF(&test_flyout), x, y));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SNAPSHOTS, egui_view_command_bar_flyout_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_PRIMARY_ITEMS, egui_view_command_bar_flyout_clamp_primary_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_MAX_SECONDARY_ITEMS, egui_view_command_bar_flyout_clamp_secondary_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_command_bar_flyout_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_command_bar_flyout_text_len("Save"));
    EGUI_TEST_ASSERT_TRUE(egui_view_command_bar_flyout_part_is_primary(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0)));
    EGUI_TEST_ASSERT_TRUE(egui_view_command_bar_flyout_part_is_secondary(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(0)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), egui_view_command_bar_flyout_find_primary_part(&g_snapshots[0], 1, 0, 1));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(2), egui_view_command_bar_flyout_find_secondary_part(&g_snapshots[0], 1, 1, 1));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), egui_view_command_bar_flyout_resolve_default_part(&test_flyout, &g_snapshots[1]));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, egui_view_command_bar_flyout_mix_disabled(sample).full);
}

static void test_command_bar_flyout_touch_trigger_and_same_target_release(void)
{
    egui_dim_t trigger_x;
    egui_dim_t trigger_y;
    egui_dim_t primary_x;
    egui_dim_t primary_y;
    egui_dim_t secondary_x;
    egui_dim_t secondary_y;

    setup_flyout(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    egui_view_command_bar_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 1);
    layout_flyout();

    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, &trigger_x, &trigger_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, trigger_x + 120, trigger_y + 40));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, trigger_x + 120, trigger_y + 40));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_command_bar_flyout_get_open(EGUI_VIEW_OF(&test_flyout)));
    assert_pressed_cleared(&test_flyout);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, trigger_x + 120, trigger_y + 40));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, trigger_x, trigger_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_command_bar_flyout_get_open(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    layout_flyout();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), &primary_x, &primary_y));
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(0), &secondary_x, &secondary_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_flyout);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, secondary_x, secondary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, primary_x, primary_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, primary_x, primary_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), g_action_part);
    assert_pressed_cleared(&test_flyout);
}

static void test_command_bar_flyout_keyboard_navigation_activation_and_escape(void)
{
    setup_flyout(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(1), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(0), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(1), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(0), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(1), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(2), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_SECONDARY_PART(2), g_action_part);
    assert_pressed_cleared(&test_flyout);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_command_bar_flyout_get_open(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_command_bar_flyout_get_open(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), egui_view_command_bar_flyout_get_current_part(EGUI_VIEW_OF(&test_flyout)));
}

static void test_command_bar_flyout_disabled_compact_and_view_disabled_guards(void)
{
    egui_dim_t primary_x;
    egui_dim_t primary_y;

    setup_flyout(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_flyout();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), &primary_x, &primary_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    egui_view_command_bar_flyout_set_disabled_mode(EGUI_VIEW_OF(&test_flyout), 1);
    assert_pressed_cleared(&test_flyout);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_command_bar_flyout_set_disabled_mode(EGUI_VIEW_OF(&test_flyout), 0);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    egui_view_command_bar_flyout_set_compact_mode(EGUI_VIEW_OF(&test_flyout), 1);
    assert_pressed_cleared(&test_flyout);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));

    egui_view_command_bar_flyout_set_compact_mode(EGUI_VIEW_OF(&test_flyout), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_flyout), 0);
    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, primary_x, primary_y));
    assert_pressed_cleared(&test_flyout);
    seed_pressed_state(&test_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&test_flyout);
}

static void test_command_bar_flyout_static_preview_consumes_input_and_keeps_state(void)
{
    command_bar_flyout_preview_snapshot_t initial_snapshot;
    egui_dim_t trigger_x;
    egui_dim_t trigger_y;

    setup_preview_flyout();
    layout_preview_flyout();
    capture_preview_snapshot(&initial_snapshot);
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&preview_flyout), EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, &trigger_x, &trigger_y));

    seed_pressed_state(&preview_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PART_TRIGGER, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, trigger_x, trigger_y));
    assert_preview_state_unchanged(&initial_snapshot);

    seed_pressed_state(&preview_flyout, EGUI_VIEW_COMMAND_BAR_FLYOUT_PRIMARY_PART(0), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_END));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_command_bar_flyout_run(void)
{
    EGUI_TEST_SUITE_BEGIN(command_bar_flyout);
    EGUI_TEST_RUN(test_command_bar_flyout_setters_clamp_and_clear_pressed_state);
    EGUI_TEST_RUN(test_command_bar_flyout_snapshot_open_and_part_guards);
    EGUI_TEST_RUN(test_command_bar_flyout_metrics_hit_testing_and_helpers);
    EGUI_TEST_RUN(test_command_bar_flyout_touch_trigger_and_same_target_release);
    EGUI_TEST_RUN(test_command_bar_flyout_keyboard_navigation_activation_and_escape);
    EGUI_TEST_RUN(test_command_bar_flyout_disabled_compact_and_view_disabled_guards);
    EGUI_TEST_RUN(test_command_bar_flyout_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
