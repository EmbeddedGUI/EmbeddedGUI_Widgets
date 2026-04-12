#ifndef _EGUI_VIEW_PERSON_PICTURE_H_
#define _EGUI_VIEW_PERSON_PICTURE_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_PERSON_PICTURE_TONE_ACCENT  0
#define EGUI_VIEW_PERSON_PICTURE_TONE_SUCCESS 1
#define EGUI_VIEW_PERSON_PICTURE_TONE_WARNING 2
#define EGUI_VIEW_PERSON_PICTURE_TONE_NEUTRAL 3

#define EGUI_VIEW_PERSON_PICTURE_PRESENCE_NONE    0
#define EGUI_VIEW_PERSON_PICTURE_PRESENCE_LIVE    1
#define EGUI_VIEW_PERSON_PICTURE_PRESENCE_BUSY    2
#define EGUI_VIEW_PERSON_PICTURE_PRESENCE_AWAY    3
#define EGUI_VIEW_PERSON_PICTURE_PRESENCE_OFFLINE 4

typedef struct egui_view_person_picture egui_view_person_picture_t;
struct egui_view_person_picture
{
    egui_view_t base;
    egui_mask_circle_t image_mask;
    const char *display_name;
    const char *initials;
    const char *fallback_glyph;
    const egui_image_t *image;
    const egui_font_t *font;
    const egui_font_t *icon_font;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t foreground_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    egui_color_t muted_color;
    uint8_t tone;
    uint8_t presence;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_person_picture_init(egui_view_t *self);
void egui_view_person_picture_set_display_name(egui_view_t *self, const char *display_name);
const char *egui_view_person_picture_get_display_name(egui_view_t *self);
void egui_view_person_picture_set_initials(egui_view_t *self, const char *initials);
const char *egui_view_person_picture_get_initials(egui_view_t *self);
void egui_view_person_picture_set_fallback_glyph(egui_view_t *self, const char *glyph);
const char *egui_view_person_picture_get_fallback_glyph(egui_view_t *self);
void egui_view_person_picture_set_image(egui_view_t *self, const egui_image_t *image);
const egui_image_t *egui_view_person_picture_get_image(egui_view_t *self);
void egui_view_person_picture_set_tone(egui_view_t *self, uint8_t tone);
uint8_t egui_view_person_picture_get_tone(egui_view_t *self);
void egui_view_person_picture_set_presence(egui_view_t *self, uint8_t presence);
uint8_t egui_view_person_picture_get_presence(egui_view_t *self);
void egui_view_person_picture_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_person_picture_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_person_picture_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_person_picture_get_compact_mode(egui_view_t *self);
void egui_view_person_picture_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_person_picture_get_read_only_mode(egui_view_t *self);
void egui_view_person_picture_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t foreground_color,
                                          egui_color_t accent_color, egui_color_t success_color, egui_color_t warning_color, egui_color_t neutral_color,
                                          egui_color_t muted_color);
void egui_view_person_picture_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_PERSON_PICTURE_H_ */
