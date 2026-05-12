#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_border.h"

#include "../../HelloCustomWidgets/layout/border/egui_view_border.h"
#include "../../HelloCustomWidgets/layout/border/egui_view_border.c"

typedef struct border_preview_snapshot border_preview_snapshot_t;
struct border_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    egui_view_t *child;
    egui_color_t background_color;
    egui_color_t border_color;
    egui_color_t accent_color;
    egui_dim_t corner_radius;
    egui_dim_t border_width;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_t child_x;
    egui_dim_t child_y;
    egui_dim_margin_padding_t padding_left;
    egui_dim_margin_padding_t padding_right;
    egui_dim_margin_padding_t padding_top;
    egui_dim_margin_padding_t padding_bottom;
};

static egui_view_border_t test_border;
static egui_view_label_t test_child;
static egui_view_border_t preview_border;
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

static void setup_border(void)
{
    egui_view_border_init(EGUI_VIEW_OF(&test_border));
    egui_view_set_size(EGUI_VIEW_OF(&test_border), 120, 72);
}

static void setup_preview_border(void)
{
    egui_view_border_init(EGUI_VIEW_OF(&preview_border));
    egui_view_set_size(EGUI_VIEW_OF(&preview_border), 88, 44);
    setup_child(&preview_child, 64, 18, "Compact");
    egui_view_border_set_child(EGUI_VIEW_OF(&preview_border), EGUI_VIEW_OF(&preview_child));
    egui_view_border_apply_compact_style(EGUI_VIEW_OF(&preview_border));
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_border), on_preview_click);
    egui_view_border_override_static_preview_api(EGUI_VIEW_OF(&preview_border), &preview_api);
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

static void capture_preview_snapshot(border_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_border)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_border)->api;
    snapshot->child = preview_border.child;
    snapshot->background_color = preview_border.background_color;
    snapshot->border_color = preview_border.border_color;
    snapshot->accent_color = preview_border.accent_color;
    snapshot->corner_radius = preview_border.corner_radius;
    snapshot->border_width = preview_border.border_width;
    snapshot->compact_mode = preview_border.compact_mode;
    snapshot->read_only_mode = preview_border.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_border)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_border));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_border)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_border)->is_focused;
    snapshot->child_x = EGUI_VIEW_OF(&preview_child)->region.location.x;
    snapshot->child_y = EGUI_VIEW_OF(&preview_child)->region.location.y;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_border)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_border)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_border)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_border)->padding.bottom;
}

static void assert_preview_state_unchanged(const border_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_border)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_border)->api == snapshot->api);
    EGUI_TEST_ASSERT_TRUE(preview_border.child == snapshot->child);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->background_color.full, preview_border.background_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_border.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_border.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->corner_radius, preview_border.corner_radius);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_width, preview_border.border_width);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_border.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_border.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_border)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_border)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_border)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_border)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->child_x, EGUI_VIEW_OF(&preview_child)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->child_y, EGUI_VIEW_OF(&preview_child)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_border)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_border)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_border)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_border)->padding.bottom);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_border_init_defaults(void)
{
    setup_border();

    EGUI_TEST_ASSERT_TRUE(egui_view_border_get_child(EGUI_VIEW_OF(&test_border)) == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(10, egui_view_border_get_corner_radius(EGUI_VIEW_OF(&test_border)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_border_get_border_width(EGUI_VIEW_OF(&test_border)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_border_get_compact_mode(EGUI_VIEW_OF(&test_border)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_border_get_read_only_mode(EGUI_VIEW_OF(&test_border)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_SURFACE.full, test_border.background_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_BORDER.full, test_border.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY.full, test_border.accent_color.full);
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(14, EGUI_VIEW_OF(&test_border)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(14, EGUI_VIEW_OF(&test_border)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_border)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_border)->padding.bottom);
#endif
}

static void test_border_child_and_padding_layout(void)
{
    setup_border();
    setup_child(&test_child, 40, 16, "Child");

    egui_view_border_set_child(EGUI_VIEW_OF(&test_border), EGUI_VIEW_OF(&test_child));
    EGUI_TEST_ASSERT_TRUE(egui_view_border_get_child(EGUI_VIEW_OF(&test_border)) == EGUI_VIEW_OF(&test_child));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_group_get_child_count(EGUI_VIEW_OF(&test_border)));
#if EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
    EGUI_TEST_ASSERT_EQUAL_INT(26, EGUI_VIEW_OF(&test_child)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(16, EGUI_VIEW_OF(&test_child)->region.location.y);

    egui_view_border_set_padding(EGUI_VIEW_OF(&test_border), 8, 8, 6, 6);
    EGUI_TEST_ASSERT_EQUAL_INT(32, EGUI_VIEW_OF(&test_child)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(22, EGUI_VIEW_OF(&test_child)->region.location.y);
#else
    EGUI_TEST_ASSERT_EQUAL_INT(40, EGUI_VIEW_OF(&test_child)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(28, EGUI_VIEW_OF(&test_child)->region.location.y);

    egui_view_border_set_padding(EGUI_VIEW_OF(&test_border), 8, 8, 6, 6);
    EGUI_TEST_ASSERT_EQUAL_INT(40, EGUI_VIEW_OF(&test_child)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(28, EGUI_VIEW_OF(&test_child)->region.location.y);
#endif
}

static void test_border_styles_palette_and_clamps(void)
{
    setup_border();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_border), 1);
    egui_view_border_set_corner_radius(EGUI_VIEW_OF(&test_border), 99);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_border)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(24, egui_view_border_get_corner_radius(EGUI_VIEW_OF(&test_border)));

    egui_view_border_set_border_width(EGUI_VIEW_OF(&test_border), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_border_get_border_width(EGUI_VIEW_OF(&test_border)));

    egui_view_border_set_palette(EGUI_VIEW_OF(&test_border), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213), EGUI_COLOR_HEX(0x212223));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_border.background_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x111213).full, test_border.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x212223).full, test_border.accent_color.full);

    egui_view_border_apply_compact_style(EGUI_VIEW_OF(&test_border));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_border_get_compact_mode(EGUI_VIEW_OF(&test_border)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_border_get_read_only_mode(EGUI_VIEW_OF(&test_border)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_border_get_corner_radius(EGUI_VIEW_OF(&test_border)));

    egui_view_border_apply_read_only_style(EGUI_VIEW_OF(&test_border));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_border_get_compact_mode(EGUI_VIEW_OF(&test_border)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_border_get_read_only_mode(EGUI_VIEW_OF(&test_border)));
}

static void test_border_static_preview_consumes_input_and_keeps_state(void)
{
    border_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_border();
    layout_view(EGUI_VIEW_OF(&preview_border), 12, 18, 88, 44);
    get_view_center(EGUI_VIEW_OF(&preview_border), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_border), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_border), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_border), EGUI_MOTION_EVENT_ACTION_UP, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_border), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_border), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_border_run(void)
{
    EGUI_TEST_SUITE_BEGIN(border);
    EGUI_TEST_RUN(test_border_init_defaults);
    EGUI_TEST_RUN(test_border_child_and_padding_layout);
    EGUI_TEST_RUN(test_border_styles_palette_and_clamps);
    EGUI_TEST_RUN(test_border_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
