#ifndef _HCW_SELECTION_MARKER_H_
#define _HCW_SELECTION_MARKER_H_

#include "egui.h"
#include "uicode_disp0.h"

static egui_dim_t hcw_selection_marker_left_inset(egui_dim_t radius, egui_dim_t row_from_edge)
{
    egui_dim_t inset;
    int32_t dy;
    int32_t radius_square;

    if (radius <= 0)
    {
        return 0;
    }

    if (row_from_edge >= radius)
    {
        return 0;
    }

    dy = radius - row_from_edge;
    radius_square = (int32_t)radius * (int32_t)radius;
    for (inset = 0; inset <= radius; ++inset)
    {
        int32_t dx = radius - inset;

        if (dx * dx + dy * dy <= radius_square)
        {
            return inset;
        }
    }

    return radius;
}

static void hcw_selection_marker_draw_left(const egui_region_t *region, egui_dim_t radius, egui_dim_t width, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_t *canvas = &uicode_get_core()->canvas;
    egui_dim_t marker_w;
    egui_dim_t row;

    if (region == NULL || region->size.width <= 0 || region->size.height <= 0 || alpha == EGUI_ALPHA_0)
    {
        return;
    }

    if (radius <= 0)
    {
        radius = 1;
    }
    marker_w = width < radius ? radius : width;
    if (marker_w > region->size.width)
    {
        marker_w = region->size.width;
    }
    if (marker_w <= 0)
    {
        return;
    }
    if (radius > region->size.height / 2)
    {
        radius = region->size.height / 2;
    }
    if (radius > marker_w)
    {
        radius = marker_w;
    }

    for (row = 0; row < region->size.height; ++row)
    {
        egui_dim_t inset = 0;

        if (row < radius)
        {
            inset = hcw_selection_marker_left_inset(radius, row);
        }
        else if (row >= region->size.height - radius)
        {
            inset = hcw_selection_marker_left_inset(radius, (egui_dim_t)(region->size.height - row - 1));
        }

        if (inset >= marker_w)
        {
            inset = marker_w - 1;
        }
        egui_canvas_draw_rectangle_fill(canvas, region->location.x + inset, region->location.y + row, marker_w - inset, 1, color, alpha);
    }
}

#endif /* _HCW_SELECTION_MARKER_H_ */
