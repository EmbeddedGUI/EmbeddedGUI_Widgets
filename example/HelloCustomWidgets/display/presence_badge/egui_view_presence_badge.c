#include "egui_view_presence_badge.h"

static uint8_t egui_view_presence_badge_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_presence_badge_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x83909D), 54);
}

static uint8_t egui_view_presence_badge_clamp_status(uint8_t status)
{
    if (status > EGUI_VIEW_PRESENCE_BADGE_STATUS_OFFLINE)
    {
        return EGUI_VIEW_PRESENCE_BADGE_STATUS_AVAILABLE;
    }
    return status;
}

static egui_color_t egui_view_presence_badge_resolve_status_color(egui_view_presence_badge_t *local, uint8_t status)
{
    switch (status)
    {
    case EGUI_VIEW_PRESENCE_BADGE_STATUS_BUSY:
        return local->busy_color;
    case EGUI_VIEW_PRESENCE_BADGE_STATUS_AWAY:
        return local->away_color;
    case EGUI_VIEW_PRESENCE_BADGE_STATUS_DO_NOT_DISTURB:
        return local->do_not_disturb_color;
    case EGUI_VIEW_PRESENCE_BADGE_STATUS_OFFLINE:
        return local->offline_color;
    default:
        return local->available_color;
    }
}

static void egui_view_presence_badge_local_region_to_screen(egui_view_t *self, const egui_region_t *local_region, egui_region_t *screen_region)
{
    screen_region->location.x = self->region_screen.location.x + local_region->location.x;
    screen_region->location.y = self->region_screen.location.y + local_region->location.y;
    screen_region->size.width = local_region->size.width;
    screen_region->size.height = local_region->size.height;
}

static void egui_view_presence_badge_get_regions(egui_view_presence_badge_t *local, egui_view_t *self, egui_region_t *outer_region, egui_region_t *mark_region)
{
    egui_region_t work_region;
    egui_dim_t badge_size;
    egui_dim_t outer_inset;
    egui_dim_t mark_inset;

    egui_view_get_work_region(self, &work_region);
    badge_size = EGUI_MIN(work_region.size.width, work_region.size.height);
    if (badge_size <= 0)
    {
        outer_region->location.x = 0;
        outer_region->location.y = 0;
        outer_region->size.width = 0;
        outer_region->size.height = 0;
        *mark_region = *outer_region;
        return;
    }

    outer_inset = local->compact_mode ? 0 : (badge_size >= 20 ? 2 : 1);
    if (badge_size - outer_inset * 2 <= 0)
    {
        outer_inset = 0;
    }
    badge_size -= outer_inset * 2;

    outer_region->size.width = badge_size;
    outer_region->size.height = badge_size;
    outer_region->location.x = work_region.location.x + (work_region.size.width - badge_size) / 2;
    outer_region->location.y = work_region.location.y + (work_region.size.height - badge_size) / 2;

    mark_inset = badge_size >= 18 ? 2 : 1;
    if (local->compact_mode && mark_inset > 1)
    {
        mark_inset = 1;
    }
    if (badge_size - mark_inset * 2 <= 0)
    {
        mark_inset = 0;
    }

    mark_region->location.x = outer_region->location.x + mark_inset;
    mark_region->location.y = outer_region->location.y + mark_inset;
    mark_region->size.width = outer_region->size.width - mark_inset * 2;
    mark_region->size.height = outer_region->size.height - mark_inset * 2;
}

static void egui_view_presence_badge_draw_do_not_disturb_glyph(egui_view_t *self, const egui_region_t *region, egui_color_t glyph_color)
{
    egui_dim_t center_y;
    egui_dim_t start_x;
    egui_dim_t end_x;
    egui_dim_t stroke_width;
    egui_dim_t horizontal_inset;

    if (region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }

    horizontal_inset = region->size.width / 4;
    if (horizontal_inset < 2)
    {
        horizontal_inset = 2;
    }
    if (horizontal_inset * 2 >= region->size.width)
    {
        horizontal_inset = region->size.width / 3;
    }

    start_x = region->location.x + horizontal_inset;
    end_x = region->location.x + region->size.width - horizontal_inset;
    center_y = region->location.y + region->size.height / 2;
    stroke_width = region->size.height >= 14 ? 2 : 1;

    egui_canvas_draw_line(start_x, center_y, end_x, center_y, stroke_width, glyph_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));
}

static void egui_view_presence_badge_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_presence_badge_t);
    egui_region_t outer_region;
    egui_region_t mark_region;
    egui_dim_t center_x;
    egui_dim_t center_y;
    egui_dim_t outer_radius;
    egui_dim_t mark_radius;
    egui_color_t surface_color = local->surface_color;
    egui_color_t outline_color = local->outline_color;
    egui_color_t status_color = egui_view_presence_badge_resolve_status_color(local, local->status);
    egui_color_t glyph_color = local->glyph_color;

    egui_view_presence_badge_get_regions(local, self, &outer_region, &mark_region);
    if (outer_region.size.width <= 0 || outer_region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF6F8FA), 28);
        outline_color = egui_rgb_mix(outline_color, local->muted_color, 34);
        status_color = egui_rgb_mix(status_color, local->muted_color, 46);
        glyph_color = egui_rgb_mix(glyph_color, local->muted_color, 42);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_presence_badge_mix_disabled(surface_color);
        outline_color = egui_view_presence_badge_mix_disabled(outline_color);
        status_color = egui_view_presence_badge_mix_disabled(status_color);
        glyph_color = egui_view_presence_badge_mix_disabled(glyph_color);
    }

    center_x = outer_region.location.x + outer_region.size.width / 2;
    center_y = outer_region.location.y + outer_region.size.height / 2;
    outer_radius = outer_region.size.width / 2;
    mark_radius = mark_region.size.width / 2;

    egui_canvas_draw_circle_fill_basic(center_x, center_y, outer_radius, surface_color, egui_color_alpha_mix(self->alpha, 96));

    if (local->status == EGUI_VIEW_PRESENCE_BADGE_STATUS_OFFLINE)
    {
        if (mark_radius > 0)
        {
            egui_canvas_draw_circle_basic(center_x, center_y, mark_radius, 1, status_color, egui_color_alpha_mix(self->alpha, 92));
            if (mark_radius > 2 && !local->compact_mode)
            {
                egui_canvas_draw_circle_basic(center_x, center_y, mark_radius - 1, 1, status_color, egui_color_alpha_mix(self->alpha, 40));
            }
        }
    }
    else
    {
        egui_canvas_draw_circle_fill_basic(center_x, center_y, mark_radius, status_color, egui_color_alpha_mix(self->alpha, 92));
        if (local->status == EGUI_VIEW_PRESENCE_BADGE_STATUS_DO_NOT_DISTURB)
        {
            egui_view_presence_badge_draw_do_not_disturb_glyph(self, &mark_region, glyph_color);
        }
    }

    egui_canvas_draw_circle_basic(center_x, center_y, outer_radius, 1, outline_color, egui_color_alpha_mix(self->alpha, local->compact_mode ? 22 : 30));
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_presence_badge_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_presence_badge_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_presence_badge_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_presence_badge_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_presence_badge_set_status(egui_view_t *self, uint8_t status)
{
    EGUI_LOCAL_INIT(egui_view_presence_badge_t);

    egui_view_presence_badge_clear_pressed_state(self);
    local->status = egui_view_presence_badge_clamp_status(status);
    egui_view_invalidate(self);
}

uint8_t egui_view_presence_badge_get_status(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_presence_badge_t);

    return local->status;
}

void egui_view_presence_badge_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t outline_color, egui_color_t available_color,
                                          egui_color_t busy_color, egui_color_t away_color, egui_color_t do_not_disturb_color,
                                          egui_color_t offline_color, egui_color_t glyph_color, egui_color_t muted_color)
{
    EGUI_LOCAL_INIT(egui_view_presence_badge_t);

    egui_view_presence_badge_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->outline_color = outline_color;
    local->available_color = available_color;
    local->busy_color = busy_color;
    local->away_color = away_color;
    local->do_not_disturb_color = do_not_disturb_color;
    local->offline_color = offline_color;
    local->glyph_color = glyph_color;
    local->muted_color = muted_color;
    egui_view_invalidate(self);
}

void egui_view_presence_badge_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_presence_badge_t);

    egui_view_presence_badge_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_presence_badge_get_compact_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_presence_badge_t);

    return local->compact_mode;
}

void egui_view_presence_badge_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_presence_badge_t);

    egui_view_presence_badge_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_presence_badge_get_read_only_mode(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_presence_badge_t);

    return local->read_only_mode;
}

uint8_t egui_view_presence_badge_get_indicator_region(egui_view_t *self, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_presence_badge_t);
    egui_region_t outer_region;
    egui_region_t mark_region;

    if (region == NULL)
    {
        return 0;
    }

    egui_view_presence_badge_get_regions(local, self, &outer_region, &mark_region);
    if (outer_region.size.width <= 0 || outer_region.size.height <= 0)
    {
        return 0;
    }

    egui_view_presence_badge_local_region_to_screen(self, &outer_region, region);
    return 1;
}

void egui_view_presence_badge_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_presence_badge_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_presence_badge_on_static_key_event;
#endif
}

static egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_presence_badge_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_presence_badge_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_presence_badge_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_presence_badge_t);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_presence_badge_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    egui_view_set_focusable(self, 0);

    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->outline_color = EGUI_COLOR_HEX(0xD5DEE6);
    local->available_color = EGUI_COLOR_HEX(0x107C41);
    local->busy_color = EGUI_COLOR_HEX(0xC4314B);
    local->away_color = EGUI_COLOR_HEX(0xC17C00);
    local->do_not_disturb_color = EGUI_COLOR_HEX(0xC4314B);
    local->offline_color = EGUI_COLOR_HEX(0x7A8796);
    local->glyph_color = EGUI_COLOR_WHITE;
    local->muted_color = EGUI_COLOR_HEX(0x6B7A89);
    local->status = EGUI_VIEW_PRESENCE_BADGE_STATUS_AVAILABLE;
    local->compact_mode = 0;
    local->read_only_mode = 0;

    egui_view_set_view_name(self, "egui_view_presence_badge");
}
