#include "egui.h"
#include "test/egui_test.h"
#include "test_menu_bar.h"

#include <string.h>

#include "../../HelloCustomWidgets/navigation/menu_bar/egui_view_menu_bar.h"
#include "../../HelloCustomWidgets/navigation/menu_bar/egui_view_menu_bar.c"

static egui_view_menu_bar_t test_menu_bar;
static egui_view_menu_bar_t preview_menu_bar;
static egui_view_api_t preview_api;
static uint8_t g_selection_snapshot = EGUI_VIEW_MENU_BAR_HIT_NONE;
static uint8_t g_selection_item = EGUI_VIEW_MENU_BAR_ITEM_NONE;
static uint8_t g_selection_count = 0;
static uint8_t g_activation_snapshot = EGUI_VIEW_MENU_BAR_HIT_NONE;
static uint8_t g_activation_item = EGUI_VIEW_MENU_BAR_ITEM_NONE;
static uint8_t g_activation_count = 0;

static const egui_view_menu_bar_menu_t g_test_menus[] = {
        {"File", 1, 1},
        {"View", 0, 1},
        {"Tools", 0, 1},
};

static const egui_view_menu_bar_panel_item_t g_snapshot_0_items[] = {
        {"New", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Hidden", "", 0, 0, 0, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Recent", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_SUBMENU},
};

static const egui_view_menu_bar_panel_item_t g_snapshot_1_items[] = {
        {"Inspect", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Panels", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_SUBMENU},
        {"Density", "", 2, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_panel_item_t g_snapshot_2_items[] = {
        {"Sync", "", 1, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Delete", "", 3, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_snapshot_t g_test_snapshots[] = {
        {g_test_menus, 3, 0, g_snapshot_0_items, 3, 0, 1},
        {g_test_menus, 3, 1, g_snapshot_1_items, 3, 1, 1},
        {g_test_menus, 3, 2, g_snapshot_2_items, 2, 0, 1},
};

static const egui_view_menu_bar_panel_item_t g_disabled_focus_items[] = {
        {"Alpha", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Disabled", "", 0, 0, 0, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
        {"Gamma", "", 0, 0, 1, 0, EGUI_VIEW_MENU_BAR_TRAILING_NONE},
};

static const egui_view_menu_bar_snapshot_t g_disabled_focus_snapshots[] = {
        {g_test_menus, 3, 0, g_disabled_focus_items, 3, 1, 1},
};

static const egui_view_menu_bar_menu_t g_initial_disabled_menus[] = {
        {"File", 1, 0},
        {"View", 0, 1},
        {"Tools", 0, 1},
};

static const egui_view_menu_bar_snapshot_t g_initial_disabled_snapshots[] = {
        {g_initial_disabled_menus, 3, 0, g_snapshot_0_items, 3, 0, 1},
        {g_initial_disabled_menus, 3, 1, g_snapshot_1_items, 3, 1, 1},
        {g_initial_disabled_menus, 3, 2, g_snapshot_2_items, 2, 0, 1},
};

static const egui_view_menu_bar_menu_t g_middle_disabled_menus[] = {
        {"File", 1, 1},
        {"View", 0, 0},
        {"Tools", 0, 1},
};

static const egui_view_menu_bar_snapshot_t g_middle_disabled_snapshots[] = {
        {g_middle_disabled_menus, 3, 0, g_snapshot_0_items, 3, 0, 1},
        {g_middle_disabled_menus, 3, 1, g_snapshot_1_items, 3, 1, 1},
        {g_middle_disabled_menus, 3, 2, g_snapshot_2_items, 2, 0, 1},
};

static void reset_menu_bar_events(void)
{
    g_selection_snapshot = EGUI_VIEW_MENU_BAR_HIT_NONE;
    g_selection_item = EGUI_VIEW_MENU_BAR_ITEM_NONE;
    g_selection_count = 0;
    g_activation_snapshot = EGUI_VIEW_MENU_BAR_HIT_NONE;
    g_activation_item = EGUI_VIEW_MENU_BAR_ITEM_NONE;
    g_activation_count = 0;
}

static void on_selection_changed(egui_view_t *self, uint8_t snapshot_index, uint8_t item_index)
{
    EGUI_UNUSED(self);
    g_selection_snapshot = snapshot_index;
    g_selection_item = item_index;
    g_selection_count++;
}

static void on_item_activated(egui_view_t *self, uint8_t snapshot_index, uint8_t item_index)
{
    EGUI_UNUSED(self);
    g_activation_snapshot = snapshot_index;
    g_activation_item = item_index;
    g_activation_count++;
}

static void setup_menu_bar(const egui_view_menu_bar_snapshot_t *snapshots, uint8_t snapshot_count)
{
    egui_view_menu_bar_init(EGUI_VIEW_OF(&test_menu_bar));
    egui_view_set_size(EGUI_VIEW_OF(&test_menu_bar), 196, 112);
    egui_view_menu_bar_set_snapshots(EGUI_VIEW_OF(&test_menu_bar), snapshots, snapshot_count);
    egui_view_menu_bar_set_on_selection_changed_listener(EGUI_VIEW_OF(&test_menu_bar), on_selection_changed);
    egui_view_menu_bar_set_on_item_activated_listener(EGUI_VIEW_OF(&test_menu_bar), on_item_activated);
    reset_menu_bar_events();
}

static void setup_preview_menu_bar(const egui_view_menu_bar_snapshot_t *snapshots, uint8_t snapshot_count, uint8_t snapshot_index)
{
    egui_view_menu_bar_init(EGUI_VIEW_OF(&preview_menu_bar));
    egui_view_set_size(EGUI_VIEW_OF(&preview_menu_bar), 104, 74);
    egui_view_menu_bar_set_snapshots(EGUI_VIEW_OF(&preview_menu_bar), snapshots, snapshot_count);
    egui_view_menu_bar_set_font(EGUI_VIEW_OF(&preview_menu_bar), NULL);
    egui_view_menu_bar_set_meta_font(EGUI_VIEW_OF(&preview_menu_bar), NULL);
    egui_view_menu_bar_set_compact_mode(EGUI_VIEW_OF(&preview_menu_bar), 1);
    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&preview_menu_bar), snapshot_index);
    egui_view_menu_bar_override_static_preview_api(EGUI_VIEW_OF(&preview_menu_bar), &preview_api);
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

static void layout_menu_bar(egui_dim_t width, egui_dim_t height)
{
    layout_view(EGUI_VIEW_OF(&test_menu_bar), 0, 0, width, height);
}

static void layout_preview_menu_bar(void)
{
    layout_view(EGUI_VIEW_OF(&preview_menu_bar), 12, 18, 104, 74);
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

static int send_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_menu_bar), type, x, y);
}

static int send_preview_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_menu_bar), type, x, y);
}

static int send_key_event(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_menu_bar), type, key_code);
}

static int send_preview_key_event(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_menu_bar), type, key_code);
}

static void get_layout(egui_view_menu_bar_layout_t *layout)
{
    const egui_view_menu_bar_snapshot_t *snapshot = egui_view_menu_bar_get_snapshot(&test_menu_bar);

    EGUI_TEST_ASSERT_NOT_NULL(snapshot);
    EGUI_TEST_ASSERT_TRUE(egui_view_menu_bar_build_layout(&test_menu_bar, EGUI_VIEW_OF(&test_menu_bar), snapshot, layout));
}

static void get_menu_center(uint8_t menu_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_view_menu_bar_layout_t layout;

    get_layout(&layout);
    EGUI_TEST_ASSERT_TRUE(menu_index < layout.menu_count);
    *x = layout.top_item_regions[menu_index].location.x + layout.top_item_regions[menu_index].size.width / 2;
    *y = layout.top_item_regions[menu_index].location.y + layout.top_item_regions[menu_index].size.height / 2;
}

static void get_item_center(uint8_t item_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_view_menu_bar_layout_t layout;

    get_layout(&layout);
    EGUI_TEST_ASSERT_TRUE(item_index < layout.panel_item_count);
    *x = layout.panel_item_regions[item_index].location.x + layout.panel_item_regions[item_index].size.width / 2;
    *y = layout.panel_item_regions[item_index].location.y + layout.panel_item_regions[item_index].size.height / 2;
}

static void get_preview_layout(egui_view_menu_bar_layout_t *layout)
{
    const egui_view_menu_bar_snapshot_t *snapshot = egui_view_menu_bar_get_snapshot(&preview_menu_bar);

    EGUI_TEST_ASSERT_NOT_NULL(snapshot);
    EGUI_TEST_ASSERT_TRUE(egui_view_menu_bar_build_layout(&preview_menu_bar, EGUI_VIEW_OF(&preview_menu_bar), snapshot, layout));
}

static void get_preview_menu_center(uint8_t menu_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_view_menu_bar_layout_t layout;

    get_preview_layout(&layout);
    EGUI_TEST_ASSERT_TRUE(menu_index < layout.menu_count);
    *x = layout.top_item_regions[menu_index].location.x + layout.top_item_regions[menu_index].size.width / 2;
    *y = layout.top_item_regions[menu_index].location.y + layout.top_item_regions[menu_index].size.height / 2;
}

static void seed_pressed_state(uint8_t pressed_menu, uint8_t pressed_item)
{
    EGUI_VIEW_OF(&test_menu_bar)->is_pressed = 1;
    test_menu_bar.pressed_menu = pressed_menu;
    test_menu_bar.pressed_item = pressed_item;
}

static void assert_pressed_cleared(void)
{
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_HIT_NONE, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_ITEM_NONE, test_menu_bar.pressed_item);
}

static void test_menu_bar_set_snapshots_resolves_disabled_focus_item(void)
{
    setup_menu_bar(g_disabled_focus_snapshots, 1);

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);
}

static void test_menu_bar_set_snapshots_skips_disabled_initial_snapshot(void)
{
    setup_menu_bar(g_initial_disabled_snapshots, 3);

    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);
}

static void test_menu_bar_set_current_item_skips_disabled_target(void)
{
    setup_menu_bar(g_test_snapshots, 3);

    egui_view_menu_bar_set_current_item(EGUI_VIEW_OF(&test_menu_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);

    egui_view_menu_bar_set_current_item(EGUI_VIEW_OF(&test_menu_bar), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_selection_item);
}

static void test_menu_bar_set_current_snapshot_ignores_disabled_menu_snapshot(void)
{
    setup_menu_bar(g_middle_disabled_snapshots, 3);

    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&test_menu_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);

    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&test_menu_bar), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_item);
}

static void test_menu_bar_set_current_snapshot_resets_runtime_state(void)
{
    setup_menu_bar(g_test_snapshots, 3);

    test_menu_bar.pressed_item = 2;
    test_menu_bar.pressed_menu = 1;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_menu_bar), 1);
    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&test_menu_bar), 1);

    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_ITEM_NONE, test_menu_bar.pressed_item);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_HIT_NONE, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_item);
}

static void test_menu_bar_setters_clear_pressed_state(void)
{
    setup_menu_bar(g_test_snapshots, 3);

    seed_pressed_state(0, EGUI_VIEW_MENU_BAR_ITEM_NONE);
    egui_view_menu_bar_set_font(EGUI_VIEW_OF(&test_menu_bar), NULL);
    assert_pressed_cleared();

    seed_pressed_state(0, EGUI_VIEW_MENU_BAR_ITEM_NONE);
    egui_view_menu_bar_set_meta_font(EGUI_VIEW_OF(&test_menu_bar), NULL);
    assert_pressed_cleared();

    seed_pressed_state(0, EGUI_VIEW_MENU_BAR_ITEM_NONE);
    egui_view_menu_bar_set_palette(EGUI_VIEW_OF(&test_menu_bar), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD9E0E6), EGUI_COLOR_HEX(0x18222D),
                                   EGUI_COLOR_HEX(0x6A7683), EGUI_COLOR_HEX(0x2A60C7), EGUI_COLOR_HEX(0x178454), EGUI_COLOR_HEX(0xB77719),
                                   EGUI_COLOR_HEX(0xB13A32), EGUI_COLOR_HEX(0xD7DEE5));
    assert_pressed_cleared();

    seed_pressed_state(0, EGUI_VIEW_MENU_BAR_ITEM_NONE);
    egui_view_menu_bar_set_snapshots(EGUI_VIEW_OF(&test_menu_bar), g_test_snapshots, 3);
    assert_pressed_cleared();

    seed_pressed_state(0, EGUI_VIEW_MENU_BAR_ITEM_NONE);
    egui_view_menu_bar_set_current_snapshot(EGUI_VIEW_OF(&test_menu_bar), 1);
    assert_pressed_cleared();

    seed_pressed_state(EGUI_VIEW_MENU_BAR_HIT_NONE, 1);
    egui_view_menu_bar_set_current_item(EGUI_VIEW_OF(&test_menu_bar), 2);
    assert_pressed_cleared();

    seed_pressed_state(0, EGUI_VIEW_MENU_BAR_ITEM_NONE);
    egui_view_menu_bar_set_compact_mode(EGUI_VIEW_OF(&test_menu_bar), 1);
    assert_pressed_cleared();

    seed_pressed_state(0, EGUI_VIEW_MENU_BAR_ITEM_NONE);
    egui_view_menu_bar_set_read_only_mode(EGUI_VIEW_OF(&test_menu_bar), 1);
    assert_pressed_cleared();
}

static void test_menu_bar_touch_menu_switches_snapshot(void)
{
    egui_dim_t x = 0;
    egui_dim_t y = 0;

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_menu_center(2, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_item);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_HIT_NONE, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);
}

static void test_menu_bar_touch_panel_item_selects_and_activates(void)
{
    egui_dim_t x = 0;
    egui_dim_t y = 0;

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_item_center(2, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_selection_item);
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu_bar.pressed_item);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_activation_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_activation_item);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_ITEM_NONE, test_menu_bar.pressed_item);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);
}

static void test_menu_bar_touch_cancel_clears_pressed_state_without_side_effect(void)
{
    egui_dim_t x = 0;
    egui_dim_t y = 0;

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_menu_center(2, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
    assert_pressed_cleared();
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_count);
}

static void test_menu_bar_touch_release_requires_same_hit_target(void)
{
    egui_dim_t x_start = 0;
    egui_dim_t y_start = 0;
    egui_dim_t x_end = 0;
    egui_dim_t y_end = 0;

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_menu_center(2, &x_start, &y_start);
    get_menu_center(0, &x_end, &y_end);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x_start, y_start));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_end, y_end));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x_end, y_end));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_HIT_NONE, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x_start, y_start));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_end, y_end));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_start, y_start));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x_start, y_start));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_item);

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_item_center(2, &x_start, &y_start);
    get_item_center(0, &x_end, &y_end);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x_start, y_start));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu_bar.pressed_item);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_end, y_end));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_menu_bar.pressed_item);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x_end, y_end));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_ITEM_NONE, test_menu_bar.pressed_item);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_item_center(2, &x_start, &y_start);
    get_item_center(0, &x_end, &y_end);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x_start, y_start));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_end, y_end));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, x_start, y_start));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x_start, y_start));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_activation_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_activation_item);
}

static void test_menu_bar_key_navigation_moves_menu_and_rows(void)
{
    setup_menu_bar(g_test_snapshots, 3);

    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_selection_item);

    reset_menu_bar_events();
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_item);

    reset_menu_bar_events();
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_item);

    reset_menu_bar_events();
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_menu_bar.pressed_item);
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_activation_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_item);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_ITEM_NONE, test_menu_bar.pressed_item);
}

static void test_menu_bar_key_navigation_skips_disabled_menu_snapshot(void)
{
    setup_menu_bar(g_middle_disabled_snapshots, 3);

    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_item);

    reset_menu_bar_events();
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_item);

    reset_menu_bar_events();
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_selection_snapshot);

    reset_menu_bar_events();
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&test_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_snapshot);
}

static void test_menu_bar_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x = 0;
    egui_dim_t y = 0;

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_menu_center(1, &x, &y);

    egui_view_menu_bar_set_read_only_mode(EGUI_VIEW_OF(&test_menu_bar), 1);
    seed_pressed_state(1, 2);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared();

    seed_pressed_state(1, 2);
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared();
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_count);

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_menu_center(1, &x, &y);
    egui_view_set_enable(EGUI_VIEW_OF(&test_menu_bar), 0);

    seed_pressed_state(1, 2);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared();

    seed_pressed_state(1, 2);
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared();
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_count);
}

static void test_menu_bar_read_only_mode_clears_pressed_and_ignores_input(void)
{
    egui_dim_t x = 0;
    egui_dim_t y = 0;

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_menu_center(1, &x, &y);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);

    egui_view_menu_bar_set_read_only_mode(EGUI_VIEW_OF(&test_menu_bar), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_menu_bar.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_HIT_NONE, test_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_ITEM_NONE, test_menu_bar.pressed_item);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_menu_bar)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_count);

    egui_view_menu_bar_set_read_only_mode(EGUI_VIEW_OF(&test_menu_bar), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_menu_bar.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_selection_item);
}

static void test_menu_bar_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t x = 0;
    egui_dim_t y = 0;

    setup_preview_menu_bar(g_test_snapshots, 3, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&preview_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&preview_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_menu_bar.compact_mode);
    layout_preview_menu_bar();
    get_preview_menu_center(1, &x, &y);

    EGUI_VIEW_OF(&preview_menu_bar)->is_pressed = 1;
    preview_menu_bar.pressed_menu = 1;
    preview_menu_bar.pressed_item = 1;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_menu_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_HIT_NONE, preview_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_ITEM_NONE, preview_menu_bar.pressed_item);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&preview_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&preview_menu_bar)));

    EGUI_VIEW_OF(&preview_menu_bar)->is_pressed = 1;
    preview_menu_bar.pressed_menu = EGUI_VIEW_MENU_BAR_HIT_NONE;
    preview_menu_bar.pressed_item = 1;
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_TRUE(send_preview_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_menu_bar)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_HIT_NONE, preview_menu_bar.pressed_menu);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_MENU_BAR_ITEM_NONE, preview_menu_bar.pressed_item);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_snapshot(EGUI_VIEW_OF(&preview_menu_bar)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_menu_bar_get_current_item(EGUI_VIEW_OF(&preview_menu_bar)));
}

static void test_menu_bar_disabled_ignores_input(void)
{
    egui_dim_t x = 0;
    egui_dim_t y = 0;

    setup_menu_bar(g_test_snapshots, 3);
    layout_menu_bar(196, 112);
    get_menu_center(1, &x, &y);

    egui_view_set_enable(EGUI_VIEW_OF(&test_menu_bar), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_activation_count);
}

void test_menu_bar_run(void)
{
    EGUI_TEST_SUITE_BEGIN(menu_bar);

    EGUI_TEST_RUN(test_menu_bar_set_snapshots_resolves_disabled_focus_item);
    EGUI_TEST_RUN(test_menu_bar_set_snapshots_skips_disabled_initial_snapshot);
    EGUI_TEST_RUN(test_menu_bar_set_current_item_skips_disabled_target);
    EGUI_TEST_RUN(test_menu_bar_set_current_snapshot_ignores_disabled_menu_snapshot);
    EGUI_TEST_RUN(test_menu_bar_set_current_snapshot_resets_runtime_state);
    EGUI_TEST_RUN(test_menu_bar_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_menu_bar_touch_menu_switches_snapshot);
    EGUI_TEST_RUN(test_menu_bar_touch_panel_item_selects_and_activates);
    EGUI_TEST_RUN(test_menu_bar_touch_cancel_clears_pressed_state_without_side_effect);
    EGUI_TEST_RUN(test_menu_bar_touch_release_requires_same_hit_target);
    EGUI_TEST_RUN(test_menu_bar_key_navigation_moves_menu_and_rows);
    EGUI_TEST_RUN(test_menu_bar_key_navigation_skips_disabled_menu_snapshot);
    EGUI_TEST_RUN(test_menu_bar_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_menu_bar_read_only_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_menu_bar_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_menu_bar_disabled_ignores_input);

    EGUI_TEST_SUITE_END();
}
