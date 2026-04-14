#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_expander.h"

#include "../../HelloCustomWidgets/layout/expander/egui_view_expander.h"
#include "../../HelloCustomWidgets/layout/expander/egui_view_expander.c"

static egui_view_expander_t test_expander;
static egui_view_expander_t preview_expander;
static egui_view_api_t preview_api;
static uint8_t g_selection_count;
static uint8_t g_last_selection_index;
static uint8_t g_expanded_count;
static uint8_t g_last_expanded_index;
static uint8_t g_last_expanded_value;

static const egui_view_expander_item_t g_items[] = {
        {"WORK", "Workspace policy", "Ready", "Pinned groups stay open", "Draft rules stay visible", "Always on", EGUI_VIEW_EXPANDER_TONE_ACCENT, 1},
        {"SYNC", "Sync rules", "3 rules", "Metered uploads wait for Wi-Fi", "Night copy stays local", "Queue review", EGUI_VIEW_EXPANDER_TONE_SUCCESS, 0},
        {"RELEASE", "Release notes", "Hold", "Pilot warnings stay staged", "Manual signoff closes it", "Manual hold", EGUI_VIEW_EXPANDER_TONE_WARNING, 1},
};

static const egui_view_expander_item_t g_overflow_items[] = {
        {"A", "Alpha", "1", "A", "A", "Open", EGUI_VIEW_EXPANDER_TONE_ACCENT, 0},
        {"B", "Beta", "2", "B", "B", "Open", EGUI_VIEW_EXPANDER_TONE_SUCCESS, 0},
        {"C", "Gamma", "3", "C", "C", "Open", EGUI_VIEW_EXPANDER_TONE_WARNING, 0},
        {"D", "Delta", "4", "D", "D", "Open", EGUI_VIEW_EXPANDER_TONE_NEUTRAL, 0},
        {"E", "Echo", "5", "E", "E", "Open", EGUI_VIEW_EXPANDER_TONE_ACCENT, 0},
};

static void on_selection_changed(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(self);
    g_selection_count++;
    g_last_selection_index = index;
}

static void on_expanded_changed(egui_view_t *self, uint8_t index, uint8_t expanded)
{
    EGUI_UNUSED(self);
    g_expanded_count++;
    g_last_expanded_index = index;
    g_last_expanded_value = expanded;
}

static void reset_listener_state(void)
{
    g_selection_count = 0;
    g_last_selection_index = EGUI_VIEW_EXPANDER_INDEX_NONE;
    g_expanded_count = 0;
    g_last_expanded_index = EGUI_VIEW_EXPANDER_INDEX_NONE;
    g_last_expanded_value = 0xFF;
}

static void setup_expander(void)
{
    egui_view_expander_init(EGUI_VIEW_OF(&test_expander));
    egui_view_set_size(EGUI_VIEW_OF(&test_expander), 194, 110);
    egui_view_expander_set_items(EGUI_VIEW_OF(&test_expander), g_items, 3);
    egui_view_expander_set_on_selection_changed_listener(EGUI_VIEW_OF(&test_expander), on_selection_changed);
    egui_view_expander_set_on_expanded_changed_listener(EGUI_VIEW_OF(&test_expander), on_expanded_changed);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_expander), 1);
#endif
    reset_listener_state();
}

static void setup_preview_expander(void)
{
    egui_view_expander_init(EGUI_VIEW_OF(&preview_expander));
    egui_view_set_size(EGUI_VIEW_OF(&preview_expander), 104, 76);
    egui_view_expander_set_items(EGUI_VIEW_OF(&preview_expander), g_items, 3);
    egui_view_expander_set_current_index(EGUI_VIEW_OF(&preview_expander), 1);
    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&preview_expander), 1);
    egui_view_expander_set_compact_mode(EGUI_VIEW_OF(&preview_expander), 1);
    egui_view_expander_set_on_selection_changed_listener(EGUI_VIEW_OF(&preview_expander), on_selection_changed);
    egui_view_expander_set_on_expanded_changed_listener(EGUI_VIEW_OF(&preview_expander), on_expanded_changed);
    egui_view_expander_override_static_preview_api(EGUI_VIEW_OF(&preview_expander), &preview_api);
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

static void layout_expander(egui_dim_t width, egui_dim_t height)
{
    layout_view(EGUI_VIEW_OF(&test_expander), 10, 20, width, height);
}

static void layout_preview_expander(void)
{
    layout_view(EGUI_VIEW_OF(&preview_expander), 12, 18, 104, 76);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_expander), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_expander), type, x, y);
}

static int send_key_action(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_expander), type, key_code);
}

static int send_preview_key_action(uint8_t type, uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_expander), type, key_code);
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

static void seed_pressed_state(egui_view_expander_t *widget, egui_view_t *view, uint8_t pressed_index, uint8_t visual_pressed)
{
    widget->pressed_index = pressed_index;
    egui_view_set_pressed(view, visual_pressed ? true : false);
}

static void assert_pressed_cleared(egui_view_expander_t *widget, egui_view_t *view)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_EXPANDER_INDEX_NONE, widget->pressed_index);
    EGUI_TEST_ASSERT_FALSE(view->is_pressed);
}

static void assert_widget_state(egui_view_expander_t *widget, uint8_t current_index, uint8_t expanded_index, uint8_t compact_mode, uint8_t read_only_mode)
{
    EGUI_TEST_ASSERT_EQUAL_INT(current_index, egui_view_expander_get_current_index(EGUI_VIEW_OF(widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(expanded_index, egui_view_expander_get_expanded_index(EGUI_VIEW_OF(widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(compact_mode, widget->compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(read_only_mode, widget->read_only_mode);
}

static void assert_listener_state(uint8_t selection_count, uint8_t selection_index, uint8_t expanded_count, uint8_t expanded_index, uint8_t expanded_value)
{
    EGUI_TEST_ASSERT_EQUAL_INT(selection_count, g_selection_count);
    EGUI_TEST_ASSERT_EQUAL_INT(selection_index, g_last_selection_index);
    EGUI_TEST_ASSERT_EQUAL_INT(expanded_count, g_expanded_count);
    EGUI_TEST_ASSERT_EQUAL_INT(expanded_index, g_last_expanded_index);
    EGUI_TEST_ASSERT_EQUAL_INT(expanded_value, g_last_expanded_value);
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

static void get_metrics(egui_view_expander_metrics_t *metrics)
{
    expander_get_metrics(&test_expander, EGUI_VIEW_OF(&test_expander), metrics);
}

static void get_header_center(uint8_t index, egui_dim_t *x, egui_dim_t *y)
{
    egui_view_expander_metrics_t metrics;

    get_metrics(&metrics);
    *x = metrics.header_regions[index].location.x + metrics.header_regions[index].size.width / 2;
    *y = metrics.header_regions[index].location.y + metrics.header_regions[index].size.height / 2;
}

static void test_expander_set_items_clamp_and_listener_guards(void)
{
    setup_expander();

    test_expander.current_index = 7;
    test_expander.expanded_index = 8;
    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 2, 1);
    egui_view_expander_set_items(EGUI_VIEW_OF(&test_expander), g_overflow_items, 5);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_EXPANDER_MAX_ITEMS, egui_view_expander_get_item_count(EGUI_VIEW_OF(&test_expander)));
    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    EGUI_TEST_ASSERT_TRUE(expander_get_current_item(&test_expander) == &g_overflow_items[0]);

    egui_view_expander_set_current_index(EGUI_VIEW_OF(&test_expander), 2);
    assert_widget_state(&test_expander, 2, 0, 0, 0);
    assert_listener_state(1, 2, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 2, 1);
    egui_view_expander_set_current_index(EGUI_VIEW_OF(&test_expander), 2);
    assert_widget_state(&test_expander, 2, 0, 0, 0);
    assert_listener_state(1, 2, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    egui_view_expander_set_current_index(EGUI_VIEW_OF(&test_expander), 9);
    assert_widget_state(&test_expander, 3, 0, 0, 0);
    assert_listener_state(2, 3, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&test_expander), 9);
    assert_widget_state(&test_expander, 3, 3, 0, 0);
    assert_listener_state(2, 3, 1, 3, 1);

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 3, 1);
    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&test_expander), 3);
    assert_widget_state(&test_expander, 3, 3, 0, 0);
    assert_listener_state(2, 3, 1, 3, 1);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    egui_view_expander_set_items(EGUI_VIEW_OF(&test_expander), NULL, 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_expander_get_item_count(EGUI_VIEW_OF(&test_expander)));
    assert_widget_state(&test_expander, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);
    EGUI_TEST_ASSERT_TRUE(expander_get_current_item(&test_expander) == NULL);
}

static void test_expander_font_palette_helpers_and_expand_listener(void)
{
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_expander();
    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 1, 1);
    egui_view_expander_set_font(EGUI_VIEW_OF(&test_expander), NULL);
    EGUI_TEST_ASSERT_TRUE(test_expander.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 1, 1);
    egui_view_expander_set_meta_font(EGUI_VIEW_OF(&test_expander), NULL);
    EGUI_TEST_ASSERT_TRUE(test_expander.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 1, 1);
    egui_view_expander_set_compact_mode(EGUI_VIEW_OF(&test_expander), 2);
    assert_widget_state(&test_expander, 0, 0, 1, 0);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));
    egui_view_expander_set_compact_mode(EGUI_VIEW_OF(&test_expander), 0);
    assert_widget_state(&test_expander, 0, 0, 0, 0);

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 1, 1);
    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&test_expander), 0);
    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 2, 1);
    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&test_expander), EGUI_VIEW_EXPANDER_INDEX_NONE);
    assert_widget_state(&test_expander, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 1, 0, 0);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 2, 1);
    egui_view_expander_set_read_only_mode(EGUI_VIEW_OF(&test_expander), 3);
    assert_widget_state(&test_expander, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 1);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));
    egui_view_expander_set_read_only_mode(EGUI_VIEW_OF(&test_expander), 0);
    assert_widget_state(&test_expander, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 2, 1);
    egui_view_expander_set_palette(EGUI_VIEW_OF(&test_expander), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                   EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172),
                                   EGUI_COLOR_HEX(0x808182), EGUI_COLOR_HEX(0x909192));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_expander.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_expander.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_expander.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_expander.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_expander.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, test_expander.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, test_expander.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, test_expander.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x909192).full, test_expander.neutral_color.full);
    assert_widget_state(&test_expander, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_EXPANDER_MAX_ITEMS, expander_clamp_item_count(9));
    EGUI_TEST_ASSERT_EQUAL_INT(0, expander_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(6, expander_text_len("Review"));
    EGUI_TEST_ASSERT_EQUAL_INT(47, expander_meta_width("Ready", 0, 80));
    EGUI_TEST_ASSERT_EQUAL_INT(20, expander_meta_width("A", 1, 80));
    EGUI_TEST_ASSERT_EQUAL_INT(20, expander_pill_width(NULL, 0, 20, 80));
    EGUI_TEST_ASSERT_EQUAL_INT(28, expander_pill_width("Open", 1, 12, 28));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, expander_tone_color(&test_expander, EGUI_VIEW_EXPANDER_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x707172).full, expander_tone_color(&test_expander, EGUI_VIEW_EXPANDER_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x808182).full, expander_tone_color(&test_expander, EGUI_VIEW_EXPANDER_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x909192).full, expander_tone_color(&test_expander, EGUI_VIEW_EXPANDER_TONE_NEUTRAL).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, expander_tone_color(&test_expander, 99).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 68).full, expander_mix_disabled(sample).full);

    egui_view_expander_toggle_current(EGUI_VIEW_OF(&test_expander));
    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 2, 0, 1);
}

static void test_expander_metrics_and_hit_testing(void)
{
    egui_view_expander_metrics_t metrics;
    egui_dim_t x0;
    egui_dim_t y0;
    egui_dim_t x2;
    egui_dim_t y2;

    setup_expander();
    layout_expander(194, 110);
    get_metrics(&metrics);
    EGUI_TEST_ASSERT_TRUE(metrics.content_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.content_region.size.height > 0);
    EGUI_TEST_ASSERT_TRUE(metrics.header_regions[1].location.y > metrics.header_regions[0].location.y);
    EGUI_TEST_ASSERT_TRUE(metrics.body_regions[0].size.height > 0);
    EGUI_TEST_ASSERT_EQUAL_INT(metrics.body_regions[0].size.height, metrics.body_height);
    EGUI_TEST_ASSERT_EQUAL_INT(0, metrics.body_regions[1].size.height);

    get_header_center(0, &x0, &y0);
    get_header_center(2, &x2, &y2);
    EGUI_TEST_ASSERT_EQUAL_INT(0, expander_hit_index(&test_expander, EGUI_VIEW_OF(&test_expander), x0, y0));
    EGUI_TEST_ASSERT_EQUAL_INT(2, expander_hit_index(&test_expander, EGUI_VIEW_OF(&test_expander), x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_EXPANDER_INDEX_NONE,
                               expander_hit_index(&test_expander, EGUI_VIEW_OF(&test_expander), metrics.body_regions[0].location.x + 2,
                                                  metrics.body_regions[0].location.y + metrics.body_regions[0].size.height / 2));

    EGUI_TEST_ASSERT_TRUE(expander_get_current_item(&test_expander) == &g_items[0]);
    test_expander.current_index = 8;
    EGUI_TEST_ASSERT_TRUE(expander_get_current_item(&test_expander) == NULL);
    test_expander.current_index = 0;

    egui_view_expander_set_compact_mode(EGUI_VIEW_OF(&test_expander), 1);
    egui_view_set_size(EGUI_VIEW_OF(&test_expander), 108, 76);
    layout_expander(108, 76);
    get_metrics(&metrics);
    assert_widget_state(&test_expander, 0, 0, 1, 0);
    EGUI_TEST_ASSERT_TRUE(metrics.header_regions[0].size.height <= EGUI_VIEW_EXPANDER_COMPACT_HEADER_HEIGHT);
    EGUI_TEST_ASSERT_TRUE(metrics.body_regions[0].size.height <= EGUI_VIEW_EXPANDER_COMPACT_BODY_HEIGHT);
}

static void test_expander_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t header0_x;
    egui_dim_t header0_y;
    egui_dim_t header1_x;
    egui_dim_t header1_y;
    egui_dim_t header2_x;
    egui_dim_t header2_y;
    egui_dim_t outside_x;
    egui_dim_t outside_y;

    setup_expander();
    layout_expander(194, 110);
    get_header_center(0, &header0_x, &header0_y);
    get_header_center(1, &header1_x, &header1_y);
    get_header_center(2, &header2_x, &header2_y);
    get_view_outside_point(EGUI_VIEW_OF(&test_expander), &outside_x, &outside_y);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, 0, 0));
    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header1_x, header1_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_expander.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_expander)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_expander)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, outside_x, outside_y));
    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header1_x, header1_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_expander)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, header1_x, header1_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_expander)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, header1_x, header1_y));
    assert_widget_state(&test_expander, 1, 1, 0, 0);
    assert_listener_state(1, 1, 1, 1, 1);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header2_x, header2_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, outside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_expander)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, header2_x, header2_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_expander)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, header2_x, header2_y));
    assert_widget_state(&test_expander, 2, 2, 0, 0);
    assert_listener_state(2, 2, 2, 2, 1);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header0_x, header0_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_expander.pressed_index);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, header0_x, header0_y));
    assert_widget_state(&test_expander, 2, 2, 0, 0);
    assert_listener_state(2, 2, 2, 2, 1);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));
}

static void test_expander_compact_mode_clears_pressed_and_keeps_toggle_behavior(void)
{
    egui_dim_t x1;
    egui_dim_t y1;

    setup_expander();
    layout_expander(194, 110);
    get_header_center(1, &x1, &y1);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_expander.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_expander)->is_pressed);

    egui_view_expander_set_compact_mode(EGUI_VIEW_OF(&test_expander), 1);
    assert_widget_state(&test_expander, 0, 0, 1, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    get_header_center(1, &x1, &y1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x1, y1));
    assert_widget_state(&test_expander, 1, 1, 1, 0);
    assert_listener_state(1, 1, 1, 1, 1);
}

static void test_expander_keyboard_navigation_and_guards(void)
{
    setup_expander();

    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    assert_widget_state(&test_expander, 1, 0, 0, 0);
    assert_listener_state(1, 1, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    assert_widget_state(&test_expander, 2, 0, 0, 0);
    assert_listener_state(2, 2, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_listener_state(3, 0, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_listener_state(3, 0, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    assert_widget_state(&test_expander, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);
    assert_listener_state(3, 0, 1, 0, 0);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    assert_widget_state(&test_expander, 0, 0, 0, 0);
    assert_listener_state(3, 0, 2, 0, 1);
}

static void test_expander_read_only_mode_ignores_input_and_clears_pressed_state(void)
{
    egui_dim_t header2_x;
    egui_dim_t header2_y;

    setup_expander();
    layout_expander(194, 110);
    egui_view_expander_set_current_index(EGUI_VIEW_OF(&test_expander), 1);
    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&test_expander), 1);
    reset_listener_state();
    get_header_center(2, &header2_x, &header2_y);

    assert_widget_state(&test_expander, 1, 1, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 1, 1);
    egui_view_expander_set_read_only_mode(EGUI_VIEW_OF(&test_expander), 1);
    assert_widget_state(&test_expander, 1, 1, 0, 1);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 2, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header2_x, header2_y));
    assert_widget_state(&test_expander, 1, 1, 0, 1);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, header2_x, header2_y));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 2, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    assert_widget_state(&test_expander, 1, 1, 0, 1);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 2, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    assert_widget_state(&test_expander, 1, 1, 0, 1);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    egui_view_expander_set_read_only_mode(EGUI_VIEW_OF(&test_expander), 0);
    assert_widget_state(&test_expander, 1, 1, 0, 0);
    reset_listener_state();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    assert_widget_state(&test_expander, 2, 1, 0, 0);
    assert_listener_state(1, 2, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    assert_widget_state(&test_expander, 2, 2, 0, 0);
    assert_listener_state(1, 2, 1, 2, 1);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    assert_widget_state(&test_expander, 2, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);
    assert_listener_state(1, 2, 2, 2, 0);
}

static void test_expander_view_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_dim_t header2_x;
    egui_dim_t header2_y;

    setup_expander();
    layout_expander(194, 110);
    egui_view_expander_set_current_index(EGUI_VIEW_OF(&test_expander), 1);
    egui_view_expander_set_expanded_index(EGUI_VIEW_OF(&test_expander), 1);
    reset_listener_state();
    get_header_center(2, &header2_x, &header2_y);

    assert_widget_state(&test_expander, 1, 1, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 1, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_expander), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, header2_x, header2_y));
    assert_widget_state(&test_expander, 1, 1, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, header2_x, header2_y));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 1, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    assert_widget_state(&test_expander, 1, 1, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 1, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_SPACE));
    assert_widget_state(&test_expander, 1, 1, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));
    egui_view_set_enable(EGUI_VIEW_OF(&test_expander), 1);
    assert_widget_state(&test_expander, 1, 1, 0, 0);
    reset_listener_state();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    assert_widget_state(&test_expander, 2, 1, 0, 0);
    assert_listener_state(1, 2, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    assert_widget_state(&test_expander, 2, 2, 0, 0);
    assert_listener_state(1, 2, 1, 2, 1);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_SPACE));
    assert_widget_state(&test_expander, 2, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);
    assert_listener_state(1, 2, 2, 2, 0);
}

static void test_expander_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_expander();
    layout_preview_expander();
    get_view_center(EGUI_VIEW_OF(&preview_expander), &x, &y);

    assert_widget_state(&preview_expander, 1, 1, 1, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    seed_pressed_state(&preview_expander, EGUI_VIEW_OF(&preview_expander), 2, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_widget_state(&preview_expander, 1, 1, 1, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&preview_expander, EGUI_VIEW_OF(&preview_expander));

    seed_pressed_state(&preview_expander, EGUI_VIEW_OF(&preview_expander), 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_DOWN));
    assert_widget_state(&preview_expander, 1, 1, 1, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&preview_expander, EGUI_VIEW_OF(&preview_expander));
}

static void test_expander_empty_items_ignore_input(void)
{
    egui_dim_t x0;
    egui_dim_t y0;

    setup_expander();
    layout_expander(194, 110);
    get_view_center(EGUI_VIEW_OF(&test_expander), &x0, &y0);

    egui_view_expander_set_items(EGUI_VIEW_OF(&test_expander), NULL, 0);
    assert_widget_state(&test_expander, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 0, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x0, y0));
    assert_widget_state(&test_expander, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));

    seed_pressed_state(&test_expander, EGUI_VIEW_OF(&test_expander), 0, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    assert_widget_state(&test_expander, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, 0);
    assert_listener_state(0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0, EGUI_VIEW_EXPANDER_INDEX_NONE, 0xFF);
    assert_pressed_cleared(&test_expander, EGUI_VIEW_OF(&test_expander));
}

void test_expander_run(void)
{
    EGUI_TEST_SUITE_BEGIN(expander);
    EGUI_TEST_RUN(test_expander_set_items_clamp_and_listener_guards);
    EGUI_TEST_RUN(test_expander_font_palette_helpers_and_expand_listener);
    EGUI_TEST_RUN(test_expander_metrics_and_hit_testing);
    EGUI_TEST_RUN(test_expander_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_expander_compact_mode_clears_pressed_and_keeps_toggle_behavior);
    EGUI_TEST_RUN(test_expander_keyboard_navigation_and_guards);
    EGUI_TEST_RUN(test_expander_read_only_mode_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_expander_view_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_expander_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_expander_empty_items_ignore_input);
    EGUI_TEST_SUITE_END();
}
