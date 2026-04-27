#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_inline_ui_container.h"

#include "../../HelloCustomWidgets/layout/inline_ui_container/egui_view_inline_ui_container.h"
#include "../../HelloCustomWidgets/layout/inline_ui_container/egui_view_inline_ui_container.c"

typedef struct inline_ui_preview_snapshot inline_ui_preview_snapshot_t;
struct inline_ui_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    egui_view_t *child;
    const egui_font_t *text_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t host_surface_color;
    egui_color_t host_border_color;
    egui_color_t accent_color;
    char prefix_text[EGUI_VIEW_INLINE_UI_CONTAINER_MAX_TEXT_LEN + 1];
    char suffix_text[EGUI_VIEW_INLINE_UI_CONTAINER_MAX_TEXT_LEN + 1];
    egui_dim_t text_slot_width;
    egui_dim_t inline_gap;
    egui_dim_t baseline_offset;
    egui_dim_t corner_radius;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_t child_x;
    egui_dim_t child_y;
};

static egui_view_inline_ui_container_t test_control;
static egui_view_label_t test_child;
static egui_view_inline_ui_container_t preview_control;
static egui_view_label_t preview_child;
static egui_view_api_t preview_api;
static uint8_t click_count;

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

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void setup_child(egui_view_label_t *child, egui_dim_t width, egui_dim_t height, const char *text)
{
    egui_view_label_init(EGUI_VIEW_OF(child), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(child), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(child), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(child), EGUI_ALIGN_CENTER);
}

static void setup_inline_ui_container(void)
{
    egui_view_inline_ui_container_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 160, 42);
}

static void setup_preview_control(void)
{
    egui_view_inline_ui_container_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 92, 30);
    setup_child(&preview_child, 48, 16, "Tag");
    egui_view_inline_ui_container_set_child(EGUI_VIEW_OF(&preview_control), EGUI_VIEW_OF(&preview_child));
    egui_view_inline_ui_container_apply_compact_style(EGUI_VIEW_OF(&preview_control));
    egui_view_inline_ui_container_set_text(EGUI_VIEW_OF(&preview_control), "Fit", "line");
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_control), on_preview_click);
    egui_view_inline_ui_container_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
    click_count = 0;
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

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    egui_key_event_t event;
    int handled = 0;

    memset(&event, 0, sizeof(event));
    event.type = EGUI_KEY_EVENT_ACTION_DOWN;
    event.key_code = key_code;
    handled |= view->api->dispatch_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->dispatch_key_event(view, &event);
    return handled;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(inline_ui_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->child = preview_control.child;
    snapshot->text_font = preview_control.text_font;
    snapshot->surface_color = preview_control.surface_color;
    snapshot->border_color = preview_control.border_color;
    snapshot->text_color = preview_control.text_color;
    snapshot->host_surface_color = preview_control.host_surface_color;
    snapshot->host_border_color = preview_control.host_border_color;
    snapshot->accent_color = preview_control.accent_color;
    egui_view_inline_ui_container_copy_text(snapshot->prefix_text, sizeof(snapshot->prefix_text), preview_control.prefix_text);
    egui_view_inline_ui_container_copy_text(snapshot->suffix_text, sizeof(snapshot->suffix_text), preview_control.suffix_text);
    snapshot->text_slot_width = preview_control.text_slot_width;
    snapshot->inline_gap = preview_control.inline_gap;
    snapshot->baseline_offset = preview_control.baseline_offset;
    snapshot->corner_radius = preview_control.corner_radius;
    snapshot->compact_mode = preview_control.compact_mode;
    snapshot->read_only_mode = preview_control.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->child_x = EGUI_VIEW_OF(&preview_child)->region.location.x;
    snapshot->child_y = EGUI_VIEW_OF(&preview_child)->region.location.y;
}

static void assert_preview_state_unchanged(const inline_ui_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_TRUE(preview_control.child == snapshot->child);
    EGUI_TEST_ASSERT_TRUE(preview_control.text_font == snapshot->text_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_control.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->host_surface_color.full, preview_control.host_surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->host_border_color.full, preview_control.host_border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    assert_string_equal(snapshot->prefix_text, preview_control.prefix_text);
    assert_string_equal(snapshot->suffix_text, preview_control.suffix_text);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_slot_width, preview_control.text_slot_width);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->inline_gap, preview_control.inline_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->baseline_offset, preview_control.baseline_offset);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->corner_radius, preview_control.corner_radius);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_control.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_control.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->child_x, EGUI_VIEW_OF(&preview_child)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->child_y, EGUI_VIEW_OF(&preview_child)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_inline_ui_container_init_defaults(void)
{
    setup_inline_ui_container();

    EGUI_TEST_ASSERT_TRUE(egui_view_inline_ui_container_get_child(EGUI_VIEW_OF(&test_control)) == NULL);
    assert_string_equal("Before", egui_view_inline_ui_container_get_prefix_text(EGUI_VIEW_OF(&test_control)));
    assert_string_equal("after", egui_view_inline_ui_container_get_suffix_text(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_TRUE(test_control.text_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_EQUAL_INT(48, egui_view_inline_ui_container_get_text_slot_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_inline_ui_container_get_inline_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_inline_ui_container_get_baseline_offset(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_inline_ui_container_get_corner_radius(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_inline_ui_container_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_inline_ui_container_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, test_control.accent_color.full);
}

static void test_inline_ui_container_child_regions_and_layout(void)
{
    egui_region_t prefix_region;
    egui_region_t host_region;
    egui_region_t suffix_region;

    setup_inline_ui_container();
    setup_child(&test_child, 50, 16, "Child");
    egui_view_inline_ui_container_set_child(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_child));
    EGUI_TEST_ASSERT_TRUE(egui_view_inline_ui_container_get_child(EGUI_VIEW_OF(&test_control)) == EGUI_VIEW_OF(&test_child));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_group_get_child_count(EGUI_VIEW_OF(&test_control)));

    layout_view(EGUI_VIEW_OF(&test_control), 10, 20, 160, 42);
    egui_view_inline_ui_container_layout_child(EGUI_VIEW_OF(&test_control));
    egui_view_inline_ui_container_get_regions(EGUI_VIEW_OF(&test_control), &prefix_region, &host_region, &suffix_region);
    EGUI_TEST_ASSERT_EQUAL_INT(48, prefix_region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(62, host_region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(22, host_region.size.height);
    EGUI_TEST_ASSERT_EQUAL_INT(prefix_region.location.x + prefix_region.size.width + 6, host_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(host_region.location.x + host_region.size.width + 6, suffix_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(host_region.location.x + 6, EGUI_VIEW_OF(&test_child)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(host_region.location.y + 3, EGUI_VIEW_OF(&test_child)->region.location.y);

    egui_view_inline_ui_container_set_metrics(EGUI_VIEW_OF(&test_control), 36, 4, -2, 6);
    egui_view_inline_ui_container_get_regions(EGUI_VIEW_OF(&test_control), NULL, &host_region, NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(36, egui_view_inline_ui_container_get_text_slot_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_inline_ui_container_get_inline_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(-2, egui_view_inline_ui_container_get_baseline_offset(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(36 + 4 + EGUI_VIEW_OF(&test_control)->padding.left, host_region.location.x);
}

static void test_inline_ui_container_styles_palette_text_and_clamps(void)
{
    char long_text[] = "This text is intentionally longer than inline storage";

    setup_inline_ui_container();
    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_inline_ui_container_set_text(EGUI_VIEW_OF(&test_control), long_text, "tail");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_INLINE_UI_CONTAINER_MAX_TEXT_LEN, (int)strlen(test_control.prefix_text));
    assert_string_equal("tail", test_control.suffix_text);

    egui_view_inline_ui_container_set_metrics(EGUI_VIEW_OF(&test_control), -1, 99, -99, 99);
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_inline_ui_container_get_text_slot_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(14, egui_view_inline_ui_container_get_inline_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(-8, egui_view_inline_ui_container_get_baseline_offset(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(14, egui_view_inline_ui_container_get_corner_radius(EGUI_VIEW_OF(&test_control)));

    egui_view_inline_ui_container_set_metrics(EGUI_VIEW_OF(&test_control), 99, -1, 99, -1);
    EGUI_TEST_ASSERT_EQUAL_INT(76, egui_view_inline_ui_container_get_text_slot_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_inline_ui_container_get_inline_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_inline_ui_container_get_baseline_offset(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_inline_ui_container_get_corner_radius(EGUI_VIEW_OF(&test_control)));

    egui_view_inline_ui_container_apply_accent_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_inline_ui_container_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_inline_ui_container_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(-2, egui_view_inline_ui_container_get_baseline_offset(EGUI_VIEW_OF(&test_control)));

    egui_view_inline_ui_container_apply_compact_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_inline_ui_container_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_inline_ui_container_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, test_control.accent_color.full);

    egui_view_inline_ui_container_apply_read_only_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_inline_ui_container_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_inline_ui_container_get_read_only_mode(EGUI_VIEW_OF(&test_control)));

    egui_view_inline_ui_container_set_font(EGUI_VIEW_OF(&test_control), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    EGUI_TEST_ASSERT_TRUE(test_control.text_font == (const egui_font_t *)&egui_res_font_montserrat_10_4);

    egui_view_inline_ui_container_set_palette(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213),
                                              EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233), EGUI_COLOR_HEX(0x414243),
                                              EGUI_COLOR_HEX(0x515253));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x313233).full, test_control.host_surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x515253).full, test_control.accent_color.full);
}

static void test_inline_ui_container_static_preview_consumes_input_and_keeps_state(void)
{
    inline_ui_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_control();
    layout_view(EGUI_VIEW_OF(&preview_control), 12, 18, 92, 30);
    egui_view_inline_ui_container_layout_child(EGUI_VIEW_OF(&preview_control));
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

void test_inline_ui_container_run(void)
{
    EGUI_TEST_SUITE_BEGIN(inline_ui_container);
    EGUI_TEST_RUN(test_inline_ui_container_init_defaults);
    EGUI_TEST_RUN(test_inline_ui_container_child_regions_and_layout);
    EGUI_TEST_RUN(test_inline_ui_container_styles_palette_text_and_clamps);
    EGUI_TEST_RUN(test_inline_ui_container_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
