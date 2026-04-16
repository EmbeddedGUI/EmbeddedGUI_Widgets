#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_tag.h"

#include "../../HelloCustomWidgets/display/tag/egui_view_tag.h"
#include "../../HelloCustomWidgets/display/tag/egui_view_tag.c"

typedef struct tag_preview_snapshot tag_preview_snapshot_t;
struct tag_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const char *text;
    const char *secondary_text;
    const egui_font_t *font;
    const egui_font_t *secondary_font;
    const egui_font_t *icon_font;
    egui_view_on_tag_dismiss_listener_t on_dismiss;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t secondary_color;
    egui_color_t accent_color;
    const egui_view_api_t *api;
    egui_alpha_t alpha;
    uint8_t dismissible;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    uint8_t dismiss_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_tag_t test_tag_widget;
static egui_view_tag_t preview_tag_widget;
static egui_view_api_t preview_api;
static uint8_t g_dismiss_count;

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

static void on_tag_dismiss(egui_view_t *self)
{
    EGUI_UNUSED(self);
    g_dismiss_count++;
}

static void setup_tag(void)
{
    egui_view_tag_init(EGUI_VIEW_OF(&test_tag_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_tag_widget), 128, 28);
    egui_view_tag_apply_standard_style(EGUI_VIEW_OF(&test_tag_widget));
    egui_view_tag_set_text(EGUI_VIEW_OF(&test_tag_widget), "Assigned");
    egui_view_tag_set_secondary_text(EGUI_VIEW_OF(&test_tag_widget), "Today");
    egui_view_tag_set_font(EGUI_VIEW_OF(&test_tag_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_tag_set_secondary_font(EGUI_VIEW_OF(&test_tag_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tag_set_on_dismiss_listener(EGUI_VIEW_OF(&test_tag_widget), on_tag_dismiss);
    egui_view_tag_set_dismissible(EGUI_VIEW_OF(&test_tag_widget), 1);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&test_tag_widget), 1);
#endif
    g_dismiss_count = 0;
}

static void setup_preview_tag(void)
{
    egui_view_tag_init(EGUI_VIEW_OF(&preview_tag_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_tag_widget), 88, 24);
    egui_view_tag_apply_compact_style(EGUI_VIEW_OF(&preview_tag_widget));
    egui_view_tag_set_text(EGUI_VIEW_OF(&preview_tag_widget), "Compact");
    egui_view_tag_set_secondary_text(EGUI_VIEW_OF(&preview_tag_widget), "Preview");
    egui_view_tag_set_font(EGUI_VIEW_OF(&preview_tag_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tag_set_secondary_font(EGUI_VIEW_OF(&preview_tag_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_tag_set_on_dismiss_listener(EGUI_VIEW_OF(&preview_tag_widget), on_tag_dismiss);
    egui_view_tag_override_static_preview_api(EGUI_VIEW_OF(&preview_tag_widget), &preview_api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(EGUI_VIEW_OF(&preview_tag_widget), 0);
#endif
    g_dismiss_count = 0;
}

static void layout_tag(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
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
    handled |= view->api->dispatch_key_event(view, &event);
    event.type = EGUI_KEY_EVENT_ACTION_UP;
    handled |= view->api->dispatch_key_event(view, &event);
    return handled;
}

static int send_preview_key_to_view(egui_view_t *view, uint8_t key_code)
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

static void get_dismiss_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    egui_region_t region;

    EGUI_TEST_ASSERT_TRUE(egui_view_tag_get_dismiss_region(view, &region));
    *x = region.location.x + region.size.width / 2;
    *y = region.location.y + region.size.height / 2;
}

static void capture_preview_snapshot(tag_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_tag_widget)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_tag_widget)->background;
    snapshot->text = preview_tag_widget.text;
    snapshot->secondary_text = preview_tag_widget.secondary_text;
    snapshot->font = preview_tag_widget.font;
    snapshot->secondary_font = preview_tag_widget.secondary_font;
    snapshot->icon_font = preview_tag_widget.icon_font;
    snapshot->on_dismiss = preview_tag_widget.on_dismiss;
    snapshot->surface_color = preview_tag_widget.surface_color;
    snapshot->border_color = preview_tag_widget.border_color;
    snapshot->text_color = preview_tag_widget.text_color;
    snapshot->secondary_color = preview_tag_widget.secondary_color;
    snapshot->accent_color = preview_tag_widget.accent_color;
    snapshot->api = EGUI_VIEW_OF(&preview_tag_widget)->api;
    snapshot->alpha = EGUI_VIEW_OF(&preview_tag_widget)->alpha;
    snapshot->dismissible = preview_tag_widget.dismissible;
    snapshot->compact_mode = preview_tag_widget.compact_mode;
    snapshot->read_only_mode = preview_tag_widget.read_only_mode;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_tag_widget));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_tag_widget)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_tag_widget)->is_pressed;
    snapshot->dismiss_pressed = preview_tag_widget.dismiss_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_tag_widget)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_tag_widget)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_tag_widget)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_tag_widget)->padding.bottom;
}

static void assert_preview_state_unchanged(const tag_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_tag_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_tag_widget)->background == snapshot->background);
    assert_optional_string_equal(snapshot->text, preview_tag_widget.text);
    assert_optional_string_equal(snapshot->secondary_text, preview_tag_widget.secondary_text);
    EGUI_TEST_ASSERT_TRUE(preview_tag_widget.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_tag_widget.secondary_font == snapshot->secondary_font);
    EGUI_TEST_ASSERT_TRUE(preview_tag_widget.icon_font == snapshot->icon_font);
    EGUI_TEST_ASSERT_TRUE(preview_tag_widget.on_dismiss == snapshot->on_dismiss);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_tag_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_tag_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_tag_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->secondary_color.full, preview_tag_widget.secondary_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_tag_widget.accent_color.full);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_tag_widget)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_tag_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->dismissible, preview_tag_widget.dismissible);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_tag_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_tag_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_tag_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_tag_widget)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_tag_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->dismiss_pressed, preview_tag_widget.dismiss_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_tag_widget)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_tag_widget)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_tag_widget)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_tag_widget)->padding.bottom);
}

static void test_tag_style_helpers_update_modes_and_palette(void)
{
    egui_view_tag_t *local;

    setup_tag();
    local = &test_tag_widget;

    EGUI_TEST_ASSERT_FALSE(local->compact_mode);
    EGUI_TEST_ASSERT_FALSE(local->read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, local->surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, local->accent_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_tag_apply_compact_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->compact_mode);
    EGUI_TEST_ASSERT_FALSE(local->read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0C7C73).full, local->accent_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_tag_apply_read_only_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->compact_mode);
    EGUI_TEST_ASSERT_TRUE(local->read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x9AA6B2).full, local->accent_color.full);
}

static void test_tag_setters_clear_pressed_state_and_update_content(void)
{
    egui_view_tag_t *local;

    setup_tag();
    local = &test_tag_widget;

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_tag_set_text(EGUI_VIEW_OF(local), "Needs review");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(local->text, "Needs review"));

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_tag_set_secondary_text(EGUI_VIEW_OF(local), "2 files");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(local->secondary_text, "2 files"));

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_tag_set_font(EGUI_VIEW_OF(local), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->font == (const egui_font_t *)&egui_res_font_montserrat_12_4);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_tag_set_secondary_font(EGUI_VIEW_OF(local), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->secondary_font == (const egui_font_t *)&egui_res_font_montserrat_10_4);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_tag_set_icon_font(EGUI_VIEW_OF(local), EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->icon_font == EGUI_FONT_ICON_MS_16);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_tag_set_dismissible(EGUI_VIEW_OF(local), 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(local->dismissible);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_tag_set_palette(EGUI_VIEW_OF(local), EGUI_COLOR_HEX(0xF8F9FA), EGUI_COLOR_HEX(0xCCD3DA), EGUI_COLOR_HEX(0x22303F), EGUI_COLOR_HEX(0x708091),
                              EGUI_COLOR_HEX(0x13579B));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x13579B).full, local->accent_color.full);
}

static void test_tag_dismiss_region_visibility_tracks_mode_changes(void)
{
    egui_region_t region;

    setup_tag();
    layout_tag(EGUI_VIEW_OF(&test_tag_widget), 12, 18, 128, 28);

    EGUI_TEST_ASSERT_TRUE(egui_view_tag_get_dismiss_region(EGUI_VIEW_OF(&test_tag_widget), &region));

    egui_view_tag_set_dismissible(EGUI_VIEW_OF(&test_tag_widget), 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_tag_get_dismiss_region(EGUI_VIEW_OF(&test_tag_widget), &region));

    egui_view_tag_set_dismissible(EGUI_VIEW_OF(&test_tag_widget), 1);
    egui_view_tag_set_read_only_mode(EGUI_VIEW_OF(&test_tag_widget), 1);
    EGUI_TEST_ASSERT_FALSE(egui_view_tag_get_dismiss_region(EGUI_VIEW_OF(&test_tag_widget), &region));
}

static void test_tag_touch_same_target_release_dismisses_once(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;
    egui_dim_t outside_x;

    setup_tag();
    layout_tag(EGUI_VIEW_OF(&test_tag_widget), 12, 18, 128, 28);
    get_dismiss_center(EGUI_VIEW_OF(&test_tag_widget), &inside_x, &inside_y);
    outside_x = inside_x + 24;

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tag_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tag_widget)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_MOTION_EVENT_ACTION_UP, outside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_dismiss_count);

    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_MOTION_EVENT_ACTION_MOVE, outside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tag_widget)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_MOTION_EVENT_ACTION_MOVE, inside_x, inside_y));
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&test_tag_widget)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_MOTION_EVENT_ACTION_UP, inside_x, inside_y));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tag_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_dismiss_count);
}

static void test_tag_keyboard_delete_and_enter_dismiss(void)
{
    setup_tag();

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_KEY_CODE_DELETE));
    EGUI_TEST_ASSERT_EQUAL_INT(1, g_dismiss_count);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tag_widget)->is_pressed);

    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_KEY_CODE_ENTER));
    EGUI_TEST_ASSERT_EQUAL_INT(2, g_dismiss_count);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tag_widget)->is_pressed);
}

static void test_tag_read_only_and_disabled_input_do_not_dismiss(void)
{
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_tag();
    layout_tag(EGUI_VIEW_OF(&test_tag_widget), 12, 18, 128, 28);
    get_dismiss_center(EGUI_VIEW_OF(&test_tag_widget), &inside_x, &inside_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_tag_widget), 1);
    egui_view_tag_set_read_only_mode(EGUI_VIEW_OF(&test_tag_widget), 1);
    EGUI_TEST_ASSERT_FALSE(send_key_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_KEY_CODE_DELETE));
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_dismiss_count);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tag_widget)->is_pressed);

    egui_view_tag_set_read_only_mode(EGUI_VIEW_OF(&test_tag_widget), 0);
    egui_view_set_enable(EGUI_VIEW_OF(&test_tag_widget), 0);
    EGUI_TEST_ASSERT_FALSE(send_key_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_KEY_CODE_DELETE));
    EGUI_TEST_ASSERT_FALSE(send_touch_to_view(EGUI_VIEW_OF(&test_tag_widget), EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_dismiss_count);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_tag_widget)->is_pressed);
}

static void test_tag_static_preview_consumes_input_and_keeps_state(void)
{
    tag_preview_snapshot_t initial_snapshot;
    egui_dim_t inside_x;
    egui_dim_t inside_y;

    setup_preview_tag();
    layout_tag(EGUI_VIEW_OF(&preview_tag_widget), 12, 18, 88, 24);
    get_dismiss_center(EGUI_VIEW_OF(&preview_tag_widget), &inside_x, &inside_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_tag_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_tag_widget), EGUI_MOTION_EVENT_ACTION_DOWN, inside_x, inside_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_dismiss_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_tag_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_preview_key_to_view(EGUI_VIEW_OF(&preview_tag_widget), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_dismiss_count);
}

void test_tag_run(void)
{
    EGUI_TEST_SUITE_BEGIN(tag);
    EGUI_TEST_RUN(test_tag_style_helpers_update_modes_and_palette);
    EGUI_TEST_RUN(test_tag_setters_clear_pressed_state_and_update_content);
    EGUI_TEST_RUN(test_tag_dismiss_region_visibility_tracks_mode_changes);
    EGUI_TEST_RUN(test_tag_touch_same_target_release_dismisses_once);
    EGUI_TEST_RUN(test_tag_keyboard_delete_and_enter_dismiss);
    EGUI_TEST_RUN(test_tag_read_only_and_disabled_input_do_not_dismiss);
    EGUI_TEST_RUN(test_tag_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
