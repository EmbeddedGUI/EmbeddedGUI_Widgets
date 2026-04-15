#include "egui.h"
#include "test/egui_test.h"
#include "test_rating_control.h"

#include <string.h>

#include "../../HelloCustomWidgets/input/rating_control/egui_view_rating_control.h"
#include "../../HelloCustomWidgets/input/rating_control/egui_view_rating_control.c"

typedef struct rating_control_preview_snapshot rating_control_preview_snapshot_t;
struct rating_control_preview_snapshot
{
    egui_region_t region_screen;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_rating_control_changed_listener_t on_changed;
    const char *title;
    const char *low_label;
    const char *high_label;
    const char **value_labels;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t shadow_color;
    uint8_t label_count;
    uint8_t item_count;
    uint8_t current_value;
    uint8_t current_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t clear_enabled;
    uint8_t pressed_part;
    egui_alpha_t alpha;
};

static egui_view_rating_control_t test_rating_control;
static egui_view_rating_control_t preview_rating_control;
static egui_view_api_t preview_api;
static uint8_t g_changed_value = 0xFF;
static uint8_t g_changed_part = EGUI_VIEW_RATING_CONTROL_PART_NONE;
static uint8_t g_changed_count = 0;

static const char *preview_value_labels[] = {"No rating", "Poor", "Fair", "Good", "Great", "Excellent"};

static void reset_changed_state(void)
{
    g_changed_value = 0xFF;
    g_changed_part = EGUI_VIEW_RATING_CONTROL_PART_NONE;
    g_changed_count = 0;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void on_rating_changed(egui_view_t *self, uint8_t value, uint8_t part)
{
    EGUI_UNUSED(self);
    g_changed_value = value;
    g_changed_part = part;
    g_changed_count++;
}

static void setup_rating_control(uint8_t value)
{
    egui_view_rating_control_init(EGUI_VIEW_OF(&test_rating_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_rating_control), 196, 92);
    egui_view_rating_control_set_item_count(EGUI_VIEW_OF(&test_rating_control), 5);
    egui_view_rating_control_set_clear_enabled(EGUI_VIEW_OF(&test_rating_control), 1);
    egui_view_rating_control_set_on_changed_listener(EGUI_VIEW_OF(&test_rating_control), on_rating_changed);
    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), value);
    reset_changed_state();
}

static void setup_preview_rating_control(uint8_t value)
{
    egui_view_rating_control_init(EGUI_VIEW_OF(&preview_rating_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_rating_control), 104, 42);
    egui_view_rating_control_set_title(EGUI_VIEW_OF(&preview_rating_control), "Service quality");
    egui_view_rating_control_set_low_label(EGUI_VIEW_OF(&preview_rating_control), "Low");
    egui_view_rating_control_set_high_label(EGUI_VIEW_OF(&preview_rating_control), "High");
    egui_view_rating_control_set_value_labels(EGUI_VIEW_OF(&preview_rating_control), preview_value_labels, EGUI_ARRAY_SIZE(preview_value_labels));
    egui_view_rating_control_set_font(EGUI_VIEW_OF(&preview_rating_control), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_rating_control_set_meta_font(EGUI_VIEW_OF(&preview_rating_control), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_rating_control_set_palette(EGUI_VIEW_OF(&preview_rating_control), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xDADFE5), EGUI_COLOR_HEX(0x2B3138),
                                         EGUI_COLOR_HEX(0x74808C), EGUI_COLOR_HEX(0xC78C16), EGUI_COLOR_HEX(0xE0E6EC));
    egui_view_rating_control_set_item_count(EGUI_VIEW_OF(&preview_rating_control), 5);
    egui_view_rating_control_set_clear_enabled(EGUI_VIEW_OF(&preview_rating_control), 1);
    egui_view_rating_control_set_on_changed_listener(EGUI_VIEW_OF(&preview_rating_control), on_rating_changed);
    egui_view_rating_control_set_value(EGUI_VIEW_OF(&preview_rating_control), value);
    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&preview_rating_control), value > 0 ? value : 1);
    egui_view_rating_control_set_compact_mode(EGUI_VIEW_OF(&preview_rating_control), 1);
    egui_view_rating_control_override_static_preview_api(EGUI_VIEW_OF(&preview_rating_control), &preview_api);
    reset_changed_state();
}

static void layout_rating_control(egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(EGUI_VIEW_OF(&test_rating_control), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_rating_control)->region_screen, &region);
}

static void layout_preview_rating_control(egui_dim_t x, egui_dim_t y)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = 104;
    region.size.height = 42;
    egui_view_layout(EGUI_VIEW_OF(&preview_rating_control), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_rating_control)->region_screen, &region);
}

static void capture_preview_snapshot(rating_control_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_rating_control)->region_screen;
    snapshot->font = preview_rating_control.font;
    snapshot->meta_font = preview_rating_control.meta_font;
    snapshot->on_changed = preview_rating_control.on_changed;
    snapshot->title = preview_rating_control.title;
    snapshot->low_label = preview_rating_control.low_label;
    snapshot->high_label = preview_rating_control.high_label;
    snapshot->value_labels = preview_rating_control.value_labels;
    snapshot->surface_color = preview_rating_control.surface_color;
    snapshot->border_color = preview_rating_control.border_color;
    snapshot->text_color = preview_rating_control.text_color;
    snapshot->muted_text_color = preview_rating_control.muted_text_color;
    snapshot->accent_color = preview_rating_control.accent_color;
    snapshot->shadow_color = preview_rating_control.shadow_color;
    snapshot->label_count = preview_rating_control.label_count;
    snapshot->item_count = preview_rating_control.item_count;
    snapshot->current_value = preview_rating_control.current_value;
    snapshot->current_part = preview_rating_control.current_part;
    snapshot->compact_mode = preview_rating_control.compact_mode;
    snapshot->read_only_mode = preview_rating_control.read_only_mode;
    snapshot->clear_enabled = preview_rating_control.clear_enabled;
    snapshot->pressed_part = preview_rating_control.pressed_part;
    snapshot->alpha = EGUI_VIEW_OF(&preview_rating_control)->alpha;
}

static void assert_preview_state_unchanged(const rating_control_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_rating_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_rating_control.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_rating_control.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_rating_control.on_changed == snapshot->on_changed);
    EGUI_TEST_ASSERT_TRUE(preview_rating_control.title == snapshot->title);
    EGUI_TEST_ASSERT_TRUE(preview_rating_control.low_label == snapshot->low_label);
    EGUI_TEST_ASSERT_TRUE(preview_rating_control.high_label == snapshot->high_label);
    EGUI_TEST_ASSERT_TRUE(preview_rating_control.value_labels == snapshot->value_labels);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_rating_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_rating_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_rating_control.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_rating_control.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_rating_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->shadow_color.full, preview_rating_control.shadow_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->label_count, preview_rating_control.label_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_count, preview_rating_control.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_value, preview_rating_control.current_value);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->current_part, preview_rating_control.current_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_rating_control.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_rating_control.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->clear_enabled, preview_rating_control.clear_enabled);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_part, preview_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_rating_control)->alpha);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, g_changed_value);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, g_changed_part);
}

static void get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(view, part, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static int send_touch_event_to_view(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
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

static int send_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_event_to_view(EGUI_VIEW_OF(&test_rating_control), type, x, y);
}

static int send_key_event(uint8_t type, uint8_t key_code)
{
    return dispatch_key_event_to_view(EGUI_VIEW_OF(&test_rating_control), type, key_code);
}

static int send_preview_touch_event(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_event_to_view(EGUI_VIEW_OF(&preview_rating_control), type, x, y);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_rating_control), key_code);
}

static void assert_changed_state(uint8_t count, uint8_t value, uint8_t part)
{
    EGUI_TEST_ASSERT_EQUAL_INT(count, g_changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(value, g_changed_value);
    EGUI_TEST_ASSERT_EQUAL_INT(part, g_changed_part);
}

static void test_rating_control_setters_clear_pressed_state(void)
{
    static const char *labels[] = {"0", "1", "2", "3", "4", "5"};

    setup_rating_control(5);

    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 5;
    egui_view_rating_control_set_item_count(EGUI_VIEW_OF(&test_rating_control), 3);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_rating_control.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));

    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 3;
    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), 2);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));

    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 2;
    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));

    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 1;
    egui_view_rating_control_set_value_labels(EGUI_VIEW_OF(&test_rating_control), labels, EGUI_ARRAY_SIZE(labels));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ARRAY_SIZE(labels), test_rating_control.label_count);

    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 1;
    egui_view_rating_control_set_palette(EGUI_VIEW_OF(&test_rating_control), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                         EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);

    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), 2);
    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = EGUI_VIEW_RATING_CONTROL_PART_CLEAR;
    egui_view_rating_control_set_clear_enabled(EGUI_VIEW_OF(&test_rating_control), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_rating_control.clear_enabled);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));

    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 2;
    egui_view_rating_control_set_compact_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_rating_control.compact_mode);

    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 2;
    egui_view_rating_control_set_read_only_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_rating_control.read_only_mode);
}

static void test_rating_control_enter_clear_commits_zero(void)
{
    setup_rating_control(3);

    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_CLEAR, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 0, EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
}

static void test_rating_control_space_clear_commits_zero(void)
{
    setup_rating_control(4);

    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 0, EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
}

static void test_rating_control_escape_on_clear_commits_zero(void)
{
    setup_rating_control(2);

    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 0, EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
}

static void test_rating_control_left_from_clear_restores_committed_part(void)
{
    setup_rating_control(3);

    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_enter_star_commits_current_part(void)
{
    setup_rating_control(1);

    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 3);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 3, 3);
}

static void test_rating_control_home_end_commit_bounds(void)
{
    setup_rating_control(3);

    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 5, 5);

    reset_changed_state();
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 1, 1);
}

static void test_rating_control_right_increments_and_stops_at_max(void)
{
    setup_rating_control(3);

    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 4, 4);

    reset_changed_state();
    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), 5);
    reset_changed_state();
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_up_matches_left_navigation(void)
{
    setup_rating_control(3);

    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 2, 2);
}

static void test_rating_control_down_increments_and_ignores_clear_focus(void)
{
    setup_rating_control(3);

    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 4, 4);

    reset_changed_state();
    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_CLEAR, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_escape_on_star_resets_focus_without_clearing(void)
{
    setup_rating_control(4);

    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 2);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_tab_cycles_to_clear_and_wraps(void)
{
    setup_rating_control(3);

    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 5);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_CLEAR, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);

    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_tab_skips_clear_when_value_empty(void)
{
    setup_rating_control(0);

    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 5);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_tab_skips_clear_when_disabled_by_flag(void)
{
    setup_rating_control(4);

    egui_view_rating_control_set_clear_enabled(EGUI_VIEW_OF(&test_rating_control), 0);
    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 5);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_tab_skips_clear_in_compact_mode(void)
{
    setup_rating_control(4);

    egui_view_rating_control_set_compact_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 5);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_TAB));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_navigation_ignored_when_read_only(void)
{
    setup_rating_control(4);

    egui_view_rating_control_set_read_only_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 3);
    EGUI_TEST_ASSERT_FALSE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_navigation_ignored_when_disabled(void)
{
    setup_rating_control(4);

    egui_view_set_enable(EGUI_VIEW_OF(&test_rating_control), 0);
    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 3);
    EGUI_TEST_ASSERT_FALSE(egui_view_rating_control_handle_navigation_key(EGUI_VIEW_OF(&test_rating_control), EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_part_region_follows_visibility_rules(void)
{
    egui_region_t clear_region;
    egui_region_t star_region;

    setup_rating_control(4);
    layout_rating_control(12, 18, 196, 92);

    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR, &clear_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), 1, &star_region));
    EGUI_TEST_ASSERT_TRUE(clear_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(star_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(clear_region.location.x > star_region.location.x);
    EGUI_TEST_ASSERT_TRUE(clear_region.location.y < star_region.location.y);

    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR, &clear_region));

    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), 4);
    egui_view_rating_control_set_clear_enabled(EGUI_VIEW_OF(&test_rating_control), 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR, &clear_region));

    egui_view_rating_control_set_clear_enabled(EGUI_VIEW_OF(&test_rating_control), 1);
    egui_view_rating_control_set_compact_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR, &clear_region));

    egui_view_rating_control_set_compact_mode(EGUI_VIEW_OF(&test_rating_control), 0);
    egui_view_rating_control_set_read_only_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR, &clear_region));
}

static void test_rating_control_item_count_value_and_label_count_normalize(void)
{
    static const char *labels[16] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15"};

    setup_rating_control(4);

    egui_view_rating_control_set_item_count(EGUI_VIEW_OF(&test_rating_control), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_rating_control.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));

    egui_view_rating_control_set_item_count(EGUI_VIEW_OF(&test_rating_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_MAX_ITEMS, test_rating_control.item_count);

    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_MAX_ITEMS, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_MAX_ITEMS, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));

    egui_view_rating_control_set_value_labels(EGUI_VIEW_OF(&test_rating_control), labels, 16);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_MAX_ITEMS + 1, test_rating_control.label_count);
}

static void test_rating_control_set_value_clear_uses_clear_notification(void)
{
    setup_rating_control(4);

    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 0, EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
}

static void test_rating_control_set_value_same_value_is_quiet(void)
{
    setup_rating_control(4);

    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), 4);
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_set_current_part_ignores_invalid_target(void)
{
    setup_rating_control(3);

    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);

    egui_view_rating_control_set_value(EGUI_VIEW_OF(&test_rating_control), 0);
    reset_changed_state();
    egui_view_rating_control_set_current_part(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_set_item_count_shrinks_value_without_notifying(void)
{
    setup_rating_control(5);

    egui_view_rating_control_set_item_count(EGUI_VIEW_OF(&test_rating_control), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(3, test_rating_control.item_count);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_touch_star_commits_value(void)
{
    egui_region_t star_region;
    egui_dim_t x;
    egui_dim_t y;

    setup_rating_control(2);
    layout_rating_control(12, 18, 196, 92);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), 4, &star_region));
    x = star_region.location.x + star_region.size.width / 2;
    y = star_region.location.y + star_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_rating_control.pressed_part);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    assert_changed_state(1, 4, 4);
}

static void test_rating_control_touch_clear_commits_zero(void)
{
    egui_region_t clear_region;
    egui_dim_t x;
    egui_dim_t y;

    setup_rating_control(4);
    layout_rating_control(12, 18, 196, 92);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), EGUI_VIEW_RATING_CONTROL_PART_CLEAR, &clear_region));
    x = clear_region.location.x + clear_region.size.width / 2;
    y = clear_region.location.y + clear_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_CLEAR, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_CLEAR, test_rating_control.pressed_part);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    assert_changed_state(1, 0, EGUI_VIEW_RATING_CONTROL_PART_CLEAR);
}

static void test_rating_control_touch_move_outside_cancels_commit(void)
{
    egui_region_t star_region;
    egui_dim_t x;
    egui_dim_t y;

    setup_rating_control(2);
    layout_rating_control(12, 18, 196, 92);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), 3, &star_region));
    x = star_region.location.x + star_region.size.width / 2;
    y = star_region.location.y + star_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, 400, 400));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);

    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, 400, 400));
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_touch_gap_prefers_right_star(void)
{
    egui_region_t left_region;
    egui_region_t right_region;
    egui_dim_t gap_start;
    egui_dim_t gap_width;
    egui_dim_t x;
    egui_dim_t y;

    setup_rating_control(2);
    layout_rating_control(12, 18, 106, 42);
    egui_view_rating_control_set_compact_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    layout_rating_control(12, 18, 106, 42);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), 3, &left_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), 4, &right_region));
    gap_start = left_region.location.x + left_region.size.width;
    gap_width = right_region.location.x - gap_start;
    x = gap_start + gap_width - 1;
    y = left_region.location.y + left_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 4, 4);
}

static void test_rating_control_touch_drag_to_new_star_commits_latest_part(void)
{
    egui_region_t start_region;
    egui_region_t end_region;
    egui_dim_t start_x;
    egui_dim_t start_y;
    egui_dim_t end_x;
    egui_dim_t end_y;

    setup_rating_control(1);
    layout_rating_control(12, 18, 196, 92);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), 2, &start_region));
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), 5, &end_region));
    start_x = start_region.location.x + start_region.size.width / 2;
    start_y = start_region.location.y + start_region.size.height / 2;
    end_x = end_region.location.x + end_region.size.width / 2;
    end_y = end_region.location.y + end_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, start_x, start_y));
    EGUI_TEST_ASSERT_EQUAL_INT(2, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(2, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_MOVE, end_x, end_y));
    EGUI_TEST_ASSERT_EQUAL_INT(5, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, end_x, end_y));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    assert_changed_state(1, 5, 5);
}

static void test_rating_control_touch_cancel_clears_pressed_state_without_commit(void)
{
    egui_region_t star_region;
    egui_dim_t x;
    egui_dim_t y;

    setup_rating_control(3);
    layout_rating_control(12, 18, 196, 92);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), 5, &star_region));
    x = star_region.location.x + star_region.size.width / 2;
    y = star_region.location.y + star_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(5, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch_event(EGUI_MOTION_EVENT_ACTION_CANCEL, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_touch_ignored_when_read_only_or_disabled(void)
{
    egui_region_t star_region;
    egui_dim_t x;
    egui_dim_t y;

    setup_rating_control(4);
    layout_rating_control(12, 18, 196, 92);
    EGUI_TEST_ASSERT_TRUE(egui_view_rating_control_get_part_region(EGUI_VIEW_OF(&test_rating_control), 2, &star_region));
    x = star_region.location.x + star_region.size.width / 2;
    y = star_region.location.y + star_region.size.height / 2;

    egui_view_rating_control_set_read_only_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);

    egui_view_rating_control_set_read_only_mode(EGUI_VIEW_OF(&test_rating_control), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_rating_control), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_read_only_and_disabled_guards_clear_pressed_state(void)
{
    egui_dim_t x;
    egui_dim_t y;

    setup_rating_control(4);
    layout_rating_control(12, 18, 196, 92);
    get_part_center(EGUI_VIEW_OF(&test_rating_control), 2, &x, &y);

    egui_view_rating_control_set_read_only_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 2;
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 2;
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 2;
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);

    egui_view_rating_control_set_read_only_mode(EGUI_VIEW_OF(&test_rating_control), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_rating_control), 0);
    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 2;
    EGUI_TEST_ASSERT_FALSE(send_touch_event(EGUI_MOTION_EVENT_ACTION_UP, x, y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 2;
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
    EGUI_VIEW_OF(&test_rating_control)->is_pressed = 1;
    test_rating_control.pressed_part = 2;
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_rating_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_RATING_CONTROL_PART_NONE, test_rating_control.pressed_part);
}

static void test_rating_control_key_event_action_down_consumes_navigation_without_commit(void)
{
    setup_rating_control(3);

    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_key_event_action_up_commits_navigation(void)
{
    setup_rating_control(3);

    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    assert_changed_state(1, 4, 4);
}

static void test_rating_control_key_event_unknown_key_falls_back(void)
{
    setup_rating_control(3);

    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_A));
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_A));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_key_event_read_only_down_is_consumed_but_up_is_quiet(void)
{
    setup_rating_control(4);

    egui_view_rating_control_set_read_only_mode(EGUI_VIEW_OF(&test_rating_control), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_key_event_disabled_down_is_consumed_but_up_is_quiet(void)
{
    setup_rating_control(4);

    egui_view_set_enable(EGUI_VIEW_OF(&test_rating_control), 0);
    EGUI_TEST_ASSERT_TRUE(send_key_event(EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_FALSE(send_key_event(EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_value(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_rating_control_get_current_part(EGUI_VIEW_OF(&test_rating_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_changed_count);
}

static void test_rating_control_static_preview_consumes_input_and_keeps_state(void)
{
    rating_control_preview_snapshot_t initial_snapshot;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_rating_control(4);
    layout_preview_rating_control(12, 18);
    get_part_center(EGUI_VIEW_OF(&preview_rating_control), 4, &x, &y);
    capture_preview_snapshot(&initial_snapshot);

    EGUI_VIEW_OF(&preview_rating_control)->is_pressed = 1;
    preview_rating_control.pressed_part = 4;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch_event(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_rating_control)->is_pressed = 1;
    preview_rating_control.pressed_part = 2;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_RIGHT));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_rating_control_run(void)
{
    EGUI_TEST_SUITE_BEGIN(rating_control);

    EGUI_TEST_RUN(test_rating_control_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_rating_control_enter_clear_commits_zero);
    EGUI_TEST_RUN(test_rating_control_space_clear_commits_zero);
    EGUI_TEST_RUN(test_rating_control_escape_on_clear_commits_zero);
    EGUI_TEST_RUN(test_rating_control_left_from_clear_restores_committed_part);
    EGUI_TEST_RUN(test_rating_control_enter_star_commits_current_part);
    EGUI_TEST_RUN(test_rating_control_home_end_commit_bounds);
    EGUI_TEST_RUN(test_rating_control_right_increments_and_stops_at_max);
    EGUI_TEST_RUN(test_rating_control_up_matches_left_navigation);
    EGUI_TEST_RUN(test_rating_control_down_increments_and_ignores_clear_focus);
    EGUI_TEST_RUN(test_rating_control_escape_on_star_resets_focus_without_clearing);
    EGUI_TEST_RUN(test_rating_control_tab_cycles_to_clear_and_wraps);
    EGUI_TEST_RUN(test_rating_control_tab_skips_clear_when_value_empty);
    EGUI_TEST_RUN(test_rating_control_tab_skips_clear_when_disabled_by_flag);
    EGUI_TEST_RUN(test_rating_control_tab_skips_clear_in_compact_mode);
    EGUI_TEST_RUN(test_rating_control_navigation_ignored_when_read_only);
    EGUI_TEST_RUN(test_rating_control_navigation_ignored_when_disabled);
    EGUI_TEST_RUN(test_rating_control_part_region_follows_visibility_rules);
    EGUI_TEST_RUN(test_rating_control_item_count_value_and_label_count_normalize);
    EGUI_TEST_RUN(test_rating_control_set_value_clear_uses_clear_notification);
    EGUI_TEST_RUN(test_rating_control_set_value_same_value_is_quiet);
    EGUI_TEST_RUN(test_rating_control_set_current_part_ignores_invalid_target);
    EGUI_TEST_RUN(test_rating_control_set_item_count_shrinks_value_without_notifying);
    EGUI_TEST_RUN(test_rating_control_touch_star_commits_value);
    EGUI_TEST_RUN(test_rating_control_touch_clear_commits_zero);
    EGUI_TEST_RUN(test_rating_control_touch_move_outside_cancels_commit);
    EGUI_TEST_RUN(test_rating_control_touch_gap_prefers_right_star);
    EGUI_TEST_RUN(test_rating_control_touch_drag_to_new_star_commits_latest_part);
    EGUI_TEST_RUN(test_rating_control_touch_cancel_clears_pressed_state_without_commit);
    EGUI_TEST_RUN(test_rating_control_touch_ignored_when_read_only_or_disabled);
    EGUI_TEST_RUN(test_rating_control_read_only_and_disabled_guards_clear_pressed_state);
    EGUI_TEST_RUN(test_rating_control_key_event_action_down_consumes_navigation_without_commit);
    EGUI_TEST_RUN(test_rating_control_key_event_action_up_commits_navigation);
    EGUI_TEST_RUN(test_rating_control_key_event_unknown_key_falls_back);
    EGUI_TEST_RUN(test_rating_control_key_event_read_only_down_is_consumed_but_up_is_quiet);
    EGUI_TEST_RUN(test_rating_control_key_event_disabled_down_is_consumed_but_up_is_quiet);
    EGUI_TEST_RUN(test_rating_control_static_preview_consumes_input_and_keeps_state);

    EGUI_TEST_SUITE_END();
}
