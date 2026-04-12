#ifndef _EGUI_VIEW_SYMBOL_ICON_H_
#define _EGUI_VIEW_SYMBOL_ICON_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct egui_view_symbol_icon egui_view_symbol_icon_t;
struct egui_view_symbol_icon
{
    egui_view_t base;
    const char *symbol;
    const egui_font_t *icon_font;
    egui_color_t icon_color;
};

void egui_view_symbol_icon_init(egui_view_t *self);
void egui_view_symbol_icon_apply_standard_style(egui_view_t *self);
void egui_view_symbol_icon_apply_subtle_style(egui_view_t *self);
void egui_view_symbol_icon_apply_accent_style(egui_view_t *self);
void egui_view_symbol_icon_set_symbol(egui_view_t *self, const char *symbol);
const char *egui_view_symbol_icon_get_symbol(egui_view_t *self);
void egui_view_symbol_icon_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_symbol_icon_set_palette(egui_view_t *self, egui_color_t icon_color);
void egui_view_symbol_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_SYMBOL_ICON_H_ */
