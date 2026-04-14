#ifndef _EGUI_VIEW_PERSONA_H_
#define _EGUI_VIEW_PERSONA_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_PERSONA_TONE_ACCENT  0
#define EGUI_VIEW_PERSONA_TONE_SUCCESS 1
#define EGUI_VIEW_PERSONA_TONE_WARNING 2
#define EGUI_VIEW_PERSONA_TONE_NEUTRAL 3

#define EGUI_VIEW_PERSONA_STATUS_NONE           0
#define EGUI_VIEW_PERSONA_STATUS_AVAILABLE      1
#define EGUI_VIEW_PERSONA_STATUS_BUSY           2
#define EGUI_VIEW_PERSONA_STATUS_AWAY           3
#define EGUI_VIEW_PERSONA_STATUS_DO_NOT_DISTURB 4
#define EGUI_VIEW_PERSONA_STATUS_OFFLINE        5

typedef struct egui_view_persona egui_view_persona_t;
struct egui_view_persona
{
    egui_view_t base;
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
    uint8_t tone;
    uint8_t status;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_persona_init(egui_view_t *self);
void egui_view_persona_set_display_name(egui_view_t *self, const char *display_name);
const char *egui_view_persona_get_display_name(egui_view_t *self);
void egui_view_persona_set_secondary_text(egui_view_t *self, const char *secondary_text);
const char *egui_view_persona_get_secondary_text(egui_view_t *self);
void egui_view_persona_set_tertiary_text(egui_view_t *self, const char *tertiary_text);
const char *egui_view_persona_get_tertiary_text(egui_view_t *self);
void egui_view_persona_set_quaternary_text(egui_view_t *self, const char *quaternary_text);
const char *egui_view_persona_get_quaternary_text(egui_view_t *self);
void egui_view_persona_set_initials(egui_view_t *self, const char *initials);
const char *egui_view_persona_get_initials(egui_view_t *self);
void egui_view_persona_set_status(egui_view_t *self, uint8_t status);
uint8_t egui_view_persona_get_status(egui_view_t *self);
void egui_view_persona_set_tone(egui_view_t *self, uint8_t tone);
uint8_t egui_view_persona_get_tone(egui_view_t *self);
void egui_view_persona_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_persona_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_persona_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_persona_get_compact_mode(egui_view_t *self);
void egui_view_persona_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_persona_get_read_only_mode(egui_view_t *self);
void egui_view_persona_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t section_color,
                                   egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color,
                                   egui_color_t warning_color, egui_color_t neutral_color);
uint8_t egui_view_persona_get_avatar_region(egui_view_t *self, egui_region_t *region);
uint8_t egui_view_persona_get_presence_region(egui_view_t *self, egui_region_t *region);
void egui_view_persona_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_PERSONA_H_ */
