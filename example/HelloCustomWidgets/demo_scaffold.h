#ifndef _HELLO_CUSTOM_WIDGETS_DEMO_SCAFFOLD_H_
#define _HELLO_CUSTOM_WIDGETS_DEMO_SCAFFOLD_H_

#include "egui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HELLO_CUSTOM_WIDGETS_CANVAS_WIDTH    EGUI_CONFIG_SCEEN_WIDTH
#define HELLO_CUSTOM_WIDGETS_CANVAS_HEIGHT   EGUI_CONFIG_SCEEN_HEIGHT
#define HELLO_CUSTOM_WIDGETS_TITLE_TOP_INSET 32

void hello_custom_widgets_demo_scale_tree(egui_view_t *view);
void hello_custom_widgets_demo_hide_views(egui_view_t **views, uint8_t count);
void hello_custom_widgets_demo_apply_title_only_scaffold(egui_view_t *root, egui_view_t *title, egui_view_t **chrome_views, uint8_t chrome_view_count);
void hello_custom_widgets_demo_get_triptych_rects(const egui_region_t *region, egui_dim_t top_inset, egui_region_t *main_rect, egui_region_t *left_rect,
                                                  egui_region_t *right_rect);

#ifdef __cplusplus
}
#endif

#endif /* _HELLO_CUSTOM_WIDGETS_DEMO_SCAFFOLD_H_ */
