#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_label_control.h"

#include "../../HelloCustomWidgets/display/label_control/egui_view_label_control.h"
#include "../../HelloCustomWidgets/display/label_control/egui_view_label_control.c"

typedef struct label_control_preview_snapshot label_control_preview_snapshot_t;
struct label_control_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    const egui_font_t *text_font;
    const egui_font_t *hint_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t hint_color;
    egui_color_t accent_color;
    egui_color_t required_color;
    char text[EGUI_VIEW_LABEL_CONTROL_MAX_TEXT_LEN + 1];
    char target_hint[EGUI_VIEW_LABEL_CONTROL_MAX_HINT_LEN + 1];
    egui_alpha_t alpha;
    uint8_t align_type;
    uint8_t access_key_index;
    uint8_t required;
    uint8_t target_highlighted;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_margin_padding_t padding_left;
    egui_dim_margin_padding_t padding_right;
    egui_dim_margin_padding_t padding_top;
    egui_dim_margin_padding_t padding_bottom;
};

static egui_view_label_control_t test_control;
static egui_view_label_control_t preview_control;
static egui_view_api_t preview_api;

static void assert_region_equal(const egui_region_t *expected, const egui_region_t *actual)
{
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.x, actual->location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->location.y, actual->location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.width, actual->size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(expected->size.height, actual->size.height);
}

static void assert_string_equal(const char *expected, const char *actual)
{
    EGUI_TEST_ASSERT_TRUE(expected != NULL);
    EGUI_TEST_ASSERT_TRUE(actual != NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(expected, actual));
}

static void setup_label_control(void)
{
    egui_view_label_control_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 150, 42);
}

static void setup_preview_control(void)
{
    egui_view_label_control_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 92, 30);
    egui_view_label_control_apply_compact_style(EGUI_VIEW_OF(&preview_control));
    egui_view_label_control_set_text(EGUI_VIEW_OF(&preview_control), "Compact");
    egui_view_label_control_set_target_hint(EGUI_VIEW_OF(&preview_control), "Hidden");
    egui_view_label_control_set_access_key_index(EGUI_VIEW_OF(&preview_control), 0);
    egui_view_label_control_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
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

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(label_control_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->text_font = preview_control.text_font;
    snapshot->hint_font = preview_control.hint_font;
    snapshot->surface_color = preview_control.surface_color;
    snapshot->border_color = preview_control.border_color;
    snapshot->text_color = preview_control.text_color;
    snapshot->hint_color = preview_control.hint_color;
    snapshot->accent_color = preview_control.accent_color;
    snapshot->required_color = preview_control.required_color;
    egui_view_label_control_copy_text(snapshot->text, sizeof(snapshot->text), preview_control.text);
    egui_view_label_control_copy_text(snapshot->target_hint, sizeof(snapshot->target_hint), preview_control.target_hint);
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->align_type = preview_control.align_type;
    snapshot->access_key_index = preview_control.access_key_index;
    snapshot->required = preview_control.required;
    snapshot->target_highlighted = preview_control.target_highlighted;
    snapshot->compact_mode = preview_control.compact_mode;
    snapshot->read_only_mode = preview_control.read_only_mode;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_control)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_control)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_control)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_control)->padding.bottom;
}

static void assert_preview_state_unchanged(const label_control_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_TRUE(preview_control.text_font == snapshot->text_font);
    EGUI_TEST_ASSERT_TRUE(preview_control.hint_font == snapshot->hint_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_control.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->hint_color.full, preview_control.hint_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->required_color.full, preview_control.required_color.full);
    assert_string_equal(snapshot->text, preview_control.text);
    assert_string_equal(snapshot->target_hint, preview_control.target_hint);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->align_type, preview_control.align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->access_key_index, preview_control.access_key_index);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->required, preview_control.required);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->target_highlighted, preview_control.target_highlighted);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_control.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_control.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_control)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_control)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_control)->padding.bottom);
}

static void test_label_control_init_defaults(void)
{
    setup_label_control();

    assert_string_equal("Label", egui_view_label_control_get_text(EGUI_VIEW_OF(&test_control)));
    assert_string_equal("Target", egui_view_label_control_get_target_hint(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_TRUE(test_control.text_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_control.hint_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, egui_view_label_control_get_align_type(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_LABEL_CONTROL_ACCESS_NONE, egui_view_label_control_get_access_key_index(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_get_required(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_get_target_highlighted(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_control.accent_color.full);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(2, EGUI_VIEW_OF(&test_control)->padding.right);
#endif
}

static void test_label_control_setters_and_access_key_clamp(void)
{
    char long_text[] = "This label text is intentionally longer than the fixed storage";

    setup_label_control();
    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_label_control_set_text(EGUI_VIEW_OF(&test_control), long_text);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_LABEL_CONTROL_MAX_TEXT_LEN, egui_view_label_control_text_len(test_control.text));

    egui_view_label_control_set_text(EGUI_VIEW_OF(&test_control), "Username");
    egui_view_label_control_set_target_hint(EGUI_VIEW_OF(&test_control), "TextBox target");
    egui_view_label_control_set_access_key_index(EGUI_VIEW_OF(&test_control), 0);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_get_access_key_index(EGUI_VIEW_OF(&test_control)));

    egui_view_label_control_set_access_key_index(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_LABEL_CONTROL_ACCESS_NONE, egui_view_label_control_get_access_key_index(EGUI_VIEW_OF(&test_control)));

    egui_view_label_control_set_access_key_index(EGUI_VIEW_OF(&test_control), 4);
    egui_view_label_control_set_text(EGUI_VIEW_OF(&test_control), "Go");
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_LABEL_CONTROL_ACCESS_NONE, egui_view_label_control_get_access_key_index(EGUI_VIEW_OF(&test_control)));

    egui_view_label_control_set_required(EGUI_VIEW_OF(&test_control), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_label_control_get_required(EGUI_VIEW_OF(&test_control)));
    egui_view_label_control_set_target_highlighted(EGUI_VIEW_OF(&test_control), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_label_control_get_target_highlighted(EGUI_VIEW_OF(&test_control)));
    egui_view_label_control_set_align_type(EGUI_VIEW_OF(&test_control), EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, egui_view_label_control_get_align_type(EGUI_VIEW_OF(&test_control)));
}

static void test_label_control_styles_palette_and_fonts(void)
{
    setup_label_control();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_label_control_apply_accent_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_label_control_get_target_highlighted(EGUI_VIEW_OF(&test_control)));

    egui_view_label_control_apply_compact_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_label_control_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_get_target_highlighted(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, test_control.accent_color.full);

    egui_view_label_control_apply_read_only_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_label_control_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_label_control_get_read_only_mode(EGUI_VIEW_OF(&test_control)));

    egui_view_label_control_set_fonts(EGUI_VIEW_OF(&test_control), (const egui_font_t *)&egui_res_font_montserrat_10_4,
                                      (const egui_font_t *)&egui_res_font_montserrat_8_4);
    EGUI_TEST_ASSERT_TRUE(test_control.text_font == (const egui_font_t *)&egui_res_font_montserrat_10_4);
    EGUI_TEST_ASSERT_TRUE(test_control.hint_font == (const egui_font_t *)&egui_res_font_montserrat_8_4);

    egui_view_label_control_set_palette(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213),
                                        EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233), EGUI_COLOR_HEX(0x414243),
                                        EGUI_COLOR_HEX(0x515253));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x414243).full, test_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x515253).full, test_control.required_color.full);
}

static void test_label_control_static_preview_consumes_input_and_keeps_state(void)
{
    label_control_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_control();
    layout_view(EGUI_VIEW_OF(&preview_control), 12, 18, 92, 30);
    get_view_center(EGUI_VIEW_OF(&preview_control), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_control), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_control), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_control), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_control), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_control), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

static void test_label_control_text_helpers(void)
{
    char label[16];
    char value[24];

    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_label_control_text_len(NULL));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_label_control_text_len("Username"));
    egui_view_label_control_copy_elided(label, sizeof(label), "Synchronization", 8);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp("Synch...", label));
    egui_view_label_control_fit_text_to_width(NULL, "Disconnected target", value, sizeof(value), 24, 4);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp("Dis...", value));
}

void test_label_control_run(void)
{
    EGUI_TEST_SUITE_BEGIN(label_control);
    EGUI_TEST_RUN(test_label_control_init_defaults);
    EGUI_TEST_RUN(test_label_control_setters_and_access_key_clamp);
    EGUI_TEST_RUN(test_label_control_styles_palette_and_fonts);
    EGUI_TEST_RUN(test_label_control_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_RUN(test_label_control_text_helpers);
    EGUI_TEST_SUITE_END();
}
