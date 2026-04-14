#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_persona.h"

#include "../../HelloCustomWidgets/display/persona/egui_view_persona.h"
#include "../../HelloCustomWidgets/display/persona/egui_view_persona.c"

static egui_view_persona_t test_persona;
static egui_view_persona_t preview_persona;
static egui_view_api_t preview_api;

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

static void setup_persona(void)
{
    egui_view_persona_init(EGUI_VIEW_OF(&test_persona));
    egui_view_set_size(EGUI_VIEW_OF(&test_persona), 196, 104);
}

static void setup_preview_persona(void)
{
    egui_view_persona_init(EGUI_VIEW_OF(&preview_persona));
    egui_view_set_size(EGUI_VIEW_OF(&preview_persona), 104, 78);
    egui_view_persona_set_display_name(EGUI_VIEW_OF(&preview_persona), "Maya Yu");
    egui_view_persona_set_secondary_text(EGUI_VIEW_OF(&preview_persona), "Research lead");
    egui_view_persona_set_status(EGUI_VIEW_OF(&preview_persona), EGUI_VIEW_PERSONA_STATUS_AWAY);
    egui_view_persona_set_compact_mode(EGUI_VIEW_OF(&preview_persona), 1);
    egui_view_persona_override_static_preview_api(EGUI_VIEW_OF(&preview_persona), &preview_api);
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

static void test_persona_init_uses_defaults(void)
{
    setup_persona();

    EGUI_TEST_ASSERT_NULL(egui_view_persona_get_display_name(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_NULL(egui_view_persona_get_secondary_text(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_NULL(egui_view_persona_get_tertiary_text(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_NULL(egui_view_persona_get_quaternary_text(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_NULL(egui_view_persona_get_initials(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_STATUS_NONE, egui_view_persona_get_status(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_TONE_NEUTRAL, egui_view_persona_get_tone(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_persona_get_compact_mode(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_EQUAL_INT(0, egui_view_persona_get_read_only_mode(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_TRUE(test_persona.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(test_persona.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_TRUE(egui_view_persona_default_icon_font(20) == EGUI_FONT_ICON_MS_16);
    EGUI_TEST_ASSERT_TRUE(egui_view_persona_default_icon_font(36) == EGUI_FONT_ICON_MS_20);
    EGUI_TEST_ASSERT_TRUE(egui_view_persona_default_icon_font(52) == EGUI_FONT_ICON_MS_24);
}

static void test_persona_setters_and_resolvers_clear_pressed_state(void)
{
    egui_color_t surface = EGUI_COLOR_HEX(0x101112);
    egui_color_t border = EGUI_COLOR_HEX(0x202122);
    egui_color_t section = EGUI_COLOR_HEX(0x303132);
    egui_color_t text = EGUI_COLOR_HEX(0x404142);
    egui_color_t muted = EGUI_COLOR_HEX(0x505152);
    egui_color_t accent = EGUI_COLOR_HEX(0x606162);
    egui_color_t success = EGUI_COLOR_HEX(0x707172);
    egui_color_t warning = EGUI_COLOR_HEX(0x808182);
    egui_color_t neutral = EGUI_COLOR_HEX(0x909192);
    char resolved[3];

    setup_persona();

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_display_name(EGUI_VIEW_OF(&test_persona), "Lena Marsh");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_display_name(EGUI_VIEW_OF(&test_persona)), "Lena Marsh") == 0);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_secondary_text(EGUI_VIEW_OF(&test_persona), "Principal designer");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_secondary_text(EGUI_VIEW_OF(&test_persona)), "Principal designer") == 0);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_tertiary_text(EGUI_VIEW_OF(&test_persona), "Spacing audit");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_tertiary_text(EGUI_VIEW_OF(&test_persona)), "Spacing audit") == 0);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_quaternary_text(EGUI_VIEW_OF(&test_persona), "Available");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_quaternary_text(EGUI_VIEW_OF(&test_persona)), "Available") == 0);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_initials(EGUI_VIEW_OF(&test_persona), "ar");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_initials(EGUI_VIEW_OF(&test_persona)), "ar") == 0);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_persona_resolve_initials("ar", NULL, resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "AR") == 0);
    egui_view_persona_resolve_initials(NULL, "Lena Marsh", resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "LM") == 0);
    egui_view_persona_resolve_initials(NULL, "solo", resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "SO") == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_status(EGUI_VIEW_OF(&test_persona), EGUI_VIEW_PERSONA_STATUS_BUSY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_STATUS_BUSY, egui_view_persona_get_status(EGUI_VIEW_OF(&test_persona)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_persona_set_status(EGUI_VIEW_OF(&test_persona), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_STATUS_NONE, egui_view_persona_get_status(EGUI_VIEW_OF(&test_persona)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_tone(EGUI_VIEW_OF(&test_persona), EGUI_VIEW_PERSONA_TONE_SUCCESS);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_TONE_SUCCESS, egui_view_persona_get_tone(EGUI_VIEW_OF(&test_persona)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_persona_set_tone(EGUI_VIEW_OF(&test_persona), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_TONE_NEUTRAL, egui_view_persona_get_tone(EGUI_VIEW_OF(&test_persona)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_font(EGUI_VIEW_OF(&test_persona), NULL);
    EGUI_TEST_ASSERT_TRUE(test_persona.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_meta_font(EGUI_VIEW_OF(&test_persona), NULL);
    EGUI_TEST_ASSERT_TRUE(test_persona.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_palette(EGUI_VIEW_OF(&test_persona), surface, border, section, text, muted, accent, success, warning, neutral);
    EGUI_TEST_ASSERT_EQUAL_INT(surface.full, test_persona.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(border.full, test_persona.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(section.full, test_persona.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(text.full, test_persona.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(muted.full, test_persona.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(accent.full, test_persona.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(success.full, test_persona.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(warning.full, test_persona.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(neutral.full, test_persona.neutral_color.full);
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_compact_mode(EGUI_VIEW_OF(&test_persona), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_persona_get_compact_mode(EGUI_VIEW_OF(&test_persona)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_read_only_mode(EGUI_VIEW_OF(&test_persona), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_persona_get_read_only_mode(EGUI_VIEW_OF(&test_persona)));
    assert_pressed_cleared(EGUI_VIEW_OF(&test_persona));
}

static void test_persona_helpers_compute_regions_and_colors(void)
{
    egui_region_t avatar_region;
    egui_region_t presence_region;
    egui_color_t sample = EGUI_COLOR_HEX(0x123456);

    setup_persona();
    egui_view_persona_set_display_name(EGUI_VIEW_OF(&test_persona), "Lena Marsh");
    egui_view_persona_set_secondary_text(EGUI_VIEW_OF(&test_persona), "Principal designer");
    layout_view(EGUI_VIEW_OF(&test_persona), 10, 20, 196, 104);

    EGUI_TEST_ASSERT_TRUE(egui_view_persona_get_avatar_region(EGUI_VIEW_OF(&test_persona), &avatar_region));
    EGUI_TEST_ASSERT_TRUE(avatar_region.size.width > 0);
    EGUI_TEST_ASSERT_FALSE(egui_view_persona_should_draw_presence(&test_persona));
    EGUI_TEST_ASSERT_FALSE(egui_view_persona_get_presence_region(EGUI_VIEW_OF(&test_persona), &presence_region));
    EGUI_TEST_ASSERT_EQUAL_INT(test_persona.accent_color.full, egui_view_persona_tone_color(&test_persona, EGUI_VIEW_PERSONA_TONE_ACCENT).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_persona.success_color.full, egui_view_persona_tone_color(&test_persona, EGUI_VIEW_PERSONA_TONE_SUCCESS).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_persona.warning_color.full, egui_view_persona_tone_color(&test_persona, EGUI_VIEW_PERSONA_TONE_WARNING).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_persona.neutral_color.full, egui_view_persona_tone_color(&test_persona, EGUI_VIEW_PERSONA_TONE_NEUTRAL).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0x107C41).full, egui_view_persona_status_color(&test_persona, EGUI_VIEW_PERSONA_STATUS_AVAILABLE).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC4314B).full, egui_view_persona_status_color(&test_persona, EGUI_VIEW_PERSONA_STATUS_BUSY).full);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_COLOR_HEX(0xC17C00).full, egui_view_persona_status_color(&test_persona, EGUI_VIEW_PERSONA_STATUS_AWAY).full);
    EGUI_TEST_ASSERT_EQUAL_INT(test_persona.neutral_color.full, egui_view_persona_status_color(&test_persona, EGUI_VIEW_PERSONA_STATUS_OFFLINE).full);
    EGUI_TEST_ASSERT_EQUAL_INT(egui_rgb_mix(sample, EGUI_COLOR_HEX(0x83909D), 54).full, egui_view_persona_mix_disabled(sample).full);

    egui_view_persona_set_status(EGUI_VIEW_OF(&test_persona), EGUI_VIEW_PERSONA_STATUS_DO_NOT_DISTURB);
    EGUI_TEST_ASSERT_TRUE(egui_view_persona_should_draw_presence(&test_persona));
    EGUI_TEST_ASSERT_TRUE(egui_view_persona_get_presence_region(EGUI_VIEW_OF(&test_persona), &presence_region));
    EGUI_TEST_ASSERT_TRUE(presence_region.size.width > 0);

    egui_view_persona_set_compact_mode(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_set_size(EGUI_VIEW_OF(&test_persona), 104, 78);
    layout_view(EGUI_VIEW_OF(&test_persona), 4, 6, 104, 78);
    EGUI_TEST_ASSERT_TRUE(egui_view_persona_get_avatar_region(EGUI_VIEW_OF(&test_persona), &avatar_region));
    EGUI_TEST_ASSERT_TRUE(avatar_region.size.width <= 32);
}

static void test_persona_static_preview_consumes_input_and_clears_pressed_state(void)
{
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_persona();
    layout_view(EGUI_VIEW_OF(&preview_persona), 12, 18, 104, 78);
    get_view_center(EGUI_VIEW_OF(&preview_persona), &center_x, &center_y);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_persona), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch(EGUI_VIEW_OF(&preview_persona), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_persona));
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_persona_get_compact_mode(EGUI_VIEW_OF(&preview_persona)));
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_STATUS_AWAY, egui_view_persona_get_status(EGUI_VIEW_OF(&preview_persona)));

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_persona), 1);
    EGUI_TEST_ASSERT_TRUE(send_key(EGUI_VIEW_OF(&preview_persona), EGUI_KEY_CODE_ENTER));
    assert_pressed_cleared(EGUI_VIEW_OF(&preview_persona));
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_display_name(EGUI_VIEW_OF(&preview_persona)), "Maya Yu") == 0);
}

void test_persona_run(void)
{
    EGUI_TEST_SUITE_BEGIN(persona);
    EGUI_TEST_RUN(test_persona_init_uses_defaults);
    EGUI_TEST_RUN(test_persona_setters_and_resolvers_clear_pressed_state);
    EGUI_TEST_RUN(test_persona_helpers_compute_regions_and_colors);
    EGUI_TEST_RUN(test_persona_static_preview_consumes_input_and_clears_pressed_state);
    EGUI_TEST_SUITE_END();
}
