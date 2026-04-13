#ifndef _HELLO_CUSTOM_WIDGETS_DOCK_PANEL_H_
#define _HELLO_CUSTOM_WIDGETS_DOCK_PANEL_H_

#include "../../../../sdk/EmbeddedGUI/src/widget/egui_view_group.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    HCW_DOCK_PANEL_DOCK_LEFT = 0,
    HCW_DOCK_PANEL_DOCK_TOP,
    HCW_DOCK_PANEL_DOCK_RIGHT,
    HCW_DOCK_PANEL_DOCK_BOTTOM,
    HCW_DOCK_PANEL_DOCK_FILL,
} hcw_dock_panel_dock_t;

typedef struct hcw_dock_panel hcw_dock_panel_t;
struct hcw_dock_panel
{
    egui_view_group_t base;
    uint8_t last_child_fill;
    egui_dim_t content_inset;
};

void hcw_dock_panel_init(egui_view_t *self);
void hcw_dock_panel_apply_standard_style(egui_view_t *self);
void hcw_dock_panel_apply_compact_style(egui_view_t *self);
void hcw_dock_panel_set_last_child_fill(egui_view_t *self, uint8_t last_child_fill);
void hcw_dock_panel_set_child_dock(egui_view_t *child, uint8_t dock_type);
void hcw_dock_panel_layout_childs(egui_view_t *self);
void hcw_dock_panel_override_static_preview_api(egui_view_t *self, egui_view_api_t *api);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_DOCK_PANEL_H_ */
