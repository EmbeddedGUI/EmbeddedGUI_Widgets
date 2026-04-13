#ifndef _EGUI_VIEW_RICH_EDIT_BOX_H_
#define _EGUI_VIEW_RICH_EDIT_BOX_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EGUI_VIEW_RICH_EDIT_BOX_MAX_DOCUMENTS 6
#define EGUI_VIEW_RICH_EDIT_BOX_MAX_PRESETS   3

#define EGUI_VIEW_RICH_EDIT_BOX_STYLE_BODY      0
#define EGUI_VIEW_RICH_EDIT_BOX_STYLE_CALLOUT   1
#define EGUI_VIEW_RICH_EDIT_BOX_STYLE_CHECKLIST 2

#define EGUI_VIEW_RICH_EDIT_BOX_PART_EDITOR   0
#define EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_0 1
#define EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_1 2
#define EGUI_VIEW_RICH_EDIT_BOX_PART_PRESET_2 3
#define EGUI_VIEW_RICH_EDIT_BOX_PART_NONE     0xFF

typedef struct egui_view_rich_edit_box_preset egui_view_rich_edit_box_preset_t;
struct egui_view_rich_edit_box_preset
{
    const char *label;
    const char *meta;
    uint8_t style;
};

typedef struct egui_view_rich_edit_box_document egui_view_rich_edit_box_document_t;
struct egui_view_rich_edit_box_document
{
    const char *header;
    const char *title;
    const char *summary;
    const char *footer;
    const char *draft_text;
    const egui_view_rich_edit_box_preset_t *presets;
    uint8_t preset_count;
    uint8_t selected_preset;
};

typedef void (*egui_view_on_rich_edit_box_action_listener_t)(egui_view_t *self, uint8_t document_index, uint8_t preset_index, uint8_t style,
                                                             uint8_t text_length);

typedef struct egui_view_rich_edit_box egui_view_rich_edit_box_t;
struct egui_view_rich_edit_box
{
    egui_view_t base;
    const egui_view_rich_edit_box_document_t *documents;
    const egui_font_t *font;
    const egui_font_t *meta_font;
    egui_view_on_rich_edit_box_action_listener_t on_action;
    egui_color_t surface_color;
    egui_color_t border_color;
    egui_color_t editor_color;
    egui_color_t text_color;
    egui_color_t muted_text_color;
    egui_color_t accent_color;
    egui_color_t shadow_color;
    uint8_t document_count;
    uint8_t current_document;
    uint8_t current_part;
    uint8_t current_preset;
    uint8_t applied_preset;
    uint8_t compact_mode;
    uint8_t read_only_mode;
    uint8_t pressed_part;
    uint8_t text_len;
    char editor_text[EGUI_CONFIG_TEXTINPUT_MAX_LENGTH + 1];
};

void egui_view_rich_edit_box_init(egui_view_t *self);
void egui_view_rich_edit_box_set_documents(egui_view_t *self, const egui_view_rich_edit_box_document_t *documents, uint8_t document_count);
void egui_view_rich_edit_box_set_current_document(egui_view_t *self, uint8_t document_index);
uint8_t egui_view_rich_edit_box_get_current_document(egui_view_t *self);
void egui_view_rich_edit_box_set_current_preset(egui_view_t *self, uint8_t preset_index);
uint8_t egui_view_rich_edit_box_get_current_preset(egui_view_t *self);
uint8_t egui_view_rich_edit_box_get_applied_preset(egui_view_t *self);
uint8_t egui_view_rich_edit_box_apply_current_preset(egui_view_t *self);
void egui_view_rich_edit_box_set_on_action_listener(egui_view_t *self, egui_view_on_rich_edit_box_action_listener_t listener);
void egui_view_rich_edit_box_set_font(egui_view_t *self, const egui_font_t *font);
void egui_view_rich_edit_box_set_meta_font(egui_view_t *self, const egui_font_t *font);
void egui_view_rich_edit_box_set_compact_mode(egui_view_t *self, uint8_t compact_mode);
void egui_view_rich_edit_box_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode);
void egui_view_rich_edit_box_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color, egui_color_t editor_color,
                                         egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color,
                                         egui_color_t shadow_color);
const char *egui_view_rich_edit_box_get_text(egui_view_t *self);
uint8_t egui_view_rich_edit_box_get_text_length(egui_view_t *self);
uint8_t egui_view_rich_edit_box_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region);
void egui_view_rich_edit_box_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_VIEW_RICH_EDIT_BOX_H_ */
