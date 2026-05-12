#ifndef _HCW_TOP_ACCENT_H_
#define _HCW_TOP_ACCENT_H_

#include "egui.h"
#include "uicode_disp0.h"

static egui_dim_t hcw_top_accent_inset(egui_dim_t radius, egui_dim_t row_from_top)
{
    egui_dim_t inset;
    int32_t dy;
    int32_t radius_square;

    if (radius <= 0 || row_from_top >= radius)
    {
        return 0;
    }

    dy = radius - row_from_top;
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

static egui_dim_t hcw_top_accent_resolved_height(const egui_region_t *region, egui_dim_t radius, egui_dim_t height)
{
    egui_dim_t min_cap_h;
    egui_dim_t strip_h;

    if (region == NULL || region->size.width <= 0 || region->size.height <= 0 || height <= 0)
    {
        return 0;
    }

    strip_h = height > region->size.height ? region->size.height : height;
    if (radius <= 0)
    {
        return strip_h;
    }
    if (radius > region->size.width / 2)
    {
        radius = region->size.width / 2;
    }
    if (radius > region->size.height / 2)
    {
        radius = region->size.height / 2;
    }
    min_cap_h = radius + 1;
    if (strip_h < min_cap_h)
    {
        strip_h = min_cap_h;
    }
    if (strip_h > region->size.height)
    {
        strip_h = region->size.height;
    }

    return strip_h;
}

static egui_dim_t hcw_top_accent_content_offset(const egui_region_t *region, egui_dim_t radius, egui_dim_t height, egui_dim_t gap)
{
    return hcw_top_accent_resolved_height(region, radius, height) + gap;
}

static void hcw_top_accent_draw(const egui_region_t *region, egui_dim_t radius, egui_dim_t height, egui_color_t color, egui_alpha_t alpha)
{
    egui_canvas_t *canvas = &uicode_get_core()->canvas;
    egui_dim_t strip_h;
    egui_dim_t row;

    if (region == NULL || region->size.width <= 0 || region->size.height <= 0 || height <= 0 || alpha == EGUI_ALPHA_0)
    {
        return;
    }

    strip_h = hcw_top_accent_resolved_height(region, radius, height);
    if (strip_h <= 0)
    {
        return;
    }
    if (radius <= 0)
    {
        egui_canvas_draw_rectangle_fill(canvas, region->location.x, region->location.y, region->size.width, strip_h, color, alpha);
        return;
    }
    if (radius > region->size.width / 2)
    {
        radius = region->size.width / 2;
    }
    if (radius > region->size.height / 2)
    {
        radius = region->size.height / 2;
    }

    for (row = 0; row < strip_h; ++row)
    {
        egui_dim_t inset = hcw_top_accent_inset(radius, row);
        egui_dim_t width;

        if (inset * 2 >= region->size.width)
        {
            continue;
        }

        width = region->size.width - inset * 2;
        egui_canvas_draw_rectangle_fill(canvas, region->location.x + inset, region->location.y + row, width, 1, color, alpha);
    }
}

#endif /* _HCW_TOP_ACCENT_H_ */
