#include "demo_scaffold.h"

static egui_dim_t scale_width(egui_dim_t value)
{
    if (value <= 0)
    {
        return value;
    }
    return (egui_dim_t)(value * 2);
}

static egui_dim_t scale_height(egui_dim_t value)
{
    if (value <= 0)
    {
        return value;
    }
    return (egui_dim_t)((value * 3 + 1) / 2);
}

static void scale_margin(egui_view_margin_t *margin)
{
    margin->left = scale_width(margin->left);
    margin->right = scale_width(margin->right);
    margin->top = scale_height(margin->top);
    margin->bottom = scale_height(margin->bottom);
}

static void scale_padding(egui_view_padding_t *padding)
{
    padding->left = scale_width(padding->left);
    padding->right = scale_width(padding->right);
    padding->top = scale_height(padding->top);
    padding->bottom = scale_height(padding->bottom);
}

static void scale_tree_internal(egui_view_t *view)
{
    egui_dnode_t *node;

    if (view == NULL)
    {
        return;
    }

    view->region.size.width = scale_width(view->region.size.width);
    view->region.size.height = scale_height(view->region.size.height);
    scale_margin(&view->margin);
    scale_padding(&view->padding);

    if (view->api == NULL || view->api->draw != egui_view_group_draw)
    {
        return;
    }

    EGUI_DLIST_FOR_EACH_NODE(&EGUI_CAST_TO(egui_view_group_t, view)->childs, node)
    {
        egui_view_t *child = EGUI_DLIST_ENTRY(node, egui_view_t, node);
        scale_tree_internal(child);
    }
}

void hello_custom_widgets_demo_scale_tree(egui_view_t *view)
{
    scale_tree_internal(view);
}

void hello_custom_widgets_demo_hide_views(egui_view_t **views, uint8_t count)
{
    uint8_t i;

    if (views == NULL)
    {
        return;
    }

    for (i = 0; i < count; ++i)
    {
        if (views[i] == NULL)
        {
            continue;
        }
        egui_view_set_gone(views[i], 1);
    }
}

void hello_custom_widgets_demo_apply_title_only_scaffold(egui_view_t *root, egui_view_t *title, egui_view_t **chrome_views, uint8_t chrome_view_count)
{
    hello_custom_widgets_demo_scale_tree(root);
    hello_custom_widgets_demo_hide_views(chrome_views, chrome_view_count);

    if (title != NULL)
    {
        title->region.size.width = root == NULL ? title->region.size.width : root->region.size.width;
        title->margin.left = 0;
        title->margin.right = 0;
    }
}

void hello_custom_widgets_demo_get_triptych_rects(const egui_region_t *region, egui_dim_t top_inset, egui_region_t *main_rect, egui_region_t *left_rect,
                                                  egui_region_t *right_rect)
{
    egui_dim_t outer_gap;
    egui_dim_t column_gap;
    egui_dim_t preview_h;
    egui_dim_t preview_y;
    egui_dim_t main_y;
    egui_dim_t main_gap_y;
    egui_dim_t preview_w;

    if (region == NULL)
    {
        return;
    }

    outer_gap = EGUI_MAX(24, region->size.width / 20);
    column_gap = EGUI_MAX(24, region->size.width / 20);
    main_gap_y = EGUI_MAX(20, region->size.height / 20);
    preview_h = EGUI_MAX(140, region->size.height / 4 + 20);
    if (preview_h > region->size.height - top_inset - outer_gap - main_gap_y - 80)
    {
        preview_h = region->size.height - top_inset - outer_gap - main_gap_y - 80;
    }
    preview_y = region->location.y + region->size.height - outer_gap - preview_h;
    main_y = region->location.y + top_inset + 8;
    preview_w = (region->size.width - outer_gap * 2 - column_gap) / 2;

    if (main_rect != NULL)
    {
        main_rect->location.x = region->location.x + outer_gap;
        main_rect->location.y = main_y;
        main_rect->size.width = region->size.width - outer_gap * 2;
        main_rect->size.height = preview_y - main_y - main_gap_y;
    }

    if (left_rect != NULL)
    {
        left_rect->location.x = region->location.x + outer_gap;
        left_rect->location.y = preview_y;
        left_rect->size.width = preview_w;
        left_rect->size.height = preview_h;
    }

    if (right_rect != NULL)
    {
        right_rect->location.x = region->location.x + outer_gap + preview_w + column_gap;
        right_rect->location.y = preview_y;
        right_rect->size.width = preview_w;
        right_rect->size.height = preview_h;
    }
}
