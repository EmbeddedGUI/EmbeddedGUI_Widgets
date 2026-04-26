#include "egui_view_image_control.h"

#include "image/egui_image_std.h"

#define IMAGE_CONTROL_LANDSCAPE_WIDTH  16
#define IMAGE_CONTROL_LANDSCAPE_HEIGHT 10
#define IMAGE_CONTROL_PORTRAIT_WIDTH   10
#define IMAGE_CONTROL_PORTRAIT_HEIGHT  16
#define IMAGE_CONTROL_SQUARE_SIZE      12

static uint16_t landscape_data[IMAGE_CONTROL_LANDSCAPE_WIDTH * IMAGE_CONTROL_LANDSCAPE_HEIGHT];
static uint16_t portrait_data[IMAGE_CONTROL_PORTRAIT_WIDTH * IMAGE_CONTROL_PORTRAIT_HEIGHT];
static uint16_t square_data[IMAGE_CONTROL_SQUARE_SIZE * IMAGE_CONTROL_SQUARE_SIZE];
static uint8_t image_data_ready;

static const egui_image_std_info_t landscape_info = {
        .data_buf = landscape_data,
        .alpha_buf = NULL,
        .data_type = EGUI_IMAGE_DATA_TYPE_RGB565,
        .alpha_type = EGUI_IMAGE_ALPHA_TYPE_1,
        .width = IMAGE_CONTROL_LANDSCAPE_WIDTH,
        .height = IMAGE_CONTROL_LANDSCAPE_HEIGHT,
};

static const egui_image_std_info_t portrait_info = {
        .data_buf = portrait_data,
        .alpha_buf = NULL,
        .data_type = EGUI_IMAGE_DATA_TYPE_RGB565,
        .alpha_type = EGUI_IMAGE_ALPHA_TYPE_1,
        .width = IMAGE_CONTROL_PORTRAIT_WIDTH,
        .height = IMAGE_CONTROL_PORTRAIT_HEIGHT,
};

static const egui_image_std_info_t square_info = {
        .data_buf = square_data,
        .alpha_buf = NULL,
        .data_type = EGUI_IMAGE_DATA_TYPE_RGB565,
        .alpha_type = EGUI_IMAGE_ALPHA_TYPE_1,
        .width = IMAGE_CONTROL_SQUARE_SIZE,
        .height = IMAGE_CONTROL_SQUARE_SIZE,
};

extern const egui_image_std_t egui_res_image_control_landscape_rgb565;
EGUI_IMAGE_SUB_DEFINE_CONST(egui_image_std_t, egui_res_image_control_landscape_rgb565, &landscape_info);

extern const egui_image_std_t egui_res_image_control_portrait_rgb565;
EGUI_IMAGE_SUB_DEFINE_CONST(egui_image_std_t, egui_res_image_control_portrait_rgb565, &portrait_info);

extern const egui_image_std_t egui_res_image_control_square_rgb565;
EGUI_IMAGE_SUB_DEFINE_CONST(egui_image_std_t, egui_res_image_control_square_rgb565, &square_info);

static egui_view_image_control_t *egui_view_image_control_local(egui_view_t *self)
{
    return (egui_view_image_control_t *)self;
}

static void egui_view_image_control_ensure_image_data(void)
{
    if (image_data_ready)
    {
        return;
    }

    for (uint8_t y = 0; y < IMAGE_CONTROL_LANDSCAPE_HEIGHT; ++y)
    {
        for (uint8_t x = 0; x < IMAGE_CONTROL_LANDSCAPE_WIDTH; ++x)
        {
            uint16_t pixel = 0xA6DF;

            if (y >= 6)
            {
                pixel = (x < 7) ? 0x3D67 : 0x2C43;
            }
            else if (y >= 4)
            {
                pixel = (x + y < 11) ? 0x6E9A : 0x4D73;
            }
            if (x >= 11 && x <= 13 && y >= 1 && y <= 3)
            {
                pixel = 0xFFE0;
            }
            landscape_data[y * IMAGE_CONTROL_LANDSCAPE_WIDTH + x] = pixel;
        }
    }

    for (uint8_t y = 0; y < IMAGE_CONTROL_PORTRAIT_HEIGHT; ++y)
    {
        for (uint8_t x = 0; x < IMAGE_CONTROL_PORTRAIT_WIDTH; ++x)
        {
            uint16_t pixel = 0xE71C;

            if (y < 4)
            {
                pixel = 0xFDB5;
            }
            else if (x < 3)
            {
                pixel = 0xBCAA;
            }
            else if (x > 6 && y > 6)
            {
                pixel = 0xA3E9;
            }
            if ((x == 1 || x == 8) && y > 4 && y < 14)
            {
                pixel = 0xFFFF;
            }
            portrait_data[y * IMAGE_CONTROL_PORTRAIT_WIDTH + x] = pixel;
        }
    }

    for (uint8_t y = 0; y < IMAGE_CONTROL_SQUARE_SIZE; ++y)
    {
        for (uint8_t x = 0; x < IMAGE_CONTROL_SQUARE_SIZE; ++x)
        {
            uint16_t pixel = 0xC638;

            if (x < 6 && y < 6)
            {
                pixel = 0x9EFC;
            }
            else if (x >= 6 && y < 6)
            {
                pixel = 0xFE75;
            }
            else if (x < 6)
            {
                pixel = 0xA77F;
            }
            else
            {
                pixel = 0x5E69;
            }
            if (x == y || x + y == IMAGE_CONTROL_SQUARE_SIZE - 1)
            {
                pixel = 0xFFFF;
            }
            square_data[y * IMAGE_CONTROL_SQUARE_SIZE + x] = pixel;
        }
    }

    image_data_ready = 1;
}

static uint8_t egui_view_image_control_clamp_stretch(uint8_t stretch)
{
    if (stretch > EGUI_VIEW_IMAGE_CONTROL_STRETCH_UNIFORM)
    {
        return EGUI_VIEW_IMAGE_CONTROL_STRETCH_UNIFORM;
    }
    return stretch;
}

static uint8_t egui_view_image_control_clear_pressed_state(egui_view_t *self)
{
    uint8_t had_pressed = egui_view_get_pressed(self);

    egui_view_set_pressed(self, 0);
    return had_pressed;
}

static egui_color_t egui_view_image_control_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, EGUI_COLOR_HEX(0x8A97A5), 58);
}

static void egui_view_image_control_draw_placeholder(egui_view_t *self, const egui_region_t *region, egui_color_t color)
{
    egui_canvas_t *canvas = &uicode_get_core()->canvas;
    egui_alpha_t alpha = egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100);

    egui_canvas_draw_rectangle_fill(canvas, region->location.x, region->location.y, region->size.width, region->size.height, color, egui_color_alpha_mix(self->alpha, 42));
    egui_canvas_draw_line(canvas, region->location.x + 3, region->location.y + region->size.height - 4,
                          region->location.x + region->size.width / 2, region->location.y + region->size.height / 2, 1, color, alpha);
    egui_canvas_draw_line(canvas, region->location.x + region->size.width / 2, region->location.y + region->size.height / 2,
                          region->location.x + region->size.width - 4, region->location.y + 4, 1, color, alpha);
}

static void egui_view_image_control_get_content_region(egui_view_t *self, const egui_view_image_control_t *local, egui_region_t *content_region)
{
    egui_region_t region;
    egui_dim_t inset = local->compact_mode ? 2 : 4;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= inset * 2 || region.size.height <= inset * 2)
    {
        inset = 0;
    }

    content_region->location.x = region.location.x + inset;
    content_region->location.y = region.location.y + inset;
    content_region->size.width = region.size.width - inset * 2;
    content_region->size.height = region.size.height - inset * 2;
}

static void egui_view_image_control_get_uniform_region(const egui_region_t *bounds, egui_dim_t image_width, egui_dim_t image_height, egui_region_t *target)
{
    if (image_width <= 0 || image_height <= 0 || bounds->size.width <= 0 || bounds->size.height <= 0)
    {
        *target = *bounds;
        return;
    }

    if ((int32_t)bounds->size.width * image_height <= (int32_t)bounds->size.height * image_width)
    {
        target->size.width = bounds->size.width;
        target->size.height = (egui_dim_t)(((int32_t)bounds->size.width * image_height + image_width / 2) / image_width);
    }
    else
    {
        target->size.height = bounds->size.height;
        target->size.width = (egui_dim_t)(((int32_t)bounds->size.height * image_width + image_height / 2) / image_height);
    }

    if (target->size.width <= 0)
    {
        target->size.width = 1;
    }
    if (target->size.height <= 0)
    {
        target->size.height = 1;
    }

    target->location.x = bounds->location.x + (bounds->size.width - target->size.width) / 2;
    target->location.y = bounds->location.y + (bounds->size.height - target->size.height) / 2;
}

static void egui_view_image_control_get_none_region(const egui_region_t *bounds, egui_dim_t image_width, egui_dim_t image_height, egui_region_t *target)
{
    target->size.width = image_width;
    target->size.height = image_height;
    if (target->size.width > bounds->size.width)
    {
        target->size.width = bounds->size.width;
    }
    if (target->size.height > bounds->size.height)
    {
        target->size.height = bounds->size.height;
    }
    if (target->size.width <= 0)
    {
        target->size.width = bounds->size.width;
    }
    if (target->size.height <= 0)
    {
        target->size.height = bounds->size.height;
    }
    target->location.x = bounds->location.x + (bounds->size.width - target->size.width) / 2;
    target->location.y = bounds->location.y + (bounds->size.height - target->size.height) / 2;
}

static void egui_view_image_control_draw_image(egui_view_t *self, egui_view_image_control_t *local, const egui_region_t *content_region)
{
    egui_region_t target = *content_region;
    egui_dim_t image_width = 0;
    egui_dim_t image_height = 0;

    if (local->image == NULL || content_region->size.width <= 0 || content_region->size.height <= 0)
    {
        egui_view_image_control_draw_placeholder(self, content_region, local->placeholder_color);
        return;
    }

    if (!egui_image_get_size(local->image, &image_width, &image_height))
    {
        egui_view_image_control_draw_placeholder(self, content_region, local->placeholder_color);
        return;
    }

    if (local->stretch == EGUI_VIEW_IMAGE_CONTROL_STRETCH_NONE)
    {
        egui_view_image_control_get_none_region(content_region, image_width, image_height, &target);
        egui_image_draw_image_resize(local->image, &uicode_get_core()->canvas, target.location.x, target.location.y, target.size.width, target.size.height);
    }
    else if (local->stretch == EGUI_VIEW_IMAGE_CONTROL_STRETCH_FILL)
    {
        egui_image_draw_image_resize(local->image, &uicode_get_core()->canvas, content_region->location.x, content_region->location.y,
                                     content_region->size.width, content_region->size.height);
    }
    else
    {
        egui_view_image_control_get_uniform_region(content_region, image_width, image_height, &target);
        egui_image_draw_image_resize(local->image, &uicode_get_core()->canvas, target.location.x, target.location.y, target.size.width, target.size.height);
    }
}

static void egui_view_image_control_on_draw(egui_view_t *self)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);
    egui_region_t region;
    egui_region_t content_region;
    egui_color_t surface_color = local->surface_color;
    egui_color_t border_color = local->border_color;
    egui_dim_t radius = local->compact_mode ? 5 : 7;

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    if (local->read_only_mode)
    {
        surface_color = egui_rgb_mix(surface_color, EGUI_COLOR_HEX(0xF6F8FA), 42);
        border_color = egui_rgb_mix(border_color, local->muted_color, 50);
    }
    if (!egui_view_get_enable(self))
    {
        surface_color = egui_view_image_control_mix_disabled(surface_color);
        border_color = egui_view_image_control_mix_disabled(border_color);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                          surface_color, egui_color_alpha_mix(self->alpha, EGUI_ALPHA_100));

    egui_view_image_control_get_content_region(self, local, &content_region);
    egui_view_image_control_draw_image(self, local, &content_region);

    if (local->read_only_mode || !egui_view_get_enable(self))
    {
        egui_canvas_draw_rectangle_fill(&uicode_get_core()->canvas, content_region.location.x, content_region.location.y, content_region.size.width,
                                        content_region.size.height, surface_color, egui_color_alpha_mix(self->alpha, 34));
    }

    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius, 1,
                                     border_color, egui_color_alpha_mix(self->alpha, local->read_only_mode ? 48 : 64));
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_image_control_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_image_control_clear_pressed_state(self);
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_image_control_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_UNUSED(event);
    egui_view_image_control_clear_pressed_state(self);
    return 1;
}
#endif

void egui_view_image_control_set_source(egui_view_t *self, const egui_image_t *image, const char *source_name)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    egui_view_image_control_clear_pressed_state(self);
    local->image = image != NULL ? image : egui_view_image_control_get_default_image();
    if (image == NULL)
    {
        local->source_name = "Landscape";
    }
    else
    {
        local->source_name = (source_name != NULL && source_name[0] != '\0') ? source_name : "Image";
    }
    egui_view_invalidate(self);
}

const egui_image_t *egui_view_image_control_get_source(egui_view_t *self)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    return local->image != NULL ? local->image : egui_view_image_control_get_default_image();
}

const char *egui_view_image_control_get_source_name(egui_view_t *self)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    return local->source_name != NULL ? local->source_name : "Landscape";
}

void egui_view_image_control_set_stretch(egui_view_t *self, uint8_t stretch)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    egui_view_image_control_clear_pressed_state(self);
    local->stretch = egui_view_image_control_clamp_stretch(stretch);
    egui_view_invalidate(self);
}

uint8_t egui_view_image_control_get_stretch(egui_view_t *self)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    return local->stretch;
}

void egui_view_image_control_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    egui_view_image_control_clear_pressed_state(self);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_image_control_get_compact_mode(egui_view_t *self)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    return local->compact_mode;
}

void egui_view_image_control_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    egui_view_image_control_clear_pressed_state(self);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

uint8_t egui_view_image_control_get_read_only_mode(egui_view_t *self)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    return local->read_only_mode;
}

void egui_view_image_control_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t border_color,
                                         egui_color_t placeholder_color, egui_color_t muted_color)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    egui_view_image_control_clear_pressed_state(self);
    local->surface_color = surface_color;
    local->border_color = border_color;
    local->placeholder_color = placeholder_color;
    local->muted_color = muted_color;
    egui_view_invalidate(self);
}

void egui_view_image_control_apply_standard_style(egui_view_t *self)
{
    egui_view_image_control_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xB8C7D7), EGUI_COLOR_HEX(0x0F6CBD), EGUI_COLOR_HEX(0x798694));
    egui_view_image_control_set_compact_mode(self, 0);
    egui_view_image_control_set_read_only_mode(self, 0);
}

void egui_view_image_control_apply_compact_style(egui_view_t *self)
{
    egui_view_image_control_set_palette(self, EGUI_COLOR_HEX(0xFFFFFF), EGUI_COLOR_HEX(0xD0D9E2), EGUI_COLOR_HEX(0x0F7B45), EGUI_COLOR_HEX(0x7E8A97));
    egui_view_image_control_set_compact_mode(self, 1);
    egui_view_image_control_set_read_only_mode(self, 0);
}

void egui_view_image_control_apply_read_only_style(egui_view_t *self)
{
    egui_view_image_control_set_palette(self, EGUI_COLOR_HEX(0xF7F9FB), EGUI_COLOR_HEX(0xD3DCE5), EGUI_COLOR_HEX(0x6B7785), EGUI_COLOR_HEX(0x7E8A97));
    egui_view_image_control_set_compact_mode(self, 1);
    egui_view_image_control_set_read_only_mode(self, 1);
}

void egui_view_image_control_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_image_control_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_image_control_on_static_key_event;
#endif
}

const egui_image_t *egui_view_image_control_get_landscape_image(void)
{
    egui_view_image_control_ensure_image_data();
    return (const egui_image_t *)&egui_res_image_control_landscape_rgb565;
}

const egui_image_t *egui_view_image_control_get_portrait_image(void)
{
    egui_view_image_control_ensure_image_data();
    return (const egui_image_t *)&egui_res_image_control_portrait_rgb565;
}

const egui_image_t *egui_view_image_control_get_square_image(void)
{
    egui_view_image_control_ensure_image_data();
    return (const egui_image_t *)&egui_res_image_control_square_rgb565;
}

const egui_image_t *egui_view_image_control_get_default_image(void)
{
    return egui_view_image_control_get_landscape_image();
}

static const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_image_control_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
        .on_touch_event = egui_view_on_touch_event,
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_image_control_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_on_key_event,
#endif
};

void egui_view_image_control_init(egui_view_t *self)
{
    egui_view_image_control_t *local = egui_view_image_control_local(self);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_image_control_t);
    egui_view_set_background(self, NULL);
    egui_view_set_shadow(self, NULL);
    egui_view_set_padding_all(self, 0);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, 0);
#endif

    local->image = egui_view_image_control_get_default_image();
    local->source_name = "Landscape";
    local->stretch = EGUI_VIEW_IMAGE_CONTROL_STRETCH_UNIFORM;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->surface_color = EGUI_COLOR_HEX(0xFFFFFF);
    local->border_color = EGUI_COLOR_HEX(0xB8C7D7);
    local->placeholder_color = EGUI_COLOR_HEX(0x0F6CBD);
    local->muted_color = EGUI_COLOR_HEX(0x798694);

    egui_view_set_view_name(self, "egui_view_image_control");
}
