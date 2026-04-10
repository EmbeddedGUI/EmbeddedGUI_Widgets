#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_menu_flyout.h"

#include "../../HelloCustomWidgets/navigation/menu_flyout/egui_view_menu_flyout.h"
#include "../../HelloCustomWidgets/navigation/menu_flyout/egui_view_menu_flyout.c"

static egui_view_menu_flyout_t test_flyout;
static egui_view_menu_flyout_t preview_flyout;
static egui_view_api_t preview_api;
static uint8_t click_count;

static const egui_view_menu_flyout_item_t g_items_a[] = {
        {"NW", "New tab", "Ctrl+N", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"OP", "Open recent", "", 0, 1, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_SUBMENU},
        {"DL", "Delete", "", 3, 0, 1, 1, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
};

static const egui_view_menu_flyout_item_t g_items_b[] = {
        {"PN", "Pin", "On", 1, 1, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
        {"LY", "Layout", "", 2, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_SUBMENU},
};

static const egui_view_menu_flyout_item_t g_items_c[] = {
        {"AR", "Archive", "", 0, 0, 1, 0, EGUI_VIEW_MENU_FLYOUT_TRAILING_NONE},
};

static const egui_view_menu_flyout_snapshot_t g_snapshots[] = {
        {g_items_a, 3, 1},
        {g_items_b, 2, 0},
        {g_items_c, 1, 0},
};

static const egui_view_menu_flyout_snapshot_t g_overflow_snapshots[] = {
        {g_items_a, 3, 0},
        {g_items_b, 2, 0},
        {g_items_c, 1, 0},
        {g_items_a, 3, 1},
        {g_items_b, 2, 1},
        {g_items_c, 1, 0},
        {g_items_a, 3, 2},
};

static void on_flyout_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void setup_flyout(void)
{
    egui_view_menu_flyout_init(EGUI_VIEW_OF(&test_flyout));
    egui_view_set_size(EGUI_VIEW_OF(&test_flyout), 120, 92);
    egui_view_menu_flyout_set_snapshots(EGUI_VIEW_OF(&test_flyout), g_snapshots, 3);
    click_count = 0;
}

static void setup_preview_flyout(uint8_t snapshot_index)
{
    egui_view_menu_flyout_init(EGUI_VIEW_OF(&preview_flyout));
    egui_view_set_size(EGUI_VIEW_OF(&preview_flyout), 104, 74);
    egui_view_menu_flyout_set_snapshots(EGUI_VIEW_OF(&preview_flyout), g_snapshots, 3);
    egui_view_menu_flyout_set_compact_mode(EGUI_VIEW_OF(&preview_flyout), 1);
    egui_view_menu_flyout_set_current_snapshot(EGUI_VIEW_OF(&preview_flyout), snapshot_index);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_flyout), on_flyout_click);
    egui_view_menu_flyout_override_static_preview_api(EGUI_VIEW_OF(&preview_flyout), &preview_api);
    click_count = 0;
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
    layout_view(EGUI_VIEW_OF(&test_flyout), 10, 20, 120, 92);
}

static void layout_preview_flyout(void)
{
    layout_view(EGUI_VIEW_OF(&preview_flyout), 12, 18, 104, 74);
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void get_view_outside_point(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x - 4;
    *y = view->region_screen.location.y - 4;
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

static int send_key_action_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_action_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_action_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_flyout), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_flyout), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_flyout), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_flyout), key_code);
}

static void seed_pressed_state(egui_view_t *view)
{
    view->is_pressed = 1;
}

static void assert_pressed_cleared(egui_view_t *view)
{
    EGUI_TEST_ASSERT_FALSE(view->is_pressed);
}

static void test_menu_flyout_set_snapshots_clamps_count_and_resets_current_snapshot(void)
{
    setup_flyout();
    egui_view_menu_flyout_set_snapshots(EGUI_VIEW_OF(&test_flyout), g_overflow_snapshots, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_FLYOUT_MAX_SNAPSHOTS, test_flyout.snapshot_count);

    test_flyout.current_snapshot = 5;
    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_snapshots(EGUI_VIEW_OF(&test_flyout), g_snapshots, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_flyout.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
}

static void test_menu_flyout_set_current_snapshot_guards_and_clears_pressed_state(void)
{
    setup_flyout();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));

    egui_view_menu_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 2);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));

    egui_view_menu_flyout_set_snapshots(EGUI_VIEW_OF(&test_flyout), NULL, 0);
    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
}

static void test_menu_flyout_setters_clear_pressed_state(void)
{
    setup_flyout();

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_font(EGUI_VIEW_OF(&test_flyout), NULL);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_TRUE(test_flyout.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_meta_font(EGUI_VIEW_OF(&test_flyout), NULL);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_TRUE(test_flyout.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_palette(EGUI_VIEW_OF(&test_flyout), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                      EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                      EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_flyout.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, test_flyout.danger_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x909192).full, test_flyout.shadow_color.full);

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_snapshots(EGUI_VIEW_OF(&test_flyout), g_snapshots, 3);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 1);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_current_snapshot(EGUI_VIEW_OF(&test_flyout), 1);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_compact_mode(EGUI_VIEW_OF(&test_flyout), 1);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_TRUE(test_flyout.compact_mode);

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    egui_view_menu_flyout_set_disabled_mode(EGUI_VIEW_OF(&test_flyout), 1);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_TRUE(test_flyout.disabled_mode);
}

static void test_menu_flyout_touch_and_enter_trigger_click_listener(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_flyout();
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_flyout), on_flyout_click);
    layout_flyout();
    get_view_center(EGUI_VIEW_OF(&test_flyout), &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(2, click_count);
}

static void test_menu_flyout_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_flyout();
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_flyout), on_flyout_click);
    layout_flyout();
    get_view_center(EGUI_VIEW_OF(&test_flyout), &inside_x, &inside_y);
    get_view_outside_point(EGUI_VIEW_OF(&test_flyout), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flyout)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, inside_x, inside_y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, click_count);
}

static void test_menu_flyout_disabled_and_view_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_flyout();
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&test_flyout), on_flyout_click);
    layout_flyout();
    get_view_center(EGUI_VIEW_OF(&test_flyout), &x, &y);

    test_flyout.disabled_mode = 1;
    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));

    test_flyout.disabled_mode = 0;
    egui_view_set_enable(EGUI_VIEW_OF(&test_flyout), 0);
    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));

    seed_pressed_state(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&test_flyout)));
}

static void test_menu_flyout_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_flyout(1);
    layout_preview_flyout();
    get_view_center(EGUI_VIEW_OF(&preview_flyout), &x, &y);

    seed_pressed_state(EGUI_VIEW_OF(&preview_flyout));
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&preview_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);

    seed_pressed_state(EGUI_VIEW_OF(&preview_flyout));
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_flyout));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_flyout_get_current_snapshot(EGUI_VIEW_OF(&preview_flyout)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_menu_flyout_internal_helpers_clamp_focus_and_meta(void)
{
    egui_view_menu_flyout_snapshot_t snapshot = {g_items_a, 3, 9};

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_FLYOUT_MAX_SNAPSHOTS, egui_view_menu_flyout_clamp_snapshot_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_FLYOUT_MAX_ITEMS, egui_view_menu_flyout_clamp_item_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_menu_flyout_text_len("Ctrl"));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_focus_index(NULL, 3));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_focus_index(&snapshot, 3));
    EGUI_TEST_ASSERT_EQUAL_INT(20, egui_view_menu_flyout_meta_width("Ctrl+Shift+P", 0, 20));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_flyout_meta_width(NULL, 1, 20));
}

void test_menu_flyout_run(void)
{
    EGUI_TEST_SUITE_BEGIN(menu_flyout);
    EGUI_TEST_RUN(test_menu_flyout_set_snapshots_clamps_count_and_resets_current_snapshot);
    EGUI_TEST_RUN(test_menu_flyout_set_current_snapshot_guards_and_clears_pressed_state);
    EGUI_TEST_RUN(test_menu_flyout_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_menu_flyout_touch_and_enter_trigger_click_listener);
    EGUI_TEST_RUN(test_menu_flyout_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_menu_flyout_disabled_and_view_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_menu_flyout_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_menu_flyout_internal_helpers_clamp_focus_and_meta);
    EGUI_TEST_SUITE_END();
}
