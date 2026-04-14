#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_card_expander.h"

#include "../../HelloCustomWidgets/layout/card_expander/egui_view_card_expander.h"
#include "../../HelloCustomWidgets/layout/card_expander/egui_view_card_expander.c"

static egui_view_card_expander_t test_widget;
static egui_view_card_expander_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_action_count;
static uint8_t g_action_snapshot;
static uint8_t g_action_part;

static const egui_view_card_expander_snapshot_t g_snapshots[] = {
        {"SYNC", "WF", "Workspace policy", "Review card details only when the section needs attention.",
         "Expanded cards keep the detail text in the same card shell, so the action stays close to the summary.",
         "Wi-Fi only sync stays enabled.", EGUI_VIEW_CARD_EXPANDER_TONE_ACCENT, 1, 1},
        {"ACCESS", "ID", "Identity review", "Collapsed state keeps the card lean until more detail is needed.",
         "Partner approval stays hidden while the section is collapsed.", "Manual review remains required.",
         EGUI_VIEW_CARD_EXPANDER_TONE_SUCCESS, 0, 0},
        {"DEPLOY", "UP", "Release approval", "Warning tone still uses the same card shell and header target.",
         "Expanded body keeps staged rollout notes visible without switching to a separate settings panel.",
         "Pilot rollout is still blocked.", EGUI_VIEW_CARD_EXPANDER_TONE_WARNING, 1, 1},
};

static const egui_view_card_expander_snapshot_t g_overflow_snapshots[] = {
        {"A", "A", "A", "A", "A", "A", EGUI_VIEW_CARD_EXPANDER_TONE_ACCENT, 1, 1},
        {"B", "B", "B", "B", "B", "B", EGUI_VIEW_CARD_EXPANDER_TONE_SUCCESS, 0, 0},
        {"C", "C", "C", "C", "C", "C", EGUI_VIEW_CARD_EXPANDER_TONE_WARNING, 1, 1},
        {"D", "D", "D", "D", "D", "D", EGUI_VIEW_CARD_EXPANDER_TONE_NEUTRAL, 0, 0},
        {"E", "E", "E", "E", "E", "E", EGUI_VIEW_CARD_EXPANDER_TONE_ACCENT, 1, 1},
        {"F", "F", "F", "F", "F", "F", EGUI_VIEW_CARD_EXPANDER_TONE_SUCCESS, 0, 0},
        {"G", "G", "G", "G", "G", "G", EGUI_VIEW_CARD_EXPANDER_TONE_WARNING, 1, 1},
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
    g_action_part = EGUI_VIEW_CARD_EXPANDER_PART_NONE;
}

static void setup_widget(const egui_view_card_expander_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_card_expander_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 196, 124);
    egui_view_card_expander_set_snapshots(EGUI_VIEW_OF(&test_widget), snapshots, snapshot_count);
    egui_view_card_expander_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_card_expander_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_card_expander_set_on_action_listener(EGUI_VIEW_OF(&test_widget), on_action);
    reset_action_state();
}

static void setup_preview_widget(void)
{
    egui_view_card_expander_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 76);
    egui_view_card_expander_set_snapshots(EGUI_VIEW_OF(&preview_widget), &g_snapshots[0], 1);
    egui_view_card_expander_set_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_card_expander_set_meta_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_card_expander_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_card_expander_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
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
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 20, 196, 124);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 104, 76);
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

    if (!egui_view_card_expander_get_part_region(view, part, &region))
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

static void seed_pressed_state(egui_view_card_expander_t *widget, uint8_t part, uint8_t visual_pressed)
{
    widget->pressed_part = part;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_card_expander_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_card_expander_set_snapshots_clamp_and_defaults(void)
{
    setup_widget(g_overflow_snapshots, EGUI_ARRAY_SIZE(g_overflow_snapshots));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_MAX_SNAPSHOTS, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    test_widget.current_snapshot = 5;
    egui_view_card_expander_set_snapshots(EGUI_VIEW_OF(&test_widget), g_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_card_expander_set_current_part(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_CARD_EXPANDER_PART_NONE);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));

    egui_view_card_expander_set_snapshots(EGUI_VIEW_OF(&test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_NONE, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
}

static void test_card_expander_setters_clear_pressed_and_update_state(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));

    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    egui_view_card_expander_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 0);
    egui_view_card_expander_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    egui_view_card_expander_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.compact_mode);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    egui_view_card_expander_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    assert_pressed_cleared(&test_widget);
    egui_view_card_expander_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);

    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    egui_view_card_expander_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                        EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                        EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
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

    egui_view_card_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));

    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    egui_view_card_expander_set_expanded(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);
}

static void test_card_expander_regions_activate_and_listener(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    egui_view_card_expander_set_expanded(EGUI_VIEW_OF(&test_widget), 0);

    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_CARD_EXPANDER_PART_HEADER, &x, &y));
    EGUI_TEST_ASSERT_TRUE(egui_view_card_expander_activate_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, g_action_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));

    reset_action_state();
    egui_view_card_expander_set_on_action_listener(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_card_expander_activate_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_card_expander_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t header_x;
    egui_dim_t header_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    egui_view_card_expander_set_expanded(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_CARD_EXPANDER_PART_HEADER, &header_x, &header_y));
    get_view_outside_point(EGUI_VIEW_OF(&test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header_x, header_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header_x, header_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, header_x, header_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, header_x, header_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    reset_action_state();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_CARD_EXPANDER_PART_HEADER, &header_x, &header_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header_x, header_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, header_x, header_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_widget);
}

static void test_card_expander_key_navigation_activate_and_escape(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    egui_view_card_expander_set_expanded(EGUI_VIEW_OF(&test_widget), 0);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, test_widget.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
}

static void test_card_expander_read_only_and_disabled_guards(void)
{
    egui_dim_t header_x;
    egui_dim_t header_y;
    uint8_t initial_snapshot;
    uint8_t initial_part;
    uint8_t initial_expanded;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_CARD_EXPANDER_PART_HEADER, &header_x, &header_y));
    egui_view_card_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 2);
    initial_snapshot = egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget));
    initial_part = egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget));
    initial_expanded = egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header_x, header_y));
    egui_view_card_expander_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header_x, header_y));
    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_part, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_expanded, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    egui_view_card_expander_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, g_action_part);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_expanded ? 0 : 1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));

    reset_action_state();
    egui_view_card_expander_set_current_snapshot(EGUI_VIEW_OF(&test_widget), initial_snapshot);
    egui_view_card_expander_set_current_part(EGUI_VIEW_OF(&test_widget), initial_part);
    initial_snapshot = egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget));
    initial_part = egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget));
    initial_expanded = egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget));
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header_x, header_y));
    assert_pressed_cleared(&test_widget);
    seed_pressed_state(&test_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_part, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_expanded, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 1);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, g_action_part);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_expanded ? 0 : 1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&test_widget)));
}

static void test_card_expander_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t header_x;
    egui_dim_t header_y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&preview_widget), EGUI_VIEW_CARD_EXPANDER_PART_HEADER, &header_x, &header_y));

    preview_widget.current_part = EGUI_VIEW_CARD_EXPANDER_PART_HEADER;
    seed_pressed_state(&preview_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header_x, header_y));
    assert_pressed_cleared(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    preview_widget.current_part = EGUI_VIEW_CARD_EXPANDER_PART_HEADER;
    seed_pressed_state(&preview_widget, EGUI_VIEW_CARD_EXPANDER_PART_HEADER, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_card_expander_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_CARD_EXPANDER_PART_HEADER, egui_view_card_expander_get_current_part(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_card_expander_get_expanded(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

void test_card_expander_run(void)
{
    EGUI_TEST_SUITE_BEGIN(card_expander);
    EGUI_TEST_RUN(test_card_expander_set_snapshots_clamp_and_defaults);
    EGUI_TEST_RUN(test_card_expander_setters_clear_pressed_and_update_state);
    EGUI_TEST_RUN(test_card_expander_regions_activate_and_listener);
    EGUI_TEST_RUN(test_card_expander_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_card_expander_key_navigation_activate_and_escape);
    EGUI_TEST_RUN(test_card_expander_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_card_expander_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
