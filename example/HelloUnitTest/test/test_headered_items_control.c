#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_headered_items_control.h"

#include "../../HelloCustomWidgets/layout/headered_items_control/egui_view_headered_items_control.h"
#include "../../HelloCustomWidgets/layout/headered_items_control/egui_view_headered_items_control.c"

typedef struct headered_items_control_preview_snapshot headered_items_control_preview_snapshot_t;
struct headered_items_control_preview_snapshot
{
    egui_region_t region_screen;
    const egui_view_api_t *api;
    egui_view_t *header;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t header_color;
    egui_color_t item_surface_color;
    egui_color_t accent_color;
    egui_dim_t corner_radius;
    egui_dim_t border_width;
    egui_dim_t header_gap;
    egui_dim_t item_gap;
    uint8_t layout_mode;
    uint8_t header_align_type;
    uint8_t item_align_type;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    int item_count;
    egui_dim_t header_x;
    egui_dim_t header_y;
    egui_dim_t item0_x;
    egui_dim_t item0_y;
    egui_dim_t item1_x;
    egui_dim_t item1_y;
    egui_dim_margin_padding_t content_padding_left;
    egui_dim_margin_padding_t content_padding_right;
    egui_dim_margin_padding_t content_padding_top;
    egui_dim_margin_padding_t content_padding_bottom;
};

static egui_view_headered_items_control_t test_control;
static egui_view_label_t test_header;
static egui_view_label_t test_item0;
static egui_view_label_t test_item1;
static egui_view_label_t test_item2;
static egui_view_headered_items_control_t preview_control;
static egui_view_label_t preview_header;
static egui_view_label_t preview_item0;
static egui_view_label_t preview_item1;
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

static void setup_label(egui_view_label_t *label, egui_dim_t width, egui_dim_t height, const char *text)
{
    egui_view_label_init(EGUI_VIEW_OF(label), uicode_get_core());
    egui_view_set_size(EGUI_VIEW_OF(label), width, height);
    egui_view_label_set_text(EGUI_VIEW_OF(label), text);
    egui_view_label_set_align_type(EGUI_VIEW_OF(label), EGUI_ALIGN_CENTER);
}

static void setup_headered_items_control(void)
{
    egui_view_headered_items_control_init(EGUI_VIEW_OF(&test_control));
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 150, 100);
}

static void setup_header_and_three_items(void)
{
    setup_label(&test_header, 72, 16, "Header");
    setup_label(&test_item0, 60, 14, "One");
    setup_label(&test_item1, 70, 14, "Two");
    setup_label(&test_item2, 50, 14, "Three");
    egui_view_headered_items_control_set_header(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_header));
    egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_item0));
    egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_item1));
    egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_item2));
}

static void setup_preview_control(void)
{
    egui_view_headered_items_control_init(EGUI_VIEW_OF(&preview_control));
    egui_view_set_size(EGUI_VIEW_OF(&preview_control), 92, 68);
    setup_label(&preview_header, 58, 12, "Wrap");
    setup_label(&preview_item0, 36, 12, "One");
    setup_label(&preview_item1, 36, 12, "Two");
    egui_view_headered_items_control_set_header(EGUI_VIEW_OF(&preview_control), EGUI_VIEW_OF(&preview_header));
    egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&preview_control), EGUI_VIEW_OF(&preview_item0));
    egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&preview_control), EGUI_VIEW_OF(&preview_item1));
    egui_view_headered_items_control_apply_wrap_style(EGUI_VIEW_OF(&preview_control));
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_control), on_preview_click);
    egui_view_headered_items_control_override_static_preview_api(EGUI_VIEW_OF(&preview_control), &preview_api);
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

static void capture_preview_snapshot(headered_items_control_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_control)->region_screen;
    snapshot->api = EGUI_VIEW_OF(&preview_control)->api;
    snapshot->header = preview_control.header;
    snapshot->surface_color = preview_control.surface_color;
    snapshot->border_color = preview_control.border_color;
    snapshot->header_color = preview_control.header_color;
    snapshot->item_surface_color = preview_control.item_surface_color;
    snapshot->accent_color = preview_control.accent_color;
    snapshot->corner_radius = preview_control.corner_radius;
    snapshot->border_width = preview_control.border_width;
    snapshot->header_gap = preview_control.header_gap;
    snapshot->item_gap = preview_control.item_gap;
    snapshot->layout_mode = preview_control.layout_mode;
    snapshot->header_align_type = preview_control.header_align_type;
    snapshot->item_align_type = preview_control.item_align_type;
    snapshot->compact_mode = preview_control.compact_mode;
    snapshot->read_only_mode = preview_control.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_control)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_control));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_control)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_control)->is_focused;
    snapshot->item_count = egui_view_headered_items_control_get_item_count(EGUI_VIEW_OF(&preview_control));
    snapshot->header_x = EGUI_VIEW_OF(&preview_header)->region.location.x;
    snapshot->header_y = EGUI_VIEW_OF(&preview_header)->region.location.y;
    snapshot->item0_x = EGUI_VIEW_OF(&preview_item0)->region.location.x;
    snapshot->item0_y = EGUI_VIEW_OF(&preview_item0)->region.location.y;
    snapshot->item1_x = EGUI_VIEW_OF(&preview_item1)->region.location.x;
    snapshot->item1_y = EGUI_VIEW_OF(&preview_item1)->region.location.y;
    snapshot->content_padding_left = preview_control.content_padding_left;
    snapshot->content_padding_right = preview_control.content_padding_right;
    snapshot->content_padding_top = preview_control.content_padding_top;
    snapshot->content_padding_bottom = preview_control.content_padding_bottom;
}

static void assert_preview_state_unchanged(const headered_items_control_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_control)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_control)->api == snapshot->api);
    EGUI_TEST_ASSERT_TRUE(preview_control.header == snapshot->header);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->header_color.full, preview_control.header_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_surface_color.full, preview_control.item_surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_control.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->corner_radius, preview_control.corner_radius);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_width, preview_control.border_width);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->header_gap, preview_control.header_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_gap, preview_control.item_gap);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->layout_mode, preview_control.layout_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->header_align_type, preview_control.header_align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_align_type, preview_control.item_align_type);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_control.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_control.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_control)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_control)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item_count, egui_view_headered_items_control_get_item_count(EGUI_VIEW_OF(&preview_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->header_x, EGUI_VIEW_OF(&preview_header)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->header_y, EGUI_VIEW_OF(&preview_header)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item0_x, EGUI_VIEW_OF(&preview_item0)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item0_y, EGUI_VIEW_OF(&preview_item0)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item1_x, EGUI_VIEW_OF(&preview_item1)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->item1_y, EGUI_VIEW_OF(&preview_item1)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_padding_left, preview_control.content_padding_left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_padding_right, preview_control.content_padding_right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_padding_top, preview_control.content_padding_top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->content_padding_bottom, preview_control.content_padding_bottom);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_headered_items_control_init_defaults(void)
{
    setup_headered_items_control();

    EGUI_TEST_ASSERT_TRUE(egui_view_headered_items_control_get_header(EGUI_VIEW_OF(&test_control)) == NULL);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_headered_items_control_get_item_count(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_VERTICAL,
                               egui_view_headered_items_control_get_layout_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_TOP_LEFT, egui_view_headered_items_control_get_header_align_type(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_ALIGN_TOP_LEFT, egui_view_headered_items_control_get_item_align_type(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(8, egui_view_headered_items_control_get_corner_radius(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_headered_items_control_get_border_width(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_headered_items_control_get_header_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_headered_items_control_get_item_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_headered_items_control_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_headered_items_control_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_SURFACE.full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_BORDER_STRONG.full, test_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_SURFACE_SUBTLE.full, test_control.header_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PANEL.full, test_control.item_surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(18, test_control.content_padding_left);
    EGUI_TEST_ASSERT_EQUAL_INT(18, test_control.content_padding_right);
    EGUI_TEST_ASSERT_EQUAL_INT(14, test_control.content_padding_top);
    EGUI_TEST_ASSERT_EQUAL_INT(14, test_control.content_padding_bottom);
    EGUI_TEST_ASSERT_EQUAL_INT(HCW_COLOR_PRIMARY.full, test_control.accent_color.full);
}

static void test_headered_items_control_header_and_linear_layout(void)
{
    setup_headered_items_control();
    setup_header_and_three_items();

    EGUI_TEST_ASSERT_TRUE(egui_view_headered_items_control_get_header(EGUI_VIEW_OF(&test_control)) == EGUI_VIEW_OF(&test_header));
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_headered_items_control_get_item_count(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_group_get_child_count(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(18, EGUI_VIEW_OF(&test_header)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(14, EGUI_VIEW_OF(&test_header)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(18, EGUI_VIEW_OF(&test_item0)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(36, EGUI_VIEW_OF(&test_item0)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(18, EGUI_VIEW_OF(&test_item1)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(55, EGUI_VIEW_OF(&test_item1)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(18, EGUI_VIEW_OF(&test_item2)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(74, EGUI_VIEW_OF(&test_item2)->region.location.y);

    egui_view_headered_items_control_set_header_align_type(EGUI_VIEW_OF(&test_control), EGUI_ALIGN_TOP_RIGHT);
    egui_view_headered_items_control_set_item_align_type(EGUI_VIEW_OF(&test_control), EGUI_ALIGN_BOTTOM_RIGHT);
    EGUI_TEST_ASSERT_EQUAL_INT(60, EGUI_VIEW_OF(&test_header)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(62, EGUI_VIEW_OF(&test_item0)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(36, EGUI_VIEW_OF(&test_item0)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(62, EGUI_VIEW_OF(&test_item1)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(55, EGUI_VIEW_OF(&test_item1)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(62, EGUI_VIEW_OF(&test_item2)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(74, EGUI_VIEW_OF(&test_item2)->region.location.y);

    egui_view_set_size(EGUI_VIEW_OF(&test_item0), 28, 14);
    egui_view_set_size(EGUI_VIEW_OF(&test_item1), 34, 14);
    egui_view_set_size(EGUI_VIEW_OF(&test_item2), 20, 14);
    egui_view_headered_items_control_set_layout_mode(EGUI_VIEW_OF(&test_control), EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_HORIZONTAL);
    egui_view_headered_items_control_set_item_align_type(EGUI_VIEW_OF(&test_control), EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER);
    EGUI_TEST_ASSERT_EQUAL_INT(18, EGUI_VIEW_OF(&test_item0)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(54, EGUI_VIEW_OF(&test_item0)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(51, EGUI_VIEW_OF(&test_item1)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(54, EGUI_VIEW_OF(&test_item1)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(90, EGUI_VIEW_OF(&test_item2)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(54, EGUI_VIEW_OF(&test_item2)->region.location.y);
}

static void test_headered_items_control_wrap_layout_and_clear(void)
{
    setup_headered_items_control();
    egui_view_set_size(EGUI_VIEW_OF(&test_control), 120, 90);
    setup_label(&test_header, 62, 14, "Header");
    setup_label(&test_item0, 42, 14, "One");
    setup_label(&test_item1, 45, 14, "Two");
    setup_label(&test_item2, 50, 14, "Three");
    egui_view_headered_items_control_set_header(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_header));
    egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_item0));
    egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_item1));
    egui_view_headered_items_control_add_item(EGUI_VIEW_OF(&test_control), EGUI_VIEW_OF(&test_item2));
    egui_view_headered_items_control_apply_wrap_style(EGUI_VIEW_OF(&test_control));

    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_header)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(10, EGUI_VIEW_OF(&test_header)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_item0)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(28, EGUI_VIEW_OF(&test_item0)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(59, EGUI_VIEW_OF(&test_item1)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(28, EGUI_VIEW_OF(&test_item1)->region.location.y);
    EGUI_TEST_ASSERT_EQUAL_INT(12, EGUI_VIEW_OF(&test_item2)->region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(47, EGUI_VIEW_OF(&test_item2)->region.location.y);

    egui_view_headered_items_control_clear_items(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_TRUE(egui_view_headered_items_control_get_header(EGUI_VIEW_OF(&test_control)) == EGUI_VIEW_OF(&test_header));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_headered_items_control_get_item_count(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_group_get_child_count(EGUI_VIEW_OF(&test_control)));
}

static void test_headered_items_control_styles_palette_and_clamps(void)
{
    setup_headered_items_control();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_control), 1);
    egui_view_headered_items_control_set_corner_radius(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_control)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_headered_items_control_get_corner_radius(EGUI_VIEW_OF(&test_control)));

    egui_view_headered_items_control_set_border_width(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(3, egui_view_headered_items_control_get_border_width(EGUI_VIEW_OF(&test_control)));

    egui_view_headered_items_control_set_header_gap(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(16, egui_view_headered_items_control_get_header_gap(EGUI_VIEW_OF(&test_control)));

    egui_view_headered_items_control_set_item_gap(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(18, egui_view_headered_items_control_get_item_gap(EGUI_VIEW_OF(&test_control)));

    egui_view_headered_items_control_set_layout_mode(EGUI_VIEW_OF(&test_control), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_VERTICAL,
                               egui_view_headered_items_control_get_layout_mode(EGUI_VIEW_OF(&test_control)));

    egui_view_headered_items_control_set_palette(EGUI_VIEW_OF(&test_control), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213),
                                                EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233), EGUI_COLOR_HEX(0x414243));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_control.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x111213).full, test_control.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x212223).full, test_control.header_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x313233).full, test_control.item_surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x414243).full, test_control.accent_color.full);

    egui_view_headered_items_control_apply_strip_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_HORIZONTAL,
                               egui_view_headered_items_control_get_layout_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(18, test_control.content_padding_left);
    EGUI_TEST_ASSERT_EQUAL_INT(14, test_control.content_padding_top);
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_headered_items_control_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_headered_items_control_get_read_only_mode(EGUI_VIEW_OF(&test_control)));

    egui_view_headered_items_control_apply_wrap_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_HEADERED_ITEMS_CONTROL_LAYOUT_WRAP, egui_view_headered_items_control_get_layout_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_headered_items_control_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_headered_items_control_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(6, egui_view_headered_items_control_get_corner_radius(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(4, egui_view_headered_items_control_get_header_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(5, egui_view_headered_items_control_get_item_gap(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(12, test_control.content_padding_left);
    EGUI_TEST_ASSERT_EQUAL_INT(10, test_control.content_padding_top);

    egui_view_headered_items_control_apply_read_only_style(EGUI_VIEW_OF(&test_control));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_headered_items_control_get_compact_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_headered_items_control_get_read_only_mode(EGUI_VIEW_OF(&test_control)));
    EGUI_TEST_ASSERT_EQUAL_INT(12, test_control.content_padding_left);
    EGUI_TEST_ASSERT_EQUAL_INT(10, test_control.content_padding_top);
}

static void test_headered_items_control_static_preview_consumes_input_and_keeps_state(void)
{
    headered_items_control_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_control();
    layout_view(EGUI_VIEW_OF(&preview_control), 12, 18, 92, 68);
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

void test_headered_items_control_run(void)
{
    EGUI_TEST_SUITE_BEGIN(headered_items_control);
    EGUI_TEST_RUN(test_headered_items_control_init_defaults);
    EGUI_TEST_RUN(test_headered_items_control_header_and_linear_layout);
    EGUI_TEST_RUN(test_headered_items_control_wrap_layout_and_clear);
    EGUI_TEST_RUN(test_headered_items_control_styles_palette_and_clamps);
    EGUI_TEST_RUN(test_headered_items_control_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
