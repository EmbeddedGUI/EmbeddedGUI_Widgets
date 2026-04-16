#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_pivot.h"

#include "../../HelloCustomWidgets/navigation/pivot/egui_view_pivot.h"
#include "../../HelloCustomWidgets/navigation/pivot/egui_view_pivot.c"

typedef struct pivot_preview_snapshot pivot_preview_snapshot_t;
struct pivot_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const hcw_pivot_item_t *items;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    hcw_pivot_on_changed_listener_t on_changed;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t card_surface_color;
    uint8_t item_count;
    uint8_t current_index;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_index;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

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
        {"Home", "Quick", "Home", "Pinned work", "2 tabs", HCW_PIVOT_TONE_ACCENT},
        {"Queue", "Next", "Queue", "Ready items", "2 tabs", HCW_PIVOT_TONE_NEUTRAL},
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

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

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
    egui_view_set_size(EGUI_VIEW_OF(&preview_pivot), 104, 72);
    hcw_pivot_set_items(EGUI_VIEW_OF(&preview_pivot), g_preview_items, EGUI_ARRAY_SIZE(g_preview_items));
    hcw_pivot_set_font(EGUI_VIEW_OF(&preview_pivot), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_pivot_set_meta_font(EGUI_VIEW_OF(&preview_pivot), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_pivot_apply_compact_style(EGUI_VIEW_OF(&preview_pivot));
    hcw_pivot_set_palette(EGUI_VIEW_OF(&preview_pivot), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD6DEE6), EGUI_COLOR_HEX(0x22303C),
                          EGUI_COLOR_HEX(0x73808C), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0xF7F9FB));
    hcw_pivot_set_current_index(EGUI_VIEW_OF(&preview_pivot), 0);
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
    return view->api->dispatch_touch_event(view, &event);
}

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
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

static void capture_preview_snapshot(pivot_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_pivot)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_pivot)->background;
    snapshot->items = preview_pivot.items;
    snapshot->font = preview_pivot.font;
    snapshot->meta_font = preview_pivot.meta_font;
    snapshot->on_changed = preview_pivot.on_changed;
    snapshot->api = EGUI_VIEW_OF(&preview_pivot)->api;
    snapshot->surface_color = preview_pivot.surface_color;
    snapshot->border_color = preview_pivot.border_color;
    snapshot->text_color = preview_pivot.text_color;
    snapshot->muted_text_color = preview_pivot.muted_text_color;
    snapshot->accent_color = preview_pivot.accent_color;
    snapshot->card_surface_color = preview_pivot.card_surface_color;
    snapshot->item_count = preview_pivot.item_count;
    snapshot->current_index = preview_pivot.current_index;
    snapshot->compact_mode = preview_pivot.compact_mode;
    snapshot->read_only_mode = preview_pivot.read_only_mode;
    snapshot->pressed_index = preview_pivot.pressed_index;
    snapshot->alpha = EGUI_VIEW_OF(&preview_pivot)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_pivot));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_pivot)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_pivot)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_pivot)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_pivot)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_pivot)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_pivot)->padding.bottom;
}

static void assert_preview_state_unchanged(const pivot_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_pivot)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_pivot)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_pivot.items == snapshot->items);
    EGUI_TEST_ASSERT_TRUE(preview_pivot.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_pivot.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_pivot.on_changed == snapshot->on_changed);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_pivot)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_pivot.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_pivot.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_pivot.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_pivot.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_pivot.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->card_surface_color.full, preview_pivot.card_surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_count, preview_pivot.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_index, preview_pivot.current_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_pivot.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_pivot.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_index, preview_pivot.pressed_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_pivot)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_pivot)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_pivot)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_pivot)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_pivot)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_pivot)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_pivot)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_pivot)->padding.bottom);
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

static void test_pivot_static_preview_consumes_input_and_keeps_state(void)
{
    pivot_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_pivot), 12, 20, 104, 72);
    EGUI_TEST_ASSERT_TRUE(get_header_center(&preview_pivot, 1, &x, &y));
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(&preview_pivot, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_pivot), EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);

    seed_pressed_state(&preview_pivot, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_pivot), EGUI_KEY_CODE_END));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
}

void test_pivot_run(void)
{
    EGUI_TEST_SUITE_BEGIN(pivot);
    EGUI_TEST_RUN(test_pivot_style_helpers_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_pivot_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_pivot_keyboard_navigation_and_guards);
    EGUI_TEST_RUN(test_pivot_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
