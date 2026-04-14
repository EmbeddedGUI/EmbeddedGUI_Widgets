#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_pips_pager.h"

#include "../../HelloCustomWidgets/navigation/pips_pager/egui_view_pips_pager.h"
#include "../../HelloCustomWidgets/navigation/pips_pager/egui_view_pips_pager.c"

static egui_view_pips_pager_t test_pager;
static egui_view_pips_pager_t preview_pager;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static uint8_t last_index;
static uint8_t last_total;
static uint8_t last_part;

static void on_changed(egui_view_t *self, uint8_t current_index, uint8_t total_count, uint8_t part)
{
    EGUI_UNUSED(self);
    changed_count++;
    last_index = current_index;
    last_total = total_count;
    last_part = part;
}

static void reset_listener_state(void)
{
    changed_count = 0;
    last_index = 0xFF;
    last_total = 0xFF;
    last_part = EGUI_VIEW_PIPS_PAGER_PART_NONE;
}

static void setup_widget(uint8_t total_count, uint8_t current_index, uint8_t visible_count)
{
    egui_view_pips_pager_init(EGUI_VIEW_OF(&test_pager));
    egui_view_set_size(EGUI_VIEW_OF(&test_pager), 136, 82);
    egui_view_pips_pager_set_page_metrics(EGUI_VIEW_OF(&test_pager), total_count, current_index, visible_count);
    egui_view_pips_pager_set_current_part(EGUI_VIEW_OF(&test_pager), EGUI_VIEW_PIPS_PAGER_PART_PIP);
    egui_view_pips_pager_set_on_changed_listener(EGUI_VIEW_OF(&test_pager), on_changed);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    egui_view_pips_pager_init(EGUI_VIEW_OF(&preview_pager));
    egui_view_set_size(EGUI_VIEW_OF(&preview_pager), 104, 58);
    egui_view_pips_pager_set_page_metrics(EGUI_VIEW_OF(&preview_pager), 8, 4, 4);
    egui_view_pips_pager_set_current_part(EGUI_VIEW_OF(&preview_pager), EGUI_VIEW_PIPS_PAGER_PART_NEXT);
    egui_view_pips_pager_set_compact_mode(EGUI_VIEW_OF(&preview_pager), 1);
    egui_view_pips_pager_set_on_changed_listener(EGUI_VIEW_OF(&preview_pager), on_changed);
    egui_view_pips_pager_override_static_preview_api(EGUI_VIEW_OF(&preview_pager), &preview_api);
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
    layout_view(EGUI_VIEW_OF(&test_pager), 10, 20, 136, 82);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_pager), 12, 18, 104, 58);
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
    return send_touch_to_view(EGUI_VIEW_OF(&test_pager), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_pager), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_pager), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_pager), key_code);
}

static uint8_t get_part_center_for_pager(egui_view_pips_pager_t *pager, uint8_t part, uint8_t index, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_pips_pager_get_part_region(EGUI_VIEW_OF(pager), part, index, &region))
    {
        return 0;
    }

    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void seed_pressed_state(egui_view_pips_pager_t *pager, uint8_t part, uint8_t index, uint8_t visual_pressed)
{
    egui_view_set_pressed(EGUI_VIEW_OF(pager), 0);
    pager->pressed_part = part;
    pager->pressed_index = index;
    if (visual_pressed)
    {
        egui_view_set_pressed(EGUI_VIEW_OF(pager), 1);
    }
}

static void assert_pressed_cleared(egui_view_pips_pager_t *pager)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NONE, pager->pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, pager->pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(pager)->is_pressed);
}

static void test_pips_pager_clamps_metrics_and_regions(void)
{
    egui_region_t region;

    setup_widget(3, 9, 0);
    layout_widget();
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_pips_pager_get_total_count(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_pips_pager_get_visible_count(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_TRUE(egui_view_pips_pager_get_part_region(EGUI_VIEW_OF(&test_pager), EGUI_VIEW_PIPS_PAGER_PART_PIP, 2, &region));
    EGUI_TEST_ASSERT_FALSE(egui_view_pips_pager_get_part_region(EGUI_VIEW_OF(&test_pager), EGUI_VIEW_PIPS_PAGER_PART_PIP, 3, &region));
}

static void test_pips_pager_setters_clear_pressed_state(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t border = EGUI_COLOR_HEX(0x202122);
    egui_color_t text = EGUI_COLOR_HEX(0x303132);
    egui_color_t muted = EGUI_COLOR_HEX(0x404142);
    egui_color_t accent = EGUI_COLOR_HEX(0x505152);
    egui_color_t inactive = EGUI_COLOR_HEX(0x606162);
    egui_color_t preview = EGUI_COLOR_HEX(0x707172);

    setup_widget(8, 3, 5);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, 1);
    egui_view_pips_pager_set_title(EGUI_VIEW_OF(&test_pager), "Gallery");
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_TRUE(strcmp("Gallery", test_pager.title) == 0);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, 0);
    egui_view_pips_pager_set_helper(EGUI_VIEW_OF(&test_pager), "Discrete preview");
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_TRUE(strcmp("Discrete preview", test_pager.helper) == 0);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PREVIOUS, 0, 1);
    egui_view_pips_pager_set_font(EGUI_VIEW_OF(&test_pager), NULL);
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_TRUE(test_pager.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PREVIOUS, 0, 0);
    egui_view_pips_pager_set_meta_font(EGUI_VIEW_OF(&test_pager), NULL);
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_TRUE(test_pager.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PIP, 3, 1);
    egui_view_pips_pager_set_palette(EGUI_VIEW_OF(&test_pager), surface, border, text, muted, accent, inactive, preview);
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_pager.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_pager.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_pager.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_pager.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_pager.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(inactive.full, test_pager.inactive_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(preview.full, test_pager.preview_color.full);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PIP, 3, 1);
    egui_view_pips_pager_set_current_index(EGUI_VIEW_OF(&test_pager), 9);
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_EQUAL_INT(7, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));

    setup_widget(8, 3, 5);
    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PREVIOUS, 0, 1);
    egui_view_pips_pager_set_current_part(EGUI_VIEW_OF(&test_pager), EGUI_VIEW_PIPS_PAGER_PART_NEXT);
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NEXT, egui_view_pips_pager_get_current_part(EGUI_VIEW_OF(&test_pager)));

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, 1);
    egui_view_pips_pager_set_page_metrics(EGUI_VIEW_OF(&test_pager), 0, 9, 0);
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_pips_pager_get_total_count(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_pips_pager_get_visible_count(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NONE, egui_view_pips_pager_get_current_part(EGUI_VIEW_OF(&test_pager)));

    setup_widget(8, 3, 5);
    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, 1);
    egui_view_pips_pager_set_compact_mode(EGUI_VIEW_OF(&test_pager), 2);
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_pager.compact_mode);

    setup_widget(8, 3, 5);
    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PIP, 3, 0);
    egui_view_pips_pager_set_read_only_mode(EGUI_VIEW_OF(&test_pager), 3);
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_pager.read_only_mode);
}

static void test_pips_pager_tab_and_keyboard_navigation(void)
{
    setup_widget(8, 3, 5);

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PIP, egui_view_pips_pager_get_current_part(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NEXT, egui_view_pips_pager_get_current_part(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PREVIOUS, egui_view_pips_pager_get_current_part(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PIP, egui_view_pips_pager_get_current_part(EGUI_VIEW_OF(&test_pager)));

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, last_index);
    EGUI_TEST_ASSERT_EQUAL_INT(8, last_total);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PIP, last_part);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(7, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_PLUS));
    EGUI_TEST_ASSERT_EQUAL_INT(7, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_MINUS));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
}

static void test_pips_pager_touch_previous_next_obeys_same_target_release(void)
{
    egui_dim_t prev_x;
    egui_dim_t prev_y;
    egui_dim_t next_x;
    egui_dim_t next_y;

    setup_widget(8, 3, 5);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PREVIOUS, 0, &prev_x, &prev_y));
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, &next_x, &next_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, prev_x, prev_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PREVIOUS, test_pager.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PREVIOUS, test_pager.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared(&test_pager);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, prev_x, prev_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, next_x, next_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, prev_x, prev_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, prev_x, prev_y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, last_index);
    EGUI_TEST_ASSERT_EQUAL_INT(8, last_total);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PREVIOUS, last_part);
    assert_pressed_cleared(&test_pager);
}

static void test_pips_pager_touch_pip_requires_same_index_release(void)
{
    egui_dim_t pip5_x;
    egui_dim_t pip5_y;
    egui_dim_t pip6_x;
    egui_dim_t pip6_y;

    setup_widget(8, 4, 5);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PIP, 5, &pip5_x, &pip5_y));
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PIP, 6, &pip6_x, &pip6_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, pip5_x, pip5_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PIP, test_pager.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(5, test_pager.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, pip6_x, pip6_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, pip6_x, pip6_y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared(&test_pager);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, pip5_x, pip5_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, pip6_x, pip6_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, pip5_x, pip5_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, pip5_x, pip5_y));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(5, last_index);
    EGUI_TEST_ASSERT_EQUAL_INT(8, last_total);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PIP, last_part);
    assert_pressed_cleared(&test_pager);
}

static void test_pips_pager_touch_cancel_clears_pressed_state_without_notify(void)
{
    egui_dim_t next_x;
    egui_dim_t next_y;

    setup_widget(8, 3, 5);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, &next_x, &next_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NEXT, test_pager.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, next_x, next_y));
}

static void test_pips_pager_compact_mode_guard_clears_pressed_state(void)
{
    egui_dim_t next_x;
    egui_dim_t next_y;

    setup_widget(8, 3, 5);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, &next_x, &next_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    egui_view_pips_pager_set_compact_mode(EGUI_VIEW_OF(&test_pager), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_pager.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    assert_pressed_cleared(&test_pager);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&test_pager);

    layout_widget();
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_pips_pager_set_compact_mode(EGUI_VIEW_OF(&test_pager), 0);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, &next_x, &next_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(4, last_index);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NEXT, last_part);
}

static void test_pips_pager_read_only_and_view_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t next_x;
    egui_dim_t next_y;

    setup_widget(8, 3, 5);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, &next_x, &next_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    egui_view_pips_pager_set_read_only_mode(EGUI_VIEW_OF(&test_pager), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_pager.read_only_mode);
    assert_pressed_cleared(&test_pager);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PIP, 3, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    assert_pressed_cleared(&test_pager);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PIP, 3, 0);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_pips_pager_set_read_only_mode(EGUI_VIEW_OF(&test_pager), 0);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(4, last_index);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_PIP, last_part);

    setup_widget(8, 3, 5);
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, &next_x, &next_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_pager)->is_pressed);
    egui_view_set_enable(EGUI_VIEW_OF(&test_pager), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    assert_pressed_cleared(&test_pager);

    seed_pressed_state(&test_pager, EGUI_VIEW_PIPS_PAGER_PART_PIP, 3, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&test_pager);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_set_enable(EGUI_VIEW_OF(&test_pager), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, next_x, next_y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&test_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(4, last_index);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NEXT, last_part);
}

static void test_pips_pager_static_preview_consumes_input_and_keeps_state(void)
{
    egui_dim_t next_x;
    egui_dim_t next_y;

    setup_preview_widget();
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&preview_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NEXT, egui_view_pips_pager_get_current_part(EGUI_VIEW_OF(&preview_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, preview_pager.compact_mode);
    layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(get_part_center_for_pager(&preview_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, &next_x, &next_y));

    seed_pressed_state(&preview_pager, EGUI_VIEW_PIPS_PAGER_PART_NEXT, 0, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, next_x, next_y));
    assert_pressed_cleared(&preview_pager);
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&preview_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NEXT, egui_view_pips_pager_get_current_part(EGUI_VIEW_OF(&preview_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    seed_pressed_state(&preview_pager, EGUI_VIEW_PIPS_PAGER_PART_PIP, 5, 0);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_RIGHT));
    assert_pressed_cleared(&preview_pager);
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_pips_pager_get_current_index(EGUI_VIEW_OF(&preview_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PIPS_PAGER_PART_NEXT, egui_view_pips_pager_get_current_part(EGUI_VIEW_OF(&preview_pager)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

void test_pips_pager_run(void)
{
    EGUI_TEST_SUITE_BEGIN(pips_pager);
    EGUI_TEST_RUN(test_pips_pager_clamps_metrics_and_regions);
    EGUI_TEST_RUN(test_pips_pager_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_pips_pager_tab_and_keyboard_navigation);
    EGUI_TEST_RUN(test_pips_pager_touch_previous_next_obeys_same_target_release);
    EGUI_TEST_RUN(test_pips_pager_touch_pip_requires_same_index_release);
    EGUI_TEST_RUN(test_pips_pager_touch_cancel_clears_pressed_state_without_notify);
    EGUI_TEST_RUN(test_pips_pager_compact_mode_guard_clears_pressed_state);
    EGUI_TEST_RUN(test_pips_pager_read_only_and_view_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_pips_pager_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
