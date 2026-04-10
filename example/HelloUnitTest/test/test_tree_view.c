#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_tree_view.h"

#include "../../HelloCustomWidgets/navigation/tree_view/egui_view_tree_view.h"
#include "../../HelloCustomWidgets/navigation/tree_view/egui_view_tree_view.c"

static egui_view_tree_view_t test_tree_view;
static egui_view_tree_view_t preview_tree_view;
static egui_view_api_t preview_api;
static uint8_t selection_change_count;
static uint8_t last_selection_index;

static const egui_view_tree_view_item_t tree_items_primary[] = {
        {"Workspace", "3", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Controls", "12", 1, EGUI_VIEW_TREE_VIEW_TONE_ACCENT, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Buttons", "", 2, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
        {"Tree View", "Draft", 2, EGUI_VIEW_TREE_VIEW_TONE_ACCENT, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
};

static const egui_view_tree_view_item_t tree_items_secondary[] = {
        {"Workspace", "3", 0, EGUI_VIEW_TREE_VIEW_TONE_NEUTRAL, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"Docs", "7", 1, EGUI_VIEW_TREE_VIEW_TONE_ACCENT, EGUI_VIEW_TREE_VIEW_KIND_FOLDER, 1, 1},
        {"API", "New", 2, EGUI_VIEW_TREE_VIEW_TONE_SUCCESS, EGUI_VIEW_TREE_VIEW_KIND_LEAF, 0, 0},
};

static const egui_view_tree_view_snapshot_t tree_snapshots[] = {
        {"Solution", "4 rows", "Controls open", tree_items_primary, 4, 1},
        {"Docs", "3 rows", "API highlighted", tree_items_secondary, 3, 2},
};

static const egui_view_tree_view_snapshot_t overflow_snapshots[] = {
        {"Solution", "4 rows", "Controls open", tree_items_primary, 4, 1},
        {"Docs", "3 rows", "API highlighted", tree_items_secondary, 3, 2},
        {"Solution", "4 rows", "Controls open", tree_items_primary, 4, 2},
        {"Docs", "3 rows", "API highlighted", tree_items_secondary, 3, 0},
        {"Solution", "4 rows", "Controls open", tree_items_primary, 4, 3},
        {"Docs", "3 rows", "API highlighted", tree_items_secondary, 3, 1},
        {"Solution", "4 rows", "Controls open", tree_items_primary, 4, 0},
};

static void on_selection_changed(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(self);
    selection_change_count++;
    last_selection_index = index;
}

static void reset_listener_state(void)
{
    selection_change_count = 0;
    last_selection_index = EGUI_VIEW_TREE_VIEW_INDEX_NONE;
}

static void setup_widget(void)
{
    egui_view_tree_view_init(EGUI_VIEW_OF(&test_tree_view));
    egui_view_set_size(EGUI_VIEW_OF(&test_tree_view), 144, 120);
    egui_view_tree_view_set_snapshots(EGUI_VIEW_OF(&test_tree_view), tree_snapshots, (uint8_t)(sizeof(tree_snapshots) / sizeof(tree_snapshots[0])));
    egui_view_tree_view_set_on_selection_changed_listener(EGUI_VIEW_OF(&test_tree_view), on_selection_changed);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    egui_view_tree_view_init(EGUI_VIEW_OF(&preview_tree_view));
    egui_view_set_size(EGUI_VIEW_OF(&preview_tree_view), 104, 80);
    egui_view_tree_view_set_snapshots(EGUI_VIEW_OF(&preview_tree_view), tree_snapshots, (uint8_t)(sizeof(tree_snapshots) / sizeof(tree_snapshots[0])));
    egui_view_tree_view_set_current_snapshot(EGUI_VIEW_OF(&preview_tree_view), 1);
    egui_view_tree_view_set_compact_mode(EGUI_VIEW_OF(&preview_tree_view), 1);
    egui_view_tree_view_set_on_selection_changed_listener(EGUI_VIEW_OF(&preview_tree_view), on_selection_changed);
    egui_view_tree_view_override_static_preview_api(EGUI_VIEW_OF(&preview_tree_view), &preview_api);
    reset_listener_state();
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
    layout_view(EGUI_VIEW_OF(&test_tree_view), 10, 20, 144, 120);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_tree_view), 12, 18, 104, 80);
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

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= view->api->on_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->on_key_event(view, &event);
    return handled;
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_tree_view), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_tree_view), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_tree_view), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_tree_view), key_code);
}

static uint8_t get_item_center_for_tree(egui_view_tree_view_t *tree, uint8_t index, egui_dim_t *x, egui_dim_t *y)
{
    egui_view_tree_view_metrics_t metrics;
    const egui_view_tree_view_snapshot_t *snapshot = egui_view_tree_view_get_snapshot(tree);
    uint8_t item_count = snapshot == NULL ? 0 : egui_view_tree_view_clamp_item_count(snapshot->item_count);

    if (snapshot == NULL || item_count == 0)
    {
        return 0;
    }

    egui_view_tree_view_get_metrics(tree, EGUI_VIEW_OF(tree), item_count, &metrics);
    if (index >= metrics.visible_item_count)
    {
        return 0;
    }

    *x = metrics.item_regions[index].location.x + metrics.item_regions[index].size.width / 2;
    *y = metrics.item_regions[index].location.y + metrics.item_regions[index].size.height / 2;
    return 1;
}

static void seed_pressed_state(egui_view_tree_view_t *tree, uint8_t index, uint8_t visual_pressed)
{
    egui_view_set_pressed(EGUI_VIEW_OF(tree), 0);
    tree->pressed_index = index;
    if (visual_pressed)
    {
        egui_view_set_pressed(EGUI_VIEW_OF(tree), 1);
    }
}

static void assert_pressed_cleared(egui_view_tree_view_t *tree)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TREE_VIEW_INDEX_NONE, tree->pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(tree)->is_pressed);
}

static void test_tree_view_set_snapshots_clamps_and_clears_pressed_state(void)
{
    setup_widget();

    test_tree_view.current_snapshot = EGUI_VIEW_TREE_VIEW_MAX_SNAPSHOTS;
    test_tree_view.current_index = 3;
    seed_pressed_state(&test_tree_view, 3, 1);
    egui_view_tree_view_set_snapshots(EGUI_VIEW_OF(&test_tree_view), overflow_snapshots, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_TREE_VIEW_MAX_SNAPSHOTS, test_tree_view.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tree_view_get_current_snapshot(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    assert_pressed_cleared(&test_tree_view);

    test_tree_view.current_snapshot = 1;
    test_tree_view.current_index = 2;
    seed_pressed_state(&test_tree_view, 2, 1);
    egui_view_tree_view_set_snapshots(EGUI_VIEW_OF(&test_tree_view), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tree_view.snapshot_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tree_view_get_current_snapshot(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    assert_pressed_cleared(&test_tree_view);
}

static void test_tree_view_snapshot_index_and_setters_clear_pressed_state(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t section = EGUI_COLOR_HEX(0x202122);
    egui_color_t border = EGUI_COLOR_HEX(0x303132);
    egui_color_t text = EGUI_COLOR_HEX(0x404142);
    egui_color_t muted = EGUI_COLOR_HEX(0x505152);
    egui_color_t accent = EGUI_COLOR_HEX(0x606162);
    egui_color_t success = EGUI_COLOR_HEX(0x707172);
    egui_color_t warning = EGUI_COLOR_HEX(0x808182);
    egui_color_t neutral = EGUI_COLOR_HEX(0x909192);

    setup_widget();

    egui_view_tree_view_set_current_index(EGUI_VIEW_OF(&test_tree_view), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, selection_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, last_selection_index);

    seed_pressed_state(&test_tree_view, 2, 1);
    egui_view_tree_view_set_current_index(EGUI_VIEW_OF(&test_tree_view), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, selection_change_count);
    assert_pressed_cleared(&test_tree_view);

    seed_pressed_state(&test_tree_view, 1, 1);
    egui_view_tree_view_set_current_snapshot(EGUI_VIEW_OF(&test_tree_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tree_view_get_current_snapshot(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, selection_change_count);
    assert_pressed_cleared(&test_tree_view);

    seed_pressed_state(&test_tree_view, 1, 0);
    egui_view_tree_view_set_current_snapshot(EGUI_VIEW_OF(&test_tree_view), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tree_view_get_current_snapshot(EGUI_VIEW_OF(&test_tree_view)));
    assert_pressed_cleared(&test_tree_view);

    seed_pressed_state(&test_tree_view, 1, 1);
    egui_view_tree_view_set_font(EGUI_VIEW_OF(&test_tree_view), NULL);
    assert_pressed_cleared(&test_tree_view);
    EGUI_TEST_ASSERT_TRUE(test_tree_view.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_tree_view, 1, 0);
    egui_view_tree_view_set_meta_font(EGUI_VIEW_OF(&test_tree_view), NULL);
    assert_pressed_cleared(&test_tree_view);
    EGUI_TEST_ASSERT_TRUE(test_tree_view.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_tree_view, 1, 1);
    egui_view_tree_view_set_palette(EGUI_VIEW_OF(&test_tree_view), surface, section, border, text, muted, accent, success, warning, neutral);
    assert_pressed_cleared(&test_tree_view);
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_tree_view.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(section.full, test_tree_view.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_tree_view.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_tree_view.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_tree_view.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_tree_view.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(success.full, test_tree_view.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(warning.full, test_tree_view.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(neutral.full, test_tree_view.neutral_color.full);

    seed_pressed_state(&test_tree_view, 1, 0);
    egui_view_tree_view_set_compact_mode(EGUI_VIEW_OF(&test_tree_view), 2);
    assert_pressed_cleared(&test_tree_view);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tree_view.compact_mode);

    seed_pressed_state(&test_tree_view, 1, 1);
    egui_view_tree_view_set_read_only_mode(EGUI_VIEW_OF(&test_tree_view), 3);
    assert_pressed_cleared(&test_tree_view);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tree_view.read_only_mode);
}

static void test_tree_view_snapshot_switch_and_index_clamp(void)
{
    setup_widget();
    egui_view_tree_view_set_current_index(EGUI_VIEW_OF(&test_tree_view), 3);
    reset_listener_state();

    egui_view_tree_view_set_current_snapshot(EGUI_VIEW_OF(&test_tree_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tree_view_get_current_snapshot(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, selection_change_count);

    setup_widget();
    egui_view_tree_view_set_current_index(EGUI_VIEW_OF(&test_tree_view), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, selection_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, last_selection_index);
}

static void test_tree_view_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t x2;
    egui_dim_t y2;
    egui_dim_t x3;
    egui_dim_t y3;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_tree(&test_tree_view, 2, &x2, &y2));
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_tree(&test_tree_view, 3, &x3, &y3));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_tree_view.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tree_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x3, y3));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_tree_view.pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tree_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x3, y3));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, selection_change_count);
    assert_pressed_cleared(&test_tree_view);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x2, y2));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x3, y3));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tree_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tree_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, selection_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, last_selection_index);
    assert_pressed_cleared(&test_tree_view);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x3, y3));
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_tree_view.pressed_index);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, x3, y3));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, selection_change_count);
    assert_pressed_cleared(&test_tree_view);
}

static void test_tree_view_keyboard_navigation(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, selection_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, last_selection_index);
}

static void test_tree_view_compact_mode_clears_pressed_and_keeps_selection_behavior(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget();
    egui_view_tree_view_set_current_index(EGUI_VIEW_OF(&test_tree_view), 2);
    reset_listener_state();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_tree(&test_tree_view, 0, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_tree_view.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tree_view)->is_pressed);

    egui_view_tree_view_set_compact_mode(EGUI_VIEW_OF(&test_tree_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tree_view.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    assert_pressed_cleared(&test_tree_view);

    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_tree(&test_tree_view, 0, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, selection_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_selection_index);
}

static void test_tree_view_read_only_and_view_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_tree(&test_tree_view, 3, &x, &y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tree_view)->is_pressed);
    egui_view_tree_view_set_read_only_mode(EGUI_VIEW_OF(&test_tree_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_tree_view.read_only_mode);
    assert_pressed_cleared(&test_tree_view);

    seed_pressed_state(&test_tree_view, 1, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&test_tree_view);

    seed_pressed_state(&test_tree_view, 1, 0);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_END));
    assert_pressed_cleared(&test_tree_view);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, selection_change_count);

    egui_view_tree_view_set_read_only_mode(EGUI_VIEW_OF(&test_tree_view), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, selection_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, last_selection_index);

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_tree(&test_tree_view, 2, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tree_view)->is_pressed);

    egui_view_set_enable(EGUI_VIEW_OF(&test_tree_view), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&test_tree_view);

    seed_pressed_state(&test_tree_view, 1, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_END));
    assert_pressed_cleared(&test_tree_view);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, selection_change_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_tree_view), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&test_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, selection_change_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, last_selection_index);
}

static void test_tree_view_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_tree(&preview_tree_view, 1, &x, &y));

    seed_pressed_state(&preview_tree_view, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&preview_tree_view);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&preview_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, selection_change_count);

    seed_pressed_state(&preview_tree_view, 1, 0);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_END));
    assert_pressed_cleared(&preview_tree_view);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_tree_view_get_current_index(EGUI_VIEW_OF(&preview_tree_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, selection_change_count);
}

void test_tree_view_run(void)
{
    EGUI_TEST_SUITE_BEGIN(tree_view);
    EGUI_TEST_RUN(test_tree_view_set_snapshots_clamps_and_clears_pressed_state);
    EGUI_TEST_RUN(test_tree_view_snapshot_index_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_tree_view_snapshot_switch_and_index_clamp);
    EGUI_TEST_RUN(test_tree_view_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_tree_view_keyboard_navigation);
    EGUI_TEST_RUN(test_tree_view_compact_mode_clears_pressed_and_keeps_selection_behavior);
    EGUI_TEST_RUN(test_tree_view_read_only_and_view_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_tree_view_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
