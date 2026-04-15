#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_number_box.h"

#include "../../HelloCustomWidgets/input/number_box/egui_view_number_box.h"
#include "../../HelloCustomWidgets/input/number_box/egui_view_number_box.c"

typedef struct number_box_preview_snapshot number_box_preview_snapshot_t;
struct number_box_preview_snapshot
{
    egui_region_t region_screen;
    egui_view_on_number_box_changed_listener_t on_value_changed;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    const char *label;
    const char *suffix;
    const char *helper;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    int16_t value;
    int16_t min_value;
    int16_t max_value;
    int16_t step;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_part;
    egui_alpha_t alpha;
    uint8_t enable;
};

static egui_view_number_box_t test_box;
static egui_view_number_box_t preview_box;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static int16_t changed_value;

static void on_value_changed(egui_view_t *self, int16_t value)
{
    EGUI_UNUSED(self);
    changed_count++;
    changed_value = value;
}

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void assert_optional_string_equal(const char *expected, const char *actual)
{
    if (expected == NULL || actual == NULL)
    {
        EGUI_TEST_ASSERT_TRUE(expected == actual);
        return;
    }
    EGUI_TEST_ASSERT_TRUE(strcmp(expected, actual) == 0);
}

static void setup_number_box(void)
{
    egui_view_number_box_init(EGUI_VIEW_OF(&test_box));
    egui_view_set_size(EGUI_VIEW_OF(&test_box), 196, 70);
    egui_view_number_box_set_label(EGUI_VIEW_OF(&test_box), "Spacing");
    egui_view_number_box_set_suffix(EGUI_VIEW_OF(&test_box), "px");
    egui_view_number_box_set_helper(EGUI_VIEW_OF(&test_box), "0 to 64, step 4");
    egui_view_number_box_set_range(EGUI_VIEW_OF(&test_box), 0, 64);
    egui_view_number_box_set_step(EGUI_VIEW_OF(&test_box), 4);
    egui_view_number_box_set_value(EGUI_VIEW_OF(&test_box), 24);
    egui_view_number_box_set_on_value_changed_listener(EGUI_VIEW_OF(&test_box), on_value_changed);
    changed_count = 0;
    changed_value = -1;
}

static void setup_preview_number_box(void)
{
    egui_view_number_box_init(EGUI_VIEW_OF(&preview_box));
    egui_view_set_size(EGUI_VIEW_OF(&preview_box), 104, 44);
    egui_view_number_box_set_font(EGUI_VIEW_OF(&preview_box), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_number_box_set_meta_font(EGUI_VIEW_OF(&preview_box), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_number_box_set_suffix(EGUI_VIEW_OF(&preview_box), "ms");
    egui_view_number_box_set_range(EGUI_VIEW_OF(&preview_box), 0, 24);
    egui_view_number_box_set_step(EGUI_VIEW_OF(&preview_box), 2);
    egui_view_number_box_set_compact_mode(EGUI_VIEW_OF(&preview_box), 1);
    egui_view_number_box_set_palette(EGUI_VIEW_OF(&preview_box), EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD8DFE6), EGUI_COLOR_HEX(0x1A2734),
                                     EGUI_COLOR_HEX(0x6B7A89), EGUI_COLOR_HEX(0x2F76B7));
    egui_view_number_box_set_value(EGUI_VIEW_OF(&preview_box), 12);
    egui_view_number_box_set_on_value_changed_listener(EGUI_VIEW_OF(&preview_box), on_value_changed);
    egui_view_number_box_override_static_preview_api(EGUI_VIEW_OF(&preview_box), &preview_api);
    changed_count = 0;
    changed_value = -1;
}

static void layout_number_box(egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
{
    egui_region_t region;

    region.location.x = x;
    region.location.y = y;
    region.size.width = width;
    region.size.height = height;
    egui_view_layout(EGUI_VIEW_OF(&test_box), &region);
    egui_region_copy(&EGUI_VIEW_OF(&test_box)->region_screen, &region);
}

static void layout_preview_number_box(void)
{
    egui_region_t region;

    region.location.x = 10;
    region.location.y = 20;
    region.size.width = 104;
    region.size.height = 44;
    egui_view_layout(EGUI_VIEW_OF(&preview_box), &region);
    egui_region_copy(&EGUI_VIEW_OF(&preview_box)->region_screen, &region);
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

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_box), type, x, y);
}

static int send_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&test_box), key_code);
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_box), type, x, y);
}

static int send_preview_key(uint8_t key_code)
{
    return send_key_to_view(EGUI_VIEW_OF(&preview_box), key_code);
}

static void capture_preview_snapshot(number_box_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_box)->region_screen;
    snapshot->on_value_changed = preview_box.on_value_changed;
    snapshot->font = preview_box.font;
    snapshot->meta_font = preview_box.meta_font;
    snapshot->label = preview_box.label;
    snapshot->suffix = preview_box.suffix;
    snapshot->helper = preview_box.helper;
    snapshot->surface_color = preview_box.surface_color;
    snapshot->border_color = preview_box.border_color;
    snapshot->text_color = preview_box.text_color;
    snapshot->muted_text_color = preview_box.muted_text_color;
    snapshot->accent_color = preview_box.accent_color;
    snapshot->value = preview_box.value;
    snapshot->min_value = preview_box.min_value;
    snapshot->max_value = preview_box.max_value;
    snapshot->step = preview_box.step;
    snapshot->compact_mode = preview_box.compact_mode;
    snapshot->read_only_mode = preview_box.read_only_mode;
    snapshot->pressed_part = preview_box.pressed_part;
    snapshot->alpha = EGUI_VIEW_OF(&preview_box)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_box));
}

static void assert_preview_state_unchanged(const number_box_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_box)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_box.on_value_changed == snapshot->on_value_changed);
    EGUI_TEST_ASSERT_TRUE(preview_box.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_box.meta_font == snapshot->meta_font);
    assert_optional_string_equal(snapshot->label, preview_box.label);
    assert_optional_string_equal(snapshot->suffix, preview_box.suffix);
    assert_optional_string_equal(snapshot->helper, preview_box.helper);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_box.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_box.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_box.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_box.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_box.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->value, preview_box.value);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->min_value, preview_box.min_value);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->max_value, preview_box.max_value);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->step, preview_box.step);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_box.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_box.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_part, preview_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_box)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_box)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(-1, changed_value);
}

static void test_number_box_range_and_value_clamp(void)
{
    setup_number_box();

    egui_view_number_box_set_range(EGUI_VIEW_OF(&test_box), 20, 10);
    EGUI_TEST_ASSERT_EQUAL_INT(10, test_box.min_value);
    EGUI_TEST_ASSERT_EQUAL_INT(20, test_box.max_value);
    EGUI_TEST_ASSERT_EQUAL_INT(20, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_number_box_set_value(EGUI_VIEW_OF(&test_box), -1);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(10, changed_value);

    egui_view_number_box_set_value(EGUI_VIEW_OF(&test_box), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(20, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(20, changed_value);

    egui_view_number_box_set_value(EGUI_VIEW_OF(&test_box), 20);
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
}

static void test_number_box_step_normalization(void)
{
    setup_number_box();

    egui_view_number_box_set_step(EGUI_VIEW_OF(&test_box), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_box.step);

    egui_view_number_box_set_step(EGUI_VIEW_OF(&test_box), -3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_box.step);

    egui_view_number_box_set_step(EGUI_VIEW_OF(&test_box), 5);
    EGUI_TEST_ASSERT_EQUAL_INT(5, test_box.step);
}

static void test_number_box_setters_clear_pressed_state_and_clamp(void)
{
    setup_number_box();

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_DEC;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), true);
    egui_view_number_box_set_range(EGUI_VIEW_OF(&test_box), 20, 10);
    EGUI_TEST_ASSERT_EQUAL_INT(10, test_box.min_value);
    EGUI_TEST_ASSERT_EQUAL_INT(20, test_box.max_value);
    EGUI_TEST_ASSERT_EQUAL_INT(20, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_INC;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), true);
    egui_view_number_box_set_step(EGUI_VIEW_OF(&test_box), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_box.step);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_DEC;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), true);
    egui_view_number_box_set_value(EGUI_VIEW_OF(&test_box), 20);
    EGUI_TEST_ASSERT_EQUAL_INT(20, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_number_box_font_modes_and_palette(void)
{
    static const char *value_label = "Value";
    static const char *value_suffix = "ms";
    static const char *value_helper = "Helper";

    setup_number_box();

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_DEC;
    EGUI_VIEW_OF(&test_box)->is_pressed = true;
    egui_view_number_box_set_font(EGUI_VIEW_OF(&test_box), NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_INC;
    EGUI_VIEW_OF(&test_box)->is_pressed = true;
    egui_view_number_box_set_meta_font(EGUI_VIEW_OF(&test_box), NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(test_box.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_box.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    egui_view_number_box_set_label(EGUI_VIEW_OF(&test_box), value_label);
    egui_view_number_box_set_suffix(EGUI_VIEW_OF(&test_box), value_suffix);
    egui_view_number_box_set_helper(EGUI_VIEW_OF(&test_box), value_helper);
    EGUI_TEST_ASSERT_TRUE(test_box.label == value_label);
    EGUI_TEST_ASSERT_TRUE(test_box.suffix == value_suffix);
    EGUI_TEST_ASSERT_TRUE(test_box.helper == value_helper);

    egui_view_number_box_set_compact_mode(EGUI_VIEW_OF(&test_box), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_box.compact_mode);

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_DEC;
    EGUI_VIEW_OF(&test_box)->is_pressed = true;
    egui_view_number_box_set_compact_mode(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_INC;
    EGUI_VIEW_OF(&test_box)->is_pressed = true;
    egui_view_number_box_set_read_only_mode(EGUI_VIEW_OF(&test_box), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_box.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_DEC;
    EGUI_VIEW_OF(&test_box)->is_pressed = true;
    egui_view_number_box_set_palette(EGUI_VIEW_OF(&test_box), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132),
                                     EGUI_COLOR_HEX(0x404142), EGUI_COLOR_HEX(0x505152));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x101112).full, test_box.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x202122).full, test_box.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x303132).full, test_box.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x404142).full, test_box.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_box.accent_color.full);
}

static void test_number_box_touch_increment_and_decrement(void)
{
    egui_view_number_box_metrics_t metrics;
    egui_dim_t inc_x;
    egui_dim_t inc_y;
    egui_dim_t dec_x;
    egui_dim_t dec_y;

    setup_number_box();
    layout_number_box(10, 20, 196, 70);
    egui_view_number_box_get_metrics(&test_box, EGUI_VIEW_OF(&test_box), &metrics);

    inc_x = metrics.inc_region.location.x + metrics.inc_region.size.width / 2;
    inc_y = metrics.inc_region.location.y + metrics.inc_region.size.height / 2;
    dec_x = metrics.dec_region.location.x + metrics.dec_region.size.width / 2;
    dec_y = metrics.dec_region.location.y + metrics.dec_region.size.height / 2;

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_INC, egui_view_number_box_hit_part(&test_box, EGUI_VIEW_OF(&test_box), inc_x, inc_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_DEC, egui_view_number_box_hit_part(&test_box, EGUI_VIEW_OF(&test_box), dec_x, dec_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_INC, test_box.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(28, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(28, changed_value);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, dec_x, dec_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_DEC, test_box.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, dec_x, dec_y));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(24, changed_value);
}

static void test_number_box_release_on_different_target_does_not_commit(void)
{
    egui_view_number_box_metrics_t metrics;
    egui_dim_t inc_x;
    egui_dim_t inc_y;
    egui_dim_t dec_x;
    egui_dim_t dec_y;

    setup_number_box();
    layout_number_box(10, 20, 196, 70);
    egui_view_number_box_get_metrics(&test_box, EGUI_VIEW_OF(&test_box), &metrics);
    inc_x = metrics.inc_region.location.x + metrics.inc_region.size.width / 2;
    inc_y = metrics.inc_region.location.y + metrics.inc_region.size.height / 2;
    dec_x = metrics.dec_region.location.x + metrics.dec_region.size.width / 2;
    dec_y = metrics.dec_region.location.y + metrics.dec_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, dec_x, dec_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_number_box_same_target_release_requires_return_to_origin(void)
{
    egui_view_number_box_metrics_t metrics;
    egui_dim_t inc_x;
    egui_dim_t inc_y;
    egui_dim_t dec_x;
    egui_dim_t dec_y;

    setup_number_box();
    layout_number_box(10, 20, 196, 70);
    egui_view_number_box_get_metrics(&test_box, EGUI_VIEW_OF(&test_box), &metrics);
    inc_x = metrics.inc_region.location.x + metrics.inc_region.size.width / 2;
    inc_y = metrics.inc_region.location.y + metrics.inc_region.size.height / 2;
    dec_x = metrics.dec_region.location.x + metrics.dec_region.size.width / 2;
    dec_y = metrics.dec_region.location.y + metrics.dec_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, dec_x, dec_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_INC, test_box.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, dec_x, dec_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, dec_x, dec_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_INC, test_box.pressed_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(28, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(28, changed_value);
}

static void test_number_box_compact_mode_clears_pressed_and_ignores_input(void)
{
    egui_view_number_box_metrics_t metrics;
    egui_dim_t inc_x;
    egui_dim_t inc_y;

    setup_number_box();
    layout_number_box(10, 20, 196, 70);
    egui_view_number_box_get_metrics(&test_box, EGUI_VIEW_OF(&test_box), &metrics);
    inc_x = metrics.inc_region.location.x + metrics.inc_region.size.width / 2;
    inc_y = metrics.inc_region.location.y + metrics.inc_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_INC, test_box.pressed_part);

    egui_view_number_box_set_compact_mode(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_box.compact_mode);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_DEC;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), true);
    egui_view_number_box_set_compact_mode(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_INC;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), true);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);

    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_number_box_set_compact_mode(EGUI_VIEW_OF(&test_box), 0);
    layout_number_box(10, 20, 196, 70);
    egui_view_number_box_get_metrics(&test_box, EGUI_VIEW_OF(&test_box), &metrics);
    inc_x = metrics.inc_region.location.x + metrics.inc_region.size.width / 2;
    inc_y = metrics.inc_region.location.y + metrics.inc_region.size.height / 2;
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    EGUI_TEST_ASSERT_EQUAL_INT(28, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
}

static void test_number_box_read_only_mode_clears_pressed_and_ignores_input(void)
{
    egui_view_number_box_metrics_t metrics;
    egui_dim_t inc_x;
    egui_dim_t inc_y;

    setup_number_box();
    layout_number_box(10, 20, 196, 70);
    egui_view_number_box_get_metrics(&test_box, EGUI_VIEW_OF(&test_box), &metrics);
    inc_x = metrics.inc_region.location.x + metrics.inc_region.size.width / 2;
    inc_y = metrics.inc_region.location.y + metrics.inc_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_INC, test_box.pressed_part);

    egui_view_number_box_set_read_only_mode(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_box.read_only_mode);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_DEC;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), true);
    egui_view_number_box_set_read_only_mode(EGUI_VIEW_OF(&test_box), 1);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_INC;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), true);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_number_box_set_read_only_mode(EGUI_VIEW_OF(&test_box), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, test_box.read_only_mode);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    EGUI_TEST_ASSERT_EQUAL_INT(28, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
}

static void test_number_box_disabled_ignores_input_and_clears_pressed_state(void)
{
    egui_view_number_box_metrics_t metrics;
    egui_dim_t inc_x;
    egui_dim_t inc_y;

    setup_number_box();
    layout_number_box(10, 20, 196, 70);
    egui_view_number_box_get_metrics(&test_box, EGUI_VIEW_OF(&test_box), &metrics);
    inc_x = metrics.inc_region.location.x + metrics.inc_region.size.width / 2;
    inc_y = metrics.inc_region.location.y + metrics.inc_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_box)->is_pressed);
    egui_view_set_enable(EGUI_VIEW_OF(&test_box), 0);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);

    test_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_DEC;
    egui_view_set_pressed(EGUI_VIEW_OF(&test_box), true);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_number_box_touch_cancel_clears_pressed_state(void)
{
    egui_view_number_box_metrics_t metrics;
    egui_dim_t dec_x;
    egui_dim_t dec_y;

    setup_number_box();
    layout_number_box(10, 20, 196, 70);
    egui_view_number_box_get_metrics(&test_box, EGUI_VIEW_OF(&test_box), &metrics);
    dec_x = metrics.dec_region.location.x + metrics.dec_region.size.width / 2;
    dec_y = metrics.dec_region.location.y + metrics.dec_region.size.height / 2;

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, dec_x, dec_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_DEC, test_box.pressed_part);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, dec_x, dec_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_box)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_NUMBER_BOX_PART_NONE, test_box.pressed_part);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, dec_x, dec_y));
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_number_box_get_value(EGUI_VIEW_OF(&test_box)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_number_box_static_preview_consumes_input_and_keeps_state(void)
{
    number_box_preview_snapshot_t initial_snapshot;
    egui_view_number_box_metrics_t metrics;
    egui_dim_t x;
    egui_dim_t y;

    setup_preview_number_box();
    layout_preview_number_box();
    egui_view_number_box_get_metrics(&preview_box, EGUI_VIEW_OF(&preview_box), &metrics);
    x = metrics.field_region.location.x + metrics.field_region.size.width / 2;
    y = metrics.field_region.location.y + metrics.field_region.size.height / 2;
    capture_preview_snapshot(&initial_snapshot);

    EGUI_VIEW_OF(&preview_box)->is_pressed = true;
    preview_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_INC;
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, x, y));
    assert_preview_state_unchanged(&initial_snapshot);

    EGUI_VIEW_OF(&preview_box)->is_pressed = true;
    preview_box.pressed_part = EGUI_VIEW_NUMBER_BOX_PART_DEC;
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_number_box_run(void)
{
    EGUI_TEST_SUITE_BEGIN(number_box);
    EGUI_TEST_RUN(test_number_box_range_and_value_clamp);
    EGUI_TEST_RUN(test_number_box_step_normalization);
    EGUI_TEST_RUN(test_number_box_setters_clear_pressed_state_and_clamp);
    EGUI_TEST_RUN(test_number_box_font_modes_and_palette);
    EGUI_TEST_RUN(test_number_box_touch_increment_and_decrement);
    EGUI_TEST_RUN(test_number_box_release_on_different_target_does_not_commit);
    EGUI_TEST_RUN(test_number_box_same_target_release_requires_return_to_origin);
    EGUI_TEST_RUN(test_number_box_compact_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_number_box_read_only_mode_clears_pressed_and_ignores_input);
    EGUI_TEST_RUN(test_number_box_disabled_ignores_input_and_clears_pressed_state);
    EGUI_TEST_RUN(test_number_box_touch_cancel_clears_pressed_state);
    EGUI_TEST_RUN(test_number_box_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
