#include "egui_view_path_icon.h"

#define EGUI_VIEW_PATH_ICON_MAX_POINTS     16
#define EGUI_VIEW_PATH_ICON_MAX_RAW_POINTS 48
#define EGUI_VIEW_PATH_ICON_SCALE_Q10      1024
#define EGUI_VIEW_PATH_ICON_QUAD_STEPS     2
#define EGUI_VIEW_PATH_ICON_CUBIC_STEPS    3

typedef struct egui_view_path_icon_point egui_view_path_icon_point_t;
struct egui_view_path_icon_point
{
    egui_dim_t x;
    egui_dim_t y;
};

typedef struct egui_view_path_icon_transform egui_view_path_icon_transform_t;
struct egui_view_path_icon_transform
{
    egui_dim_t offset_x;
    egui_dim_t offset_y;
    int32_t scale_q10;
};

static const egui_view_path_icon_command_t egui_view_path_icon_bookmark_commands[] = {
        { EGUI_VIEW_PATH_ICON_COMMAND_MOVE_TO, 12, 8, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO, 36, 8, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO, 36, 40, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO, 24, 30, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO, 12, 40, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_CLOSE, 0, 0, 0, 0, 0, 0 },
};

static const egui_view_path_icon_command_t egui_view_path_icon_heart_commands[] = {
        { EGUI_VIEW_PATH_ICON_COMMAND_MOVE_TO, 24, 38, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_QUAD_TO, 12, 30, 12, 18, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_QUAD_TO, 12, 10, 20, 10, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_QUAD_TO, 24, 10, 24, 16, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_QUAD_TO, 24, 10, 28, 10, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_QUAD_TO, 36, 10, 36, 18, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_QUAD_TO, 36, 30, 24, 38, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_CLOSE, 0, 0, 0, 0, 0, 0 },
};

static const egui_view_path_icon_command_t egui_view_path_icon_send_commands[] = {
        { EGUI_VIEW_PATH_ICON_COMMAND_MOVE_TO, 10, 24, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO, 40, 12, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO, 30, 36, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO, 24, 29, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO, 18, 34, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO, 20, 26, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_ICON_COMMAND_CLOSE, 0, 0, 0, 0, 0, 0 },
};

static const egui_view_path_icon_data_t egui_view_path_icon_bookmark_data = {
        .viewport_width = 48,
        .viewport_height = 48,
        .command_count = EGUI_ARRAY_SIZE(egui_view_path_icon_bookmark_commands),
        .commands = egui_view_path_icon_bookmark_commands,
};

static const egui_view_path_icon_data_t egui_view_path_icon_heart_data = {
        .viewport_width = 48,
        .viewport_height = 48,
        .command_count = EGUI_ARRAY_SIZE(egui_view_path_icon_heart_commands),
        .commands = egui_view_path_icon_heart_commands,
};

static const egui_view_path_icon_data_t egui_view_path_icon_send_data = {
        .viewport_width = 48,
        .viewport_height = 48,
        .command_count = EGUI_ARRAY_SIZE(egui_view_path_icon_send_commands),
        .commands = egui_view_path_icon_send_commands,
};

static egui_view_path_icon_t *egui_view_path_icon_local(egui_view_t *self)
{
    return (egui_view_path_icon_t *)self;
}

static const egui_view_path_icon_data_t *egui_view_path_icon_default_data(void)
{
    return &egui_view_path_icon_bookmark_data;
}

static uint8_t egui_view_path_icon_data_is_valid(const egui_view_path_icon_data_t *data)
{
    if (data == NULL)
    {
        return 0;
    }

    return (data->viewport_width > 0 && data->viewport_height > 0 && data->command_count > 0 && data->commands != NULL) ? 1 : 0;
}

static uint8_t egui_view_path_icon_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static void egui_view_path_icon_apply_color(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_path_icon_t *local = egui_view_path_icon_local(self);

    egui_view_path_icon_clear_pressed_state(self);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
    local->icon_color = icon_color;
    egui_view_invalidate(self);
}

void egui_view_path_icon_apply_standard_style(egui_view_t *self)
{
    egui_view_path_icon_apply_color(self, EGUI_COLOR_HEX(0x0F6CBD));
}

void egui_view_path_icon_apply_subtle_style(egui_view_t *self)
{
    egui_view_path_icon_apply_color(self, EGUI_COLOR_HEX(0x6F7C8A));
}

void egui_view_path_icon_apply_accent_style(egui_view_t *self)
{
    egui_view_path_icon_apply_color(self, EGUI_COLOR_HEX(0xA15C00));
}

void egui_view_path_icon_set_data(egui_view_t *self, const egui_view_path_icon_data_t *data)
{
    egui_view_path_icon_t *local = egui_view_path_icon_local(self);

    egui_view_path_icon_clear_pressed_state(self);
    local->data = egui_view_path_icon_data_is_valid(data) ? data : egui_view_path_icon_default_data();
    egui_view_invalidate(self);
}

const egui_view_path_icon_data_t *egui_view_path_icon_get_data(egui_view_t *self)
{
    egui_view_path_icon_t *local = egui_view_path_icon_local(self);

    if (!egui_view_path_icon_data_is_valid(local->data))
    {
        local->data = egui_view_path_icon_default_data();
    }
    return local->data;
}

const egui_view_path_icon_data_t *egui_view_path_icon_get_bookmark_data(void)
{
    return &egui_view_path_icon_bookmark_data;
}

const egui_view_path_icon_data_t *egui_view_path_icon_get_heart_data(void)
{
    return &egui_view_path_icon_heart_data;
}

const egui_view_path_icon_data_t *egui_view_path_icon_get_send_data(void)
{
    return &egui_view_path_icon_send_data;
}

void egui_view_path_icon_set_palette(egui_view_t *self, egui_color_t icon_color)
{
    egui_view_path_icon_apply_color(self, icon_color);
}

static int16_t egui_view_path_icon_eval_quad_component(int16_t p0, int16_t c0, int16_t p1, int step, int total_steps)
{
    int32_t omt = total_steps - step;
    int32_t den = total_steps * total_steps;
    int32_t value = omt * omt * p0 + 2 * omt * step * c0 + step * step * p1;

    return (int16_t)((value + den / 2) / den);
}

static int16_t egui_view_path_icon_eval_cubic_component(int16_t p0, int16_t c0, int16_t c1, int16_t p1, int step, int total_steps)
{
    int32_t omt = total_steps - step;
    int32_t den = total_steps * total_steps * total_steps;
    int32_t value = omt * omt * omt * p0 + 3 * omt * omt * step * c0 + 3 * omt * step * step * c1 + step * step * step * p1;

    return (int16_t)((value + den / 2) / den);
}

static egui_view_path_icon_point_t egui_view_path_icon_transform_point(const egui_view_path_icon_transform_t *transform, int16_t x, int16_t y)
{
    egui_view_path_icon_point_t point;

    point.x = transform->offset_x + (egui_dim_t)(((int32_t)x * transform->scale_q10 + EGUI_VIEW_PATH_ICON_SCALE_Q10 / 2) / EGUI_VIEW_PATH_ICON_SCALE_Q10);
    point.y = transform->offset_y + (egui_dim_t)(((int32_t)y * transform->scale_q10 + EGUI_VIEW_PATH_ICON_SCALE_Q10 / 2) / EGUI_VIEW_PATH_ICON_SCALE_Q10);
    return point;
}

static void egui_view_path_icon_append_point(egui_view_path_icon_point_t *points, uint8_t *count, egui_view_path_icon_point_t point)
{
    if (*count > 0)
    {
        egui_view_path_icon_point_t *last = &points[*count - 1];

        if (last->x == point.x && last->y == point.y)
        {
            return;
        }
    }

    if (*count < EGUI_VIEW_PATH_ICON_MAX_RAW_POINTS)
    {
        points[*count] = point;
        (*count)++;
    }
}

static uint8_t egui_view_path_icon_compact_points(const egui_view_path_icon_point_t *src, uint8_t src_count, egui_dim_t *dst, uint8_t max_points)
{
    int previous_index = -1;
    uint8_t count = 0;

    if (src_count == 0 || max_points == 0)
    {
        return 0;
    }

    if (src_count <= max_points)
    {
        for (uint8_t i = 0; i < src_count; ++i)
        {
            dst[i * 2] = src[i].x;
            dst[i * 2 + 1] = src[i].y;
        }
        return src_count;
    }

    for (uint8_t i = 0; i < max_points; ++i)
    {
        int index = (i * src_count) / max_points;

        if (index <= previous_index)
        {
            index = previous_index + 1;
        }
        if (index >= src_count)
        {
            index = src_count - 1;
        }

        dst[count * 2] = src[index].x;
        dst[count * 2 + 1] = src[index].y;
        previous_index = index;
        count++;
    }

    return count;
}

static void egui_view_path_icon_draw_contour(const egui_view_path_icon_point_t *points, uint8_t count, uint8_t closed, egui_dim_t stroke_width,
                                             egui_color_t color, egui_alpha_t alpha)
{
    egui_dim_t flattened[EGUI_VIEW_PATH_ICON_MAX_POINTS * 2];

    if (count == 0)
    {
        return;
    }

    if (count >= 2 && points[0].x == points[count - 1].x && points[0].y == points[count - 1].y)
    {
        count--;
        closed = 1;
    }

    count = egui_view_path_icon_compact_points(points, count, flattened, EGUI_VIEW_PATH_ICON_MAX_POINTS);

    if (closed)
    {
        if (count >= 3)
        {
            egui_canvas_draw_polygon_fill(flattened, count, color, alpha);
        }
    }
    else if (count >= 2)
    {
        egui_canvas_draw_polyline_round_cap_hq(flattened, count, stroke_width, color, alpha);
    }
}

static void egui_view_path_icon_flush_contour(egui_view_path_icon_point_t *points, uint8_t *count, uint8_t closed, egui_dim_t stroke_width, egui_color_t color,
                                              egui_alpha_t alpha)
{
    egui_view_path_icon_draw_contour(points, *count, closed, stroke_width, color, alpha);
    *count = 0;
}

static void egui_view_path_icon_prepare_transform(const egui_region_t *region, const egui_view_path_icon_data_t *data,
                                                  egui_view_path_icon_transform_t *transform)
{
    egui_dim_t draw_width;
    egui_dim_t draw_height;
    int32_t scale_x_q10 = ((int32_t)region->size.width * EGUI_VIEW_PATH_ICON_SCALE_Q10) / data->viewport_width;
    int32_t scale_y_q10 = ((int32_t)region->size.height * EGUI_VIEW_PATH_ICON_SCALE_Q10) / data->viewport_height;

    transform->scale_q10 = EGUI_MIN(scale_x_q10, scale_y_q10);
    if (transform->scale_q10 <= 0)
    {
        transform->scale_q10 = 1;
    }

    draw_width = (egui_dim_t)(((int32_t)data->viewport_width * transform->scale_q10 + EGUI_VIEW_PATH_ICON_SCALE_Q10 / 2) / EGUI_VIEW_PATH_ICON_SCALE_Q10);
    draw_height = (egui_dim_t)(((int32_t)data->viewport_height * transform->scale_q10 + EGUI_VIEW_PATH_ICON_SCALE_Q10 / 2) / EGUI_VIEW_PATH_ICON_SCALE_Q10);

    transform->offset_x = region->location.x + (region->size.width - draw_width) / 2;
    transform->offset_y = region->location.y + (region->size.height - draw_height) / 2;
}

static void egui_view_path_icon_on_draw(egui_view_t *self)
{
    egui_view_path_icon_t *local = egui_view_path_icon_local(self);
    const egui_view_path_icon_data_t *data = egui_view_path_icon_get_data(self);
    egui_view_path_icon_transform_t transform;
    egui_view_path_icon_point_t contour_points[EGUI_VIEW_PATH_ICON_MAX_RAW_POINTS];
    egui_region_t region;
    egui_color_t icon_color = local->icon_color;
    egui_dim_t stroke_width;
    uint8_t contour_count = 0;
    uint8_t contour_closed = 0;
    uint8_t has_current = 0;
    int16_t current_x = 0;
    int16_t current_y = 0;

    if (!egui_view_path_icon_data_is_valid(data))
    {
        return;
    }

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    egui_view_path_icon_prepare_transform(&region, data, &transform);
    stroke_width = EGUI_MAX(1, EGUI_MIN(region.size.width, region.size.height) / 14);

    if (!egui_view_get_enable(self))
    {
        icon_color = egui_rgb_mix(icon_color, EGUI_COLOR_HEX(0x97A4B1), 58);
    }

    for (uint8_t i = 0; i < data->command_count; ++i)
    {
        const egui_view_path_icon_command_t *command = &data->commands[i];

        switch (command->type)
        {
        case EGUI_VIEW_PATH_ICON_COMMAND_MOVE_TO:
            if (contour_count > 0)
            {
                egui_view_path_icon_flush_contour(contour_points, &contour_count, contour_closed, stroke_width, icon_color, self->alpha);
            }
            contour_closed = 0;
            current_x = command->x1;
            current_y = command->y1;
            has_current = 1;
            egui_view_path_icon_append_point(contour_points, &contour_count, egui_view_path_icon_transform_point(&transform, current_x, current_y));
            break;

        case EGUI_VIEW_PATH_ICON_COMMAND_LINE_TO:
            if (!has_current)
            {
                current_x = command->x1;
                current_y = command->y1;
                has_current = 1;
                egui_view_path_icon_append_point(contour_points, &contour_count, egui_view_path_icon_transform_point(&transform, current_x, current_y));
                break;
            }

            current_x = command->x1;
            current_y = command->y1;
            egui_view_path_icon_append_point(contour_points, &contour_count, egui_view_path_icon_transform_point(&transform, current_x, current_y));
            break;

        case EGUI_VIEW_PATH_ICON_COMMAND_QUAD_TO:
            if (!has_current)
            {
                break;
            }

            for (int step = 1; step <= EGUI_VIEW_PATH_ICON_QUAD_STEPS; ++step)
            {
                int16_t sample_x = egui_view_path_icon_eval_quad_component(current_x, command->x1, command->x2, step, EGUI_VIEW_PATH_ICON_QUAD_STEPS);
                int16_t sample_y = egui_view_path_icon_eval_quad_component(current_y, command->y1, command->y2, step, EGUI_VIEW_PATH_ICON_QUAD_STEPS);

                egui_view_path_icon_append_point(contour_points, &contour_count, egui_view_path_icon_transform_point(&transform, sample_x, sample_y));
            }
            current_x = command->x2;
            current_y = command->y2;
            break;

        case EGUI_VIEW_PATH_ICON_COMMAND_CUBIC_TO:
            if (!has_current)
            {
                break;
            }

            for (int step = 1; step <= EGUI_VIEW_PATH_ICON_CUBIC_STEPS; ++step)
            {
                int16_t sample_x = egui_view_path_icon_eval_cubic_component(current_x, command->x1, command->x2, command->x3, step,
                                                                            EGUI_VIEW_PATH_ICON_CUBIC_STEPS);
                int16_t sample_y = egui_view_path_icon_eval_cubic_component(current_y, command->y1, command->y2, command->y3, step,
                                                                            EGUI_VIEW_PATH_ICON_CUBIC_STEPS);

                egui_view_path_icon_append_point(contour_points, &contour_count, egui_view_path_icon_transform_point(&transform, sample_x, sample_y));
            }
            current_x = command->x3;
            current_y = command->y3;
            break;

        case EGUI_VIEW_PATH_ICON_COMMAND_CLOSE:
            contour_closed = 1;
            egui_view_path_icon_flush_contour(contour_points, &contour_count, contour_closed, stroke_width, icon_color, self->alpha);
            contour_closed = 0;
            has_current = 0;
            break;

        default:
            break;
        }
    }

    if (contour_count > 0)
    {
        egui_view_path_icon_flush_contour(contour_points, &contour_count, contour_closed, stroke_width, icon_color, self->alpha);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_path_icon_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_path_icon_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_path_icon_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_path_icon_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_path_icon_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_path_icon_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_path_icon_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_path_icon_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_path_icon_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_path_icon_init(egui_view_t *self)
{
    egui_view_path_icon_t *local = egui_view_path_icon_local(self);

    egui_view_init(self);
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_path_icon_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->data = egui_view_path_icon_default_data();
    local->icon_color = EGUI_COLOR_HEX(0x0F6CBD);

    egui_view_set_view_name(self, "egui_view_path_icon");
}
