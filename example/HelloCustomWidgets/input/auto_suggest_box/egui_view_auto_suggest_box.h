#ifndef _HELLO_CUSTOM_WIDGETS_AUTO_SUGGEST_BOX_H_
#define _HELLO_CUSTOM_WIDGETS_AUTO_SUGGEST_BOX_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_textinput.h"
#include "../../uicode_disp0.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_AUTO_SUGGEST_BOX_INDEX_NONE 0xFF

#define EGUI_VIEW_AUTO_SUGGEST_BOX_PART_FIELD 0
#define EGUI_VIEW_AUTO_SUGGEST_BOX_PART_ITEM  1
#define EGUI_VIEW_AUTO_SUGGEST_BOX_PART_NONE  0xFF

typedef void (*egui_view_on_auto_suggest_box_selected_listener_t)(egui_view_t *self, uint8_t index);

typedef struct egui_view_auto_suggest_box egui_view_auto_suggest_box_t;
struct egui_view_auto_suggest_box
{
    egui_view_textinput_t textinput;
    egui_view_api_t api;
    egui_view_api_t custom_api;
    const egui_font_t *icon_font;
    egui_view_on_auto_suggest_box_selected_listener_t on_selected;
    const char **suggestions;
    uint8_t suggestion_count;
    uint8_t current_index;
    uint8_t is_expanded;
    uint8_t max_visible_items;
    uint8_t read_only_mode;
    uint8_t pressed_part;
    uint8_t pressed_row;
    egui_dim_t collapsed_height;
    egui_dim_t item_height;
    egui_dim_t icon_text_gap;
    egui_dim_t content_pad_left;
    egui_dim_t content_pad_right;
    egui_dim_t content_pad_y;
    egui_color_t bg_color;
    egui_color_t border_color;
    egui_color_t popup_color;
    egui_color_t popup_border_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t highlight_color;
    egui_color_t icon_color;
};

typedef struct egui_view_auto_suggest_box_params egui_view_auto_suggest_box_params_t;
struct egui_view_auto_suggest_box_params
{
    egui_region_t region;
    const char **suggestions;
    uint8_t suggestion_count;
    uint8_t current_index;
    const char *query;
    const char *placeholder;
};

#define EGUI_VIEW_AUTO_SUGGEST_BOX_PARAMS_INIT(_name, _x, _y, _w, _h, _suggestions, _count, _index)                                                           \
    static const egui_view_auto_suggest_box_params_t _name = {                                                                                                  \
            .region = {{(_x), (_y)}, {(_w), (_h)}}, .suggestions = (_suggestions), .suggestion_count = (_count), .current_index = (_index), .query = NULL,    \
            .placeholder = NULL}

void egui_view_auto_suggest_box_apply_params(egui_view_t *self, const egui_view_auto_suggest_box_params_t *params);
void egui_view_auto_suggest_box_init_with_params(egui_view_t *self, egui_core_t *core, const egui_view_auto_suggest_box_params_t *params);
void egui_view_auto_suggest_box_init(egui_view_t *self, egui_core_t *core);

void egui_view_auto_suggest_box_set_suggestions(egui_view_t *self, const char **suggestions, uint8_t count);
uint8_t egui_view_auto_suggest_box_get_suggestion_count(egui_view_t *self);
void egui_view_auto_suggest_box_set_query(egui_view_t *self, const char *query);
const char *egui_view_auto_suggest_box_get_query(egui_view_t *self);
void egui_view_auto_suggest_box_set_current_index(egui_view_t *self, uint8_t index);
uint8_t egui_view_auto_suggest_box_get_current_index(egui_view_t *self);
const char *egui_view_auto_suggest_box_get_current_text(egui_view_t *self);
uint8_t egui_view_auto_suggest_box_get_filtered_count(egui_view_t *self);
void egui_view_auto_suggest_box_set_max_visible_items(egui_view_t *self, uint8_t max_items);
void egui_view_auto_suggest_box_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_auto_suggest_box_set_icon_font(egui_view_t *self, const egui_font_t *font);
void egui_view_auto_suggest_box_set_placeholder(egui_view_t *self, const char *placeholder);
void egui_view_auto_suggest_box_set_on_selected_listener(egui_view_t *self, egui_view_on_auto_suggest_box_selected_listener_t listener);
void egui_view_auto_suggest_box_expand(egui_view_t *self);
void egui_view_auto_suggest_box_collapse(egui_view_t *self);
uint8_t egui_view_auto_suggest_box_is_expanded(egui_view_t *self);

void hcw_auto_suggest_box_apply_standard_style(egui_view_t *self);
void hcw_auto_suggest_box_apply_compact_style(egui_view_t *self);
void hcw_auto_suggest_box_apply_read_only_style(egui_view_t *self);
void hcw_auto_suggest_box_set_suggestions(egui_view_t *self, const char **suggestions, uint8_t count);
void hcw_auto_suggest_box_set_query(egui_view_t *self, const char *query);
void hcw_auto_suggest_box_set_current_index(egui_view_t *self, uint8_t index);
void hcw_auto_suggest_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_AUTO_SUGGEST_BOX_H_ */
