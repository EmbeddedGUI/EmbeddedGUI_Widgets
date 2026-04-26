#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_image_control.h"

#include "../../HelloCustomWidgets/display/image_control/egui_view_image_control.h"
#include "../../HelloCustomWidgets/display/image_control/egui_view_image_control.c"

typedef struct image_control_preview_snapshot image_control_preview_snapshot_t;
struct image_control_preview_snapshot
{
    egui_region_t region_screen;
    const egui_image_t *image;
    const char *source_name;
    const egui_view_api_t *api;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t placeholder_color;
    egui_color_t muted_color;
    uint8_t stretch;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_pressed;
    uint8_t is_focused;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_image_control_t test_image;
static egui_view_image_control_t preview_image;
static egui_view_api_t preview_api;
static uint8_t click_count;

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

static void on_preview_click(egui_view_t *self)
{
    EGUI_UNUSED(self);
    click_count++;
}

static void setup_image_control(void)
{
    egui_view_image_control_init(EGUI_VIEW_OF(&test_image));
    egui_view_set_size(EGUI_VIEW_OF(&test_image), 120, 72);
}

static void setup_preview_image_control(void)
{
    egui_view_image_control_init(EGUI_VIEW_OF(&preview_image));
    egui_view_set_size(EGUI_VIEW_OF(&preview_image), 92, 50);
    egui_view_image_control_apply_compact_style(EGUI_VIEW_OF(&preview_image));
    egui_view_image_control_set_source(EGUI_VIEW_OF(&preview_image), egui_view_image_control_get_square_image(), "Square");
    egui_view_image_control_set_stretch(EGUI_VIEW_OF(&preview_image), EGUI_VIEW_IMAGE_CONTROL_STRETCH_FILL);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_image), on_preview_click);
    egui_view_image_control_override_static_preview_api(EGUI_VIEW_OF(&preview_image), &preview_api);
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

static void capture_preview_snapshot(image_control_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_image)->region_screen;
    snapshot->image = preview_image.image;
    snapshot->source_name = preview_image.source_name;
    snapshot->api = EGUI_VIEW_OF(&preview_image)->api;
    snapshot->surface_color = preview_image.surface_color;
    snapshot->border_color = preview_image.border_color;
    snapshot->placeholder_color = preview_image.placeholder_color;
    snapshot->muted_color = preview_image.muted_color;
    snapshot->stretch = preview_image.stretch;
    snapshot->compact_mode = preview_image.compact_mode;
    snapshot->read_only_mode = preview_image.read_only_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_image)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_image));
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_image)->is_pressed;
    snapshot->is_focused = EGUI_VIEW_OF(&preview_image)->is_focused;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_image)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_image)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_image)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_image)->padding.bottom;
}

static void assert_preview_state_unchanged(const image_control_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_image)->region_screen);
    EGUI_TEST_ASSERT_TRUE(preview_image.image == snapshot->image);
    assert_optional_string_equal(snapshot->source_name, preview_image.source_name);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_image)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_image.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_image.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->placeholder_color.full, preview_image.placeholder_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_color.full, preview_image.muted_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->stretch, preview_image.stretch);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_image.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_image.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_image)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_image)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&preview_image)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_image)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_image)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_image)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_image)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_image)->padding.bottom);
    EGUI_TEST_ASSERT_EQUAL_INT(0, click_count);
}

static void test_image_control_init_and_builtin_sources(void)
{
    egui_dim_t width;
    egui_dim_t height;

    setup_image_control();

    EGUI_TEST_ASSERT_TRUE(egui_view_image_control_get_source(EGUI_VIEW_OF(&test_image)) == egui_view_image_control_get_default_image());
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_IMAGE_CONTROL_STRETCH_UNIFORM, egui_view_image_control_get_stretch(EGUI_VIEW_OF(&test_image)));
    assert_optional_string_equal("Landscape", egui_view_image_control_get_source_name(EGUI_VIEW_OF(&test_image)));

    EGUI_TEST_ASSERT_TRUE(egui_image_get_size(egui_view_image_control_get_landscape_image(), &width, &height));
    EGUI_TEST_ASSERT_EQUAL_INT(16, width);
    EGUI_TEST_ASSERT_EQUAL_INT(10, height);
    EGUI_TEST_ASSERT_TRUE(egui_image_get_size(egui_view_image_control_get_portrait_image(), &width, &height));
    EGUI_TEST_ASSERT_EQUAL_INT(10, width);
    EGUI_TEST_ASSERT_EQUAL_INT(16, height);
    EGUI_TEST_ASSERT_TRUE(egui_image_get_size(egui_view_image_control_get_square_image(), &width, &height));
    EGUI_TEST_ASSERT_EQUAL_INT(12, width);
    EGUI_TEST_ASSERT_EQUAL_INT(12, height);
}

static void test_image_control_source_stretch_and_state_setters(void)
{
    setup_image_control();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_image), 1);
    egui_view_image_control_set_source(EGUI_VIEW_OF(&test_image), egui_view_image_control_get_portrait_image(), "Portrait");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_image)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(egui_view_image_control_get_source(EGUI_VIEW_OF(&test_image)) == egui_view_image_control_get_portrait_image());
    assert_optional_string_equal("Portrait", egui_view_image_control_get_source_name(EGUI_VIEW_OF(&test_image)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_image), 1);
    egui_view_image_control_set_stretch(EGUI_VIEW_OF(&test_image), EGUI_VIEW_IMAGE_CONTROL_STRETCH_FILL);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_image)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_IMAGE_CONTROL_STRETCH_FILL, egui_view_image_control_get_stretch(EGUI_VIEW_OF(&test_image)));

    egui_view_image_control_set_stretch(EGUI_VIEW_OF(&test_image), 99);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_IMAGE_CONTROL_STRETCH_UNIFORM, egui_view_image_control_get_stretch(EGUI_VIEW_OF(&test_image)));

    egui_view_image_control_set_source(EGUI_VIEW_OF(&test_image), NULL, NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_image_control_get_source(EGUI_VIEW_OF(&test_image)) == egui_view_image_control_get_default_image());
    assert_optional_string_equal("Landscape", egui_view_image_control_get_source_name(EGUI_VIEW_OF(&test_image)));
}

static void test_image_control_styles_and_palette(void)
{
    setup_image_control();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_image), 1);
    egui_view_image_control_apply_compact_style(EGUI_VIEW_OF(&test_image));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_image)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_image_control_get_compact_mode(EGUI_VIEW_OF(&test_image)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_image_control_get_read_only_mode(EGUI_VIEW_OF(&test_image)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_image), 1);
    egui_view_image_control_apply_read_only_style(EGUI_VIEW_OF(&test_image));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_image)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_image_control_get_compact_mode(EGUI_VIEW_OF(&test_image)));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_image_control_get_read_only_mode(EGUI_VIEW_OF(&test_image)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_image), 1);
    egui_view_image_control_set_palette(EGUI_VIEW_OF(&test_image), EGUI_COLOR_HEX(0x010203), EGUI_COLOR_HEX(0x111213),
                                        EGUI_COLOR_HEX(0x212223), EGUI_COLOR_HEX(0x313233));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_image)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x010203).full, test_image.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x111213).full, test_image.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x212223).full, test_image.placeholder_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x313233).full, test_image.muted_color.full);
}

static void test_image_control_static_preview_consumes_input_and_keeps_state(void)
{
    image_control_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_image_control();
    layout_view(EGUI_VIEW_OF(&preview_image), 12, 18, 92, 50);
    get_view_center(EGUI_VIEW_OF(&preview_image), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_image), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_image), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_image), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_image), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_image_control_run(void)
{
    EGUI_TEST_SUITE_BEGIN(image_control);
    EGUI_TEST_RUN(test_image_control_init_and_builtin_sources);
    EGUI_TEST_RUN(test_image_control_source_stretch_and_state_setters);
    EGUI_TEST_RUN(test_image_control_styles_and_palette);
    EGUI_TEST_RUN(test_image_control_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
