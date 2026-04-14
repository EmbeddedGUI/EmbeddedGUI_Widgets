#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_flip_view.h"

#include "../../HelloCustomWidgets/navigation/flip_view/egui_view_flip_view.h"
#include "../../HelloCustomWidgets/navigation/flip_view/egui_view_flip_view.c"

static egui_view_flip_view_t test_flip_view;
static egui_view_flip_view_t preview_flip_view;
static egui_view_api_t preview_api;
static int changed_count;
static uint8_t last_changed_index;
static uint8_t last_changed_item_count;
static uint8_t last_changed_part;

static const egui_view_flip_view_item_t unit_items[] = {
        {"One", "North deck", "Primary hero card", "Step 1", EGUI_COLOR_HEX(0xE4F0FF), EGUI_COLOR_HEX(0x2563EB)},
        {"Two", "South deck", "Secondary hero card", "Step 2", EGUI_COLOR_HEX(0xE8F5EE), EGUI_COLOR_HEX(0x0F766E)},
        {"Three", "Review deck", "Third hero card", "Step 3", EGUI_COLOR_HEX(0xF8ECDC), EGUI_COLOR_HEX(0xD97706)},
        {"Four", "Archive deck", "Fourth hero card", "Step 4", EGUI_COLOR_HEX(0xF3E8FF), EGUI_COLOR_HEX(0x8B5CF6)},
};

static void on_flip_view_changed(egui_view_t *self, uint8_t current_index, uint8_t item_count, uint8_t part)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_changed_index = current_index;
    last_changed_item_count = item_count;
    last_changed_part = part;
}

static void reset_listener_state(void)
{
    changed_count = 0;
    last_changed_index = 0xFF;
    last_changed_item_count = 0;
    last_changed_part = EGUI_VIEW_FLIP_VIEW_PART_NONE;
}

static void setup_widget(uint8_t item_count, uint8_t current_index)
{
    egui_view_flip_view_init(EGUI_VIEW_OF(&test_flip_view));
    egui_view_set_size(EGUI_VIEW_OF(&test_flip_view), 150, 88);
    egui_view_flip_view_set_items(EGUI_VIEW_OF(&test_flip_view), unit_items, item_count, current_index);
    egui_view_flip_view_set_current_part(EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE);
    egui_view_flip_view_set_on_changed_listener(EGUI_VIEW_OF(&test_flip_view), on_flip_view_changed);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    egui_view_flip_view_init(EGUI_VIEW_OF(&preview_flip_view));
    egui_view_set_size(EGUI_VIEW_OF(&preview_flip_view), 104, 64);
    egui_view_flip_view_set_items(EGUI_VIEW_OF(&preview_flip_view), unit_items, 4, 1);
    egui_view_flip_view_set_current_part(EGUI_VIEW_OF(&preview_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE);
    egui_view_flip_view_set_compact_mode(EGUI_VIEW_OF(&preview_flip_view), 1);
    egui_view_flip_view_set_on_changed_listener(EGUI_VIEW_OF(&preview_flip_view), on_flip_view_changed);
    egui_view_flip_view_override_static_preview_api(EGUI_VIEW_OF(&preview_flip_view), &preview_api);
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
    layout_view(EGUI_VIEW_OF(&test_flip_view), 10, 20, 150, 88);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_flip_view), 12, 18, 104, 64);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_flip_view), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_flip_view), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_flip_view), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_flip_view), key_code);
}

static void seed_pressed_state(egui_view_flip_view_t *widget, egui_view_t *view, uint8_t part, uint8_t visual_pressed)
{
    widget->pressed_part = part;
    egui_view_set_pressed(view, visual_pressed ? true : false);
}

static void assert_pressed_cleared(egui_view_flip_view_t *widget, egui_view_t *view)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_FALSE(view->is_pressed);
}

static uint8_t get_part_center(egui_view_flip_view_t *widget, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_flip_view_get_part_region(EGUI_VIEW_OF(widget), part, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void test_flip_view_clamps_items_and_current_index(void)
{
    setup_widget(3, 9);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_flip_view_get_item_count(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_flip_view_get_current_item(EGUI_VIEW_OF(&test_flip_view)) != NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&test_flip_view)));
}

static void test_flip_view_setters_clear_pressed_state(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t border = EGUI_COLOR_HEX(0x202122);
    egui_color_t text = EGUI_COLOR_HEX(0x303132);
    egui_color_t muted = EGUI_COLOR_HEX(0x404142);
    egui_color_t inactive = EGUI_COLOR_HEX(0x505152);

    setup_widget(3, 2);

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_NEXT, 1);
    egui_view_flip_view_set_font(EGUI_VIEW_OF(&test_flip_view), NULL);
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
    EGUI_TEST_ASSERT_TRUE(test_flip_view.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_PREVIOUS, 0);
    egui_view_flip_view_set_meta_font(EGUI_VIEW_OF(&test_flip_view), NULL);
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
    EGUI_TEST_ASSERT_TRUE(test_flip_view.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE, 1);
    egui_view_flip_view_set_title(EGUI_VIEW_OF(&test_flip_view), "Updated title");
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
    EGUI_TEST_ASSERT_TRUE(strcmp("Updated title", test_flip_view.title) == 0);

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE, 0);
    egui_view_flip_view_set_helper(EGUI_VIEW_OF(&test_flip_view), "Updated helper");
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
    EGUI_TEST_ASSERT_TRUE(strcmp("Updated helper", test_flip_view.helper) == 0);

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_NEXT, 1);
    egui_view_flip_view_set_palette(EGUI_VIEW_OF(&test_flip_view), surface, border, text, muted, inactive);
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_flip_view.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_flip_view.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_flip_view.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_flip_view.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(inactive.full, test_flip_view.inactive_color.full);

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_PREVIOUS, 1);
    egui_view_flip_view_set_items(EGUI_VIEW_OF(&test_flip_view), unit_items, 3, 9);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_flip_view_get_item_count(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_NEXT, 1);
    egui_view_flip_view_set_current_index(EGUI_VIEW_OF(&test_flip_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_PREVIOUS, 1);
    egui_view_flip_view_set_current_part(EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_NEXT);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_NEXT, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&test_flip_view)));
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    egui_view_flip_view_set_current_index(EGUI_VIEW_OF(&test_flip_view), 0);
    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_PREVIOUS, 1);
    egui_view_flip_view_set_current_part(EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_PREVIOUS);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&test_flip_view)));
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE, 1);
    egui_view_flip_view_set_compact_mode(EGUI_VIEW_OF(&test_flip_view), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_flip_view.compact_mode);
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    egui_view_flip_view_set_compact_mode(EGUI_VIEW_OF(&test_flip_view), 0);
    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE, 1);
    egui_view_flip_view_set_read_only_mode(EGUI_VIEW_OF(&test_flip_view), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_flip_view.read_only_mode);
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
}

static void test_flip_view_tab_cycles_parts(void)
{
    setup_widget(4, 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_flip_view_handle_navigation_key(EGUI_VIEW_OF(&test_flip_view), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_NEXT, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_flip_view_handle_navigation_key(EGUI_VIEW_OF(&test_flip_view), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_PREVIOUS, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_TRUE(egui_view_flip_view_handle_navigation_key(EGUI_VIEW_OF(&test_flip_view), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&test_flip_view)));
}

static void test_flip_view_keyboard_navigation(void)
{
    setup_widget(4, 1);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, last_changed_index);
    EGUI_TEST_ASSERT_EQUAL_INT(4, last_changed_item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, last_changed_part);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, last_changed_index);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, last_changed_index);
}

static void test_flip_view_plus_minus_steps_items_without_boundary_notify(void)
{
    setup_widget(4, 2);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_MINUS));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
}

static void test_flip_view_touch_previous_next_and_surface_notify(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget(4, 2);
    layout_widget();

    EGUI_TEST_ASSERT_TRUE(get_part_center(&test_flip_view, EGUI_VIEW_FLIP_VIEW_PART_PREVIOUS, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_PREVIOUS, last_changed_part);

    EGUI_TEST_ASSERT_TRUE(get_part_center(&test_flip_view, EGUI_VIEW_FLIP_VIEW_PART_NEXT, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_NEXT, last_changed_part);

    EGUI_TEST_ASSERT_TRUE(get_part_center(&test_flip_view, EGUI_VIEW_FLIP_VIEW_PART_SURFACE, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, last_changed_part);
}

static void test_flip_view_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t next_x;
    egui_dim_t next_y;
    egui_dim_t surface_x;
    egui_dim_t surface_y;

    setup_widget(4, 1);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center(&test_flip_view, EGUI_VIEW_FLIP_VIEW_PART_NEXT, &next_x, &next_y));
    EGUI_TEST_ASSERT_TRUE(get_part_center(&test_flip_view, EGUI_VIEW_FLIP_VIEW_PART_SURFACE, &surface_x, &surface_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_NEXT, test_flip_view.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flip_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, surface_x, surface_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_NEXT, test_flip_view.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_flip_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, surface_x, surface_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, surface_x, surface_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_flip_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, next_x, next_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flip_view)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_NEXT, last_changed_part);
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, surface_x, surface_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, test_flip_view.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, surface_x, surface_y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
}

static void test_flip_view_surface_region_exists(void)
{
    egui_region_t region;

    setup_widget(4, 1);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(egui_view_flip_view_get_part_region(EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE, &region));
    EGUI_TEST_ASSERT_TRUE(region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(region.size.height > 0);
}

static void test_flip_view_compact_mode_clears_pressed_and_ignores_input(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget(4, 1);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center(&test_flip_view, EGUI_VIEW_FLIP_VIEW_PART_NEXT, &x, &y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_NEXT, test_flip_view.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flip_view)->is_pressed);

    egui_view_flip_view_set_compact_mode(EGUI_VIEW_OF(&test_flip_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_flip_view.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_flip_view_set_compact_mode(EGUI_VIEW_OF(&test_flip_view), 0);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
}

static void test_flip_view_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_widget(4, 1);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center(&test_flip_view, EGUI_VIEW_FLIP_VIEW_PART_NEXT, &x, &y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_flip_view)->is_pressed);
    egui_view_flip_view_set_read_only_mode(EGUI_VIEW_OF(&test_flip_view), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_flip_view.read_only_mode);
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_flip_view_set_read_only_mode(EGUI_VIEW_OF(&test_flip_view), 0);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_NEXT, 1);
    egui_view_set_enable(EGUI_VIEW_OF(&test_flip_view), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));

    seed_pressed_state(&test_flip_view, EGUI_VIEW_OF(&test_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&test_flip_view, EGUI_VIEW_OF(&test_flip_view));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_flip_view), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&test_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
}

static void test_flip_view_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_widget();
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&preview_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&preview_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_flip_view.compact_mode);
    layout_preview_widget();
    get_view_center(EGUI_VIEW_OF(&preview_flip_view), &x, &y);

    seed_pressed_state(&preview_flip_view, EGUI_VIEW_OF(&preview_flip_view), EGUI_VIEW_FLIP_VIEW_PART_NEXT, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_pressed_cleared(&preview_flip_view, EGUI_VIEW_OF(&preview_flip_view));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&preview_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&preview_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_flip_view.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    seed_pressed_state(&preview_flip_view, EGUI_VIEW_OF(&preview_flip_view), EGUI_VIEW_FLIP_VIEW_PART_SURFACE, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&preview_flip_view, EGUI_VIEW_OF(&preview_flip_view));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_flip_view_get_current_index(EGUI_VIEW_OF(&preview_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_FLIP_VIEW_PART_SURFACE, egui_view_flip_view_get_current_part(EGUI_VIEW_OF(&preview_flip_view)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_flip_view.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

void test_flip_view_run(void)
{
    EGUI_TEST_SUITE_BEGIN(flip_view);
    EGUI_TEST_RUN(test_flip_view_clamps_items_and_current_index);
    EGUI_TEST_RUN(test_flip_view_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_flip_view_tab_cycles_parts);
    EGUI_TEST_RUN(test_flip_view_keyboard_navigation);
    EGUI_TEST_RUN(test_flip_view_plus_minus_steps_items_without_boundary_notify);
    EGUI_TEST_RUN(test_flip_view_touch_previous_next_and_surface_notify);
    EGUI_TEST_RUN(test_flip_view_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_flip_view_surface_region_exists);
    EGUI_TEST_RUN(test_flip_view_compact_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_flip_view_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_flip_view_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
