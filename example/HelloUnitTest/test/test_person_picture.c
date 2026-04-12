#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_person_picture.h"

#include "../../HelloCustomWidgets/display/person_picture/egui_view_person_picture.h"
#include "../../HelloCustomWidgets/display/person_picture/egui_view_person_picture.c"

static egui_view_person_picture_t test_picture;
static egui_view_person_picture_t preview_picture;
static egui_view_api_t preview_api;
static const egui_image_t g_fake_image = {0};

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

static void setup_person_picture(void)
{
    egui_view_person_picture_init(EGUI_VIEW_OF(&test_picture));
    egui_view_set_size(EGUI_VIEW_OF(&test_picture), 56, 56);
}

static void setup_preview_picture(void)
{
    egui_view_person_picture_init(EGUI_VIEW_OF(&preview_picture));
    egui_view_set_size(EGUI_VIEW_OF(&preview_picture), 36, 36);
    egui_view_person_picture_set_display_name(EGUI_VIEW_OF(&preview_picture), "Maya Yu");
    egui_view_person_picture_set_presence(EGUI_VIEW_OF(&preview_picture), EGUI_VIEW_PERSON_PICTURE_PRESENCE_AWAY);
    egui_view_person_picture_set_compact_mode(EGUI_VIEW_OF(&preview_picture), 1);
    egui_view_person_picture_override_static_preview_api(EGUI_VIEW_OF(&preview_picture), &preview_api);
}

static int send_touch(egui_view_t *view, uint8_t type, egui_dim_t x, egui_dim_t y)
{
    egui_motion_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.location.x = x;
    event.location.y = y;
    return view->api->on_touch_event(view, &event);
}

static int send_key(egui_view_t *view, uint8_t key_code)
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

static void assert_pressed_cleared(egui_view_t *view)
{
    EGUI_TEST_ASSERT_FALSE(view->is_pressed);
}

static void test_person_picture_init_uses_defaults(void)
{
    setup_person_picture();

    EGUI_TEST_ASSERT_NULL(egui_view_person_picture_get_display_name(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_NULL(egui_view_person_picture_get_initials(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_fallback_glyph(EGUI_VIEW_OF(&test_picture)), EGUI_ICON_MS_PERSON) == 0);
    EGUI_TEST_ASSERT_NULL(egui_view_person_picture_get_image(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL, egui_view_person_picture_get_tone(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_PRESENCE_NONE, egui_view_person_picture_get_presence(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_person_picture_get_compact_mode(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_person_picture_get_read_only_mode(EGUI_VIEW_OF(&test_picture)));
    EGUI_TEST_ASSERT_TRUE(test_picture.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_default_icon_font(20) == EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_default_icon_font(32) == EGUI_FONT_ICON_MS_20);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_default_icon_font(52) == EGUI_FONT_ICON_MS_24);
}

static void test_person_picture_setters_and_resolvers_clear_pressed_state(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t border = EGUI_COLOR_HEX(0x202122);
    egui_color_t foreground = EGUI_COLOR_HEX(0x303132);
    egui_color_t accent = EGUI_COLOR_HEX(0x404142);
    egui_color_t success = EGUI_COLOR_HEX(0x505152);
    egui_color_t warning = EGUI_COLOR_HEX(0x606162);
    egui_color_t neutral = EGUI_COLOR_HEX(0x707172);
    egui_color_t muted = EGUI_COLOR_HEX(0x808182);
    char resolved[3];

    setup_person_picture();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_display_name(EGUI_VIEW_OF(&test_picture), "Lena Marsh");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_display_name(EGUI_VIEW_OF(&test_picture)), "Lena Marsh") == 0);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_initials(EGUI_VIEW_OF(&test_picture), "ar");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_initials(EGUI_VIEW_OF(&test_picture)), "ar") == 0);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_person_picture_resolve_initials("ar", NULL, resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "AR") == 0);
    egui_view_person_picture_resolve_initials(NULL, "Lena Marsh", resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "LM") == 0);
    egui_view_person_picture_resolve_initials(NULL, "solo", resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "SO") == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_fallback_glyph(EGUI_VIEW_OF(&test_picture), EGUI_ICON_MS_SETTINGS);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_fallback_glyph(EGUI_VIEW_OF(&test_picture)), EGUI_ICON_MS_SETTINGS) == 0);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_fallback_glyph(EGUI_VIEW_OF(&test_picture), NULL);
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_fallback_glyph(EGUI_VIEW_OF(&test_picture)), EGUI_ICON_MS_PERSON) == 0);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_image(EGUI_VIEW_OF(&test_picture), &g_fake_image);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_get_image(EGUI_VIEW_OF(&test_picture)) == &g_fake_image);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_tone(EGUI_VIEW_OF(&test_picture), EGUI_VIEW_PERSON_PICTURE_TONE_SUCCESS);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_TONE_SUCCESS, egui_view_person_picture_get_tone(EGUI_VIEW_OF(&test_picture)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_person_picture_set_tone(EGUI_VIEW_OF(&test_picture), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL, egui_view_person_picture_get_tone(EGUI_VIEW_OF(&test_picture)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_presence(EGUI_VIEW_OF(&test_picture), EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY, egui_view_person_picture_get_presence(EGUI_VIEW_OF(&test_picture)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_person_picture_set_presence(EGUI_VIEW_OF(&test_picture), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_PRESENCE_NONE, egui_view_person_picture_get_presence(EGUI_VIEW_OF(&test_picture)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_font(EGUI_VIEW_OF(&test_picture), NULL);
    EGUI_TEST_ASSERT_TRUE(test_picture.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_person_picture_set_icon_font(EGUI_VIEW_OF(&test_picture), EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_resolve_icon_font(&test_picture, 56) == EGUI_FONT_ICON_MS_16);
    egui_view_person_picture_set_icon_font(EGUI_VIEW_OF(&test_picture), NULL);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_resolve_icon_font(&test_picture, 56) == EGUI_FONT_ICON_MS_24);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_palette(EGUI_VIEW_OF(&test_picture), surface, border, foreground, accent, success, warning, neutral, muted);
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_picture.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_picture.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(foreground.full, test_picture.foreground_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_picture.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(success.full, test_picture.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(warning.full, test_picture.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(neutral.full, test_picture.neutral_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_picture.muted_color.full);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_compact_mode(EGUI_VIEW_OF(&test_picture), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_person_picture_get_compact_mode(EGUI_VIEW_OF(&test_picture)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_person_picture_set_read_only_mode(EGUI_VIEW_OF(&test_picture), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_person_picture_get_read_only_mode(EGUI_VIEW_OF(&test_picture)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_picture));
}

static void test_person_picture_helpers_compute_regions_and_colors(void)
{
    egui_region_t avatar_region;
    egui_region_t presence_region;
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_person_picture();
    layout_view(EGUI_VIEW_OF(&test_picture), 10, 20, 56, 56);
    egui_view_person_picture_get_avatar_region(&test_picture, EGUI_VIEW_OF(&test_picture), &avatar_region);

    EGUI_TEST_ASSERT_TRUE(avatar_region.size.width > 0);
    EGUI_TEST_ASSERT_EQUAL_INT(54, avatar_region.size.width);
    EGUI_TEST_ASSERT_EQUAL_INT(1, avatar_region.location.x);
    EGUI_TEST_ASSERT_EQUAL_INT(1, avatar_region.location.y);
    EGUI_TEST_ASSERT_FALSE(egui_view_person_picture_should_draw_presence(&test_picture));
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.accent_color.full,
                               egui_view_person_picture_tone_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.success_color.full,
                               egui_view_person_picture_tone_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.warning_color.full,
                               egui_view_person_picture_tone_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.neutral_color.full,
                               egui_view_person_picture_tone_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.success_color.full,
                               egui_view_person_picture_presence_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_PRESENCE_LIVE).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.warning_color.full,
                               egui_view_person_picture_presence_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(test_picture.warning_color, test_picture.accent_color, 24).full,
                               egui_view_person_picture_presence_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_PRESENCE_AWAY).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_picture.neutral_color.full,
                               egui_view_person_picture_presence_color(&test_picture, EGUI_VIEW_PERSON_PICTURE_PRESENCE_OFFLINE).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_DARK_GREY, 66).full, egui_view_person_picture_mix_disabled(sample).full);

    egui_view_person_picture_set_presence(EGUI_VIEW_OF(&test_picture), EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY);
    EGUI_TEST_ASSERT_TRUE(egui_view_person_picture_should_draw_presence(&test_picture));
    egui_view_person_picture_get_presence_region(&test_picture, &avatar_region, &presence_region);
    EGUI_TEST_ASSERT_TRUE(presence_region.size.width > 0);
    EGUI_TEST_ASSERT_TRUE(presence_region.location.x > avatar_region.location.x);

    egui_view_person_picture_set_compact_mode(EGUI_VIEW_OF(&test_picture), 1);
    egui_view_set_size(EGUI_VIEW_OF(&test_picture), 36, 36);
    layout_view(EGUI_VIEW_OF(&test_picture), 4, 6, 36, 36);
    egui_view_person_picture_get_avatar_region(&test_picture, EGUI_VIEW_OF(&test_picture), &avatar_region);
    EGUI_TEST_ASSERT_EQUAL_INT(36, avatar_region.size.width);
    egui_view_person_picture_get_presence_region(&test_picture, &avatar_region, &presence_region);
    EGUI_TEST_ASSERT_TRUE(presence_region.size.width <= 12);
}

static void test_person_picture_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_picture();
    layout_view(EGUI_VIEW_OF(&preview_picture), 12, 18, 36, 36);
    get_view_center(EGUI_VIEW_OF(&preview_picture), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_picture), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_picture), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_picture));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_person_picture_get_compact_mode(EGUI_VIEW_OF(&preview_picture)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSON_PICTURE_PRESENCE_AWAY, egui_view_person_picture_get_presence(EGUI_VIEW_OF(&preview_picture)));

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_picture), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_picture), EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_picture));
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_person_picture_get_display_name(EGUI_VIEW_OF(&preview_picture)), "Maya Yu") == 0);
}

void test_person_picture_run(void)
{
    EGUI_TEST_SUITE_BEGIN(person_picture);
    EGUI_TEST_RUN(test_person_picture_init_uses_defaults);
    EGUI_TEST_RUN(test_person_picture_setters_and_resolvers_clear_pressed_state);
    EGUI_TEST_RUN(test_person_picture_helpers_compute_regions_and_colors);
    EGUI_TEST_RUN(test_person_picture_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
