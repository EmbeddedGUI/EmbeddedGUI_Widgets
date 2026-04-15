#include <string.h>

#include "egui.h"
#include "test/egui_test.h"
#include "test_persona.h"

#include "../../HelloCustomWidgets/display/persona/egui_view_persona.h"
#include "../../HelloCustomWidgets/display/persona/egui_view_persona.c"

typedef struct persona_preview_snapshot persona_preview_snapshot_t;
struct persona_preview_snapshot
{
    egui_region_t region_screen;
    egui_background_t *background;
    const char *display_name;
    const char *secondary_text;
    const char *tertiary_text;
    const char *quaternary_text;
    const char *initials;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t section_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    egui_view_on_click_listener_t on_click_listener;
    const egui_view_api_t *api;
    egui_alpha_t alpha;
    uint8_t tone;
    uint8_t status;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t enable;
    uint8_t is_focused;
    uint8_t is_pressed;
    egui_dim_t padding_left;
    egui_dim_t padding_right;
    egui_dim_t padding_top;
    egui_dim_t padding_bottom;
};

static egui_view_persona_t test_persona;
static egui_view_persona_t preview_persona;
static egui_view_api_t preview_api;
static uint8_t g_click_count;

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
    g_click_count++;
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

static void setup_persona(void)
{
    egui_view_persona_init(EGUI_VIEW_OF(&test_persona));
    egui_view_set_size(EGUI_VIEW_OF(&test_persona), 196, 104);
    g_click_count = 0;
}

static void setup_preview_persona(void)
{
    egui_view_persona_init(EGUI_VIEW_OF(&preview_persona));
    egui_view_set_size(EGUI_VIEW_OF(&preview_persona), 104, 78);
    egui_view_set_on_click_listener(EGUI_VIEW_OF(&preview_persona), on_preview_click);
    egui_view_persona_set_display_name(EGUI_VIEW_OF(&preview_persona), "Maya Yu");
    egui_view_persona_set_secondary_text(EGUI_VIEW_OF(&preview_persona), "Research lead");
    egui_view_persona_set_status(EGUI_VIEW_OF(&preview_persona), EGUI_VIEW_PERSONA_STATUS_AWAY);
    egui_view_persona_set_tone(EGUI_VIEW_OF(&preview_persona), EGUI_VIEW_PERSONA_TONE_WARNING);
    egui_view_persona_set_compact_mode(EGUI_VIEW_OF(&preview_persona), 1);
    egui_view_persona_override_static_preview_api(EGUI_VIEW_OF(&preview_persona), &preview_api);
    g_click_count = 0;
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

static int dispatch_key_event_to_view(egui_view_t *view, uint8_t type, uint8_t key_code)
{
    egui_key_event_t event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.key_code = key_code;
    return view->api->on_key_event(view, &event);
}

static int send_key_to_view(egui_view_t *view, uint8_t key_code)
{
    int handled = 0;

    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_DOWN, key_code);
    handled |= dispatch_key_event_to_view(view, EGUI_KEY_EVENT_ACTION_UP, key_code);
    return handled;
}

static void get_view_center(egui_view_t *view, egui_dim_t *x, egui_dim_t *y)
{
    *x = view->region_screen.location.x + view->region_screen.size.width / 2;
    *y = view->region_screen.location.y + view->region_screen.size.height / 2;
}

static void capture_preview_snapshot(persona_preview_snapshot_t *snapshot)
{
    snapshot->region_screen = EGUI_VIEW_OF(&preview_persona)->region_screen;
    snapshot->background = EGUI_VIEW_OF(&preview_persona)->background;
    snapshot->display_name = preview_persona.display_name;
    snapshot->secondary_text = preview_persona.secondary_text;
    snapshot->tertiary_text = preview_persona.tertiary_text;
    snapshot->quaternary_text = preview_persona.quaternary_text;
    snapshot->initials = preview_persona.initials;
    snapshot->font = preview_persona.font;
    snapshot->meta_font = preview_persona.meta_font;
    snapshot->surface_color = preview_persona.surface_color;
    snapshot->border_color = preview_persona.border_color;
    snapshot->section_color = preview_persona.section_color;
    snapshot->text_color = preview_persona.text_color;
    snapshot->muted_text_color = preview_persona.muted_text_color;
    snapshot->accent_color = preview_persona.accent_color;
    snapshot->success_color = preview_persona.success_color;
    snapshot->warning_color = preview_persona.warning_color;
    snapshot->neutral_color = preview_persona.neutral_color;
    snapshot->on_click_listener = EGUI_VIEW_OF(&preview_persona)->on_click_listener;
    snapshot->api = EGUI_VIEW_OF(&preview_persona)->api;
    snapshot->alpha = EGUI_VIEW_OF(&preview_persona)->alpha;
    snapshot->tone = preview_persona.tone;
    snapshot->status = preview_persona.status;
    snapshot->compact_mode = preview_persona.compact_mode;
    snapshot->read_only_mode = preview_persona.read_only_mode;
    snapshot->enable = (uint8_t)egui_view_get_enable(EGUI_VIEW_OF(&preview_persona));
    snapshot->is_focused = EGUI_VIEW_OF(&preview_persona)->is_focused;
    snapshot->is_pressed = EGUI_VIEW_OF(&preview_persona)->is_pressed;
    snapshot->padding_left = EGUI_VIEW_OF(&preview_persona)->padding.left;
    snapshot->padding_right = EGUI_VIEW_OF(&preview_persona)->padding.right;
    snapshot->padding_top = EGUI_VIEW_OF(&preview_persona)->padding.top;
    snapshot->padding_bottom = EGUI_VIEW_OF(&preview_persona)->padding.bottom;
}

static void assert_preview_state_unchanged(const persona_preview_snapshot_t *snapshot)
{
    assert_region_equal(&snapshot->region_screen, &EGUI_VIEW_OF(&preview_persona)->region_screen);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_persona)->background == snapshot->background);
    assert_optional_string_equal(snapshot->display_name, preview_persona.display_name);
    assert_optional_string_equal(snapshot->secondary_text, preview_persona.secondary_text);
    assert_optional_string_equal(snapshot->tertiary_text, preview_persona.tertiary_text);
    assert_optional_string_equal(snapshot->quaternary_text, preview_persona.quaternary_text);
    assert_optional_string_equal(snapshot->initials, preview_persona.initials);
    EGUI_TEST_ASSERT_TRUE(preview_persona.font == snapshot->font);
    EGUI_TEST_ASSERT_TRUE(preview_persona.meta_font == snapshot->meta_font);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->surface_color.full, preview_persona.surface_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->border_color.full, preview_persona.border_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->section_color.full, preview_persona.section_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->text_color.full, preview_persona.text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->muted_text_color.full, preview_persona.muted_text_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->accent_color.full, preview_persona.accent_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->success_color.full, preview_persona.success_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->warning_color.full, preview_persona.warning_color.full);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->neutral_color.full, preview_persona.neutral_color.full);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_persona)->on_click_listener == snapshot->on_click_listener);
    EGUI_TEST_ASSERT_TRUE(EGUI_VIEW_OF(&preview_persona)->api == snapshot->api);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->alpha, EGUI_VIEW_OF(&preview_persona)->alpha);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->tone, preview_persona.tone);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->status, preview_persona.status);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->compact_mode, preview_persona.compact_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->read_only_mode, preview_persona.read_only_mode);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->enable, egui_view_get_enable(EGUI_VIEW_OF(&preview_persona)));
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_focused, EGUI_VIEW_OF(&preview_persona)->is_focused);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->is_pressed, EGUI_VIEW_OF(&preview_persona)->is_pressed);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_left, EGUI_VIEW_OF(&preview_persona)->padding.left);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_right, EGUI_VIEW_OF(&preview_persona)->padding.right);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_top, EGUI_VIEW_OF(&preview_persona)->padding.top);
    EGUI_TEST_ASSERT_EQUAL_INT(snapshot->padding_bottom, EGUI_VIEW_OF(&preview_persona)->padding.bottom);
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
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_secondary_text(EGUI_VIEW_OF(&test_persona), "Principal designer");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_secondary_text(EGUI_VIEW_OF(&test_persona)), "Principal designer") == 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_tertiary_text(EGUI_VIEW_OF(&test_persona), "Spacing audit");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_tertiary_text(EGUI_VIEW_OF(&test_persona)), "Spacing audit") == 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_quaternary_text(EGUI_VIEW_OF(&test_persona), "Available");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_quaternary_text(EGUI_VIEW_OF(&test_persona)), "Available") == 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_initials(EGUI_VIEW_OF(&test_persona), "ar");
    EGUI_TEST_ASSERT_TRUE(strcmp(egui_view_persona_get_initials(EGUI_VIEW_OF(&test_persona)), "ar") == 0);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_persona_resolve_initials("ar", NULL, resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "AR") == 0);
    egui_view_persona_resolve_initials(NULL, "Lena Marsh", resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "LM") == 0);
    egui_view_persona_resolve_initials(NULL, "solo", resolved);
    EGUI_TEST_ASSERT_TRUE(strcmp(resolved, "SO") == 0);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_status(EGUI_VIEW_OF(&test_persona), EGUI_VIEW_PERSONA_STATUS_BUSY);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_STATUS_BUSY, egui_view_persona_get_status(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_persona_set_status(EGUI_VIEW_OF(&test_persona), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_STATUS_NONE, egui_view_persona_get_status(EGUI_VIEW_OF(&test_persona)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_tone(EGUI_VIEW_OF(&test_persona), EGUI_VIEW_PERSONA_TONE_SUCCESS);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_TONE_SUCCESS, egui_view_persona_get_tone(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_persona_set_tone(EGUI_VIEW_OF(&test_persona), 9);
    EGUI_TEST_ASSERT_EQUAL_INT(EGUI_VIEW_PERSONA_TONE_NEUTRAL, egui_view_persona_get_tone(EGUI_VIEW_OF(&test_persona)));

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_font(EGUI_VIEW_OF(&test_persona), NULL);
    EGUI_TEST_ASSERT_TRUE(test_persona.font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_meta_font(EGUI_VIEW_OF(&test_persona), NULL);
    EGUI_TEST_ASSERT_TRUE(test_persona.meta_font == (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT);
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

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
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_compact_mode(EGUI_VIEW_OF(&test_persona), 2);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_persona_get_compact_mode(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);

    egui_view_set_pressed(EGUI_VIEW_OF(&test_persona), 1);
    egui_view_persona_set_read_only_mode(EGUI_VIEW_OF(&test_persona), 3);
    EGUI_TEST_ASSERT_EQUAL_INT(1, egui_view_persona_get_read_only_mode(EGUI_VIEW_OF(&test_persona)));
    EGUI_TEST_ASSERT_FALSE(EGUI_VIEW_OF(&test_persona)->is_pressed);
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

static void test_persona_static_preview_consumes_input_and_keeps_state(void)
{
    persona_preview_snapshot_t initial_snapshot;
    egui_dim_t center_x;
    egui_dim_t center_y;

    setup_preview_persona();
    layout_view(EGUI_VIEW_OF(&preview_persona), 12, 18, 104, 78);
    get_view_center(EGUI_VIEW_OF(&preview_persona), &center_x, &center_y);
    capture_preview_snapshot(&initial_snapshot);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_persona), 1);
    EGUI_TEST_ASSERT_TRUE(send_touch_to_view(EGUI_VIEW_OF(&preview_persona), EGUI_MOTION_EVENT_ACTION_DOWN, center_x, center_y));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);

    egui_view_set_pressed(EGUI_VIEW_OF(&preview_persona), 1);
    EGUI_TEST_ASSERT_TRUE(send_key_to_view(EGUI_VIEW_OF(&preview_persona), EGUI_KEY_CODE_ENTER));
    assert_preview_state_unchanged(&initial_snapshot);
    EGUI_TEST_ASSERT_EQUAL_INT(0, g_click_count);
}

void test_persona_run(void)
{
    EGUI_TEST_SUITE_BEGIN(persona);
    EGUI_TEST_RUN(test_persona_init_uses_defaults);
    EGUI_TEST_RUN(test_persona_setters_and_resolvers_clear_pressed_state);
    EGUI_TEST_RUN(test_persona_helpers_compute_regions_and_colors);
    EGUI_TEST_RUN(test_persona_static_preview_consumes_input_and_keeps_state);
    EGUI_TEST_SUITE_END();
}
