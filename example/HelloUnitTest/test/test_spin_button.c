#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_spin_button.h"

#include "../../HelloCustomWidgets/input/spin_button/egui_view_spin_button.h"
#include "../../HelloCustomWidgets/input/spin_button/egui_view_spin_button.c"

typedef struct spin_button_preview_snapshot spin_button_preview_snapshot_t;
struct spin_button_preview_snapshot
{
    egui_region_t region_screen;
    egui_view_spin_button_value_changed_listener_t on_value_changed;
    const egui_font_t *value_font;
    const egui_font_t *meta_font;
    const char *label;
    const char *suffix;
    const char *helper;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t field_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    int16_t value;
    int16_t min_value;
    int16_t max_value;
    int16_t step;
    int16_t large_step;
    uint8_t active_part;
    uint8_t focus_part;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
};

static egui_view_spin_button_t test_spin;
static egui_view_spin_button_t preview_spin;
static egui_view_api_t preview_api;
static uint8_t changed_count;
static int16_t changed_value;

static void on_value_changed(egui_view_t *self, int16_t value)
{
    EGUI_UNUSED(self);
    changed_count++;
    changed_value = value;
}

static void reset_changed_state(void)
{
    changed_count = 0;
    changed_value = -1;
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

static void setup_spin_button(void)
{
    egui_view_spin_button_init(EGUI_VIEW_OF(&test_spin));
    egui_view_set_size(EGUI_VIEW_OF(&test_spin), 196, 72);
    egui_view_spin_button_set_texts(EGUI_VIEW_OF(&test_spin), "Columns", "cols", "0 to 20, step 2");
    egui_view_spin_button_set_range(EGUI_VIEW_OF(&test_spin), 0, 20);
    egui_view_spin_button_set_step(EGUI_VIEW_OF(&test_spin), 2);
    egui_view_spin_button_set_large_step(EGUI_VIEW_OF(&test_spin), 6);
    egui_view_spin_button_set_value(EGUI_VIEW_OF(&test_spin), 10);
    egui_view_spin_button_set_on_value_changed_listener(EGUI_VIEW_OF(&test_spin), on_value_changed);
    reset_changed_state();
}

static void setup_preview_spin_button(void)
{
    egui_view_spin_button_init(EGUI_VIEW_OF(&preview_spin));
    egui_view_set_size(EGUI_VIEW_OF(&preview_spin), 104, 44);
    egui_view_spin_button_set_fonts(EGUI_VIEW_OF(&preview_spin), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                    (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_spin_button_set_texts(EGUI_VIEW_OF(&preview_spin), NULL, "px", NULL);
    egui_view_spin_button_set_range(EGUI_VIEW_OF(&preview_spin), 0, 24);
    egui_view_spin_button_set_step(EGUI_VIEW_OF(&preview_spin), 2);
    egui_view_spin_button_set_large_step(EGUI_VIEW_OF(&preview_spin), 4);
    egui_view_spin_button_set_compact_mode(EGUI_VIEW_OF(&preview_spin), 1);
    egui_view_spin_button_set_value(EGUI_VIEW_OF(&preview_spin), 8);
    egui_view_spin_button_set_on_value_changed_listener(EGUI_VIEW_OF(&preview_spin), on_value_changed);
    egui_view_spin_button_override_static_preview_api(EGUI_VIEW_OF(&preview_spin), &preview_api);
    reset_changed_state();
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

static void layout_spin_button(void)
{
    layout_view(EGUI_VIEW_OF(&test_spin), 10, 20, 196, 72);
}

static void layout_preview_spin_button(void)
{
    layout_view(EGUI_VIEW_OF(&preview_spin), 12, 18, 104, 44);
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

static int send_key_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->dispatch_key_event(view, &event);
}

static int send_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_to_view(EGUI_VIEW_OF(&test_spin), EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_to_view(EGUI_VIEW_OF(&test_spin), EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&test_spin), type, x, y);
}

static int send_preview_key(uint8_t key_code)
{
    int handled = 0;

    handled |= send_key_to_view(EGUI_VIEW_OF(&preview_spin), EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= send_key_to_view(EGUI_VIEW_OF(&preview_spin), EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static int send_preview_touch(uint8_t type, egui_dim_t x, egui_dim_t y)
{
    return send_touch_to_view(EGUI_VIEW_OF(&preview_spin), type, x, y);
}

static uint8_t get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    if (!egui_view_spin_button_get_part_region(view, part, &region))
    {
        return 0;
    }
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
    return 1;
}

static void seed_active_state(egui_view_spin_button_t *spin, uint8_t part, uint8_t visual_pressed)
{
    spin->active_part = part;
    egui_view_set_pressed(EGUI_VIEW_OF(spin), visual_pressed ? 1 : 0);
}

static void assert_active_cleared(egui_view_spin_button_t *spin)
{
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_NONE, spin->active_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(spin)->is_pressed);
}

static void capture_preview_snapshot(spin_button_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_spin)->region_screen;
    snapshot->on_value_changed = preview_spin.on_value_changed;
    snapshot->value_font = preview_spin.value_font;
    snapshot->meta_font = preview_spin.meta_font;
    snapshot->label = preview_spin.label;
    snapshot->suffix = preview_spin.suffix;
    snapshot->helper = preview_spin.helper;
    snapshot->api = EGUI_VIEW_OF(&preview_spin)->api;
    snapshot->surface_color = preview_spin.surface_color;
    snapshot->field_color = preview_spin.field_color;
    snapshot->border_color = preview_spin.border_color;
    snapshot->text_color = preview_spin.text_color;
    snapshot->muted_text_color = preview_spin.muted_text_color;
    snapshot->accent_color = preview_spin.accent_color;
    snapshot->value = preview_spin.value;
    snapshot->min_value = preview_spin.min_value;
    snapshot->max_value = preview_spin.max_value;
    snapshot->step = preview_spin.step;
    snapshot->large_step = preview_spin.large_step;
    snapshot->active_part = preview_spin.active_part;
    snapshot->focus_part = preview_spin.focus_part;
    snapshot->compact_mode = preview_spin.compact_mode;
    snapshot->read_only_mode = preview_spin.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_spin)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_spin));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_spin)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_spin)->is_focused;
}

static void assert_preview_state_unchanged(const spin_button_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_spin)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_spin.on_value_changed == snapshot->on_value_changed);
    EGUI_TEST_ASSERT_TRUE(preview_spin.value_font == snapshot->value_font);
    EGUI_TEST_ASSERT_TRUE(preview_spin.meta_font == snapshot->meta_font);
    assert_optional_string_equal(snapshot->label, preview_spin.label);
    assert_optional_string_equal(snapshot->suffix, preview_spin.suffix);
    assert_optional_string_equal(snapshot->helper, preview_spin.helper);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_spin)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_spin.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->field_color.full, preview_spin.field_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_spin.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_spin.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_spin.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_spin.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->value, preview_spin.value);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->min_value, preview_spin.min_value);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->max_value, preview_spin.max_value);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->step, preview_spin.step);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->large_step, preview_spin.large_step);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_NONE, preview_spin.active_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->focus_part, preview_spin.focus_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_spin.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_spin.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_spin)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_spin)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_spin)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_spin)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(-1, changed_value);
}

static void test_spin_button_range_and_value_clamp(void)
{
    setup_spin_button();

    egui_view_spin_button_set_range(EGUI_VIEW_OF(&test_spin), 14, 6);
    EGUI_TEST_ASSERT_EQUAL_INT(6, test_spin.min_value);
    EGUI_TEST_ASSERT_EQUAL_INT(14, test_spin.max_value);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_spin_button_set_range(EGUI_VIEW_OF(&test_spin), 0, 8);
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    egui_view_spin_button_set_value(EGUI_VIEW_OF(&test_spin), -4);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_value);

    egui_view_spin_button_set_value(EGUI_VIEW_OF(&test_spin), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(8, changed_value);

    egui_view_spin_button_set_value(EGUI_VIEW_OF(&test_spin), 8);
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
}

static void test_spin_button_step_and_large_step_normalization(void)
{
    setup_spin_button();

    egui_view_spin_button_set_step(EGUI_VIEW_OF(&test_spin), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_spin.step);
    EGUI_TEST_ASSERT_EQUAL_INT(6, test_spin.large_step);

    egui_view_spin_button_set_large_step(EGUI_VIEW_OF(&test_spin), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_spin.large_step);

    egui_view_spin_button_set_step(EGUI_VIEW_OF(&test_spin), 4);
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_spin.step);
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_spin.large_step);

    egui_view_spin_button_set_large_step(EGUI_VIEW_OF(&test_spin), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(4, test_spin.large_step);

    egui_view_spin_button_set_large_step(EGUI_VIEW_OF(&test_spin), 12);
    EGUI_TEST_ASSERT_EQUAL_INT(12, test_spin.large_step);
}

static void test_spin_button_setters_clear_active_state(void)
{
    setup_spin_button();

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    egui_view_spin_button_set_value(EGUI_VIEW_OF(&test_spin), 10);
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, 1);
    egui_view_spin_button_set_range(EGUI_VIEW_OF(&test_spin), 2, 18);
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    egui_view_spin_button_set_step(EGUI_VIEW_OF(&test_spin), 0);
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_spin.step);

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, 1);
    egui_view_spin_button_set_large_step(EGUI_VIEW_OF(&test_spin), 0);
    assert_active_cleared(&test_spin);
}

static void test_spin_button_text_font_palette_and_modes(void)
{
    static const char *value_label = "Value";
    static const char *value_suffix = "ms";
    static const char *value_helper = "Helper";

    setup_spin_button();

    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PANEL.full, test_spin.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_SURFACE_PRESS.full, test_spin.field_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_BORDER_STRONG.full, test_spin.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_STRONG.full, test_spin.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_TEXT_SOFT.full, test_spin.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY_DARK.full, test_spin.accent_color.full);

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    egui_view_spin_button_set_texts(EGUI_VIEW_OF(&test_spin), value_label, value_suffix, value_helper);
    EGUI_TEST_ASSERT_TRUE(test_spin.label == value_label);
    EGUI_TEST_ASSERT_TRUE(test_spin.suffix == value_suffix);
    EGUI_TEST_ASSERT_TRUE(test_spin.helper == value_helper);
    assert_active_cleared(&test_spin);

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, 1);
    egui_view_spin_button_set_fonts(EGUI_VIEW_OF(&test_spin), NULL, NULL);
    EGUI_TEST_ASSERT_TRUE(test_spin.value_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_spin.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_active_cleared(&test_spin);

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    egui_view_spin_button_set_compact_mode(EGUI_VIEW_OF(&test_spin), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_spin.compact_mode);
    assert_active_cleared(&test_spin);

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, 1);
    egui_view_spin_button_set_read_only_mode(EGUI_VIEW_OF(&test_spin), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, test_spin.read_only_mode);
    assert_active_cleared(&test_spin);

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    egui_view_spin_button_set_palette(EGUI_VIEW_OF(&test_spin), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213),
                                      EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233), EGUI_COLOR_HEX(0x414243),
                                      EGUI_COLOR_HEX(0x515253));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_spin.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x111213).full, test_spin.field_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x212223).full, test_spin.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x313233).full, test_spin.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x414243).full, test_spin.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x515253).full, test_spin.accent_color.full);
    assert_active_cleared(&test_spin);
}

static void test_spin_button_adjust_and_guards(void)
{
    setup_spin_button();

    EGUI_TEST_ASSERT_TRUE(egui_view_spin_button_adjust(EGUI_VIEW_OF(&test_spin), 2));
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(12, changed_value);

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    egui_view_spin_button_set_compact_mode(EGUI_VIEW_OF(&test_spin), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_spin_button_adjust(EGUI_VIEW_OF(&test_spin), 2));
    assert_active_cleared(&test_spin);

    egui_view_spin_button_set_compact_mode(EGUI_VIEW_OF(&test_spin), 0);
    egui_view_spin_button_set_read_only_mode(EGUI_VIEW_OF(&test_spin), 1);
    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_spin_button_adjust(EGUI_VIEW_OF(&test_spin), -2));
    assert_active_cleared(&test_spin);

    egui_view_spin_button_set_read_only_mode(EGUI_VIEW_OF(&test_spin), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_spin), 0);
    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_spin_button_adjust(EGUI_VIEW_OF(&test_spin), 2));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
}

static void test_spin_button_touch_increment_decrement_and_field_focus(void)
{
    egui_dim_t inc_x;
    egui_dim_t inc_y;
    egui_dim_t dec_x;
    egui_dim_t dec_y;
    egui_dim_t field_x;
    egui_dim_t field_y;

    setup_spin_button();
    layout_spin_button();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_spin), EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, &inc_x, &inc_y));
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_spin), EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, &dec_x, &dec_y));
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_spin), EGUI_VIEW_SPIN_BUTTON_PART_FIELD, &field_x, &field_y));

    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT,
                               egui_view_spin_button_hit_part(&test_spin, EGUI_VIEW_OF(&test_spin), inc_x, inc_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT,
                               egui_view_spin_button_hit_part(&test_spin, EGUI_VIEW_OF(&test_spin), dec_x, dec_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_FIELD,
                               egui_view_spin_button_hit_part(&test_spin, EGUI_VIEW_OF(&test_spin), field_x, field_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, field_x, field_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_FIELD, test_spin.focus_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spin)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_spin)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, test_spin.active_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(12, changed_value);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, dec_x, dec_y));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, test_spin.active_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, dec_x, dec_y));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(10, changed_value);
}

static void test_spin_button_same_target_release_requires_return_to_origin(void)
{
    egui_dim_t inc_x;
    egui_dim_t inc_y;
    egui_dim_t dec_x;
    egui_dim_t dec_y;

    setup_spin_button();
    layout_spin_button();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_spin), EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, &inc_x, &inc_y));
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_spin), EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, &dec_x, &dec_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, dec_x, dec_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spin)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, test_spin.active_part);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, dec_x, dec_y));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, dec_x, dec_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_spin)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_MOVE, inc_x, inc_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_spin)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);
    EGUI_TEST_ASSERT_EQUAL_INT(12, changed_value);
}

static void test_spin_button_touch_cancel_clears_active_state(void)
{
    egui_dim_t dec_x;
    egui_dim_t dec_y;

    setup_spin_button();
    layout_spin_button();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_spin), EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, &dec_x, &dec_y));

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, dec_x, dec_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_spin)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, test_spin.active_part);

    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, dec_x, dec_y));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_CANCEL, dec_x, dec_y));
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_spin_button_keyboard_navigation_and_activate(void)
{
    setup_spin_button();

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_UP));
    EGUI_TEST_ASSERT_EQUAL_INT(12, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, test_spin.focus_part);
    EGUI_TEST_ASSERT_EQUAL_INT(1, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_DOWN));
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, test_spin.focus_part);
    EGUI_TEST_ASSERT_EQUAL_INT(2, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_HOME));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_FIELD, test_spin.focus_part);
    EGUI_TEST_ASSERT_EQUAL_INT(3, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_END));
    EGUI_TEST_ASSERT_EQUAL_INT(20, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_FIELD, test_spin.focus_part);
    EGUI_TEST_ASSERT_EQUAL_INT(4, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_LEFT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, test_spin.focus_part);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_spin), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_spin)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_spin), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_SPACE));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, changed_count);

    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_KEY_CODE_RIGHT));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, test_spin.focus_part);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_spin), EGUI_KEY_EVENT_ACTION_DOWN, EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_spin)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_spin), EGUI_KEY_EVENT_ACTION_UP, EGUI_KEY_CODE_ENTER));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(20, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, changed_count);

    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_TAB));
    assert_active_cleared(&test_spin);
}

static void test_spin_button_modes_and_disabled_ignore_input(void)
{
    egui_dim_t inc_x;
    egui_dim_t inc_y;

    setup_spin_button();
    layout_spin_button();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&test_spin), EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, &inc_x, &inc_y));

    egui_view_spin_button_set_compact_mode(EGUI_VIEW_OF(&test_spin), 1);
    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_UP, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_UP));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));

    egui_view_spin_button_set_compact_mode(EGUI_VIEW_OF(&test_spin), 0);
    egui_view_spin_button_set_read_only_mode(EGUI_VIEW_OF(&test_spin), 1);
    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_UP));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));

    egui_view_spin_button_set_read_only_mode(EGUI_VIEW_OF(&test_spin), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_spin), 0);
    seed_active_state(&test_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    EGUI_TEST_ASSERT_FALSE(send_key(EGUI_KEY_CODE_UP));
    assert_active_cleared(&test_spin);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_spin_button_get_value(EGUI_VIEW_OF(&test_spin)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, changed_count);
}

static void test_spin_button_static_preview_consumes_input_and_keeps_state(void)
{
    spin_button_preview_snapshot_t initial_snapshot;
    egui_dim_t inc_x;
    egui_dim_t inc_y;

    setup_preview_spin_button();
    layout_preview_spin_button();
    EGUI_TEST_ASSERT_TRUE(get_part_center(EGUI_VIEW_OF(&preview_spin), EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, &inc_x, &inc_y));
    capture_preview_snapshot(&initial_snapshot);

    seed_active_state(&preview_spin, EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_touch(EGUI_MOTION_EVENT_ACTION_DOWN, inc_x, inc_y));
    assert_preview_state_unchanged(&initial_snapshot);

    seed_active_state(&preview_spin, EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT, 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key(EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_spin_button_run(void)
{
    EGUI_TEST_SUITE_BEGIN(spin_button);
    EGUI_TEST_RUN(test_spin_button_range_and_value_clamp);
    EGUI_TEST_RUN(test_spin_button_step_and_large_step_normalization);
    EGUI_TEST_RUN(test_spin_button_setters_clear_active_state);
    EGUI_TEST_RUN(test_spin_button_text_font_palette_and_modes);
    EGUI_TEST_RUN(test_spin_button_adjust_and_guards);
    EGUI_TEST_RUN(test_spin_button_touch_increment_decrement_and_field_focus);
    EGUI_TEST_RUN(test_spin_button_same_target_release_requires_return_to_origin);
    EGUI_TEST_RUN(test_spin_button_touch_cancel_clears_active_state);
    EGUI_TEST_RUN(test_spin_button_keyboard_navigation_and_activate);
    EGUI_TEST_RUN(test_spin_button_modes_and_disabled_ignore_input);
    EGUI_TEST_RUN(test_spin_button_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
