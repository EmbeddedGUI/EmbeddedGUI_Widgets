#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_badge.h"

#include "../../HelloCustomWidgets/display/badge/egui_view_badge.h"
#include "../../HelloCustomWidgets/display/badge/egui_view_badge.c"
#include "../../../sdk/EmbeddedGUI/src/resource/egui_icon_material_symbols.h"

typedef struct badge_preview_snapshot badge_preview_snapshot_t;
struct badge_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const egui_font_t *font;
    const egui_font_t *icon_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t text_color;
    egui_color_t accent_color;
    char text[EGUI_VIEW_BADGE_MAX_TEXT_LEN + 1];
    const char *icon;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t outline_mode;
    uint8_t subtle_mode;
    egui_alpha_t alpha;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_badge_t test_badge_widget;
static egui_view_badge_t preview_badge_widget;
static egui_view_api_t preview_api;

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

static void setup_badge(void)
{
    egui_view_badge_init(EGUI_VIEW_OF(&test_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&test_badge_widget), 128, 28);
    egui_view_badge_apply_filled_style(EGUI_VIEW_OF(&test_badge_widget));
    egui_view_badge_set_text(EGUI_VIEW_OF(&test_badge_widget), "Verified");
    egui_view_badge_set_icon(EGUI_VIEW_OF(&test_badge_widget), EGUI_ICON_MS_DONE);
    egui_view_badge_set_font(EGUI_VIEW_OF(&test_badge_widget), (const egui_font_t *)&egui_res_font_montserrat_10_4);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(&test_badge_widget), EGUI_FONT_ICON_MS_16);
}

static void setup_preview_badge(void)
{
    egui_view_badge_init(EGUI_VIEW_OF(&preview_badge_widget));
    egui_view_set_size(EGUI_VIEW_OF(&preview_badge_widget), 88, 24);
    egui_view_badge_apply_outline_style(EGUI_VIEW_OF(&preview_badge_widget));
    egui_view_badge_set_text(EGUI_VIEW_OF(&preview_badge_widget), "Beta");
    egui_view_badge_set_icon(EGUI_VIEW_OF(&preview_badge_widget), EGUI_ICON_MS_INFO);
    egui_view_badge_set_font(EGUI_VIEW_OF(&preview_badge_widget), (const egui_font_t *)&egui_res_font_montserrat_8_4);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(&preview_badge_widget), EGUI_FONT_ICON_MS_16);
    egui_view_badge_set_compact_mode(EGUI_VIEW_OF(&preview_badge_widget), 1);
    egui_view_badge_override_static_preview_api(EGUI_VIEW_OF(&preview_badge_widget), &preview_api);
}

static void layout_badge(egui_view_t *view, egui_dim_t x, egui_dim_t y, egui_dim_t width, egui_dim_t height)
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

static void capture_preview_snapshot(badge_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_badge_widget)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_badge_widget)->background;
    snapshot->font = preview_badge_widget.font;
    snapshot->icon_font = preview_badge_widget.icon_font;
    snapshot->surface_color = preview_badge_widget.surface_color;
    snapshot->border_color = preview_badge_widget.border_color;
    snapshot->text_color = preview_badge_widget.text_color;
    snapshot->accent_color = preview_badge_widget.accent_color;
    strcpy(snapshot->text, preview_badge_widget.text);
    snapshot->icon = preview_badge_widget.icon;
    snapshot->compact_mode = preview_badge_widget.compact_mode;
    snapshot->read_only_mode = preview_badge_widget.read_only_mode;
    snapshot->outline_mode = preview_badge_widget.outline_mode;
    snapshot->subtle_mode = preview_badge_widget.subtle_mode;
    snapshot->alpha = EGUI_VIEW_OF(&preview_badge_widget)->alpha;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_badge_widget));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_badge_widget)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_badge_widget)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_badge_widget)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_badge_widget)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_badge_widget)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_badge_widget)->padding.bottom;
}

static void assert_preview_state_unchanged(const badge_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_badge_widget)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_badge_widget)->background == snapshot->background);
    EGUI_TEST_ASSERT_TRUE(preview_badge_widget.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_badge_widget.icon_font == snapshot->icon_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_badge_widget.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_badge_widget.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_badge_widget.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_badge_widget.accent_color.full);
    EGUI_TEST_ASSERT_TRUE(strcmp(snapshot->text, preview_badge_widget.text) == 0);
    assert_optional_string_equal(snapshot->icon, preview_badge_widget.icon);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_badge_widget.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_badge_widget.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->outline_mode, preview_badge_widget.outline_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->subtle_mode, preview_badge_widget.subtle_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_badge_widget)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_badge_widget)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_badge_widget)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_badge_widget)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_badge_widget)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_badge_widget)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_badge_widget)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_badge_widget)->padding.bottom);
}

static void test_badge_style_helpers_update_modes_and_palette(void)
{
    egui_view_badge_t *local;

    setup_badge();
    local = &test_badge_widget;

    EGUI_TEST_ASSERT_FALSE(local->compact_mode);
    EGUI_TEST_ASSERT_FALSE(local->read_only_mode);
    EGUI_TEST_ASSERT_FALSE(local->outline_mode);
    EGUI_TEST_ASSERT_FALSE(local->subtle_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, local->surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_WHITE.full, local->text_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_apply_outline_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->outline_mode);
    EGUI_TEST_ASSERT_FALSE(local->subtle_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xFFFFFF).full, local->surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x0F6CBD).full, local->accent_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_apply_subtle_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_FALSE(local->outline_mode);
    EGUI_TEST_ASSERT_TRUE(local->subtle_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF3F7FB).full, local->surface_color.full);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_apply_read_only_style(EGUI_VIEW_OF(local));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->compact_mode);
    EGUI_TEST_ASSERT_TRUE(local->read_only_mode);
    EGUI_TEST_ASSERT_TRUE(local->subtle_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xF5F7FA).full, local->surface_color.full);
}

static void test_badge_setters_clear_pressed_state_and_update_content(void)
{
    egui_view_badge_t *local;

    setup_badge();
    local = &test_badge_widget;

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_text(EGUI_VIEW_OF(local), "Needs review");
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(local->text, "Needs review"));

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_icon(EGUI_VIEW_OF(local), EGUI_ICON_MS_WARNING);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(0, strcmp(local->icon, EGUI_ICON_MS_WARNING));

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_font(EGUI_VIEW_OF(local), (const egui_font_t *)&egui_res_font_montserrat_12_4);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->font == (const egui_font_t *)&egui_res_font_montserrat_12_4);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_icon_font(EGUI_VIEW_OF(local), EGUI_FONT_ICON_MS_20);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->icon_font == EGUI_FONT_ICON_MS_20);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_palette(EGUI_VIEW_OF(local), EGUI_COLOR_HEX(0xF8F9FA), EGUI_COLOR_HEX(0xCCD3DA), EGUI_COLOR_HEX(0x22303F),
                                EGUI_COLOR_HEX(0x13579B));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x13579B).full, local->accent_color.full);
}

static void test_badge_icon_region_visibility_tracks_icon_and_compact_mode(void)
{
    egui_region_t icon_region;
    egui_dim_t standard_width;
    egui_dim_t compact_width;

    setup_badge();
    layout_badge(EGUI_VIEW_OF(&test_badge_widget), 12, 18, 128, 28);

    EGUI_TEST_ASSERT_TRUE(egui_view_badge_get_icon_region(EGUI_VIEW_OF(&test_badge_widget), &icon_region));
    standard_width = icon_region.size.width;

    egui_view_badge_set_compact_mode(EGUI_VIEW_OF(&test_badge_widget), 1);
    layout_badge(EGUI_VIEW_OF(&test_badge_widget), 12, 18, 96, 24);
    EGUI_TEST_ASSERT_TRUE(egui_view_badge_get_icon_region(EGUI_VIEW_OF(&test_badge_widget), &icon_region));
    compact_width = icon_region.size.width;
    EGUI_TEST_ASSERT_TRUE(compact_width < standard_width);

    egui_view_badge_set_icon(EGUI_VIEW_OF(&test_badge_widget), NULL);
    EGUI_TEST_ASSERT_FALSE(egui_view_badge_get_icon_region(EGUI_VIEW_OF(&test_badge_widget), &icon_region));
}

static void test_badge_mode_setters_clear_pressed_state(void)
{
    egui_view_badge_t *local;

    setup_badge();
    local = &test_badge_widget;

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_compact_mode(EGUI_VIEW_OF(local), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->compact_mode);

    egui_view_set_pressed(EGUI_VIEW_OF(local), 1);
    egui_view_badge_set_read_only_mode(EGUI_VIEW_OF(local), 1);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(local)->is_pressed);
    EGUI_TEST_ASSERT_TRUE(local->read_only_mode);
}

static void test_badge_static_preview_consumes_input_and_keeps_state(void)
{
    badge_preview_snapshot_t initial_snapshot;
    egui_region_t badge_region;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_badge();
    layout_badge(EGUI_VIEW_OF(&preview_badge_widget), 12, 18, 88, 24);
    badge_region = EGUI_VIEW_OF(&preview_badge_widget)->region_screen;
    center_x = badge_region.location.x + badge_region.size.width / 2;
    center_y = badge_region.location.y + badge_region.size.height / 2;
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_badge_widget), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_badge_widget), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_badge_widget), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
}

void test_badge_run(void)
{
    EGUI_TEST_SUITE_BEGIN(badge);
    EGUI_TEST_RUN(test_badge_style_helpers_update_modes_and_palette);
    EGUI_TEST_RUN(test_badge_setters_clear_pressed_state_and_update_content);
    EGUI_TEST_RUN(test_badge_icon_region_visibility_tracks_icon_and_compact_mode);
    EGUI_TEST_RUN(test_badge_mode_setters_clear_pressed_state);
    EGUI_TEST_RUN(test_badge_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
