#ifndef _HELLO_CUSTOM_WIDGETS_PRESENCE_BADGE_H_
#define _HELLO_CUSTOM_WIDGETS_PRESENCE_BADGE_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_PRESENCE_BADGE_STATUS_AVAILABLE        0
#define EGUI_VIEW_PRESENCE_BADGE_STATUS_BUSY             1
#define EGUI_VIEW_PRESENCE_BADGE_STATUS_AWAY             2
#define EGUI_VIEW_PRESENCE_BADGE_STATUS_DO_NOT_DISTURB   3
#define EGUI_VIEW_PRESENCE_BADGE_STATUS_OFFLINE          4

typedef struct egui_view_presence_badge egui_view_presence_badge_t;
struct egui_view_presence_badge
{
    egui_view_t base;
    egui_color_t surface_color;
    egui_color_t outline_color;
    egui_color_t available_color;
    egui_color_t busy_color;
    egui_color_t away_color;
    egui_color_t do_not_disturb_color;
    egui_color_t offline_color;
    egui_color_t glyph_color;
    egui_color_t muted_color;
    uint8_t status;
    uint8_t compact_mode;
    uint8_t read_only_mode;
};

void egui_view_presence_badge_init(egui_view_t *self);
void egui_view_presence_badge_set_status(egui_view_t *self, uint8_t status);
uint8_t egui_view_presence_badge_get_status(egui_view_t *self);
void egui_view_presence_badge_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t outline_color, egui_color_t available_color,
                                          egui_color_t busy_color, egui_color_t away_color, egui_color_t do_not_disturb_color,
                                          egui_color_t offline_color, egui_color_t glyph_color, egui_color_t muted_color);
void egui_view_presence_badge_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
uint8_t egui_view_presence_badge_get_compact_mode(egui_view_t *self);
void egui_view_presence_badge_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
uint8_t egui_view_presence_badge_get_read_only_mode(egui_view_t *self);
uint8_t egui_view_presence_badge_get_indicator_region(egui_view_t *self, egui_region_t *region);
void egui_view_presence_badge_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_PRESENCE_BADGE_H_ */
