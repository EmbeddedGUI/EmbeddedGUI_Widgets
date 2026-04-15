#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_radio_buttons.h"

#include "../../HelloCustomWidgets/input/radio_buttons/egui_view_radio_buttons.h"
#include "../../HelloCustomWidgets/input/radio_buttons/egui_view_radio_buttons.c"

typedef struct radio_buttons_preview_snapshot radio_buttons_preview_snapshot_t;
struct radio_buttons_preview_snapshot
{
    egui_region_t region_screen;
    egui_view_on_radio_buttons_changed_listener_t on_selection_changed;
    const char *const *items;
    const egui_font_t *font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    uint8_t item_count;
    uint8_t current_index;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_index;
    egui_alpha_t alpha;
};

static egui_view_radio_buttons_t test_widget;
static egui_view_radio_buttons_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_listener_index;
static uint8_t g_listener_count;

static const char *g_items_primary[] = {"Email", "Push", "SMS"};
static const char *g_items_preview[] = {"Auto", "Manual"};
static const char *g_items_overflow[] = {"One", "Two", "Three", "Four", "Five", "Six", "Seven"};

static void on_radio_buttons_changed(egui_view_t *self, uint8_t index)
{
    EGUI_UNUSED(self);
    g_listener_index = index;
    g_listener_count++;
}

static void reset_listener_state(void)
{
    g_listener_index = EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;
    g_listener_count = 0;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void setup_widget(void)
{
    egui_view_radio_buttons_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 196, 90);
    egui_view_radio_buttons_set_items(EGUI_VIEW_OF(&test_widget), g_items_primary, 3);
    egui_view_radio_buttons_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_radio_buttons_set_on_selection_changed_listener(EGUI_VIEW_OF(&test_widget), on_radio_buttons_changed);
    egui_view_radio_buttons_set_current_index(EGUI_VIEW_OF(&test_widget), 0);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    egui_view_radio_buttons_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 60);
    egui_view_radio_buttons_set_items(EGUI_VIEW_OF(&preview_widget), g_items_preview, 2);
    egui_view_radio_buttons_set_current_index(EGUI_VIEW_OF(&preview_widget), 0);
    egui_view_radio_buttons_set_compact_mode(EGUI_VIEW_OF(&preview_widget), 1);
    egui_view_radio_buttons_set_on_selection_changed_listener(EGUI_VIEW_OF(&preview_widget), on_radio_buttons_changed);
    egui_view_radio_buttons_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
    reset_listener_state();
}

static void capture_preview_snapshot(radio_buttons_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_widget)->region_screen;
    snapshot->on_selection_changed = preview_widget.on_selection_changed;
    snapshot->items = preview_widget.items;
    snapshot->font = preview_widget.font;
    snapshot->surface_color = preview_widget.surface_color;
    snapshot->border_color = preview_widget.border_color;
    snapshot->text_color = preview_widget.text_color;
    snapshot->muted_text_color = preview_widget.muted_text_color;
    snapshot->accent_color = preview_widget.accent_color;
    snapshot->item_count = preview_widget.item_count;
    snapshot->current_index = preview_widget.current_index;
    snapshot->compact_mode = preview_widget.compact_mode;
    snapshot->read_only_mode = preview_widget.read_only_mode;
    snapshot->pressed_index = preview_widget.pressed_index;
    snapshot->alpha = EGUI_VIEW_OF(&preview_widget)->alpha;
}

static void assert_preview_state_unchanged(const radio_buttons_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_widget.on_selection_changed == snapshot->on_selection_changed);
    EGUI_TEST_ASSERT_TRUE(preview_widget.items == snapshot->items);
    EGUI_TEST_ASSERT_TRUE(preview_widget.font == snapshot->font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_widget.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_count, preview_widget.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_index, preview_widget.current_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_index, preview_widget.pressed_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE, g_listener_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_widget)->is_pressed);
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
    layout_view(EGUI_VIEW_OF(&test_widget), 10, 18, 196, 90);
}

static void layout_preview_widget(void)
{
    layout_view(EGUI_VIEW_OF(&preview_widget), 14, 22, 104, 60);
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

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_widget), type, x, y);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_widget), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_widget), key_code);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_widget), key_code);
}

static uint8_t get_item_center_for_group(egui_view_radio_buttons_t *widget, uint8_t item_index, egui_dim_t *x, egui_dim_t *y)
{
    egui_view_radio_buttons_layout_item_t items[EGUI_VIEW_RADIO_BUTTONS_MAX_ITEMS];
    uint8_t count;

    count = egui_view_radio_buttons_prepare_layout(widget, EGUI_VIEW_OF(widget), items);
    if (item_index >= count)
    {
        return 0;
    }

    *x = items[item_index].region.location.x + items[item_index].region.size.width / 2;
    *y = items[item_index].region.location.y + items[item_index].region.size.height / 2;
    return 1;
}

static void seed_pressed_state(egui_view_radio_buttons_t *widget, uint8_t index, uint8_t visual_pressed)
{
    egui_view_set_pressed(EGUI_VIEW_OF(widget), 0);
    widget->pressed_index = index;
    if (visual_pressed)
    {
        egui_view_set_pressed(EGUI_VIEW_OF(widget), 1);
    }
}

static void assert_pressed_cleared(egui_view_radio_buttons_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE, widget->pressed_index);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void test_radio_buttons_setters_clear_pressed_state_and_clamp_items(void)
{
    setup_widget();

    test_widget.current_index = EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE;
    seed_pressed_state(&test_widget, 4, 1);
    egui_view_radio_buttons_set_items(EGUI_VIEW_OF(&test_widget), g_items_overflow, 7);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RADIO_BUTTONS_MAX_ITEMS, test_widget.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    assert_pressed_cleared(&test_widget);

    egui_view_radio_buttons_set_current_index(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_listener_index);

    seed_pressed_state(&test_widget, 2, 1);
    egui_view_radio_buttons_set_current_index(EGUI_VIEW_OF(&test_widget), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    assert_pressed_cleared(&test_widget);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_radio_buttons_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_widget, 1, 0);
    egui_view_radio_buttons_set_compact_mode(EGUI_VIEW_OF(&test_widget), 2);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.compact_mode);

    seed_pressed_state(&test_widget, 1, 1);
    egui_view_radio_buttons_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 3);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.read_only_mode);

    egui_view_radio_buttons_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, 1, 0);
    egui_view_radio_buttons_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                        EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_widget.accent_color.full);

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RADIO_BUTTONS_MAX_ITEMS, egui_view_radio_buttons_clamp_count(7));
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(EGUI_COLOR_HEX(0x123456), EGUI_COLOR_DARK_GREY, 64).full,
                               egui_view_radio_buttons_mix_disabled(EGUI_COLOR_HEX(0x123456)).full);
}

static void test_radio_buttons_focus_keyboard_and_read_only_guards(void)
{
    setup_widget();

    egui_view_radio_buttons_set_current_index(EGUI_VIEW_OF(&test_widget), EGUI_VIEW_RADIO_BUTTONS_INDEX_NONE);
    reset_listener_state();
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    EGUI_VIEW_OF(&test_widget)->api->on_focus_changed(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_index);
    reset_listener_state();
#endif

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_index);

    reset_listener_state();
    egui_view_radio_buttons_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
}

static void test_radio_buttons_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t x1;
    egui_dim_t y1;
    egui_dim_t x2;
    egui_dim_t y2;

    setup_widget();
    layout_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_group(&test_widget, 1, &x1, &y1));
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_group(&test_widget, 2, &x2, &y2));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_widget.pressed_index);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_listener_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x2, y2));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, x1, y1));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_index);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_widget.pressed_index);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, x2, y2));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_listener_count);
    assert_pressed_cleared(&test_widget);

    egui_view_radio_buttons_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x1, y1));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_radio_buttons_get_current_index(EGUI_VIEW_OF(&test_widget)));
}

static void test_radio_buttons_static_preview_consumes_input_and_keeps_state(void)
{
    radio_buttons_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_widget();
    layout_preview_widget();
    EGUI_TEST_ASSERT_TRUE(get_item_center_for_group(&preview_widget, 1, &x, &y));
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(&preview_widget, 1, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    seed_pressed_state(&preview_widget, 1, 0);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_END));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_radio_buttons_run(void)
{
    EGUI_TEST_SUITE_BEGIN(radio_buttons);
    EGUI_TEST_RUN(test_radio_buttons_setters_clear_pressed_state_and_clamp_items);
    EGUI_TEST_RUN(test_radio_buttons_focus_keyboard_and_read_only_guards);
    EGUI_TEST_RUN(test_radio_buttons_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_radio_buttons_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
