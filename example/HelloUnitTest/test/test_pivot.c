#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_pivot.h"

#include "../../HelloCustomWidgets/navigation/pivot/egui_view_pivot.h"
#include "../../HelloCustomWidgets/navigation/pivot/egui_view_pivot.c"

static hcw_pivot_t test_pivot;
static hcw_pivot_t preview_pivot;
static egui_view_api_t preview_api;
static uint8_t g_listener_index;
static uint8_t g_listener_count;

static const hcw_pivot_item_t g_primary_items[] = {
        {"Overview", "Core", "Overview", "Goals and owner", "1 / 3", HCW_PIVOT_TONE_ACCENT},
        {"Activity", "Feed", "Activity", "Recent notes", "2 / 3", HCW_PIVOT_TONE_WARM},
        {"History", "Past", "History", "Archived work", "3 / 3", HCW_PIVOT_TONE_SUCCESS},
};

static const hcw_pivot_item_t g_preview_items[] = {
        {"Home", "Quick", "Home", "Pinned", "1 / 2", HCW_PIVOT_TONE_ACCENT},
        {"Logs", "Quick", "Logs", "Recent", "2 / 2", HCW_PIVOT_TONE_NEUTRAL},
};

static const hcw_pivot_item_t g_overflow_items[] = {
        {"One", "M", "One", "One", "1", HCW_PIVOT_TONE_NEUTRAL},
        {"Two", "M", "Two", "Two", "2", HCW_PIVOT_TONE_NEUTRAL},
        {"Three", "M", "Three", "Three", "3", HCW_PIVOT_TONE_NEUTRAL},
        {"Four", "M", "Four", "Four", "4", HCW_PIVOT_TONE_NEUTRAL},
        {"Five", "M", "Five", "Five", "5", HCW_PIVOT_TONE_NEUTRAL},
        {"Six", "M", "Six", "Six", "6", HCW_PIVOT_TONE_NEUTRAL},
        {"Seven", "M", "Seven", "Seven", "7", HCW_PIVOT_TONE_NEUTRAL},
};

static void on_pivot_changed(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(self);
    g_listener_index = index;
    g_listener_count++;
}

static void reset_listener_state(void)
{
    g_listener_index = HCW_PIVOT_INDEX_NONE;
    g_listener_count = 0;
}

static void setup_widget(void)
{
    hcw_pivot_init(EGUI_VIEW_OF(&test_pivot));
    egui_view_set_size(EGUI_VIEW_OF(&test_pivot), 196, 96);
    hcw_pivot_set_items(EGUI_VIEW_OF(&test_pivot), g_primary_items, EGUI_ARRAY_SIZE(g_primary_items));
    hcw_pivot_set_on_changed_listener(EGUI_VIEW_OF(&test_pivot), on_pivot_changed);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    hcw_pivot_init(EGUI_VIEW_OF(&preview_pivot));
    egui_view_set_size(EGUI_VIEW_OF(&preview_pivot), 104, 56);
    hcw_pivot_set_items(EGUI_VIEW_OF(&preview_pivot), g_preview_items, EGUI_ARRAY_SIZE(g_preview_items));
    hcw_pivot_apply_compact_style(EGUI_VIEW_OF(&preview_pivot));
    hcw_pivot_set_current_index(EGUI_VIEW_OF(&preview_pivot), 1);
    hcw_pivot_set_on_changed_listener(EGUI_VIEW_OF(&preview_pivot), on_pivot_changed);
    hcw_pivot_override_static_preview_api(EGUI_VIEW_OF(&preview_pivot), &preview_api);
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

static void seed_pressed_state(hcw_pivot_t *pivot, uint8_t index, uint8_t visual_pressed)
{
    egui_view_set_pressed(EGUI_VIEW_OF(pivot), 0);
    pivot->pressed_index = index;
    if (visual_pressed)
    {
        egui_view_set_pressed(EGUI_VIEW_OF(pivot), 1);
    }
}

static void assert_pressed_cleared(hcw_pivot_t *pivot)
{
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_PIVOT_INDEX_NONE, pivot->pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(pivot)->is_pressed);
}

static uint8_t get_header_center(hcw_pivot_t *pivot, uint8_t index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!hcw_pivot_get_header_region(EGUI_VIEW_OF(pivot), index, &region))
    {
        return 0;
    }
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void test_pivot_style_helpers_and_setters_clear_pressed_state(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_EQUAL_INT(3, test_pivot.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_pivot.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_pivot.read_only_mode);

    seed_pressed_state(&test_pivot, 1, 1);
    hcw_pivot_apply_compact_style(EGUI_VIEW_OF(&test_pivot));
    assert_pressed_cleared(&test_pivot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_pivot.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_pivot.read_only_mode);

    seed_pressed_state(&test_pivot, 1, 0);
    hcw_pivot_apply_read_only_style(EGUI_VIEW_OF(&test_pivot));
    assert_pressed_cleared(&test_pivot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_pivot.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_pivot.read_only_mode);

    seed_pressed_state(&test_pivot, 1, 1);
    hcw_pivot_set_font(EGUI_VIEW_OF(&test_pivot), NULL);
    assert_pressed_cleared(&test_pivot);
    EGUI_TEST_ASSERT_TRUE(test_pivot.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_pivot, 1, 0);
    hcw_pivot_set_meta_font(EGUI_VIEW_OF(&test_pivot), NULL);
    assert_pressed_cleared(&test_pivot);
    EGUI_TEST_ASSERT_TRUE(test_pivot.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_pivot, 2, 1);
    hcw_pivot_set_palette(EGUI_VIEW_OF(&test_pivot), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132), EGUI_COLOR_HEX(0x404142),
                          EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162));
    assert_pressed_cleared(&test_pivot);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_pivot.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x606162).full, test_pivot.card_surface_color.full);

    hcw_pivot_set_current_index(EGUI_VIEW_OF(&test_pivot), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_listener_index);

    seed_pressed_state(&test_pivot, 2, 1);
    hcw_pivot_set_current_index(EGUI_VIEW_OF(&test_pivot), 2);
    assert_pressed_cleared(&test_pivot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);

    test_pivot.current_index = 8;
    seed_pressed_state(&test_pivot, 4, 1);
    hcw_pivot_set_items(EGUI_VIEW_OF(&test_pivot), g_overflow_items, 7);
    assert_pressed_cleared(&test_pivot);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_PIVOT_MAX_ITEMS, test_pivot.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
}

static void test_pivot_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t x1;
    egui_dim_t y1;
    egui_dim_t x2;
    egui_dim_t y2;

    setup_widget();
    layout_view(EGUI_VIEW_OF(&test_pivot), 10, 18, 196, 96);

    EGUI_TEST_ASSERT_TRUE(get_header_center(&test_pivot, 1, &x1, &y1));
    EGUI_TEST_ASSERT_TRUE(get_header_center(&test_pivot, 2, &x2, &y2));
    EGUI_TEST_ASSERT_FALSE(hcw_pivot_get_header_region(EGUI_VIEW_OF(&test_pivot), 4, NULL));

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_pivot.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pivot)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_pivot)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
    assert_pressed_cleared(&test_pivot);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_MOVE, x1, y1));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pivot)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_UP, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_index);
    assert_pressed_cleared(&test_pivot);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_DOWN, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_pivot.pressed_index);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_CANCEL, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    assert_pressed_cleared(&test_pivot);
}

static void test_pivot_keyboard_navigation_and_guards(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget();
    layout_view(EGUI_VIEW_OF(&test_pivot), 10, 18, 196, 96);
    EGUI_TEST_ASSERT_TRUE(get_header_center(&test_pivot, 2, &x, &y));

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_KEY_CODE_SPACE));

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pivot)->is_pressed);
    hcw_pivot_set_read_only_mode(EGUI_VIEW_OF(&test_pivot), 1);
    assert_pressed_cleared(&test_pivot);

    seed_pressed_state(&test_pivot, 1, 1);
    EGUI_TEST_ASSERT_FALSE(send_key_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_KEY_CODE_END));
    assert_pressed_cleared(&test_pivot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));

    hcw_pivot_set_read_only_mode(EGUI_VIEW_OF(&test_pivot), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_pivot), 0);
    seed_pressed_state(&test_pivot, 1, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&test_pivot);

    egui_view_set_enable(EGUI_VIEW_OF(&test_pivot), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_pivot), EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, hcw_pivot_get_current_index(EGUI_VIEW_OF(&test_pivot)));
}

static void test_pivot_static_preview_consumes_input(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_pivot), 12, 20, 104, 56);
    EGUI_TEST_ASSERT_TRUE(get_header_center(&preview_pivot, 1, &x, &y));

    seed_pressed_state(&preview_pivot, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_pivot), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&preview_pivot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_pivot_get_current_index(EGUI_VIEW_OF(&preview_pivot)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);

    seed_pressed_state(&preview_pivot, 1, 0);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_pivot), EGUI_KEY_CODE_END));
    assert_pressed_cleared(&preview_pivot);
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_pivot_get_current_index(EGUI_VIEW_OF(&preview_pivot)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
}

void test_pivot_run(void)
{
    EGUI_TEST_SUITE_BEGIN(pivot);
    EGUI_TEST_RUN(test_pivot_style_helpers_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_pivot_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_pivot_keyboard_navigation_and_guards);
    EGUI_TEST_RUN(test_pivot_static_preview_consumes_input);
    EGUI_TEST_SUITE_END();
}
