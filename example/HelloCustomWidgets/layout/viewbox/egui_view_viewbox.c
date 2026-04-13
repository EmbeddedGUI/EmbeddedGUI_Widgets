#include <stdio.h>

#include "egui_view_viewbox.h"

#define EGUI_VIEW_VIEWBOX_STANDARD_RADIUS        10
#define EGUI_VIEW_VIEWBOX_STANDARD_OUTER_PAD_X   7
#define EGUI_VIEW_VIEWBOX_STANDARD_OUTER_PAD_Y   7
#define EGUI_VIEW_VIEWBOX_STANDARD_INNER_PAD_X   6
#define EGUI_VIEW_VIEWBOX_STANDARD_INNER_PAD_Y   6
#define EGUI_VIEW_VIEWBOX_STANDARD_BADGE_H       9
#define EGUI_VIEW_VIEWBOX_STANDARD_BADGE_GAP     4
#define EGUI_VIEW_VIEWBOX_STANDARD_TITLE_H       11
#define EGUI_VIEW_VIEWBOX_STANDARD_SUMMARY_H     9
#define EGUI_VIEW_VIEWBOX_STANDARD_TITLE_GAP     2
#define EGUI_VIEW_VIEWBOX_STANDARD_VIEWPORT_GAP  5
#define EGUI_VIEW_VIEWBOX_STANDARD_VIEWPORT_R    8
#define EGUI_VIEW_VIEWBOX_STANDARD_PRESET_H      19
#define EGUI_VIEW_VIEWBOX_STANDARD_PRESET_GAP    4
#define EGUI_VIEW_VIEWBOX_STANDARD_FOOTER_H      9
#define EGUI_VIEW_VIEWBOX_STANDARD_FOOTER_GAP    4

#define EGUI_VIEW_VIEWBOX_COMPACT_RADIUS        8
#define EGUI_VIEW_VIEWBOX_COMPACT_OUTER_PAD_X   5
#define EGUI_VIEW_VIEWBOX_COMPACT_OUTER_PAD_Y   5
#define EGUI_VIEW_VIEWBOX_COMPACT_INNER_PAD_X   5
#define EGUI_VIEW_VIEWBOX_COMPACT_INNER_PAD_Y   5
#define EGUI_VIEW_VIEWBOX_COMPACT_BADGE_H       7
#define EGUI_VIEW_VIEWBOX_COMPACT_BADGE_GAP     3
#define EGUI_VIEW_VIEWBOX_COMPACT_TITLE_H       9
#define EGUI_VIEW_VIEWBOX_COMPACT_SUMMARY_H     0
#define EGUI_VIEW_VIEWBOX_COMPACT_TITLE_GAP     0
#define EGUI_VIEW_VIEWBOX_COMPACT_VIEWPORT_GAP  4
#define EGUI_VIEW_VIEWBOX_COMPACT_VIEWPORT_R    6
#define EGUI_VIEW_VIEWBOX_COMPACT_PRESET_H      14
#define EGUI_VIEW_VIEWBOX_COMPACT_PRESET_GAP    3
#define EGUI_VIEW_VIEWBOX_COMPACT_FOOTER_H      0
#define EGUI_VIEW_VIEWBOX_COMPACT_FOOTER_GAP    0

typedef struct egui_view_viewbox_metrics egui_view_viewbox_metrics_t;
struct egui_view_viewbox_metrics
{
    egui_region_t content_region;
    egui_region_t badge_region;
    egui_region_t title_region;
    egui_region_t summary_region;
    egui_region_t viewport_region;
    egui_region_t footer_region;
    egui_region_t preset_regions[EGUI_VIEW_VIEWBOX_MAX_PRESETS];
};

typedef struct egui_view_viewbox_scale_info egui_view_viewbox_scale_info_t;
struct egui_view_viewbox_scale_info
{
    egui_region_t guide_region;
    egui_region_t content_region;
    uint16_t scale_x_permille;
    uint16_t scale_y_permille;
};

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_viewbox_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event);
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_viewbox_on_static_key_event(egui_view_t *self, egui_key_event_t *event);
#endif

static void viewbox_reset_region(egui_region_t *region)
{
    region->location.x = 0;
    region->location.y = 0;
    region->size.width = 0;
    region->size.height = 0;
}

static uint8_t viewbox_region_has_size(const egui_region_t *region)
{
    return region != NULL && region->size.width > 0 && region->size.height > 0 ? 1 : 0;
}

static uint8_t viewbox_clamp_snapshot_count(uint8_t count)
{
    return count > EGUI_VIEW_VIEWBOX_MAX_SNAPSHOTS ? EGUI_VIEW_VIEWBOX_MAX_SNAPSHOTS : count;
}

static uint8_t viewbox_clamp_preset_count(uint8_t count)
{
    return count > EGUI_VIEW_VIEWBOX_MAX_PRESETS ? EGUI_VIEW_VIEWBOX_MAX_PRESETS : count;
}

static uint8_t viewbox_clamp_stretch_mode(uint8_t stretch_mode)
{
    switch (stretch_mode)
    {
    case EGUI_VIEW_VIEWBOX_STRETCH_FILL:
    case EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY:
        return stretch_mode;
    case EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM:
    default:
        return EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM;
    }
}

static uint8_t viewbox_text_len(const char *text)
{
    uint8_t length = 0;

    if (text == NULL)
    {
        return 0;
    }
    while (text[length] != '\0')
    {
        length++;
    }
    return length;
}

static uint8_t viewbox_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_color_t viewbox_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_DARK_GREY, 68);
}

static egui_dim_t viewbox_resolve_source_dim(egui_dim_t source_dim, egui_dim_t fallback_dim)
{
    if (source_dim > 0)
    {
        return source_dim;
    }
    if (fallback_dim > 0)
    {
        return fallback_dim;
    }
    return 1;
}

static const egui_view_viewbox_snapshot_t *viewbox_get_snapshot(egui_view_viewbox_t *local)
{
    if (local->snapshots == NULL || local->snapshot_count == 0 || local->current_snapshot >= local->snapshot_count)
    {
        return NULL;
    }
    return &local->snapshots[local->current_snapshot];
}

static uint8_t viewbox_get_preset_count(const egui_view_viewbox_snapshot_t *snapshot)
{
    if (snapshot == NULL || snapshot->presets == NULL)
    {
        return 0;
    }
    return viewbox_clamp_preset_count(snapshot->preset_count);
}

static const egui_view_viewbox_preset_t *viewbox_get_preset(const egui_view_viewbox_snapshot_t *snapshot, uint8_t preset_index)
{
    if (preset_index >= viewbox_get_preset_count(snapshot))
    {
        return NULL;
    }
    return &snapshot->presets[preset_index];
}

static uint8_t viewbox_clear_pressed_state(egui_view_t *self, egui_view_viewbox_t *local)
{
    uint8_t had_pressed = self->is_pressed || local->pressed_preset != EGUI_VIEW_VIEWBOX_PRESET_NONE;

    local->pressed_preset = EGUI_VIEW_VIEWBOX_PRESET_NONE;
    egui_view_set_pressed(self, false);
    return had_pressed;
}

static uint8_t viewbox_preset_exists(const egui_view_viewbox_snapshot_t *snapshot, uint8_t preset_index)
{
    return preset_index < viewbox_get_preset_count(snapshot) ? 1 : 0;
}

static uint8_t viewbox_find_preset_by_mode(const egui_view_viewbox_snapshot_t *snapshot, uint8_t stretch_mode)
{
    uint8_t preset_count = viewbox_get_preset_count(snapshot);
    uint8_t preset_index;

    for (preset_index = 0; preset_index < preset_count; ++preset_index)
    {
        const egui_view_viewbox_preset_t *preset = &snapshot->presets[preset_index];

        if (viewbox_clamp_stretch_mode(preset->stretch_mode) == stretch_mode)
        {
            return preset_index;
        }
    }
    return EGUI_VIEW_VIEWBOX_PRESET_NONE;
}

static uint8_t viewbox_resolve_default_preset(const egui_view_viewbox_snapshot_t *snapshot)
{
    uint8_t preset_count = viewbox_get_preset_count(snapshot);

    if (preset_count == 0)
    {
        return EGUI_VIEW_VIEWBOX_PRESET_NONE;
    }
    return snapshot->selected_preset < preset_count ? snapshot->selected_preset : 0;
}

static uint8_t viewbox_resolve_default_stretch_mode(const egui_view_viewbox_snapshot_t *snapshot)
{
    const egui_view_viewbox_preset_t *preset = viewbox_get_preset(snapshot, viewbox_resolve_default_preset(snapshot));

    if (preset == NULL)
    {
        return EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM;
    }
    return viewbox_clamp_stretch_mode(preset->stretch_mode);
}

static void viewbox_sync_current_preset(egui_view_viewbox_t *local)
{
    const egui_view_viewbox_snapshot_t *snapshot = viewbox_get_snapshot(local);
    const egui_view_viewbox_preset_t *preset;

    if (!viewbox_preset_exists(snapshot, local->current_preset))
    {
        uint8_t matched_preset = viewbox_find_preset_by_mode(snapshot, viewbox_clamp_stretch_mode(local->stretch_mode));

        local->current_preset = matched_preset != EGUI_VIEW_VIEWBOX_PRESET_NONE ? matched_preset : viewbox_resolve_default_preset(snapshot);
    }

    preset = viewbox_get_preset(snapshot, local->current_preset);
    local->stretch_mode = preset != NULL ? viewbox_clamp_stretch_mode(preset->stretch_mode) : EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM;
}

static uint8_t viewbox_preset_is_interactive(egui_view_viewbox_t *local, egui_view_t *self, const egui_view_viewbox_snapshot_t *snapshot,
                                             uint8_t preset_index)
{
    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        return 0;
    }
    return viewbox_preset_exists(snapshot, preset_index);
}

static egui_dim_t viewbox_pill_width(const char *text, uint8_t compact_mode, egui_dim_t min_width, egui_dim_t max_width)
{
    egui_dim_t width = min_width;

    if (viewbox_has_text(text))
    {
        width += viewbox_text_len(text) * (compact_mode ? 4 : 5);
    }
    if (width > max_width)
    {
        width = max_width;
    }
    return width;
}

static void viewbox_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                              egui_color_t color)
{
    egui_region_t draw_region = *region;

    if (font == NULL || !viewbox_has_text(text) || !viewbox_region_has_size(region))
    {
        return;
    }
    egui_canvas_draw_text_in_rect(font, text, &draw_region, align, color, self->alpha);
}

static void viewbox_compute_uniform_region(const egui_region_t *viewport_region, egui_dim_t source_width, egui_dim_t source_height,
                                           uint8_t allow_upscale, egui_region_t *result_region, uint16_t *scale_permille_out)
{
    int32_t scale_x;
    int32_t scale_y;
    int32_t scale;
    egui_dim_t content_width;
    egui_dim_t content_height;

    viewbox_reset_region(result_region);
    if (scale_permille_out != NULL)
    {
        *scale_permille_out = 0;
    }
    if (!viewbox_region_has_size(viewport_region) || source_width <= 0 || source_height <= 0)
    {
        return;
    }

    scale_x = ((int32_t)viewport_region->size.width * 1000) / source_width;
    scale_y = ((int32_t)viewport_region->size.height * 1000) / source_height;
    scale = scale_x < scale_y ? scale_x : scale_y;
    if (!allow_upscale && scale > 1000)
    {
        scale = 1000;
    }
    if (scale <= 0)
    {
        scale = 1;
    }

    content_width = (egui_dim_t)(((int32_t)source_width * scale) / 1000);
    content_height = (egui_dim_t)(((int32_t)source_height * scale) / 1000);
    if (content_width <= 0)
    {
        content_width = 1;
    }
    if (content_height <= 0)
    {
        content_height = 1;
    }
    if (content_width > viewport_region->size.width)
    {
        content_width = viewport_region->size.width;
    }
    if (content_height > viewport_region->size.height)
    {
        content_height = viewport_region->size.height;
    }

    result_region->location.x = viewport_region->location.x + (viewport_region->size.width - content_width) / 2;
    result_region->location.y = viewport_region->location.y + (viewport_region->size.height - content_height) / 2;
    result_region->size.width = content_width;
    result_region->size.height = content_height;
    if (scale_permille_out != NULL)
    {
        *scale_permille_out = (uint16_t)scale;
    }
}

static void viewbox_compute_scale_info(const egui_region_t *viewport_region, egui_dim_t source_width, egui_dim_t source_height, uint8_t stretch_mode,
                                       egui_view_viewbox_scale_info_t *scale_info)
{
    uint16_t uniform_scale = 0;

    viewbox_reset_region(&scale_info->guide_region);
    viewbox_reset_region(&scale_info->content_region);
    scale_info->scale_x_permille = 0;
    scale_info->scale_y_permille = 0;

    if (!viewbox_region_has_size(viewport_region))
    {
        return;
    }

    viewbox_compute_uniform_region(viewport_region, source_width, source_height, 1, &scale_info->guide_region, &uniform_scale);
    switch (stretch_mode)
    {
    case EGUI_VIEW_VIEWBOX_STRETCH_FILL:
        scale_info->content_region = *viewport_region;
        scale_info->scale_x_permille = (uint16_t)(((int32_t)viewport_region->size.width * 1000) / source_width);
        scale_info->scale_y_permille = (uint16_t)(((int32_t)viewport_region->size.height * 1000) / source_height);
        break;
    case EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY:
        viewbox_compute_uniform_region(viewport_region, source_width, source_height, 0, &scale_info->content_region, &scale_info->scale_x_permille);
        scale_info->scale_y_permille = scale_info->scale_x_permille;
        break;
    case EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM:
    default:
        scale_info->content_region = scale_info->guide_region;
        scale_info->scale_x_permille = uniform_scale;
        scale_info->scale_y_permille = uniform_scale;
        break;
    }
}

static void viewbox_get_metrics(egui_view_viewbox_t *local, egui_view_t *self, const egui_view_viewbox_snapshot_t *snapshot,
                                egui_view_viewbox_metrics_t *metrics)
{
    egui_region_t work_region;
    egui_dim_t outer_pad_x = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_OUTER_PAD_X : EGUI_VIEW_VIEWBOX_STANDARD_OUTER_PAD_X;
    egui_dim_t outer_pad_y = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_OUTER_PAD_Y : EGUI_VIEW_VIEWBOX_STANDARD_OUTER_PAD_Y;
    egui_dim_t inner_pad_x = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_INNER_PAD_X : EGUI_VIEW_VIEWBOX_STANDARD_INNER_PAD_X;
    egui_dim_t inner_pad_y = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_INNER_PAD_Y : EGUI_VIEW_VIEWBOX_STANDARD_INNER_PAD_Y;
    egui_dim_t badge_h = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_BADGE_H : EGUI_VIEW_VIEWBOX_STANDARD_BADGE_H;
    egui_dim_t badge_gap = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_BADGE_GAP : EGUI_VIEW_VIEWBOX_STANDARD_BADGE_GAP;
    egui_dim_t title_h = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_TITLE_H : EGUI_VIEW_VIEWBOX_STANDARD_TITLE_H;
    egui_dim_t summary_h = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_SUMMARY_H : EGUI_VIEW_VIEWBOX_STANDARD_SUMMARY_H;
    egui_dim_t title_gap = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_TITLE_GAP : EGUI_VIEW_VIEWBOX_STANDARD_TITLE_GAP;
    egui_dim_t viewport_gap = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_VIEWPORT_GAP : EGUI_VIEW_VIEWBOX_STANDARD_VIEWPORT_GAP;
    egui_dim_t preset_h = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_PRESET_H : EGUI_VIEW_VIEWBOX_STANDARD_PRESET_H;
    egui_dim_t preset_gap = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_PRESET_GAP : EGUI_VIEW_VIEWBOX_STANDARD_PRESET_GAP;
    egui_dim_t footer_h = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_FOOTER_H : EGUI_VIEW_VIEWBOX_STANDARD_FOOTER_H;
    egui_dim_t footer_gap = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_FOOTER_GAP : EGUI_VIEW_VIEWBOX_STANDARD_FOOTER_GAP;
    egui_dim_t inner_x;
    egui_dim_t inner_y;
    egui_dim_t inner_w;
    egui_dim_t inner_h;
    egui_dim_t cursor_y;
    egui_dim_t bottom_y;
    egui_dim_t available_w;
    egui_dim_t preset_w;
    egui_dim_t rem_w;
    uint8_t preset_count = viewbox_get_preset_count(snapshot);
    uint8_t preset_index;

    egui_view_get_work_region(self, &work_region);
    metrics->content_region.location.x = work_region.location.x + outer_pad_x;
    metrics->content_region.location.y = work_region.location.y + outer_pad_y;
    metrics->content_region.size.width = work_region.size.width - outer_pad_x * 2;
    metrics->content_region.size.height = work_region.size.height - outer_pad_y * 2;
    viewbox_reset_region(&metrics->badge_region);
    viewbox_reset_region(&metrics->title_region);
    viewbox_reset_region(&metrics->summary_region);
    viewbox_reset_region(&metrics->viewport_region);
    viewbox_reset_region(&metrics->footer_region);
    for (preset_index = 0; preset_index < EGUI_VIEW_VIEWBOX_MAX_PRESETS; ++preset_index)
    {
        viewbox_reset_region(&metrics->preset_regions[preset_index]);
    }

    if (!viewbox_region_has_size(&metrics->content_region) || snapshot == NULL)
    {
        return;
    }

    inner_x = metrics->content_region.location.x + inner_pad_x;
    inner_y = metrics->content_region.location.y + inner_pad_y;
    inner_w = metrics->content_region.size.width - inner_pad_x * 2;
    inner_h = metrics->content_region.size.height - inner_pad_y * 2;
    if (inner_w <= 0 || inner_h <= 0)
    {
        return;
    }

    cursor_y = inner_y;
    if (viewbox_has_text(snapshot->header))
    {
        metrics->badge_region.location.x = inner_x;
        metrics->badge_region.location.y = cursor_y;
        metrics->badge_region.size.width = viewbox_pill_width(snapshot->header, local->compact_mode, local->compact_mode ? 18 : 24, inner_w);
        metrics->badge_region.size.height = badge_h;
        cursor_y += badge_h + badge_gap;
    }

    metrics->title_region.location.x = inner_x;
    metrics->title_region.location.y = cursor_y;
    metrics->title_region.size.width = inner_w;
    metrics->title_region.size.height = title_h;
    cursor_y += title_h;

    if (summary_h > 0 && viewbox_has_text(snapshot->summary))
    {
        cursor_y += title_gap;
        metrics->summary_region.location.x = inner_x;
        metrics->summary_region.location.y = cursor_y;
        metrics->summary_region.size.width = inner_w;
        metrics->summary_region.size.height = summary_h;
        cursor_y += summary_h;
    }

    bottom_y = inner_y + inner_h;
    if (footer_h > 0 && viewbox_has_text(snapshot->footer))
    {
        bottom_y -= footer_h;
        metrics->footer_region.location.x = inner_x;
        metrics->footer_region.location.y = bottom_y;
        metrics->footer_region.size.width = viewbox_pill_width(snapshot->footer, local->compact_mode, local->compact_mode ? 22 : 30, inner_w);
        metrics->footer_region.size.height = footer_h;
        bottom_y -= footer_gap;
    }

    if (preset_count > 0)
    {
        bottom_y -= preset_h;
        if (inner_w > preset_gap * (preset_count - 1))
        {
            available_w = inner_w - preset_gap * (preset_count - 1);
            preset_w = available_w / preset_count;
            rem_w = available_w % preset_count;
            for (preset_index = 0; preset_index < preset_count; ++preset_index)
            {
                egui_dim_t width = preset_w + (preset_index < rem_w ? 1 : 0);

                metrics->preset_regions[preset_index].location.x = inner_x;
                if (preset_index > 0)
                {
                    metrics->preset_regions[preset_index].location.x =
                            metrics->preset_regions[preset_index - 1].location.x + metrics->preset_regions[preset_index - 1].size.width + preset_gap;
                }
                metrics->preset_regions[preset_index].location.y = bottom_y;
                metrics->preset_regions[preset_index].size.width = width;
                metrics->preset_regions[preset_index].size.height = preset_h;
            }
            bottom_y -= preset_gap;
        }
    }

    cursor_y += viewport_gap;
    metrics->viewport_region.location.x = inner_x;
    metrics->viewport_region.location.y = cursor_y;
    metrics->viewport_region.size.width = inner_w;
    metrics->viewport_region.size.height = bottom_y - cursor_y;
    if (metrics->viewport_region.size.height <= 0)
    {
        viewbox_reset_region(&metrics->viewport_region);
    }
}

static uint8_t viewbox_hit_preset(egui_view_viewbox_t *local, egui_view_t *self, egui_dim_t x, egui_dim_t y)
{
    const egui_view_viewbox_snapshot_t *snapshot = viewbox_get_snapshot(local);
    egui_view_viewbox_metrics_t metrics;
    uint8_t preset_count = viewbox_get_preset_count(snapshot);
    uint8_t preset_index;

    viewbox_get_metrics(local, self, snapshot, &metrics);
    for (preset_index = 0; preset_index < preset_count; ++preset_index)
    {
        if (egui_region_pt_in_rect(&metrics.preset_regions[preset_index], x, y))
        {
            return preset_index;
        }
    }
    return EGUI_VIEW_VIEWBOX_PRESET_NONE;
}

static void viewbox_set_current_snapshot_inner(egui_view_t *self, uint8_t snapshot_index, uint8_t invalidate_on_clear)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    uint8_t had_pressed = viewbox_clear_pressed_state(self, local);

    if (local->snapshot_count == 0 || snapshot_index >= local->snapshot_count)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }
    if (local->current_snapshot == snapshot_index)
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_snapshot = snapshot_index;
    local->current_preset = viewbox_resolve_default_preset(viewbox_get_snapshot(local));
    local->stretch_mode = viewbox_resolve_default_stretch_mode(viewbox_get_snapshot(local));
    egui_view_invalidate(self);
}

static void viewbox_set_current_preset_inner(egui_view_t *self, uint8_t preset_index, uint8_t invalidate_on_clear)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    const egui_view_viewbox_snapshot_t *snapshot = viewbox_get_snapshot(local);
    const egui_view_viewbox_preset_t *preset;
    uint8_t had_pressed = viewbox_clear_pressed_state(self, local);

    if (!viewbox_preset_exists(snapshot, preset_index))
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    preset = viewbox_get_preset(snapshot, preset_index);
    if (local->current_preset == preset_index && local->stretch_mode == viewbox_clamp_stretch_mode(preset->stretch_mode))
    {
        if (had_pressed && invalidate_on_clear)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->current_preset = preset_index;
    local->stretch_mode = viewbox_clamp_stretch_mode(preset->stretch_mode);
    egui_view_invalidate(self);
}

static void viewbox_draw_content_block(egui_view_t *self, egui_view_viewbox_t *local, const egui_view_viewbox_snapshot_t *snapshot,
                                       const egui_region_t *content_region)
{
    egui_color_t fill_color = egui_rgb_mix(local->surface_color, local->accent_color, 10);
    egui_color_t border_color = egui_rgb_mix(local->border_color, local->accent_color, 28);
    egui_color_t band_color = local->accent_color;
    egui_color_t title_color = egui_rgb_mix(local->text_color, local->accent_color, 12);
    egui_color_t meta_color = egui_rgb_mix(local->muted_text_color, local->accent_color, 16);
    egui_color_t line_color = egui_rgb_mix(local->section_color, local->accent_color, 36);
    egui_dim_t radius = local->compact_mode ? 5 : 6;
    egui_dim_t pad_x = local->compact_mode ? 4 : 6;
    egui_dim_t pad_y = local->compact_mode ? 3 : 4;
    egui_dim_t title_h = local->compact_mode ? 8 : 10;
    egui_dim_t meta_h = local->compact_mode ? 7 : 8;
    egui_dim_t footer_h = local->compact_mode ? 0 : 8;
    egui_dim_t bar_h = local->compact_mode ? 2 : 3;
    egui_region_t band_region;
    egui_region_t title_region;
    egui_region_t meta_region;
    egui_region_t footer_region;
    egui_region_t line_region;
    uint8_t line_index;

    if (!viewbox_region_has_size(content_region))
    {
        return;
    }
    if (local->stretch_mode == EGUI_VIEW_VIEWBOX_STRETCH_FILL)
    {
        fill_color = egui_rgb_mix(local->surface_color, local->accent_color, 14);
        border_color = egui_rgb_mix(local->border_color, local->accent_color, 34);
    }
    else if (local->stretch_mode == EGUI_VIEW_VIEWBOX_STRETCH_DOWNSCALE_ONLY)
    {
        fill_color = egui_rgb_mix(local->surface_color, local->accent_color, 7);
        border_color = egui_rgb_mix(local->border_color, local->accent_color, 20);
    }

    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, local->surface_color, 26);
        border_color = egui_rgb_mix(border_color, local->muted_text_color, 18);
        band_color = egui_rgb_mix(band_color, local->muted_text_color, 22);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 20);
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, 24);
        line_color = egui_rgb_mix(line_color, local->muted_text_color, 20);
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = viewbox_mix_disabled(fill_color);
        border_color = viewbox_mix_disabled(border_color);
        band_color = viewbox_mix_disabled(band_color);
        title_color = viewbox_mix_disabled(title_color);
        meta_color = viewbox_mix_disabled(meta_color);
        line_color = viewbox_mix_disabled(line_color);
    }

    egui_canvas_draw_round_rectangle_fill(content_region->location.x, content_region->location.y, content_region->size.width, content_region->size.height, radius,
                                          fill_color, egui_color_alpha_mix(self->alpha, 98));
    egui_canvas_draw_round_rectangle(content_region->location.x, content_region->location.y, content_region->size.width, content_region->size.height, radius, 1,
                                     border_color, egui_color_alpha_mix(self->alpha, 56));

    band_region.location.x = content_region->location.x + 1;
    band_region.location.y = content_region->location.y + 1;
    band_region.size.width = content_region->size.width - 2;
    band_region.size.height = local->compact_mode ? 3 : 4;
    if (band_region.size.width > 0)
    {
        egui_canvas_draw_round_rectangle_fill(band_region.location.x, band_region.location.y, band_region.size.width, band_region.size.height, radius, band_color,
                                              egui_color_alpha_mix(self->alpha, 92));
    }

    title_region.location.x = content_region->location.x + pad_x;
    title_region.location.y = content_region->location.y + pad_y + (local->compact_mode ? 1 : 2);
    title_region.size.width = content_region->size.width - pad_x * 2;
    title_region.size.height = title_h;

    meta_region = title_region;
    meta_region.location.y += title_h;
    meta_region.size.height = meta_h;

    viewbox_draw_text(local->font, self, snapshot->content_title, &title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
    if (!local->compact_mode)
    {
        viewbox_draw_text(local->meta_font, self, snapshot->content_meta, &meta_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color);
    }

    if (footer_h > 0 && viewbox_has_text(snapshot->content_footer))
    {
        footer_region.location.x = content_region->location.x + pad_x;
        footer_region.location.y = content_region->location.y + content_region->size.height - pad_y - footer_h;
        footer_region.size.width = content_region->size.width - pad_x * 2;
        footer_region.size.height = footer_h;
        viewbox_draw_text(local->meta_font, self, snapshot->content_footer, &footer_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, meta_color);
    }

    line_region.location.x = content_region->location.x + pad_x;
    line_region.location.y = content_region->location.y + content_region->size.height - pad_y - (local->compact_mode ? 9 : 16);
    line_region.size.height = bar_h;
    for (line_index = 0; line_index < 3; ++line_index)
    {
        egui_dim_t shrink = (egui_dim_t)(line_index * (local->compact_mode ? 5 : 7));

        line_region.size.width = content_region->size.width - pad_x * 2 - shrink;
        if (line_region.size.width <= 0)
        {
            continue;
        }
        egui_canvas_draw_round_rectangle_fill(line_region.location.x, line_region.location.y + line_index * (local->compact_mode ? 4 : 5), line_region.size.width,
                                              line_region.size.height, 1, line_color, egui_color_alpha_mix(self->alpha, 76));
    }
}

static void viewbox_draw_preset(egui_view_t *self, egui_view_viewbox_t *local, const egui_view_viewbox_preset_t *preset, const egui_region_t *region,
                                uint8_t preset_index)
{
    uint8_t selected = preset_index == local->current_preset;
    uint8_t pressed = preset_index == local->pressed_preset && self->is_pressed;
    egui_color_t fill_color = egui_rgb_mix(local->surface_color, local->accent_color, selected ? 12 : (pressed ? 8 : 4));
    egui_color_t border_color = egui_rgb_mix(local->border_color, local->accent_color, selected ? 30 : (pressed ? 22 : 10));
    egui_color_t label_color = egui_rgb_mix(local->text_color, local->accent_color, selected ? 18 : 4);
    egui_color_t meta_color = egui_rgb_mix(local->muted_text_color, local->accent_color, selected ? 18 : 8);
    egui_dim_t radius = region->size.height / 2;
    egui_region_t label_region = *region;
    egui_region_t meta_region = *region;

    if (!viewbox_region_has_size(region) || preset == NULL)
    {
        return;
    }

    if (local->read_only_mode)
    {
        fill_color = egui_rgb_mix(fill_color, local->surface_color, 28);
        border_color = egui_rgb_mix(border_color, local->muted_text_color, 20);
        label_color = egui_rgb_mix(label_color, local->muted_text_color, 22);
        meta_color = egui_rgb_mix(meta_color, local->muted_text_color, 24);
    }
    if (!egui_view_get_enable(self))
    {
        fill_color = viewbox_mix_disabled(fill_color);
        border_color = viewbox_mix_disabled(border_color);
        label_color = viewbox_mix_disabled(label_color);
        meta_color = viewbox_mix_disabled(meta_color);
    }

    egui_canvas_draw_round_rectangle_fill(region->location.x, region->location.y, region->size.width, region->size.height, radius, fill_color,
                                          egui_color_alpha_mix(self->alpha, 98));
    egui_canvas_draw_round_rectangle(region->location.x, region->location.y, region->size.width, region->size.height, radius, 1, border_color,
                                     egui_color_alpha_mix(self->alpha, 48));

    label_region.location.x += local->compact_mode ? 4 : 5;
    label_region.size.width -= local->compact_mode ? 8 : 10;
    if (label_region.size.width <= 0)
    {
        return;
    }

    if (local->compact_mode || !viewbox_has_text(preset->meta))
    {
        viewbox_draw_text(local->meta_font != NULL ? local->meta_font : local->font, self, preset->label, &label_region,
                          EGUI_ALIGN_CENTER | EGUI_ALIGN_VCENTER, label_color);
        return;
    }

    meta_region.location.x = region->location.x + 4;
    meta_region.size.width = region->size.width - 8;
    viewbox_draw_text(local->font, self, preset->label, &label_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, label_color);
    viewbox_draw_text(local->meta_font, self, preset->meta, &meta_region, EGUI_ALIGN_RIGHT | EGUI_ALIGN_VCENTER, meta_color);
}

static void egui_view_viewbox_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    const egui_view_viewbox_snapshot_t *snapshot = viewbox_get_snapshot(local);
    egui_view_viewbox_metrics_t metrics;
    egui_view_viewbox_scale_info_t scale_info;
    egui_color_t card_fill = local->surface_color;
    egui_color_t card_border = local->border_color;
    egui_color_t badge_fill = egui_rgb_mix(local->section_color, local->accent_color, 26);
    egui_color_t badge_text = egui_rgb_mix(local->text_color, local->accent_color, 28);
    egui_color_t title_color = local->text_color;
    egui_color_t summary_color = local->muted_text_color;
    egui_color_t viewport_fill = egui_rgb_mix(local->section_color, local->accent_color, 7);
    egui_color_t viewport_border = egui_rgb_mix(local->border_color, local->accent_color, 12);
    egui_color_t guide_border = egui_rgb_mix(local->muted_text_color, local->accent_color, 24);
    egui_color_t guide_fill = egui_rgb_mix(local->surface_color, local->accent_color, 2);
    egui_color_t footer_fill = egui_rgb_mix(local->surface_color, local->accent_color, 18);
    egui_color_t footer_border = egui_rgb_mix(local->border_color, local->accent_color, 20);
    egui_color_t footer_text = egui_rgb_mix(local->muted_text_color, local->accent_color, 18);
    egui_color_t detail_text = egui_rgb_mix(local->muted_text_color, local->accent_color, 12);
    egui_color_t focus_color = local->accent_color;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_RADIUS : EGUI_VIEW_VIEWBOX_STANDARD_RADIUS;
    egui_dim_t viewport_radius = local->compact_mode ? EGUI_VIEW_VIEWBOX_COMPACT_VIEWPORT_R : EGUI_VIEW_VIEWBOX_STANDARD_VIEWPORT_R;
    egui_dim_t source_width;
    egui_dim_t source_height;
    char source_text[24];
    char scale_text[24];
    egui_region_t source_region;
    egui_region_t scale_region;
    uint8_t preset_count;
    uint8_t preset_index;

    viewbox_get_metrics(local, self, snapshot, &metrics);
    if (!viewbox_region_has_size(&metrics.content_region))
    {
        return;
    }

    if (local->read_only_mode)
    {
        badge_fill = egui_rgb_mix(badge_fill, local->surface_color, 28);
        badge_text = egui_rgb_mix(badge_text, local->muted_text_color, 18);
        title_color = egui_rgb_mix(title_color, local->muted_text_color, 14);
        summary_color = egui_rgb_mix(summary_color, local->muted_text_color, 18);
        viewport_fill = egui_rgb_mix(viewport_fill, local->surface_color, 26);
        viewport_border = egui_rgb_mix(viewport_border, local->muted_text_color, 16);
        guide_border = egui_rgb_mix(guide_border, local->muted_text_color, 18);
        guide_fill = egui_rgb_mix(guide_fill, local->surface_color, 16);
        footer_fill = egui_rgb_mix(footer_fill, local->surface_color, 18);
        footer_border = egui_rgb_mix(footer_border, local->muted_text_color, 16);
        footer_text = egui_rgb_mix(footer_text, local->muted_text_color, 18);
        detail_text = egui_rgb_mix(detail_text, local->muted_text_color, 18);
        focus_color = egui_rgb_mix(focus_color, local->muted_text_color, 16);
    }
    if (!egui_view_get_enable(self))
    {
        card_fill = viewbox_mix_disabled(card_fill);
        card_border = viewbox_mix_disabled(card_border);
        badge_fill = viewbox_mix_disabled(badge_fill);
        badge_text = viewbox_mix_disabled(badge_text);
        title_color = viewbox_mix_disabled(title_color);
        summary_color = viewbox_mix_disabled(summary_color);
        viewport_fill = viewbox_mix_disabled(viewport_fill);
        viewport_border = viewbox_mix_disabled(viewport_border);
        guide_border = viewbox_mix_disabled(guide_border);
        guide_fill = viewbox_mix_disabled(guide_fill);
        footer_fill = viewbox_mix_disabled(footer_fill);
        footer_border = viewbox_mix_disabled(footer_border);
        footer_text = viewbox_mix_disabled(footer_text);
        detail_text = viewbox_mix_disabled(detail_text);
        focus_color = viewbox_mix_disabled(focus_color);
    }

    egui_canvas_draw_round_rectangle_fill(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                          metrics.content_region.size.height, radius, card_fill, egui_color_alpha_mix(self->alpha, 96));
    egui_canvas_draw_round_rectangle(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                     metrics.content_region.size.height, radius, 1, card_border, egui_color_alpha_mix(self->alpha, 54));

    if (self->is_focused && egui_view_get_enable(self) && !local->read_only_mode)
    {
        egui_canvas_draw_round_rectangle(metrics.content_region.location.x, metrics.content_region.location.y, metrics.content_region.size.width,
                                         metrics.content_region.size.height, radius, 2, focus_color, egui_color_alpha_mix(self->alpha, 56));
    }

    if (snapshot != NULL && viewbox_region_has_size(&metrics.badge_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.badge_region.location.x, metrics.badge_region.location.y, metrics.badge_region.size.width,
                                              metrics.badge_region.size.height, metrics.badge_region.size.height / 2, badge_fill,
                                              egui_color_alpha_mix(self->alpha, 98));
        viewbox_draw_text(local->meta_font, self, snapshot->header, &metrics.badge_region, EGUI_ALIGN_CENTER, badge_text);
    }

    if (snapshot != NULL)
    {
        viewbox_draw_text(local->font, self, snapshot->title, &metrics.title_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, title_color);
        viewbox_draw_text(local->meta_font, self, snapshot->summary, &metrics.summary_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, summary_color);
    }

    if (snapshot != NULL && viewbox_region_has_size(&metrics.viewport_region))
    {
        source_width = viewbox_resolve_source_dim(snapshot->source_width, metrics.viewport_region.size.width);
        source_height = viewbox_resolve_source_dim(snapshot->source_height, metrics.viewport_region.size.height);
        viewbox_compute_scale_info(&metrics.viewport_region, source_width, source_height, viewbox_clamp_stretch_mode(local->stretch_mode), &scale_info);

        egui_canvas_draw_round_rectangle_fill(metrics.viewport_region.location.x, metrics.viewport_region.location.y, metrics.viewport_region.size.width,
                                              metrics.viewport_region.size.height, viewport_radius, viewport_fill, egui_color_alpha_mix(self->alpha, 98));
        egui_canvas_draw_round_rectangle(metrics.viewport_region.location.x, metrics.viewport_region.location.y, metrics.viewport_region.size.width,
                                         metrics.viewport_region.size.height, viewport_radius, 1, viewport_border, egui_color_alpha_mix(self->alpha, 42));

        if (viewbox_region_has_size(&scale_info.guide_region))
        {
            egui_canvas_draw_round_rectangle_fill(scale_info.guide_region.location.x, scale_info.guide_region.location.y, scale_info.guide_region.size.width,
                                                  scale_info.guide_region.size.height, local->compact_mode ? 5 : 6, guide_fill,
                                                  egui_color_alpha_mix(self->alpha, 36));
            egui_canvas_draw_round_rectangle(scale_info.guide_region.location.x, scale_info.guide_region.location.y, scale_info.guide_region.size.width,
                                             scale_info.guide_region.size.height, local->compact_mode ? 5 : 6, 1, guide_border,
                                             egui_color_alpha_mix(self->alpha, 54));
        }
        if (viewbox_region_has_size(&scale_info.content_region))
        {
            viewbox_draw_content_block(self, local, snapshot, &scale_info.content_region);
        }

        snprintf(source_text, sizeof(source_text), "%dx%d", (int)source_width, (int)source_height);
        if (scale_info.scale_x_permille == scale_info.scale_y_permille)
        {
            snprintf(scale_text, sizeof(scale_text), "%u%%", (unsigned)(scale_info.scale_x_permille / 10));
        }
        else
        {
            snprintf(scale_text, sizeof(scale_text), "%u/%u%%", (unsigned)(scale_info.scale_x_permille / 10),
                     (unsigned)(scale_info.scale_y_permille / 10));
        }

        source_region = metrics.viewport_region;
        source_region.location.x += local->compact_mode ? 4 : 6;
        source_region.location.y += local->compact_mode ? 2 : 3;
        source_region.size.height = local->compact_mode ? 7 : 8;
        source_region.size.width -= local->compact_mode ? 8 : 12;
        scale_region = source_region;
        viewbox_draw_text(local->meta_font, self, source_text, &source_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_TOP, detail_text);
        viewbox_draw_text(local->meta_font, self, scale_text, &scale_region, EGUI_ALIGN_RIGHT | EGUI_ALIGN_TOP, detail_text);
    }

    preset_count = viewbox_get_preset_count(snapshot);
    for (preset_index = 0; preset_index < preset_count; ++preset_index)
    {
        viewbox_draw_preset(self, local, viewbox_get_preset(snapshot, preset_index), &metrics.preset_regions[preset_index], preset_index);
    }

    if (snapshot != NULL && viewbox_region_has_size(&metrics.footer_region))
    {
        egui_canvas_draw_round_rectangle_fill(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                              metrics.footer_region.size.height, metrics.footer_region.size.height / 2, footer_fill,
                                              egui_color_alpha_mix(self->alpha, 96));
        egui_canvas_draw_round_rectangle(metrics.footer_region.location.x, metrics.footer_region.location.y, metrics.footer_region.size.width,
                                         metrics.footer_region.size.height, metrics.footer_region.size.height / 2, 1, footer_border,
                                         egui_color_alpha_mix(self->alpha, 34));
        viewbox_draw_text(local->meta_font, self, snapshot->footer, &metrics.footer_region, EGUI_ALIGN_CENTER, footer_text);
    }
}

void egui_view_viewbox_set_snapshots(egui_view_t *self, const egui_view_viewbox_snapshot_t *snapshots, uint8_t snapshot_count)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);

    viewbox_clear_pressed_state(self, local);
    local->snapshots = snapshots;
    local->snapshot_count = snapshots == NULL ? 0 : viewbox_clamp_snapshot_count(snapshot_count);
    if (local->current_snapshot >= local->snapshot_count)
    {
        local->current_snapshot = 0;
    }
    local->current_preset = viewbox_resolve_default_preset(viewbox_get_snapshot(local));
    local->stretch_mode = viewbox_resolve_default_stretch_mode(viewbox_get_snapshot(local));
    egui_view_invalidate(self);
}

void egui_view_viewbox_set_current_snapshot(egui_view_t *self, uint8_t snapshot_index)
{
    viewbox_set_current_snapshot_inner(self, snapshot_index, 1);
}

uint8_t egui_view_viewbox_get_current_snapshot(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    return local->current_snapshot;
}

void egui_view_viewbox_set_current_preset(egui_view_t *self, uint8_t preset_index)
{
    viewbox_set_current_preset_inner(self, preset_index, 1);
}

uint8_t egui_view_viewbox_get_current_preset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);

    viewbox_sync_current_preset(local);
    return local->current_preset;
}

void egui_view_viewbox_set_stretch_mode(egui_view_t *self, uint8_t stretch_mode)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    const egui_view_viewbox_snapshot_t *snapshot = viewbox_get_snapshot(local);
    uint8_t next_mode = viewbox_clamp_stretch_mode(stretch_mode);
    uint8_t next_preset = viewbox_find_preset_by_mode(snapshot, next_mode);
    uint8_t had_pressed = viewbox_clear_pressed_state(self, local);

    if (snapshot != NULL && viewbox_get_preset_count(snapshot) > 0 && next_preset == EGUI_VIEW_VIEWBOX_PRESET_NONE)
    {
        next_preset = viewbox_resolve_default_preset(snapshot);
        next_mode = viewbox_resolve_default_stretch_mode(snapshot);
    }

    if (local->stretch_mode == next_mode && local->current_preset == next_preset)
    {
        if (had_pressed)
        {
            egui_view_invalidate(self);
        }
        return;
    }

    local->stretch_mode = next_mode;
    local->current_preset = next_preset;
    egui_view_invalidate(self);
}

uint8_t egui_view_viewbox_get_stretch_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);

    viewbox_sync_current_preset(local);
    return viewbox_clamp_stretch_mode(local->stretch_mode);
}

uint8_t egui_view_viewbox_activate_current_preset(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    const egui_view_viewbox_snapshot_t *snapshot = viewbox_get_snapshot(local);

    viewbox_sync_current_preset(local);
    if (!viewbox_preset_is_interactive(local, self, snapshot, local->current_preset))
    {
        return 0;
    }

    if (local->on_action != NULL)
    {
        local->on_action(self, local->current_snapshot, local->current_preset, viewbox_clamp_stretch_mode(local->stretch_mode));
    }
    egui_view_invalidate(self);
    return 1;
}

void egui_view_viewbox_set_on_action_listener(egui_view_t *self, egui_view_on_viewbox_action_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    local->on_action = listener;
}

void egui_view_viewbox_set_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);

    local->font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    viewbox_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_viewbox_set_meta_font(egui_view_t *self, const egui_font_t *font)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);

    local->meta_font = font != NULL ? font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    viewbox_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_viewbox_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);

    local->compact_mode = compact_mode ? 1 : 0;
    viewbox_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_viewbox_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);

    local->read_only_mode = read_only_mode ? 1 : 0;
    viewbox_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

void egui_view_viewbox_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t section_color, egui_color_t border_color,
                                   egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);

    local->surface_color = surface_color;
    local->section_color = section_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    viewbox_clear_pressed_state(self, local);
    egui_view_invalidate(self);
}

uint8_t egui_view_viewbox_get_preset_region(egui_view_t *self, uint8_t preset_index, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    const egui_view_viewbox_snapshot_t *snapshot = viewbox_get_snapshot(local);
    egui_view_viewbox_metrics_t metrics;

    if (region == NULL || !viewbox_preset_exists(snapshot, preset_index))
    {
        return 0;
    }

    viewbox_get_metrics(local, self, snapshot, &metrics);
    if (!viewbox_region_has_size(&metrics.preset_regions[preset_index]))
    {
        return 0;
    }
    *region = metrics.preset_regions[preset_index];
    return 1;
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_viewbox_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    const egui_view_viewbox_snapshot_t *snapshot = viewbox_get_snapshot(local);
    uint8_t hit_preset;

    if (snapshot == NULL || viewbox_get_preset_count(snapshot) == 0 || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (viewbox_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        hit_preset = viewbox_hit_preset(local, self, event->location.x, event->location.y);
        if (!viewbox_preset_is_interactive(local, self, snapshot, hit_preset))
        {
            if (viewbox_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->pressed_preset = hit_preset;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->pressed_preset == EGUI_VIEW_VIEWBOX_PRESET_NONE)
        {
            return 0;
        }
        hit_preset = viewbox_hit_preset(local, self, event->location.x, event->location.y);
        egui_view_set_pressed(self, hit_preset == local->pressed_preset);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
    {
        uint8_t handled = local->pressed_preset != EGUI_VIEW_VIEWBOX_PRESET_NONE ? 1 : 0;

        hit_preset = viewbox_hit_preset(local, self, event->location.x, event->location.y);
        if (hit_preset == local->pressed_preset && viewbox_preset_is_interactive(local, self, snapshot, hit_preset))
        {
            local->current_preset = hit_preset;
            local->stretch_mode = viewbox_clamp_stretch_mode(snapshot->presets[hit_preset].stretch_mode);
            egui_view_viewbox_activate_current_preset(self);
        }
        if (viewbox_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return handled || hit_preset != EGUI_VIEW_VIEWBOX_PRESET_NONE;
    }
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (viewbox_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_viewbox_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    EGUI_UNUSED(event);

    if (viewbox_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_viewbox_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    const egui_view_viewbox_snapshot_t *snapshot = viewbox_get_snapshot(local);
    uint8_t preset_count;
    uint8_t next_preset;
    uint8_t next_snapshot;

    if (snapshot == NULL || local->read_only_mode || !egui_view_get_enable(self))
    {
        if (viewbox_clear_pressed_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    preset_count = viewbox_get_preset_count(snapshot);
    if (preset_count == 0)
    {
        return 0;
    }

    viewbox_sync_current_preset(local);
    if (event->key_code == EGUI_KEY_CODE_ENTER || event->key_code == EGUI_KEY_CODE_SPACE)
    {
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            if (!viewbox_preset_is_interactive(local, self, snapshot, local->current_preset))
            {
                return 0;
            }
            local->pressed_preset = local->current_preset;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            uint8_t handled = 0;

            if (local->pressed_preset != EGUI_VIEW_VIEWBOX_PRESET_NONE && local->pressed_preset == local->current_preset &&
                viewbox_preset_is_interactive(local, self, snapshot, local->pressed_preset))
            {
                handled = egui_view_viewbox_activate_current_preset(self);
            }
            if (viewbox_clear_pressed_state(self, local))
            {
                egui_view_invalidate(self);
            }
            return handled;
        }
        return 0;
    }

    if (viewbox_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    if (event->type != EGUI_KEY_EVENT_ACTION_UP)
    {
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_LEFT:
        next_preset = local->current_preset > 0 ? (uint8_t)(local->current_preset - 1) : 0;
        viewbox_set_current_preset_inner(self, next_preset, 0);
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        next_preset = local->current_preset + 1 < preset_count ? (uint8_t)(local->current_preset + 1) : (uint8_t)(preset_count - 1);
        viewbox_set_current_preset_inner(self, next_preset, 0);
        return 1;
    case EGUI_KEY_CODE_HOME:
        viewbox_set_current_preset_inner(self, 0, 0);
        return 1;
    case EGUI_KEY_CODE_END:
        viewbox_set_current_preset_inner(self, (uint8_t)(preset_count - 1), 0);
        return 1;
    case EGUI_KEY_CODE_TAB:
        next_preset = (uint8_t)(local->current_preset + 1);
        if (next_preset < preset_count)
        {
            viewbox_set_current_preset_inner(self, next_preset, 0);
            return 1;
        }
        next_snapshot = (uint8_t)(local->current_snapshot + 1);
        if (next_snapshot >= local->snapshot_count)
        {
            next_snapshot = 0;
        }
        if (next_snapshot != local->current_snapshot)
        {
            local->current_snapshot = next_snapshot;
            local->current_preset = viewbox_resolve_default_preset(viewbox_get_snapshot(local));
            local->stretch_mode = viewbox_resolve_default_stretch_mode(viewbox_get_snapshot(local));
            egui_view_invalidate(self);
        }
        return 1;
    default:
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_viewbox_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_viewbox_t);
    EGUI_UNUSED(event);

    if (viewbox_clear_pressed_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_viewbox_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_viewbox_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_viewbox_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_viewbox_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_viewbox_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_viewbox_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_viewbox_on_key_event,
#endif
};

void egui_view_viewbox_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_viewbox_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_viewbox_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->snapshots = NULL;
    local->font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->on_action = NULL;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->section_color = EGUI_COLOR_HEX(0xF5F8FA);
    local->border_color = EGUI_COLOR_HEX(0xD4DDE6);
    local->text_color = EGUI_COLOR_HEX(0x1B2834);
    local->muted_text_color = EGUI_COLOR_HEX(0x6D7C8A);
    local->accent_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->snapshot_count = 0;
    local->current_snapshot = 0;
    local->current_preset = EGUI_VIEW_VIEWBOX_PRESET_NONE;
    local->stretch_mode = EGUI_VIEW_VIEWBOX_STRETCH_UNIFORM;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->pressed_preset = EGUI_VIEW_VIEWBOX_PRESET_NONE;

    egui_view_set_view_name(self, "egui_view_viewbox");
}
