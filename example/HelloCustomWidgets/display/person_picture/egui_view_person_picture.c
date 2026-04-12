#include "egui_view_person_picture.h"

static uint8_t egui_view_person_picture_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_person_picture_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 66);
}

static uint8_t egui_view_person_picture_clamp_tone(uint8_t tone)
{
    if (tone > EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL)
    {
        return EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL;
    }
    return tone;
}

static uint8_t egui_view_person_picture_clamp_presence(uint8_t presence)
{
    if (presence > EGUI_VIEW_PERSON_PICTURE_PRESENCE_OFFLINE)
    {
        return EGUI_VIEW_PERSON_PICTURE_PRESENCE_NONE;
    }
    return presence;
}

static char egui_view_person_picture_normalize_initial_char(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return (char)(c - ('a' - 'A'));
    }
    if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
    {
        return c;
    }
    return '\0';
}

static uint8_t egui_view_person_picture_is_separator(char c)
{
    return (uint8_t)(c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '-' || c == '_' || c == '/' || c == '.');
}

static void egui_view_person_picture_copy_initials(const char *source, char *buffer)
{
    uint8_t len = 0;
    uint8_t i = 0;

    if (source != NULL)
    {
        while (source[i] != '\0' && len < 2)
        {
            char normalized = egui_view_person_picture_normalize_initial_char(source[i]);

            if (normalized != '\0')
            {
                buffer[len++] = normalized;
            }
            i++;
        }
    }

    buffer[len] = '\0';
}

static void egui_view_person_picture_derive_initials(const char *display_name, char *buffer)
{
    char first = '\0';
    char second = '\0';
    char fallback_second = '\0';
    uint8_t word_count = 0;
    uint8_t in_word = 0;
    uint8_t i = 0;

    if (display_name == NULL)
    {
        buffer[0] = '\0';
        return;
    }

    while (display_name[i] != '\0')
    {
        char c = display_name[i];
        char normalized = egui_view_person_picture_normalize_initial_char(c);

        if (egui_view_person_picture_is_separator(c))
        {
            in_word = 0;
            i++;
            continue;
        }

        if (!in_word)
        {
            in_word = 1;
            if (normalized != '\0')
            {
                if (word_count == 0)
                {
                    first = normalized;
                }
                else if (word_count == 1)
                {
                    second = normalized;
                    break;
                }
                word_count++;
            }
        }
        else if (word_count == 1 && fallback_second == '\0' && normalized != '\0')
        {
            fallback_second = normalized;
        }

        i++;
    }

    if (first != '\0')
    {
        buffer[0] = first;
        if (second != '\0')
        {
            buffer[1] = second;
            buffer[2] = '\0';
            return;
        }
        if (fallback_second != '\0')
        {
            buffer[1] = fallback_second;
            buffer[2] = '\0';
            return;
        }
        buffer[1] = '\0';
        return;
    }

    buffer[0] = '\0';
}

static void egui_view_person_picture_resolve_initials(const char *initials, const char *display_name, char *buffer)
{
    egui_view_person_picture_copy_initials(initials, buffer);
    if (buffer[0] == '\0')
    {
        egui_view_person_picture_derive_initials(display_name, buffer);
    }
}

static const egui_font_t *egui_view_person_picture_default_icon_font(egui_dim_t avatar_size)
{
    if (avatar_size <= 24)
    {
        return EGUI_FONT_ICON_MS_16;
    }
    if (avatar_size <= 40)
    {
        return EGUI_FONT_ICON_MS_20;
    }
    return EGUI_FONT_ICON_MS_24;
}

static const egui_font_t *egui_view_person_picture_resolve_icon_font(egui_view_person_picture_t *local, egui_dim_t avatar_size)
{
    if (local->icon_font != NULL)
    {
        return local->icon_font;
    }
    return egui_view_person_picture_default_icon_font(avatar_size);
}

static egui_color_t egui_view_person_picture_tone_color(egui_view_person_picture_t *local, uint8_t tone)
{
    switch (tone)
    {
    case EGUI_VIEW_PERSON_PICTURE_TONE_ACCENT:
        return local->accent_color;
    case EGUI_VIEW_PERSON_PICTURE_TONE_SUCCESS:
        return local->success_color;
    case EGUI_VIEW_PERSON_PICTURE_TONE_WARNING:
        return local->warning_color;
    default:
        return local->neutral_color;
    }
}

static egui_color_t egui_view_person_picture_presence_color(egui_view_person_picture_t *local, uint8_t presence)
{
    switch (presence)
    {
    case EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY:
        return local->warning_color;
    case EGUI_VIEW_PERSON_PICTURE_PRESENCE_AWAY:
        return egui_rgb_mix(local->warning_color, local->accent_color, 24);
    case EGUI_VIEW_PERSON_PICTURE_PRESENCE_OFFLINE:
        return local->neutral_color;
    default:
        return local->success_color;
    }
}

static void egui_view_person_picture_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0')
    {
        return;
    }

    egui_canvas_draw_text_in_rect(font, text, &draw_region, EGUI_ALIGN_CENTER, color, self->alpha);
}

static void egui_view_person_picture_get_avatar_region(egui_view_person_picture_t *local, egui_view_t *self, egui_region_t *avatar_region)
{
    egui_region_t work_region;
    egui_dim_t avatar_size;
    egui_dim_t inset;

    egui_view_get_work_region(self, &work_region);
    avatar_size = EGUI_MIN(work_region.size.width, work_region.size.height);
    if (avatar_size <= 0)
    {
        avatar_region->location.x = 0;
        avatar_region->location.y = 0;
        avatar_region->size.width = 0;
        avatar_region->size.height = 0;
        return;
    }

    inset = local->compact_mode ? 0 : (avatar_size >= 24 ? 1 : 0);
    if (avatar_size - inset * 2 <= 0)
    {
        inset = 0;
    }
    avatar_size -= inset * 2;

    avatar_region->size.width = avatar_size;
    avatar_region->size.height = avatar_size;
    avatar_region->location.x = work_region.location.x + (work_region.size.width - avatar_size) / 2;
    avatar_region->location.y = work_region.location.y + (work_region.size.height - avatar_size) / 2;
}

static uint8_t egui_view_person_picture_should_draw_presence(egui_view_person_picture_t *local)
{
    return (uint8_t)(local->presence != EGUI_VIEW_PERSON_PICTURE_PRESENCE_NONE);
}

static void egui_view_person_picture_get_presence_region(egui_view_person_picture_t *local, const egui_region_t *avatar_region, egui_region_t *presence_region)
{
    egui_dim_t dot_size;
    egui_dim_t margin;

    if (!egui_view_person_picture_should_draw_presence(local) || avatar_region->size.width <= 0)
    {
        presence_region->location.x = 0;
        presence_region->location.y = 0;
        presence_region->size.width = 0;
        presence_region->size.height = 0;
        return;
    }

    dot_size = avatar_region->size.width / (local->compact_mode ? 4 : 3);
    if (dot_size < (local->compact_mode ? 6 : 8))
    {
        dot_size = local->compact_mode ? 6 : 8;
    }
    if (dot_size > avatar_region->size.width / 2)
    {
        dot_size = avatar_region->size.width / 2;
    }

    margin = local->compact_mode ? 1 : 2;
    presence_region->size.width = dot_size;
    presence_region->size.height = dot_size;
    presence_region->location.x = avatar_region->location.x + avatar_region->size.width - dot_size - margin;
    presence_region->location.y = avatar_region->location.y + avatar_region->size.height - dot_size - margin;
}

static void egui_view_person_picture_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_region_t avatar_region;
    egui_region_t presence_region;
    egui_color_t fill_color;
    egui_color_t border_color;
    egui_color_t foreground_color;
    egui_color_t presence_color;
    egui_color_t presence_outline_color;
    egui_mask_t *prev_mask;
    char resolved_initials[3];
    egui_dim_t center_x;
    egui_dim_t center_y;
    egui_dim_t radius;

    egui_view_person_picture_get_avatar_region(local, self, &avatar_region);
    if (avatar_region.size.width <= 0 || avatar_region.size.height <= 0)
    {
        return;
    }

    center_x = avatar_region.location.x + avatar_region.size.width / 2;
    center_y = avatar_region.location.y + avatar_region.size.height / 2;
    radius = avatar_region.size.width / 2;

    fill_color = egui_view_person_picture_tone_color(local, local->tone);
    border_color = egui_rgb_mix(local->border_color, fill_color, local->compact_mode ? 8 : 14);
    foreground_color = local->foreground_color;
    presence_color = egui_view_person_picture_presence_color(local, local->presence);
    presence_outline_color = local->surface_color;

    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, local->surface_color, 34);
        border_color = egui_rgb_mix(border_color, local->muted_color, 54);
        foreground_color = egui_rgb_mix(foreground_color, local->muted_color, 42);
        presence_color = egui_rgb_mix(presence_color, local->muted_color, 58);
        presence_outline_color = egui_rgb_mix(presence_outline_color, local->muted_color, 18);
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_person_picture_mix_disabled(fill_color);
        border_color = egui_view_person_picture_mix_disabled(border_color);
        foreground_color = egui_view_person_picture_mix_disabled(foreground_color);
        presence_color = egui_view_person_picture_mix_disabled(presence_color);
        presence_outline_color = egui_view_person_picture_mix_disabled(presence_outline_color);
    }

    egui_canvas_draw_circle_fill_basic(center_x, center_y, radius, fill_color, egui_color_alpha_mix(self->alpha, 92));

    if (local->image != NULL)
    {
        prev_mask = egui_canvas_get_mask();
        egui_mask_set_position((egui_mask_t *)&local->image_mask, avatar_region.location.x, avatar_region.location.y);
        egui_mask_set_size((egui_mask_t *)&local->image_mask, avatar_region.size.width, avatar_region.size.height);
        egui_canvas_set_mask((egui_mask_t *)&local->image_mask);
        egui_canvas_draw_image_resize(local->image, avatar_region.location.x, avatar_region.location.y, avatar_region.size.width, avatar_region.size.height);
        egui_canvas_set_mask(prev_mask);
    }
    else
    {
        egui_view_person_picture_resolve_initials(local->initials, local->display_name, resolved_initials);
        if (resolved_initials[0] != '\0')
        {
            egui_view_person_picture_draw_text(local->font, self, resolved_initials, &avatar_region, foreground_color);
        }
        else
        {
            egui_view_person_picture_draw_text(egui_view_person_picture_resolve_icon_font(local, avatar_region.size.width), self, local->fallback_glyph,
                                               &avatar_region, foreground_color);
        }
    }

    egui_canvas_draw_circle_basic(center_x, center_y, radius, 1, border_color, egui_color_alpha_mix(self->alpha, 28));
    if (radius > 1)
    {
        egui_canvas_draw_circle_basic(center_x, center_y, radius - 1, 1, egui_rgb_mix(fill_color, local->surface_color, 16),
                                      egui_color_alpha_mix(self->alpha, 10));
    }

    if (egui_view_person_picture_should_draw_presence(local))
    {
        egui_dim_t presence_center_x;
        egui_dim_t presence_center_y;
        egui_dim_t presence_radius;

        egui_view_person_picture_get_presence_region(local, &avatar_region, &presence_region);
        if (presence_region.size.width > 0)
        {
            presence_center_x = presence_region.location.x + presence_region.size.width / 2;
            presence_center_y = presence_region.location.y + presence_region.size.height / 2;
            presence_radius = presence_region.size.width / 2;
            egui_canvas_draw_circle_fill_basic(presence_center_x, presence_center_y, presence_radius, presence_outline_color,
                                               egui_color_alpha_mix(self->alpha, 96));
            if (presence_radius > 1)
            {
                egui_canvas_draw_circle_fill_basic(presence_center_x, presence_center_y, presence_radius - 1, presence_color,
                                                   egui_color_alpha_mix(self->alpha, 92));
            }
        }
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_person_picture_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_person_picture_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_person_picture_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_person_picture_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_person_picture_set_display_name(egui_view_t *self, const char *display_name)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->display_name = display_name;
    egui_view_invalidate(self);
}

const char *egui_view_person_picture_get_display_name(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    return local->display_name;
}

void egui_view_person_picture_set_initials(egui_view_t *self, const char *initials)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->initials = initials;
    egui_view_invalidate(self);
}

const char *egui_view_person_picture_get_initials(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    return local->initials;
}

void egui_view_person_picture_set_fallback_glyph(egui_view_t *self, const char *glyph)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->fallback_glyph = (glyph != NULL && glyph[0] != '\0') ? glyph : EGUI_ICON_MS_PERSON;
    egui_view_invalidate(self);
}

const char *egui_view_person_picture_get_fallback_glyph(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    return local->fallback_glyph;
}

void egui_view_person_picture_set_image(egui_view_t *self, const egui_image_t *image)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->image = image;
    egui_view_invalidate(self);
}

const egui_image_t *egui_view_person_picture_get_image(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    return local->image;
}

void egui_view_person_picture_set_tone(egui_view_t *self, uint8_t tone)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->tone = egui_view_person_picture_clamp_tone(tone);
    egui_view_invalidate(self);
}

uint8_t egui_view_person_picture_get_tone(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    return local->tone;
}

void egui_view_person_picture_set_presence(egui_view_t *self, uint8_t presence)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->presence = egui_view_person_picture_clamp_presence(presence);
    egui_view_invalidate(self);
}

uint8_t egui_view_person_picture_get_presence(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    return local->presence;
}

void egui_view_person_picture_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_person_picture_set_icon_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->icon_font = font;
    egui_view_invalidate(self);
}

void egui_view_person_picture_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_person_picture_get_compact_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    return local->compact_mode;
}

void egui_view_person_picture_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_person_picture_get_read_only_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    return local->read_only_mode;
}

void egui_view_person_picture_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t foreground_color,
                                          egui_color_t accent_color, egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color,
                                          egui_color_t muted_color)
{
    EGUI_LOCAL_INIT(egui_view_person_picture_t);
    egui_view_person_picture_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->foreground_color = foreground_color;
    local->accent_color = accent_color;
    local->success_color = success_color;
    local->warning_color = warning_color;
    local->neutral_color = neutral_color;
    local->muted_color = muted_color;
    egui_view_invalidate(self);
}

void egui_view_person_picture_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_person_picture_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_person_picture_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_person_picture_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_person_picture_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_person_picture_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_person_picture_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_person_picture_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    egui_mask_circle_init((egui_mask_t *)&local->image_mask);
    local->display_name = NULL;
    local->initials = NULL;
    local->fallback_glyph = EGUI_ICON_MS_PERSON;
    local->image = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->icon_font = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xD2DBE3);
    local->foreground_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->success_color = EGUI_COLOR_HEX(0x0F7B45);
    local->warning_color = EGUI_COLOR_HEX(0x9D5D00);
    local->neutral_color = EGUI_COLOR_HEX(0x7A8796);
    local->muted_color = EGUI_COLOR_HEX(0x6B7A89);
    local->tone = EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL;
    local->presence = EGUI_VIEW_PERSON_PICTURE_PRESENCE_NONE;
    local->compact_mode = 0;
    local->read_only_mode = 0;

    egui_view_set_view_name(self, "egui_view_person_picture");
}
