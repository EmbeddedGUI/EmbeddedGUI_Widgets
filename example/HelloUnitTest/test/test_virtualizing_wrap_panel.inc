#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_virtualizing_wrap_panel.h"

#include "../../HelloCustomWidgets/layout/virtualizing_wrap_panel/egui_view_virtualizing_wrap_panel.h"
#include "../../HelloCustomWidgets/layout/virtualizing_wrap_panel/egui_view_virtualizing_wrap_panel.c"

#define test_widget                  virtualizing_wrap_panel_test_widget
#define preview_widget               virtualizing_wrap_panel_preview_widget
#define preview_api                  virtualizing_wrap_panel_preview_api
#define g_action_count               virtualizing_wrap_panel_action_count
#define g_action_snapshot            virtualizing_wrap_panel_action_snapshot
#define g_action_item                virtualizing_wrap_panel_action_item
#define g_items_0                    virtualizing_wrap_panel_items_0
#define g_items_1                    virtualizing_wrap_panel_items_1
#define g_snapshots                  virtualizing_wrap_panel_snapshots
#define g_overflow_items             virtualizing_wrap_panel_overflow_items
#define g_overflow_snapshots         virtualizing_wrap_panel_overflow_snapshots
#define g_preview_items              virtualizing_wrap_panel_preview_items
#define g_preview_snapshot           virtualizing_wrap_panel_preview_snapshot
#define on_action                    virtualizing_wrap_panel_on_action
#define reset_action_state           virtualizing_wrap_panel_reset_action_state
#define setup_widget                 virtualizing_wrap_panel_setup_widget
#define setup_preview_widget         virtualizing_wrap_panel_setup_preview_widget
#define layout_view                  virtualizing_wrap_panel_layout_view
#define layout_widget                virtualizing_wrap_panel_layout_widget
#define layout_preview_widget        virtualizing_wrap_panel_layout_preview_widget
#define send_touch_to_view           virtualizing_wrap_panel_send_touch_to_view
#define send_key_to_view             virtualizing_wrap_panel_send_key_to_view
#define send_touch                   virtualizing_wrap_panel_send_touch
#define send_preview_touch           virtualizing_wrap_panel_send_preview_touch
#define send_key_action              virtualizing_wrap_panel_send_key_action
#define send_preview_key_action      virtualizing_wrap_panel_send_preview_key_action
#define send_key                     virtualizing_wrap_panel_send_key
#define send_ctrl_key                virtualizing_wrap_panel_send_ctrl_key
#define send_preview_key             virtualizing_wrap_panel_send_preview_key
#define get_item_center              virtualizing_wrap_panel_get_item_center
#define get_view_outside_point       virtualizing_wrap_panel_get_view_outside_point
#define seed_pressed_state           virtualizing_wrap_panel_seed_pressed_state
#define assert_pressed_cleared       virtualizing_wrap_panel_assert_pressed_cleared

static egui_view_virtualizing_wrap_panel_t test_widget;
static egui_view_virtualizing_wrap_panel_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_action_count;
static uint8_t g_action_snapshot;
static uint8_t g_action_item;

static const egui_view_virtualizing_wrap_panel_item_t g_items_0[] = {
        {"OPS", "Inbox", "01", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 1, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"QA", "Audit", "02", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"REL", "Ship", "03", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 1, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"DOC", "Notes", "04", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"NAV", "Routes", "05", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"OBS", "Health", "06", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 1, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"SEC", "Guard", "07", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"APP", "Owners", "08", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"UX", "Tokens", "09", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 1, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"IA", "Filter", "10", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"CD", "Stages", "11", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 1, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"CAL", "Order", "12", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
};

static const egui_view_virtualizing_wrap_panel_item_t g_items_1[] = {
        {"R1", "Dense", "1", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"R2", "Window", "2", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 1, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"R3", "Rows", "3", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"R4", "Thumb", "4", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"R5", "Focus", "5", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"R6", "Track", "6", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"R7", "Read", "7", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 1, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"R8", "State", "8", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
};

static const egui_view_virtualizing_wrap_panel_snapshot_t g_snapshots[] = {
        {"OPS", "Operations queue", "Only the current window is rendered", "Ctrl+Down jumps window", g_items_0, 12, 1, 0},
        {"REL", "Release review", "Anchor follows the current item", "8 items / windowed wrap", g_items_1, 8, 4, 2},
};

static const egui_view_virtualizing_wrap_panel_item_t g_overflow_items[] = {
        {"A", "One", "1", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"B", "Two", "2", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"C", "Tri", "3", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"D", "For", "4", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"E", "Fiv", "5", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"F", "Six", "6", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"G", "Sev", "7", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"H", "Egt", "8", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"I", "Nin", "9", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"J", "Ten", "10", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"K", "Ele", "11", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"L", "Twl", "12", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"M", "Ext", "13", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
};

static const egui_view_virtualizing_wrap_panel_snapshot_t g_overflow_snapshots[] = {
        {"A", "Overflow", "A", "A", g_overflow_items, 13, 11, 9},
        {"B", "B", "B", "B", g_items_0, 12, 1, 0},
        {"C", "C", "C", "C", g_items_0, 12, 1, 0},
        {"D", "D", "D", "D", g_items_0, 12, 1, 0},
        {"E", "E", "E", "E", g_items_0, 12, 1, 0},
        {"F", "F", "F", "F", g_items_0, 12, 1, 0},
        {"G", "G", "G", "G", g_items_0, 12, 1, 0},
};

static const egui_view_virtualizing_wrap_panel_item_t g_preview_items[] = {
        {"P1", "Ship", "1", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 1, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"P2", "QA", "2", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"P3", "Docs", "3", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"P4", "Live", "4", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"P5", "Flow", "5", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_ACCENT, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"P6", "Read", "6", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_SUCCESS, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
        {"P7", "Track", "7", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_WARNING, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_BALANCED},
        {"P8", "Thumb", "8", EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_TONE_NEUTRAL, 0, EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_WIDTH_PROMINENT},
};

static const egui_view_virtualizing_wrap_panel_snapshot_t g_preview_snapshot = {
        "OPS", "Compact preview", "", "", g_preview_items, 8, 0, 0};

static void on_action(egui_view_t *self, uint8_t snapshot_index, uint8_t item_index)
{
    EGUI_UNUSED(self);
    g_action_count++;
    g_action_snapshot = snapshot_index;
    g_action_item = item_index;
}

static void reset_action_state(void)
{
    g_action_count = 0;
    g_action_snapshot = 0xFF;
    g_action_item = EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_ITEM_NONE;
}

static void setup_widget(const egui_view_virtualizing_wrap_panel_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_virtualizing_wrap_panel_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 196, 132);
    egui_view_virtualizing_wrap_panel_set_snapshots(EGUI_VIEW_OF(&test_widget), snapshots, snapshot_count);
    egui_view_virtualizing_wrap_panel_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_virtualizing_wrap_panel_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    egui_view_virtualizing_wrap_panel_set_on_action_listener(EGUI_VIEW_OF(&test_widget), on_action);
    reset_action_state();
}

static void setup_preview_widget(void)
{
    egui_view_virtualizing_wrap_panel_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 82);
    egui_view_virtualizing_wrap_panel_set_snapshots(EGUI_VIEW_OF(&preview_widget), &g_preview_snapshot, 1);
    egui_view_virtualizing_wrap_panel_set_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_virtualizing_wrap_panel_set_meta_font(EGUI_VIEW_OF(&preview_widget), NULL);
    egui_view_virtualizing_wrap_panel_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_virtualizing_wrap_panel_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
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
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 104, 82);
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

static int send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code, uint8_t is_ctrl)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    event.is_ctrl = is_ctrl ? 1 : 0;
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

static int send_key_action(uint8_t type, uint8_t key_code, uint8_t is_ctrl)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_widget), type, key_code, is_ctrl);
}

static int send_preview_key_action(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_widget), type, key_code, 0);
}

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code, 0);
    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code, 0);
    return handled;
}

static int send_ctrl_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code, 1);
    handled |= send_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code, 1);
    return handled;
}

static int send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_preview_key_action(EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_preview_key_action(EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static uint8_t get_item_center(egui_view_t *view, uint8_t item_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_virtualizing_wrap_panel_get_item_region(view, item_index, &region))
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

static void seed_pressed_state(egui_view_virtualizing_wrap_panel_t *widget, uint8_t item_index, uint8_t visual_pressed)
{
    widget->pressed_item = item_index;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(egui_view_virtualizing_wrap_panel_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_ITEM_NONE, widget->pressed_item);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_virtualizing_wrap_panel_set_snapshots_clamp_defaults_and_anchor(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget(g_overflow_snapshots, EGUI_ARRAY_SIZE(g_overflow_snapshots));
    layout_widget();

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_MAX_SNAPSHOTS, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(11, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(9, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_widget), 11, &x, &y));
    EGUI_TEST_ASSERT_FALSE(get_item_center(EGUI_VIEW_OF(&test_widget), 0, &x, &y));

    seed_pressed_state(&test_widget, 3, 1);
    test_widget.current_snapshot = 5;
    egui_view_virtualizing_wrap_panel_set_snapshots(EGUI_VIEW_OF(&test_widget), g_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_virtualizing_wrap_panel_set_current_item(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_ITEM_NONE);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));

    egui_view_virtualizing_wrap_panel_set_snapshots(EGUI_VIEW_OF(&test_widget), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_VIRTUALIZING_WRAP_PANEL_ITEM_NONE, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));
}

static void test_virtualizing_wrap_panel_setters_clear_pressed_and_update_anchor(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_virtualizing_wrap_panel_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_virtualizing_wrap_panel_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_virtualizing_wrap_panel_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.compact_mode);
    assert_pressed_cleared(&test_widget);
    egui_view_virtualizing_wrap_panel_set_compact_mode(EGUI_VIEW_OF(&test_widget), 0);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_virtualizing_wrap_panel_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);
    assert_pressed_cleared(&test_widget);
    egui_view_virtualizing_wrap_panel_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_virtualizing_wrap_panel_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                                  EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                                  EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_widget.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_widget.border_color.full);
    assert_pressed_cleared(&test_widget);

    egui_view_virtualizing_wrap_panel_set_current_item(EGUI_VIEW_OF(&test_widget), 10);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)) > 0);

    seed_pressed_state(&test_widget, 10, 1);
    egui_view_virtualizing_wrap_panel_set_window_anchor(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_virtualizing_wrap_panel_set_current_snapshot(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));
}

static void test_virtualizing_wrap_panel_regions_activate_and_listener_only_for_visible_items(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();

    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_widget), 0, &x, &y));
    EGUI_TEST_ASSERT_FALSE(get_item_center(EGUI_VIEW_OF(&test_widget), 8, &x, &y));

    egui_view_virtualizing_wrap_panel_set_current_item(EGUI_VIEW_OF(&test_widget), 8);
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_widget), 8, &x, &y));
    EGUI_TEST_ASSERT_FALSE(get_item_center(EGUI_VIEW_OF(&test_widget), 0, &x, &y));

    EGUI_TEST_ASSERT_TRUE(egui_view_virtualizing_wrap_panel_activate_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(8, g_action_item);

    reset_action_state();
    egui_view_virtualizing_wrap_panel_set_on_action_listener(EGUI_VIEW_OF(&test_widget), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_virtualizing_wrap_panel_activate_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

static void test_virtualizing_wrap_panel_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_widget), 0, &item_x, &item_y));
    get_view_outside_point(EGUI_VIEW_OF(&test_widget), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_widget.pressed_item);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    reset_action_state();
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, item_x, item_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    assert_pressed_cleared(&test_widget);
}

static void test_virtualizing_wrap_panel_key_navigation_page_jump_and_tab(void)
{
    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_ctrl_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_TRUE(egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)) > 3);
    EGUI_TEST_ASSERT_TRUE(egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)) > 0);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(11, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)) > 0);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));

    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_widget.pressed_item);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_action(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER, 0));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(4, g_action_item);
    assert_pressed_cleared(&test_widget);
}

static void test_virtualizing_wrap_panel_read_only_and_disabled_guards(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;
    uint8_t initial_snapshot;
    uint8_t initial_item;
    uint8_t initial_anchor;

    setup_widget(g_snapshots, EGUI_ARRAY_SIZE(g_snapshots));
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&test_widget), 1, &item_x, &item_y));
    initial_snapshot = egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&test_widget));
    initial_item = egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget));
    initial_anchor = egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget));

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_virtualizing_wrap_panel_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_item, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_anchor, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    egui_view_virtualizing_wrap_panel_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(11, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)) > initial_anchor);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(11, g_action_item);

    reset_action_state();
    egui_view_virtualizing_wrap_panel_set_current_snapshot(EGUI_VIEW_OF(&test_widget), initial_snapshot);
    egui_view_virtualizing_wrap_panel_set_current_item(EGUI_VIEW_OF(&test_widget), initial_item);
    seed_pressed_state(&test_widget, 1, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(initial_snapshot, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_item, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(initial_anchor, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 1);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_action_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_item);
}

static void test_virtualizing_wrap_panel_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t item_x;
    egui_dim_t item_y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(get_item_center(EGUI_VIEW_OF(&preview_widget), 0, &item_x, &item_y));

    seed_pressed_state(&preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, item_x, item_y));
    assert_pressed_cleared(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);

    seed_pressed_state(&preview_widget, 0, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&preview_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_snapshot(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_current_item(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_virtualizing_wrap_panel_get_window_anchor(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_action_count);
}

void test_virtualizing_wrap_panel_run(void)
{
    EGUI_TEST_SUITE_BEGIN(virtualizing_wrap_panel);
    EGUI_TEST_RUN(test_virtualizing_wrap_panel_set_snapshots_clamp_defaults_and_anchor);
    EGUI_TEST_RUN(test_virtualizing_wrap_panel_setters_clear_pressed_and_update_anchor);
    EGUI_TEST_RUN(test_virtualizing_wrap_panel_regions_activate_and_listener_only_for_visible_items);
    EGUI_TEST_RUN(test_virtualizing_wrap_panel_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_virtualizing_wrap_panel_key_navigation_page_jump_and_tab);
    EGUI_TEST_RUN(test_virtualizing_wrap_panel_read_only_and_disabled_guards);
    EGUI_TEST_RUN(test_virtualizing_wrap_panel_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}

#undef test_widget
#undef preview_widget
#undef preview_api
#undef g_action_count
#undef g_action_snapshot
#undef g_action_item
#undef g_items_0
#undef g_items_1
#undef g_snapshots
#undef g_overflow_items
#undef g_overflow_snapshots
#undef g_preview_items
#undef g_preview_snapshot
#undef on_action
#undef reset_action_state
#undef setup_widget
#undef setup_preview_widget
#undef layout_view
#undef layout_widget
#undef layout_preview_widget
#undef send_touch_to_view
#undef send_key_to_view
#undef send_touch
#undef send_preview_touch
#undef send_key_action
#undef send_preview_key_action
#undef send_key
#undef send_ctrl_key
#undef send_preview_key
#undef get_item_center
#undef get_view_outside_point
#undef seed_pressed_state
#undef assert_pressed_cleared
