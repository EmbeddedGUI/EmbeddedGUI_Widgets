#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_field.h"

#include "../../HelloCustomWidgets/input/field/egui_view_field.h"
#include "../../HelloCustomWidgets/input/field/egui_view_field.c"

typedef struct field_preview_snapshot field_preview_snapshot_t;
struct field_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const char *label;
    const char *field_text;
    const char *placeholder;
    const char *helper_text;
    const char *validation_text;
    const char *info_title;
    const char *info_body;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    const egui_font_t *icon_font;
    hcw_field_on_open_changed_listener_t on_open_changed;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t error_color;
    egui_color_t bubble_surface_color;
    egui_color_t shadow_color;
    uint8_t required;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t open;
    uint8_t validation_state;
    uint8_t pressed_part;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static hcw_field_t test_widget;
static hcw_field_t preview_widget;
static egui_view_api_t preview_api;
static uint8_t g_open_state;
static uint8_t g_open_count;

static void on_open_changed(egui_view_t *self, uint8_t is_open)
{
    EGUI_UNUSED(self);
    g_open_state = is_open;
    g_open_count++;
}

static void reset_listener_state(void)
{
    g_open_state = 0xFF;
    g_open_count = 0;
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

static void setup_widget(void)
{
    hcw_field_init(EGUI_VIEW_OF(&test_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_widget), 196, 126);
    hcw_field_set_label(EGUI_VIEW_OF(&test_widget), "Owner alias");
    hcw_field_set_field_text(EGUI_VIEW_OF(&test_widget), "");
    hcw_field_set_placeholder(EGUI_VIEW_OF(&test_widget), "team@example.com");
    hcw_field_set_helper_text(EGUI_VIEW_OF(&test_widget), "Used by audit routing.");
    hcw_field_set_validation_text(EGUI_VIEW_OF(&test_widget), "Enter a valid alias before saving.");
    hcw_field_set_validation_state(EGUI_VIEW_OF(&test_widget), HCW_FIELD_VALIDATION_ERROR);
    hcw_field_set_required(EGUI_VIEW_OF(&test_widget), 1);
    hcw_field_set_info_title(EGUI_VIEW_OF(&test_widget), "Alias format");
    hcw_field_set_info_body(EGUI_VIEW_OF(&test_widget), "Use the shared team mailbox.");
    hcw_field_set_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    hcw_field_set_meta_font(EGUI_VIEW_OF(&test_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_on_open_changed_listener(EGUI_VIEW_OF(&test_widget), on_open_changed);
    reset_listener_state();
}

static void setup_preview_widget(void)
{
    hcw_field_init(EGUI_VIEW_OF(&preview_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_widget), 104, 54);
    hcw_field_apply_compact_style(EGUI_VIEW_OF(&preview_widget));
    hcw_field_set_label(EGUI_VIEW_OF(&preview_widget), "Alias");
    hcw_field_set_field_text(EGUI_VIEW_OF(&preview_widget), "core-api");
    hcw_field_set_placeholder(EGUI_VIEW_OF(&preview_widget), "");
    hcw_field_set_helper_text(EGUI_VIEW_OF(&preview_widget), "");
    hcw_field_set_validation_text(EGUI_VIEW_OF(&preview_widget), "");
    hcw_field_set_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_meta_font(EGUI_VIEW_OF(&preview_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    hcw_field_set_on_open_changed_listener(EGUI_VIEW_OF(&preview_widget), on_open_changed);
    hcw_field_override_static_preview_api(EGUI_VIEW_OF(&preview_widget), &preview_api);
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

static void seed_pressed_state(hcw_field_t *widget, uint8_t visual_pressed)
{
    widget->pressed_part = HCW_FIELD_PART_INFO_BUTTON;
    egui_view_set_pressed(EGUI_VIEW_OF(widget), visual_pressed ? 1 : 0);
}

static void assert_pressed_cleared(hcw_field_t *widget)
{
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_FIELD_PART_NONE, widget->pressed_part);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(widget)->is_pressed);
}

static void capture_preview_snapshot(field_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_widget)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_widget)->background;
    snapshot->label = preview_widget.label;
    snapshot->field_text = preview_widget.field_text;
    snapshot->placeholder = preview_widget.placeholder;
    snapshot->helper_text = preview_widget.helper_text;
    snapshot->validation_text = preview_widget.validation_text;
    snapshot->info_title = preview_widget.info_title;
    snapshot->info_body = preview_widget.info_body;
    snapshot->font = preview_widget.font;
    snapshot->meta_font = preview_widget.meta_font;
    snapshot->icon_font = preview_widget.icon_font;
    snapshot->on_open_changed = preview_widget.on_open_changed;
    snapshot->surface_color = preview_widget.surface_color;
    snapshot->border_color = preview_widget.border_color;
    snapshot->text_color = preview_widget.text_color;
    snapshot->muted_text_color = preview_widget.muted_text_color;
    snapshot->accent_color = preview_widget.accent_color;
    snapshot->success_color = preview_widget.success_color;
    snapshot->warning_color = preview_widget.warning_color;
    snapshot->error_color = preview_widget.error_color;
    snapshot->bubble_surface_color = preview_widget.bubble_surface_color;
    snapshot->shadow_color = preview_widget.shadow_color;
    snapshot->required = preview_widget.required;
    snapshot->compact_mode = preview_widget.compact_mode;
    snapshot->read_only_mode = preview_widget.read_only_mode;
    snapshot->open = preview_widget.open;
    snapshot->validation_state = preview_widget.validation_state;
    snapshot->pressed_part = preview_widget.pressed_part;
    snapshot->alpha = EGUI_VIEW_OF(&preview_widget)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_widget));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_widget)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_widget)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_widget)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_widget)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_widget)->padding.bottom;
}

static void assert_preview_state_unchanged(const field_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_widget)->background == snapshot->background);
    assert_optional_string_equal(snapshot->label, preview_widget.label);
    assert_optional_string_equal(snapshot->field_text, preview_widget.field_text);
    assert_optional_string_equal(snapshot->placeholder, preview_widget.placeholder);
    assert_optional_string_equal(snapshot->helper_text, preview_widget.helper_text);
    assert_optional_string_equal(snapshot->validation_text, preview_widget.validation_text);
    assert_optional_string_equal(snapshot->info_title, preview_widget.info_title);
    assert_optional_string_equal(snapshot->info_body, preview_widget.info_body);
    EGUI_TEST_ASSERT_TRUE(preview_widget.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_widget.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_TRUE(preview_widget.icon_font == snapshot->icon_font);
    EGUI_TEST_ASSERT_TRUE(preview_widget.on_open_changed == snapshot->on_open_changed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_widget.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_widget.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_widget.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_widget.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->error_color.full, preview_widget.error_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->bubble_surface_color.full, preview_widget.bubble_surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->shadow_color.full, preview_widget.shadow_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->required, preview_widget.required);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->open, preview_widget.open);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->validation_state, preview_widget.validation_state);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->pressed_part, preview_widget.pressed_part);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_widget)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_widget)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_widget)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_widget)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_widget)->padding.bottom);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0xFF, g_open_state);
}

static void get_part_center(egui_view_t *view, uint8_t part, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(hcw_field_get_part_region(view, part, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static void test_field_style_helpers_and_setters_clear_pressed_state(void)
{
    egui_region_t bubble_region;

    setup_widget();

    EGUI_TEST_ASSERT_FALSE(test_widget.compact_mode);
    EGUI_TEST_ASSERT_FALSE(test_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_FIELD_VALIDATION_ERROR, test_widget.validation_state);
    EGUI_TEST_ASSERT_TRUE(test_widget.required);

    seed_pressed_state(&test_widget, 1);
    hcw_field_apply_compact_style(EGUI_VIEW_OF(&test_widget));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.compact_mode);
    EGUI_TEST_ASSERT_FALSE(test_widget.read_only_mode);

    seed_pressed_state(&test_widget, 0);
    hcw_field_apply_read_only_style(EGUI_VIEW_OF(&test_widget));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_FALSE(test_widget.compact_mode);
    EGUI_TEST_ASSERT_TRUE(test_widget.read_only_mode);

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_label(EGUI_VIEW_OF(&test_widget), "Approval mail");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.label, "Approval mail"));

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_field_text(EGUI_VIEW_OF(&test_widget), "ops@studio.dev");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.field_text, "ops@studio.dev"));

    seed_pressed_state(&test_widget, 0);
    hcw_field_set_placeholder(EGUI_VIEW_OF(&test_widget), "name@company.com");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.placeholder, "name@company.com"));

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_helper_text(EGUI_VIEW_OF(&test_widget), "Used for build alerts only.");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.helper_text, "Used for build alerts only."));

    seed_pressed_state(&test_widget, 0);
    hcw_field_set_validation_text(EGUI_VIEW_OF(&test_widget), "Address format looks good.");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.validation_text, "Address format looks good."));

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_validation_state(EGUI_VIEW_OF(&test_widget), HCW_FIELD_VALIDATION_SUCCESS);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_FIELD_VALIDATION_SUCCESS, test_widget.validation_state);

    seed_pressed_state(&test_widget, 0);
    hcw_field_set_required(EGUI_VIEW_OF(&test_widget), 0);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_FALSE(test_widget.required);

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_info_title(EGUI_VIEW_OF(&test_widget), "Validation");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.info_title, "Validation"));

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_info_body(EGUI_VIEW_OF(&test_widget), "Success keeps the field lightweight.");
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(test_widget.info_body, "Success keeps the field lightweight."));

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_widget, 0);
    hcw_field_set_meta_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_icon_font(EGUI_VIEW_OF(&test_widget), NULL);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.icon_font == EGUI_FONT_ICON_MS_16);

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_palette(EGUI_VIEW_OF(&test_widget), EGUI_COLOR_HEX(0x101112), EGUI_COLOR_HEX(0x202122), EGUI_COLOR_HEX(0x303132), EGUI_COLOR_HEX(0x404142),
                          EGUI_COLOR_HEX(0x505152), EGUI_COLOR_HEX(0x606162), EGUI_COLOR_HEX(0x707172), EGUI_COLOR_HEX(0x808182),
                          EGUI_COLOR_HEX(0x909192), EGUI_COLOR_HEX(0xA0A1A2));
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x505152).full, test_widget.accent_color.full);

    seed_pressed_state(&test_widget, 0);
    hcw_field_set_compact_mode(EGUI_VIEW_OF(&test_widget), 1);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.compact_mode);

    seed_pressed_state(&test_widget, 1);
    hcw_field_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    assert_pressed_cleared(&test_widget);
    EGUI_TEST_ASSERT_TRUE(test_widget.read_only_mode);

    layout_view(EGUI_VIEW_OF(&test_widget), 12, 18, 196, 126);
    EGUI_TEST_ASSERT_FALSE(hcw_field_get_part_region(EGUI_VIEW_OF(&test_widget), HCW_FIELD_PART_BUBBLE, &bubble_region));
    hcw_field_set_open(EGUI_VIEW_OF(&test_widget), 1);
    EGUI_TEST_ASSERT_TRUE(hcw_field_get_part_region(EGUI_VIEW_OF(&test_widget), HCW_FIELD_PART_BUBBLE, &bubble_region));
}

static void test_field_touch_same_target_release_and_cancel_behavior(void)
{
    egui_dim_t info_x;
    egui_dim_t info_y;
    egui_dim_t outside_x;

    setup_widget();
    layout_view(EGUI_VIEW_OF(&test_widget), 12, 18, 196, 126);
    get_part_center(EGUI_VIEW_OF(&test_widget), HCW_FIELD_PART_INFO_BUTTON, &info_x, &info_y);
    outside_x = info_x - 56;

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, info_x, info_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, info_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, outside_x, info_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_field_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_count);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, info_x, info_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, info_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_MOVE, info_x, info_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, info_x, info_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_field_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_state);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, info_x, info_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_CANCEL, info_x, info_y));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_field_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
    assert_pressed_cleared(&test_widget);
}

static void test_field_keyboard_toggle_and_escape_close(void)
{
    setup_widget();

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_field_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_open_state);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_SPACE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_field_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_state);
    assert_pressed_cleared(&test_widget);

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(1, hcw_field_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(3, g_open_count);

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ESCAPE));
    EGUI_TEST_ASSERT_EQUAL_INT(0, hcw_field_get_open(EGUI_VIEW_OF(&test_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, g_open_count);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_open_state);
    assert_pressed_cleared(&test_widget);
}

static void test_field_static_preview_consumes_input_and_keeps_state(void)
{
    field_preview_snapshot_t initial_snapshot;
    egui_dim_t field_x;
    egui_dim_t field_y;

    setup_preview_widget();
    layout_view(EGUI_VIEW_OF(&preview_widget), 12, 18, 104, 54);
    get_part_center(EGUI_VIEW_OF(&preview_widget), HCW_FIELD_PART_FIELD, &field_x, &field_y);
    capture_preview_snapshot(&initial_snapshot);

    seed_pressed_state(&preview_widget, 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_MOTION_EVENT_ACTION_DOWN, field_x, field_y));
    assert_preview_state_unchanged(&initial_snapshot);

    seed_pressed_state(&preview_widget, 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_widget), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

static void test_field_read_only_and_disabled_guard_clear_pressed_state(void)
{
    egui_dim_t info_x;
    egui_dim_t info_y;

    setup_widget();
    layout_view(EGUI_VIEW_OF(&test_widget), 12, 18, 196, 126);
    get_part_center(EGUI_VIEW_OF(&test_widget), HCW_FIELD_PART_INFO_BUTTON, &info_x, &info_y);

    hcw_field_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 1);
    seed_pressed_state(&test_widget, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, info_x, info_y));
    assert_pressed_cleared(&test_widget);
    seed_pressed_state(&test_widget, 1);
    EGUI_TEST_ASSERT_FALSE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(&test_widget);

    hcw_field_set_read_only_mode(EGUI_VIEW_OF(&test_widget), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_widget), 0);
    seed_pressed_state(&test_widget, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_DOWN, info_x, info_y));
    assert_pressed_cleared(&test_widget);
    seed_pressed_state(&test_widget, 1);
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_widget), EGUI_MOTION_EVENT_ACTION_UP, info_x, info_y));
    assert_pressed_cleared(&test_widget);
    seed_pressed_state(&test_widget, 0);
    EGUI_TEST_ASSERT_FALSE(send_key_to_view(EGUI_VIEW_OF(&test_widget), EGUI_KEY_CODE_SPACE));
    assert_pressed_cleared(&test_widget);
}

void test_field_run(void)
{
    EGUI_TEST_SUITE_BEGIN(field);
    EGUI_TEST_RUN(test_field_style_helpers_and_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_field_touch_same_target_release_and_cancel_behavior);
    EGUI_TEST_RUN(test_field_keyboard_toggle_and_escape_close);
    EGUI_TEST_RUN(test_field_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_field_read_only_and_disabled_guard_clear_pressed_state);
    EGUI_TEST_SUITE_END();
}
