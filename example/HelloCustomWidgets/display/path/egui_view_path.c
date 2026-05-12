#include "egui_view_path.h"

#define EGUI_VIEW_PATH_MAX_POINTS      24
#define EGUI_VIEW_PATH_MAX_RAW_POINTS  64
#define EGUI_VIEW_PATH_SCALE_Q10       1024
#define EGUI_VIEW_PATH_QUAD_STEPS      4
#define EGUI_VIEW_PATH_CUBIC_STEPS     6
#define EGUI_VIEW_PATH_STROKE_MIN      0
#define EGUI_VIEW_PATH_STROKE_MAX      6

typedef struct egui_view_path_point egui_view_path_point_t;
struct egui_view_path_point
{
    egui_dim_t x;
    egui_dim_t y;
};

typedef struct egui_view_path_transform egui_view_path_transform_t;
struct egui_view_path_transform
{
    egui_dim_t offset_x;
    egui_dim_t offset_y;
    int32_t scale_q10;
};

static const egui_view_path_command_t path_shield_commands[] = {
        { EGUI_VIEW_PATH_COMMAND_MOVE_TO, 50, 8, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_QUAD_TO, 80, 8, 88, 38, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_QUAD_TO, 88, 68, 50, 90, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_QUAD_TO, 12, 68, 12, 38, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_QUAD_TO, 20, 8, 50, 8, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_CLOSE, 0, 0, 0, 0, 0, 0 },
};

static const egui_view_path_command_t path_curve_commands[] = {
        { EGUI_VIEW_PATH_COMMAND_MOVE_TO, 8, 76, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_CUBIC_TO, 24, 18, 42, 18, 52, 48 },
        { EGUI_VIEW_PATH_COMMAND_CUBIC_TO, 62, 78, 78, 62, 92, 18 },
};

static const egui_view_path_command_t path_line_commands[] = {
        { EGUI_VIEW_PATH_COMMAND_MOVE_TO, 10, 70, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_LINE_TO, 34, 36, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_LINE_TO, 58, 52, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_LINE_TO, 86, 22, 0, 0, 0, 0 },
};

static const egui_view_path_command_t path_bookmark_commands[] = {
        { EGUI_VIEW_PATH_COMMAND_MOVE_TO, 24, 10, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_LINE_TO, 76, 10, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_LINE_TO, 76, 84, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_LINE_TO, 50, 64, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_LINE_TO, 24, 84, 0, 0, 0, 0 },
        { EGUI_VIEW_PATH_COMMAND_CLOSE, 0, 0, 0, 0, 0, 0 },
};

static const egui_view_path_data_t path_shield_data = {
        .viewport_width = 100,
        .viewport_height = 100,
        .command_count = EGUI_ARRAY_SIZE(path_shield_commands),
        .commands = path_shield_commands,
};

static const egui_view_path_data_t path_curve_data = {
        .viewport_width = 100,
        .viewport_height = 100,
        .command_count = EGUI_ARRAY_SIZE(path_curve_commands),
        .commands = path_curve_commands,
};

static const egui_view_path_data_t path_line_data = {
        .viewport_width = 100,
        .viewport_height = 100,
        .command_count = EGUI_ARRAY_SIZE(path_line_commands),
        .commands = path_line_commands,
};

static const egui_view_path_data_t path_bookmark_data = {
        .viewport_width = 100,
        .viewport_height = 100,
        .command_count = EGUI_ARRAY_SIZE(path_bookmark_commands),
        .commands = path_bookmark_commands,
};

static egui_view_path_t *egui_view_path_local(egui_view_t *self)
{
    return (egui_view_path_t *)self;
}

static const egui_view_path_data_t *egui_view_path_default_data(void)
{
    return &path_shield_data;
}

static uint8_t egui_view_path_data_is_valid(const egui_view_path_data_t *data)
{
    if (data == NULL)
    {
        return 0;
    }
    return (data->viewport_width > 0 && data->viewport_height > 0 && data->command_count > 0 && data->commands != NULL) ? 1 : 0;
}

static uint8_t egui_view_path_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_dim_t egui_view_path_clamp_dim(egui_dim_t value, egui_dim_t min_value, egui_dim_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static egui_color_t egui_view_path_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_TEXT_SOFT, EGUI_ALPHA_MAKE(40));
}

static int16_t egui_view_path_eval_quad_component(int16_t p0, int16_t c0, int16_t p1, int step, int total_steps)
{
    int32_t omt = total_steps - step;
    int32_t den = total_steps * total_steps;
    int32_t value = omt * omt * p0 + 2 * omt * step * c0 + step * step * p1;

    return (int16_t)((value + den / 2) / den);
}

static int16_t egui_view_path_eval_cubic_component(int16_t p0, int16_t c0, int16_t c1, int16_t p1, int step, int total_steps)
{
    int32_t omt = total_steps - step;
    int32_t den = total_steps * total_steps * total_steps;
    int32_t value = omt * omt * omt * p0 + 3 * omt * omt * step * c0 + 3 * omt * step * step * c1 + step * step * step * p1;

    return (int16_t)((value + den / 2) / den);
}

static egui_view_path_point_t egui_view_path_transform_point(const egui_view_path_transform_t *transform, int16_t x, int16_t y)
{
    egui_view_path_point_t point;

    point.x = transform->offset_x + (egui_dim_t)(((int32_t)x * transform->scale_q10 + EGUI_VIEW_PATH_SCALE_Q10 / 2) / EGUI_VIEW_PATH_SCALE_Q10);
    point.y = transform->offset_y + (egui_dim_t)(((int32_t)y * transform->scale_q10 + EGUI_VIEW_PATH_SCALE_Q10 / 2) / EGUI_VIEW_PATH_SCALE_Q10);
    return point;
}

static void egui_view_path_append_point(egui_view_path_point_t *points, uint8_t *count, egui_view_path_point_t point)
{
    if (*count > 0)
    {
        egui_view_path_point_t *last = &points[*count - 1];

        if (last->x == point.x && last->y == point.y)
        {
            return;
        }
    }

    if (*count < EGUI_VIEW_PATH_MAX_RAW_POINTS)
    {
        points[*count] = point;
        (*count)++;
    }
}

static uint8_t egui_view_path_compact_points(const egui_view_path_point_t *src, uint8_t src_count, egui_dim_t *dst, uint8_t max_points)
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

static void egui_view_path_draw_contour(const egui_view_path_point_t *points, uint8_t count, uint8_t closed, egui_dim_t stroke_width,
                                        egui_color_t fill_color, egui_color_t stroke_color, egui_alpha_t fill_alpha, egui_alpha_t stroke_alpha)
{
    egui_dim_t flattened[EGUI_VIEW_PATH_MAX_POINTS * 2];

    if (count == 0)
    {
        return;
    }

    if (count >= 2 && points[0].x == points[count - 1].x && points[0].y == points[count - 1].y)
    {
        count--;
        closed = 1;
    }

    count = egui_view_path_compact_points(points, count, flattened, EGUI_VIEW_PATH_MAX_POINTS);
    if (closed)
    {
        if (count >= 3 && fill_alpha > 0)
        {
            egui_canvas_draw_polygon_fill(&uicode_get_core()->canvas, flattened, count, fill_color, fill_alpha);
        }
        if (count >= 3 && stroke_width > 0 && stroke_alpha > 0)
        {
            egui_canvas_draw_polygon(&uicode_get_core()->canvas, flattened, count, stroke_width, stroke_color, stroke_alpha);
        }
    }
    else if (count >= 2 && stroke_width > 0 && stroke_alpha > 0)
    {
        egui_canvas_draw_polyline_round_cap_hq(&uicode_get_core()->canvas, flattened, count, stroke_width, stroke_color, stroke_alpha);
    }
}

static void egui_view_path_flush_contour(egui_view_path_point_t *points, uint8_t *count, uint8_t closed, egui_dim_t stroke_width,
                                         egui_color_t fill_color, egui_color_t stroke_color, egui_alpha_t fill_alpha, egui_alpha_t stroke_alpha)
{
    egui_view_path_draw_contour(points, *count, closed, stroke_width, fill_color, stroke_color, fill_alpha, stroke_alpha);
    *count = 0;
}

static void egui_view_path_prepare_transform(const egui_region_t *region, const egui_view_path_data_t *data, egui_view_path_transform_t *transform)
{
    egui_dim_t draw_width;
    egui_dim_t draw_height;
    int32_t scale_x_q10 = ((int32_t)region->size.width * EGUI_VIEW_PATH_SCALE_Q10) / data->viewport_width;
    int32_t scale_y_q10 = ((int32_t)region->size.height * EGUI_VIEW_PATH_SCALE_Q10) / data->viewport_height;

    transform->scale_q10 = EGUI_MIN(scale_x_q10, scale_y_q10);
    if (transform->scale_q10 <= 0)
    {
        transform->scale_q10 = 1;
    }

    draw_width = (egui_dim_t)(((int32_t)data->viewport_width * transform->scale_q10 + EGUI_VIEW_PATH_SCALE_Q10 / 2) / EGUI_VIEW_PATH_SCALE_Q10);
    draw_height = (egui_dim_t)(((int32_t)data->viewport_height * transform->scale_q10 + EGUI_VIEW_PATH_SCALE_Q10 / 2) / EGUI_VIEW_PATH_SCALE_Q10);
    transform->offset_x = region->location.x + (region->size.width - draw_width) / 2;
    transform->offset_y = region->location.y + (region->size.height - draw_height) / 2;
}

static void egui_view_path_on_draw(egui_view_t *self)
{
    egui_view_path_t *local = egui_view_path_local(self);
    const egui_view_path_data_t *data = egui_view_path_get_data(self);
    egui_view_path_transform_t transform;
    egui_view_path_point_t contour_points[EGUI_VIEW_PATH_MAX_RAW_POINTS];
    egui_region_t region;
    egui_color_t fill_color = local->fill_color;
    egui_color_t stroke_color = local->stroke_color;
    egui_color_t accent_color = local->accent_color;
    egui_alpha_t fill_alpha = EGUI_ALPHA_MAKE(96);
    egui_alpha_t stroke_alpha = EGUI_ALPHA_100;
    uint8_t contour_count = 0;
    uint8_t contour_closed = 0;
    uint8_t has_current = 0;
    int16_t current_x = 0;
    int16_t current_y = 0;

    if (!egui_view_path_data_is_valid(data))
    {
        return;
    }

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (!egui_view_get_enable(self))
    {
        fill_color = egui_view_path_mix_disabled(fill_color);
        stroke_color = egui_view_path_mix_disabled(stroke_color);
        accent_color = egui_view_path_mix_disabled(accent_color);
        fill_alpha = EGUI_ALPHA_MAKE(58);
        stroke_alpha = EGUI_ALPHA_MAKE(68);
    }
    if (egui_view_get_pressed(self))
    {
        stroke_color = egui_rgb_mix(stroke_color, accent_color, EGUI_ALPHA_MAKE(18));
    }

    egui_view_path_prepare_transform(&region, data, &transform);
    for (uint8_t i = 0; i < data->command_count; ++i)
    {
        const egui_view_path_command_t *command = &data->commands[i];

        switch (command->type)
        {
        case EGUI_VIEW_PATH_COMMAND_MOVE_TO:
            if (contour_count > 0)
            {
                egui_view_path_flush_contour(contour_points, &contour_count, contour_closed, local->stroke_width, fill_color, stroke_color,
                                             egui_color_alpha_mix(self->alpha, fill_alpha), egui_color_alpha_mix(self->alpha, stroke_alpha));
            }
            contour_closed = 0;
            current_x = command->x1;
            current_y = command->y1;
            has_current = 1;
            egui_view_path_append_point(contour_points, &contour_count, egui_view_path_transform_point(&transform, current_x, current_y));
            break;

        case EGUI_VIEW_PATH_COMMAND_LINE_TO:
            if (!has_current)
            {
                current_x = command->x1;
                current_y = command->y1;
                has_current = 1;
                egui_view_path_append_point(contour_points, &contour_count, egui_view_path_transform_point(&transform, current_x, current_y));
                break;
            }
            current_x = command->x1;
            current_y = command->y1;
            egui_view_path_append_point(contour_points, &contour_count, egui_view_path_transform_point(&transform, current_x, current_y));
            break;

        case EGUI_VIEW_PATH_COMMAND_QUAD_TO:
            if (!has_current)
            {
                break;
            }
            for (int step = 1; step <= EGUI_VIEW_PATH_QUAD_STEPS; ++step)
            {
                int16_t sample_x = egui_view_path_eval_quad_component(current_x, command->x1, command->x2, step, EGUI_VIEW_PATH_QUAD_STEPS);
                int16_t sample_y = egui_view_path_eval_quad_component(current_y, command->y1, command->y2, step, EGUI_VIEW_PATH_QUAD_STEPS);

                egui_view_path_append_point(contour_points, &contour_count, egui_view_path_transform_point(&transform, sample_x, sample_y));
            }
            current_x = command->x2;
            current_y = command->y2;
            break;

        case EGUI_VIEW_PATH_COMMAND_CUBIC_TO:
            if (!has_current)
            {
                break;
            }
            for (int step = 1; step <= EGUI_VIEW_PATH_CUBIC_STEPS; ++step)
            {
                int16_t sample_x = egui_view_path_eval_cubic_component(current_x, command->x1, command->x2, command->x3, step,
                                                                       EGUI_VIEW_PATH_CUBIC_STEPS);
                int16_t sample_y = egui_view_path_eval_cubic_component(current_y, command->y1, command->y2, command->y3, step,
                                                                       EGUI_VIEW_PATH_CUBIC_STEPS);

                egui_view_path_append_point(contour_points, &contour_count, egui_view_path_transform_point(&transform, sample_x, sample_y));
            }
            current_x = command->x3;
            current_y = command->y3;
            break;

        case EGUI_VIEW_PATH_COMMAND_CLOSE:
            contour_closed = 1;
            egui_view_path_flush_contour(contour_points, &contour_count, contour_closed, local->stroke_width, fill_color, stroke_color,
                                         egui_color_alpha_mix(self->alpha, fill_alpha), egui_color_alpha_mix(self->alpha, stroke_alpha));
            contour_closed = 0;
            has_current = 0;
            break;

        default:
            break;
        }
    }

    if (contour_count > 0)
    {
        egui_view_path_flush_contour(contour_points, &contour_count, contour_closed, local->stroke_width, fill_color, stroke_color,
                                     egui_color_alpha_mix(self->alpha, fill_alpha), egui_color_alpha_mix(self->alpha, stroke_alpha));
    }
}

void egui_view_path_set_data(egui_view_t *self, const egui_view_path_data_t *data)
{
    egui_view_path_t *local = egui_view_path_local(self);

    egui_view_path_clear_pressed_state(self);
    local->data = egui_view_path_data_is_valid(data) ? data : egui_view_path_default_data();
    egui_view_invalidate(self);
}

const egui_view_path_data_t *egui_view_path_get_data(egui_view_t *self)
{
    egui_view_path_t *local = egui_view_path_local(self);

    if (!egui_view_path_data_is_valid(local->data))
    {
        local->data = egui_view_path_default_data();
    }
    return local->data;
}

const egui_view_path_data_t *egui_view_path_get_shield_data(void)
{
    return &path_shield_data;
}

const egui_view_path_data_t *egui_view_path_get_curve_data(void)
{
    return &path_curve_data;
}

const egui_view_path_data_t *egui_view_path_get_line_data(void)
{
    return &path_line_data;
}

const egui_view_path_data_t *egui_view_path_get_bookmark_data(void)
{
    return &path_bookmark_data;
}

void egui_view_path_set_palette(egui_view_t *self, egui_color_t fill_color, egui_color_t stroke_color, egui_color_t accent_color)
{
    egui_view_path_t *local = egui_view_path_local(self);

    egui_view_path_clear_pressed_state(self);
    local->fill_color = fill_color;
    local->stroke_color = stroke_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_path_set_stroke_width(egui_view_t *self, egui_dim_t stroke_width)
{
    egui_view_path_t *local = egui_view_path_local(self);

    egui_view_path_clear_pressed_state(self);
    local->stroke_width = egui_view_path_clamp_dim(stroke_width, EGUI_VIEW_PATH_STROKE_MIN, EGUI_VIEW_PATH_STROKE_MAX);
    egui_view_invalidate(self);
}

egui_dim_t egui_view_path_get_stroke_width(egui_view_t *self)
{
    egui_view_path_t *local = egui_view_path_local(self);

    return local->stroke_width;
}

void egui_view_path_apply_standard_style(egui_view_t *self)
{
    egui_view_path_set_palette(self, HCW_COLOR_PRIMARY_TINT, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_SOFT);
    egui_view_path_set_stroke_width(self, 2);
    egui_view_path_set_data(self, &path_shield_data);
}

void egui_view_path_apply_accent_style(egui_view_t *self)
{
    egui_view_path_set_palette(self, HCW_COLOR_PRIMARY_TINT, HCW_COLOR_PRIMARY, HCW_COLOR_PRIMARY_LIGHT);
    egui_view_path_set_stroke_width(self, 3);
    egui_view_path_set_data(self, &path_curve_data);
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_path_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_path_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_path_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_path_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_path_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_path_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 0;
}

static int egui_view_path_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    if (egui_view_path_clear_pressed_state(self))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_path_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_path_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_path_on_static_key_event;
#endif
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_path_t) = {
        .draw = egui_view_draw,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_path_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .on_draw = egui_view_path_on_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_path_on_key_event,
#endif
};

void egui_view_path_init(egui_view_t *self)
{
    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_path_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    egui_view_path_apply_standard_style(self);
    egui_view_set_view_name(self, "egui_view_path");
}
