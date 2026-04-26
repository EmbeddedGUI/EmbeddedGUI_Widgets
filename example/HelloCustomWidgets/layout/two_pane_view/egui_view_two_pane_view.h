#ifndef _EGUI_VIEW_TWO_PANE_VIEW_H_
#define _EGUI_VIEW_TWO_PANE_VIEW_H_

#include "egui.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_WIDE   0
#define EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_TALL   1
#define EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_SINGLE 2
#define EGUI_VIEW_TWO_PANE_VIEW_LAYOUT_COUNT  3

#define EGUI_VIEW_TWO_PANE_VIEW_PANE_FIRST  0
#define EGUI_VIEW_TWO_PANE_VIEW_PANE_SECOND 1

#define EGUI_VIEW_TWO_PANE_VIEW_TONE_ACCENT  0
#define EGUI_VIEW_TWO_PANE_VIEW_TONE_SUCCESS 1
#define EGUI_VIEW_TWO_PANE_VIEW_TONE_WARNING 2
#define EGUI_VIEW_TWO_PANE_VIEW_TONE_NEUTRAL 3

typedef struct egui_view_two_pane_view_pane egui_view_two_pane_view_pane_t;
struct egui_view_two_pane_view_pane
{
    const char *eyebrow;
    const char *title;
    const char *meta;
    const char *body_primary;
    const char *body_secondary;
    const char *action;
    uint8_t tone;
    uint8_t emphasized;
};

typedef void (*egui_view_on_two_pane_view_layout_changed_listener_t)(egui_view_t *self, uint8_t layout_mode);
typedef void (*egui_view_on_two_pane_view_pane_changed_listener_t)(egui_view_t *self, uint8_t pane);

typedef struct egui_view_two_pane_view egui_view_two_pane_view_t;
struct egui_view_two_pane_view
{
    egui_view_t base;
    const egui_view_two_pane_view_pane_t *first_pane;
    const egui_view_two_pane_view_pane_t *second_pane;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_two_pane_view_layout_changed_listener_t on_layout_changed;
    egui_view_on_two_pane_view_pane_changed_listener_t on_pane_changed;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t section_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t success_color;
    egui_color_t warning_color;
    egui_color_t neutral_color;
    uint8_t layout_mode;
    uint8_t single_pane;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_target;
};

void egui_view_two_pane_view_init(egui_view_t *self);
void egui_view_two_pane_view_set_panes(egui_view_t *self, const egui_view_two_pane_view_pane_t *first_pane,
                                       const egui_view_two_pane_view_pane_t *second_pane);
void egui_view_two_pane_view_set_layout_mode(egui_view_t *self, uint8_t layout_mode);
uint8_t egui_view_two_pane_view_get_layout_mode(egui_view_t *self);
void egui_view_two_pane_view_set_single_pane(egui_view_t *self, uint8_t pane);
uint8_t egui_view_two_pane_view_get_single_pane(egui_view_t *self);
void egui_view_two_pane_view_toggle_single_pane(egui_view_t *self);
void egui_view_two_pane_view_set_on_layout_changed_listener(egui_view_t *self, egui_view_on_two_pane_view_layout_changed_listener_t listener);
void egui_view_two_pane_view_set_on_pane_changed_listener(egui_view_t *self, egui_view_on_two_pane_view_pane_changed_listener_t listener);
void egui_view_two_pane_view_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_two_pane_view_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_two_pane_view_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_two_pane_view_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_two_pane_view_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t section_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color, egui_color_t success_color,
                                         egui_color_t warning_color, egui_color_t neutral_color);
void egui_view_two_pane_view_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_TWO_PANE_VIEW_H_ */
