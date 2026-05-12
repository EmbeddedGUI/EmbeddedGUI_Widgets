#include "egui_view_spin_button.h"
#include "../../hcw_text_center.h"
#include "utils/egui_sprintf.h"

#define EGUI_VIEW_SPIN_BUTTON_RADIUS              9
#define EGUI_VIEW_SPIN_BUTTON_COMPACT_RADIUS      7
#define EGUI_VIEW_SPIN_BUTTON_FILL_ALPHA          EGUI_ALPHA_MAKE(96)
#define EGUI_VIEW_SPIN_BUTTON_BORDER_ALPHA        EGUI_ALPHA_MAKE(96)
#define EGUI_VIEW_SPIN_BUTTON_FIELD_FILL_ALPHA    EGUI_ALPHA_MAKE(98)
#define EGUI_VIEW_SPIN_BUTTON_FIELD_BORDER_ALPHA  EGUI_ALPHA_MAKE(98)
#define EGUI_VIEW_SPIN_BUTTON_STEPPER_FILL_ALPHA  EGUI_ALPHA_MAKE(98)
#define EGUI_VIEW_SPIN_BUTTON_STEPPER_BORDER_ALPHA EGUI_ALPHA_MAKE(98)
#define EGUI_VIEW_SPIN_BUTTON_PAD_X               10
#define EGUI_VIEW_SPIN_BUTTON_PAD_Y               8
#define EGUI_VIEW_SPIN_BUTTON_COMPACT_PAD_X       7
#define EGUI_VIEW_SPIN_BUTTON_COMPACT_PAD_Y       6
#define EGUI_VIEW_SPIN_BUTTON_LABEL_HEIGHT        10
#define EGUI_VIEW_SPIN_BUTTON_LABEL_GAP           5
#define EGUI_VIEW_SPIN_BUTTON_ROW_HEIGHT          34
#define EGUI_VIEW_SPIN_BUTTON_COMPACT_ROW_HEIGHT  24
#define EGUI_VIEW_SPIN_BUTTON_HELPER_GAP          5
#define EGUI_VIEW_SPIN_BUTTON_HELPER_HEIGHT       10
#define EGUI_VIEW_SPIN_BUTTON_STEPPER_WIDTH       28
#define EGUI_VIEW_SPIN_BUTTON_COMPACT_STEPPER     22
#define EGUI_VIEW_SPIN_BUTTON_STEPPER_GAP         5
#define EGUI_VIEW_SPIN_BUTTON_BUTTON_GAP          2

typedef struct egui_view_spin_button_metrics egui_view_spin_button_metrics_t;
struct egui_view_spin_button_metrics
{
    egui_region_t content;
    egui_region_t label_region;
    egui_region_t row_region;
    egui_region_t field_region;
    egui_region_t inc_region;
    egui_region_t dec_region;
    egui_region_t helper_region;
    uint8_t show_meta;
    uint8_t show_steppers;
};

static egui_color_t egui_view_spin_button_mix_disabled(egui_color_t color)
{
    return egui_rgb_mix(color, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(44));
}

static egui_dim_t egui_view_spin_button_measure_font_line_height(const egui_font_t *font)
{
    egui_dim_t width = 0;
    egui_dim_t height = 0;

    if (font == NULL || font->api == NULL || font->api->get_str_size == NULL)
    {
        return 0;
    }

    font->api->get_str_size(font, "A", 0, 0, &width, &height);
    return height;
}

static int16_t egui_view_spin_button_clamp_value(egui_view_spin_button_t *local, int16_t value)
{
    if (value < local->min_value)
    {
        value = local->min_value;
    }
    if (value > local->max_value)
    {
        value = local->max_value;
    }
    return value;
}

static uint8_t egui_view_spin_button_clear_active_state(egui_view_t *self, egui_view_spin_button_t *local)
{
    uint8_t had_active = (uint8_t)(self->is_pressed || local->active_part != EGUI_VIEW_SPIN_BUTTON_PART_NONE);

    local->active_part = EGUI_VIEW_SPIN_BUTTON_PART_NONE;
    egui_view_set_pressed(self, false);
    return had_active;
}

static uint8_t egui_view_spin_button_is_interactive(egui_view_spin_button_t *local, egui_view_t *self)
{
    return (uint8_t)(!local->read_only_mode && !local->compact_mode && egui_view_get_enable(self));
}

static void egui_view_spin_button_build_value_text(egui_view_spin_button_t *local)
{
    int pos = egui_sprintf_int(local->value_buffer, sizeof(local->value_buffer), local->value);

    if (local->suffix != NULL && local->suffix[0] != '\0')
    {
        pos += egui_sprintf_char(&local->value_buffer[pos], (int)sizeof(local->value_buffer) - pos, ' ');
        egui_sprintf_str(&local->value_buffer[pos], (int)sizeof(local->value_buffer) - pos, local->suffix);
    }
}

static uint8_t egui_view_spin_button_commit_value(egui_view_t *self, egui_view_spin_button_t *local, int16_t value)
{
    value = egui_view_spin_button_clamp_value(local, value);
    if (value == local->value)
    {
        return 0;
    }

    local->value = value;
    if (local->on_value_changed != NULL)
    {
        local->on_value_changed(self, value);
    }
    egui_view_invalidate(self);
    return 1;
}

static void egui_view_spin_button_get_metrics(egui_view_spin_button_t *local, egui_view_t *self, egui_view_spin_button_metrics_t *metrics)
{
    egui_region_t region;
    egui_dim_t pad_x = local->compact_mode ? EGUI_VIEW_SPIN_BUTTON_COMPACT_PAD_X : EGUI_VIEW_SPIN_BUTTON_PAD_X;
    egui_dim_t pad_y = local->compact_mode ? EGUI_VIEW_SPIN_BUTTON_COMPACT_PAD_Y : EGUI_VIEW_SPIN_BUTTON_PAD_Y;
    egui_dim_t label_h = EGUI_VIEW_SPIN_BUTTON_LABEL_HEIGHT;
    egui_dim_t helper_h = EGUI_VIEW_SPIN_BUTTON_HELPER_HEIGHT;
    egui_dim_t row_h = local->compact_mode ? EGUI_VIEW_SPIN_BUTTON_COMPACT_ROW_HEIGHT : EGUI_VIEW_SPIN_BUTTON_ROW_HEIGHT;
    egui_dim_t stepper_w = local->compact_mode ? EGUI_VIEW_SPIN_BUTTON_COMPACT_STEPPER : EGUI_VIEW_SPIN_BUTTON_STEPPER_WIDTH;
    egui_dim_t meta_line_h = egui_view_spin_button_measure_font_line_height(local->meta_font);
    egui_dim_t block_h = row_h;
    egui_dim_t block_y;
    egui_dim_t stepper_x;
    egui_dim_t button_h;

    egui_view_get_work_region(self, &region);
    if (meta_line_h > label_h)
    {
        label_h = meta_line_h;
    }
    if (meta_line_h > helper_h)
    {
        helper_h = meta_line_h;
    }

    metrics->show_meta = local->compact_mode ? 0 : 1;
    metrics->show_steppers = local->read_only_mode ? 0 : 1;
    metrics->content.location.x = region.location.x + pad_x;
    metrics->content.location.y = region.location.y + pad_y;
    metrics->content.size.width = region.size.width - pad_x * 2;
    metrics->content.size.height = region.size.height - pad_y * 2;

    if (metrics->show_meta)
    {
        block_h = label_h + EGUI_VIEW_SPIN_BUTTON_LABEL_GAP + row_h + EGUI_VIEW_SPIN_BUTTON_HELPER_GAP + helper_h;
    }
    block_y = metrics->content.location.y;
    if (metrics->content.size.height > block_h)
    {
        block_y += (metrics->content.size.height - block_h) / 2;
    }

    metrics->label_region.location.x = metrics->content.location.x;
    metrics->label_region.location.y = block_y;
    metrics->label_region.size.width = metrics->content.size.width;
    metrics->label_region.size.height = metrics->show_meta ? label_h : 0;

    metrics->row_region.location.x = metrics->content.location.x;
    metrics->row_region.location.y = metrics->show_meta ? (block_y + label_h + EGUI_VIEW_SPIN_BUTTON_LABEL_GAP)
                                                        : (metrics->content.location.y + (metrics->content.size.height - row_h) / 2);
    metrics->row_region.size.width = metrics->content.size.width;
    metrics->row_region.size.height = row_h;

    stepper_x = metrics->row_region.location.x + metrics->row_region.size.width - (metrics->show_steppers ? stepper_w : 0);
    metrics->field_region.location.x = metrics->row_region.location.x;
    metrics->field_region.location.y = metrics->row_region.location.y;
    metrics->field_region.size.width =
            metrics->row_region.size.width - (metrics->show_steppers ? (stepper_w + EGUI_VIEW_SPIN_BUTTON_STEPPER_GAP) : 0);
    metrics->field_region.size.height = row_h;

    button_h = (row_h - EGUI_VIEW_SPIN_BUTTON_BUTTON_GAP) / 2;
    metrics->inc_region.location.x = stepper_x;
    metrics->inc_region.location.y = metrics->row_region.location.y;
    metrics->inc_region.size.width = metrics->show_steppers ? stepper_w : 0;
    metrics->inc_region.size.height = metrics->show_steppers ? button_h : 0;

    metrics->dec_region.location.x = stepper_x;
    metrics->dec_region.location.y = metrics->row_region.location.y + button_h + EGUI_VIEW_SPIN_BUTTON_BUTTON_GAP;
    metrics->dec_region.size.width = metrics->show_steppers ? stepper_w : 0;
    metrics->dec_region.size.height = metrics->show_steppers ? (row_h - button_h - EGUI_VIEW_SPIN_BUTTON_BUTTON_GAP) : 0;

    metrics->helper_region.location.x = metrics->content.location.x;
    metrics->helper_region.location.y = metrics->row_region.location.y + row_h + EGUI_VIEW_SPIN_BUTTON_HELPER_GAP;
    metrics->helper_region.size.width = metrics->content.size.width;
    metrics->helper_region.size.height = metrics->show_meta ? helper_h : 0;
}

static uint8_t egui_view_spin_button_hit_part(egui_view_spin_button_t *local, egui_view_t *self, egui_dim_t screen_x, egui_dim_t screen_y)
{
    egui_view_spin_button_metrics_t metrics;
    egui_dim_t local_x = screen_x - self->region_screen.location.x;
    egui_dim_t local_y = screen_y - self->region_screen.location.y;

    egui_view_spin_button_get_metrics(local, self, &metrics);
    if (metrics.show_steppers && egui_region_pt_in_rect(&metrics.inc_region, local_x, local_y))
    {
        return EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT;
    }
    if (metrics.show_steppers && egui_region_pt_in_rect(&metrics.dec_region, local_x, local_y))
    {
        return EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT;
    }
    if (egui_region_pt_in_rect(&metrics.field_region, local_x, local_y))
    {
        return EGUI_VIEW_SPIN_BUTTON_PART_FIELD;
    }
    return EGUI_VIEW_SPIN_BUTTON_PART_NONE;
}

void egui_view_spin_button_set_value(egui_view_t *self, int16_t value)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);
    uint8_t had_active = egui_view_spin_button_clear_active_state(self, local);

    if (!egui_view_spin_button_commit_value(self, local, value) && had_active)
    {
        egui_view_invalidate(self);
    }
}

int16_t egui_view_spin_button_get_value(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);
    return local->value;
}

void egui_view_spin_button_set_range(egui_view_t *self, int16_t min_value, int16_t max_value)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);

    egui_view_spin_button_clear_active_state(self, local);
    if (min_value > max_value)
    {
        int16_t temp = min_value;
        min_value = max_value;
        max_value = temp;
    }
    local->min_value = min_value;
    local->max_value = max_value;
    local->value = egui_view_spin_button_clamp_value(local, local->value);
    egui_view_invalidate(self);
}

void egui_view_spin_button_set_step(egui_view_t *self, int16_t step)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);

    egui_view_spin_button_clear_active_state(self, local);
    if (step <= 0)
    {
        step = 1;
    }
    local->step = step;
    if (local->large_step < step)
    {
        local->large_step = step;
    }
    egui_view_invalidate(self);
}

void egui_view_spin_button_set_large_step(egui_view_t *self, int16_t large_step)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);

    egui_view_spin_button_clear_active_state(self, local);
    if (large_step < local->step)
    {
        large_step = local->step;
    }
    local->large_step = large_step;
    egui_view_invalidate(self);
}

void egui_view_spin_button_set_texts(egui_view_t *self, const char *label, const char *suffix, const char *helper)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);

    egui_view_spin_button_clear_active_state(self, local);
    local->label = label;
    local->suffix = suffix;
    local->helper = helper;
    egui_view_invalidate(self);
}

void egui_view_spin_button_set_fonts(egui_view_t *self, const egui_font_t *value_font, const egui_font_t *meta_font)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);

    egui_view_spin_button_clear_active_state(self, local);
    local->value_font = value_font != NULL ? value_font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = meta_font != NULL ? meta_font : (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    egui_view_invalidate(self);
}

void egui_view_spin_button_set_compact_mode(egui_view_t *self, uint8_t compact_mode)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);

    egui_view_spin_button_clear_active_state(self, local);
    local->compact_mode = compact_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_spin_button_set_read_only_mode(egui_view_t *self, uint8_t read_only_mode)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);

    egui_view_spin_button_clear_active_state(self, local);
    local->read_only_mode = read_only_mode ? 1 : 0;
    egui_view_invalidate(self);
}

void egui_view_spin_button_set_palette(egui_view_t *self, egui_color_t surface_color, egui_color_t field_color, egui_color_t border_color,
                                       egui_color_t text_color, egui_color_t muted_text_color, egui_color_t accent_color)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);

    egui_view_spin_button_clear_active_state(self, local);
    local->surface_color = surface_color;
    local->field_color = field_color;
    local->border_color = border_color;
    local->text_color = text_color;
    local->muted_text_color = muted_text_color;
    local->accent_color = accent_color;
    egui_view_invalidate(self);
}

void egui_view_spin_button_set_on_value_changed_listener(egui_view_t *self, egui_view_spin_button_value_changed_listener_t listener)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);
    local->on_value_changed = listener;
}

uint8_t egui_view_spin_button_adjust(egui_view_t *self, int16_t delta)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);

    if (!egui_view_spin_button_is_interactive(local, self))
    {
        egui_view_spin_button_clear_active_state(self, local);
        return 0;
    }
    return egui_view_spin_button_commit_value(self, local, (int16_t)(local->value + delta));
}

uint8_t egui_view_spin_button_get_part_region(egui_view_t *self, uint8_t part, egui_region_t *region)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);
    egui_view_spin_button_metrics_t metrics;

    if (region == NULL)
    {
        return 0;
    }
    egui_view_spin_button_get_metrics(local, self, &metrics);
    switch (part)
    {
    case EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT:
        *region = metrics.inc_region;
        break;
    case EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT:
        *region = metrics.dec_region;
        break;
    case EGUI_VIEW_SPIN_BUTTON_PART_FIELD:
        *region = metrics.field_region;
        break;
    default:
        return 0;
    }
    region->location.x += self->region_screen.location.x;
    region->location.y += self->region_screen.location.y;
    return (uint8_t)(region->size.width > 0 && region->size.height > 0);
}

static void egui_view_spin_button_draw_text(const egui_font_t *font, egui_view_t *self, const char *text, const egui_region_t *region, uint8_t align,
                                            egui_color_t color, egui_alpha_t alpha)
{
    egui_region_t draw_region = *region;

    if (text == NULL || text[0] == '\0' || region->size.width <= 0 || region->size.height <= 0)
    {
        return;
    }
    draw_region.location.y += hcw_text_center_get_delta(font, text, region, align);
    egui_canvas_draw_text_in_rect(&uicode_get_core()->canvas, font, text, &draw_region, align, color,
                                  egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(alpha)));
}

static void egui_view_spin_button_draw_arrow(egui_view_t *self, const egui_region_t *region, egui_color_t color, uint8_t up, egui_alpha_t alpha)
{
    egui_dim_t cx = region->location.x + region->size.width / 2;
    egui_dim_t cy = region->location.y + region->size.height / 2;

    if (up)
    {
        egui_canvas_draw_line(&uicode_get_core()->canvas, cx - 3, cy + 1, cx, cy - 2, 1, color,
                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(alpha)));
        egui_canvas_draw_line(&uicode_get_core()->canvas, cx, cy - 2, cx + 3, cy + 1, 1, color,
                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(alpha)));
    }
    else
    {
        egui_canvas_draw_line(&uicode_get_core()->canvas, cx - 3, cy - 1, cx, cy + 2, 1, color,
                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(alpha)));
        egui_canvas_draw_line(&uicode_get_core()->canvas, cx, cy + 2, cx + 3, cy - 1, 1, color,
                              egui_color_alpha_mix(self->alpha, EGUI_ALPHA_MAKE(alpha)));
    }
}

static void egui_view_spin_button_on_draw(egui_view_t *self)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);
    egui_region_t region;
    egui_view_spin_button_metrics_t metrics;
    egui_color_t surface;
    egui_color_t field;
    egui_color_t border;
    egui_color_t field_border;
    egui_color_t text;
    egui_color_t muted;
    egui_color_t accent;
    egui_color_t stepper_fill;
    egui_color_t stepper_border;
    egui_dim_t radius = local->compact_mode ? EGUI_VIEW_SPIN_BUTTON_COMPACT_RADIUS : EGUI_VIEW_SPIN_BUTTON_RADIUS;
    uint8_t is_enabled = egui_view_get_enable(self) ? 1 : 0;
    uint8_t is_focused = 0;

#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    is_focused = self->is_focused ? 1 : 0;
#endif

    egui_view_get_work_region(self, &region);
    if (region.size.width <= 0 || region.size.height <= 0)
    {
        return;
    }

    egui_view_spin_button_build_value_text(local);
    egui_view_spin_button_get_metrics(local, self, &metrics);

    surface = local->surface_color;
    field = local->field_color;
    border = local->border_color;
    text = local->text_color;
    muted = local->muted_text_color;
    accent = local->accent_color;
    field = egui_rgb_mix(field, accent, EGUI_ALPHA_MAKE(local->compact_mode ? 6 : 8));
    field_border = egui_rgb_mix(border, accent, EGUI_ALPHA_MAKE(local->compact_mode ? 62 : 72));
    stepper_fill = egui_rgb_mix(surface, accent, EGUI_ALPHA_MAKE(local->compact_mode ? 18 : 22));
    stepper_border = egui_rgb_mix(border, accent, EGUI_ALPHA_MAKE(local->compact_mode ? 72 : 78));

    if (local->read_only_mode)
    {
        surface = egui_rgb_mix(surface, HCW_COLOR_SURFACE_SUBTLE, EGUI_ALPHA_MAKE(12));
        field = egui_rgb_mix(field, surface, EGUI_ALPHA_MAKE(16));
        border = egui_rgb_mix(border, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_MAKE(38));
        field_border = egui_rgb_mix(field_border, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_MAKE(50));
        text = egui_rgb_mix(text, HCW_COLOR_TEXT_STRONG, EGUI_ALPHA_MAKE(54));
        muted = egui_rgb_mix(muted, HCW_COLOR_TEXT_STRONG, EGUI_ALPHA_MAKE(36));
        accent = egui_rgb_mix(accent, HCW_COLOR_TEXT_STRONG, EGUI_ALPHA_MAKE(36));
        stepper_fill = egui_rgb_mix(stepper_fill, surface, EGUI_ALPHA_MAKE(28));
        stepper_border = egui_rgb_mix(stepper_border, HCW_COLOR_BORDER_STRONG, EGUI_ALPHA_MAKE(50));
    }
    if (!is_enabled)
    {
        surface = egui_view_spin_button_mix_disabled(surface);
        field = egui_view_spin_button_mix_disabled(field);
        border = egui_view_spin_button_mix_disabled(border);
        field_border = egui_view_spin_button_mix_disabled(field_border);
        text = egui_view_spin_button_mix_disabled(text);
        muted = egui_view_spin_button_mix_disabled(muted);
        accent = egui_view_spin_button_mix_disabled(accent);
        stepper_fill = egui_view_spin_button_mix_disabled(stepper_fill);
        stepper_border = egui_view_spin_button_mix_disabled(stepper_border);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius,
                                          surface, egui_color_alpha_mix(self->alpha, EGUI_VIEW_SPIN_BUTTON_FILL_ALPHA));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, region.location.x, region.location.y, region.size.width, region.size.height, radius, 1,
                                     is_focused ? accent : border,
                                     egui_color_alpha_mix(self->alpha, is_focused ? EGUI_ALPHA_MAKE(98) : EGUI_VIEW_SPIN_BUTTON_BORDER_ALPHA));

    if (metrics.show_meta)
    {
        egui_view_spin_button_draw_text(local->meta_font, self, local->label, &metrics.label_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, muted, 96);
    }

    egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.field_region.location.x, metrics.field_region.location.y,
                                          metrics.field_region.size.width, metrics.field_region.size.height, radius - 2, field,
                                          egui_color_alpha_mix(self->alpha, EGUI_VIEW_SPIN_BUTTON_FIELD_FILL_ALPHA));
    egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.field_region.location.x, metrics.field_region.location.y,
                                     metrics.field_region.size.width, metrics.field_region.size.height, radius - 2, 1,
                                     local->focus_part == EGUI_VIEW_SPIN_BUTTON_PART_FIELD ? accent : field_border,
                                     egui_color_alpha_mix(self->alpha,
                                                          local->focus_part == EGUI_VIEW_SPIN_BUTTON_PART_FIELD ? EGUI_ALPHA_100
                                                                                                               : EGUI_VIEW_SPIN_BUTTON_FIELD_BORDER_ALPHA));
    egui_view_spin_button_draw_text(local->value_font, self, local->value_buffer, &metrics.field_region, EGUI_ALIGN_CENTER, text, 100);

    if (metrics.show_steppers)
    {
        egui_color_t inc_fill = stepper_fill;
        egui_color_t dec_fill = stepper_fill;

        if (local->active_part == EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT && self->is_pressed)
        {
            inc_fill = egui_rgb_mix(inc_fill, accent, EGUI_ALPHA_MAKE(42));
        }
        if (local->active_part == EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT && self->is_pressed)
        {
            dec_fill = egui_rgb_mix(dec_fill, accent, EGUI_ALPHA_MAKE(42));
        }

        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.inc_region.location.x, metrics.inc_region.location.y,
                                              metrics.inc_region.size.width, metrics.inc_region.size.height, radius - 3, inc_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_VIEW_SPIN_BUTTON_STEPPER_FILL_ALPHA));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.inc_region.location.x, metrics.inc_region.location.y,
                                         metrics.inc_region.size.width, metrics.inc_region.size.height, radius - 3, 1,
                                         local->focus_part == EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT ? accent : stepper_border,
                                         egui_color_alpha_mix(self->alpha,
                                                              local->focus_part == EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT
                                                                      ? EGUI_ALPHA_100
                                                                      : EGUI_VIEW_SPIN_BUTTON_STEPPER_BORDER_ALPHA));
        egui_view_spin_button_draw_arrow(self, &metrics.inc_region, accent, 1, 100);

        egui_canvas_draw_round_rectangle_fill(&uicode_get_core()->canvas, metrics.dec_region.location.x, metrics.dec_region.location.y,
                                              metrics.dec_region.size.width, metrics.dec_region.size.height, radius - 3, dec_fill,
                                              egui_color_alpha_mix(self->alpha, EGUI_VIEW_SPIN_BUTTON_STEPPER_FILL_ALPHA));
        egui_canvas_draw_round_rectangle(&uicode_get_core()->canvas, metrics.dec_region.location.x, metrics.dec_region.location.y,
                                         metrics.dec_region.size.width, metrics.dec_region.size.height, radius - 3, 1,
                                         local->focus_part == EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT ? accent : stepper_border,
                                         egui_color_alpha_mix(self->alpha,
                                                              local->focus_part == EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT
                                                                      ? EGUI_ALPHA_100
                                                                      : EGUI_VIEW_SPIN_BUTTON_STEPPER_BORDER_ALPHA));
        egui_view_spin_button_draw_arrow(self, &metrics.dec_region, accent, 0, 100);
    }

    if (metrics.show_meta)
    {
        egui_view_spin_button_draw_text(local->meta_font, self, local->helper, &metrics.helper_region, EGUI_ALIGN_LEFT | EGUI_ALIGN_VCENTER, muted, 94);
    }
}

#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
static int egui_view_spin_button_on_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);
    uint8_t hit_part;
    uint8_t handled;

    if (!egui_view_spin_button_is_interactive(local, self))
    {
        if (egui_view_spin_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    hit_part = egui_view_spin_button_hit_part(local, self, event->location.x, event->location.y);
    switch (event->type)
    {
    case EGUI_MOTION_EVENT_ACTION_DOWN:
        if (hit_part == EGUI_VIEW_SPIN_BUTTON_PART_FIELD)
        {
            local->focus_part = EGUI_VIEW_SPIN_BUTTON_PART_FIELD;
            egui_view_invalidate(self);
            return 1;
        }
        if (hit_part != EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT && hit_part != EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT)
        {
            return 0;
        }
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
        if (self->is_focusable)
        {
            egui_view_request_focus(self);
        }
#endif
        local->active_part = hit_part;
        local->focus_part = hit_part;
        egui_view_set_pressed(self, true);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_MOVE:
        if (local->active_part == EGUI_VIEW_SPIN_BUTTON_PART_NONE)
        {
            return 0;
        }
        egui_view_set_pressed(self, hit_part == local->active_part);
        egui_view_invalidate(self);
        return 1;
    case EGUI_MOTION_EVENT_ACTION_UP:
        handled = (uint8_t)(local->active_part != EGUI_VIEW_SPIN_BUTTON_PART_NONE);
        if (handled && self->is_pressed && hit_part == local->active_part)
        {
            int16_t delta = hit_part == EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT ? local->step : (int16_t)(-local->step);
            egui_view_spin_button_commit_value(self, local, (int16_t)(local->value + delta));
        }
        if (egui_view_spin_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return (uint8_t)(handled || hit_part != EGUI_VIEW_SPIN_BUTTON_PART_NONE);
    case EGUI_MOTION_EVENT_ACTION_CANCEL:
        if (egui_view_spin_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        return 0;
    }
}

static int egui_view_spin_button_on_static_touch_event(egui_view_t *self, egui_motion_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);
    EGUI_UNUSED(event);

    if (egui_view_spin_button_clear_active_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
static int egui_view_spin_button_on_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);
    uint8_t had_active = (uint8_t)(local->active_part != EGUI_VIEW_SPIN_BUTTON_PART_NONE && self->is_pressed);

    if (!egui_view_spin_button_is_interactive(local, self))
    {
        if (egui_view_spin_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return 0;
    }

    switch (event->key_code)
    {
    case EGUI_KEY_CODE_UP:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->focus_part = EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT;
            egui_view_spin_button_commit_value(self, local, (int16_t)(local->value + local->step));
        }
        return 1;
    case EGUI_KEY_CODE_DOWN:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->focus_part = EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT;
            egui_view_spin_button_commit_value(self, local, (int16_t)(local->value - local->step));
        }
        return 1;
    case EGUI_KEY_CODE_HOME:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->focus_part = EGUI_VIEW_SPIN_BUTTON_PART_FIELD;
            egui_view_spin_button_commit_value(self, local, local->min_value);
        }
        return 1;
    case EGUI_KEY_CODE_END:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->focus_part = EGUI_VIEW_SPIN_BUTTON_PART_FIELD;
            egui_view_spin_button_commit_value(self, local, local->max_value);
        }
        return 1;
    case EGUI_KEY_CODE_LEFT:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->focus_part = EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT;
            egui_view_spin_button_clear_active_state(self, local);
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_KEY_CODE_RIGHT:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->focus_part = EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT;
            egui_view_spin_button_clear_active_state(self, local);
            egui_view_invalidate(self);
        }
        return 1;
    case EGUI_KEY_CODE_ENTER:
    case EGUI_KEY_CODE_SPACE:
        if (event->type == EGUI_KEY_EVENT_ACTION_DOWN)
        {
            local->active_part = (local->focus_part == EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT) ? EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT
                                                                                             : EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT;
            egui_view_set_pressed(self, true);
            egui_view_invalidate(self);
            return 1;
        }
        if (event->type == EGUI_KEY_EVENT_ACTION_UP)
        {
            if (had_active)
            {
                int16_t delta = local->active_part == EGUI_VIEW_SPIN_BUTTON_PART_DECREMENT ? (int16_t)(-local->step) : local->step;
                egui_view_spin_button_commit_value(self, local, (int16_t)(local->value + delta));
            }
            egui_view_spin_button_clear_active_state(self, local);
            egui_view_invalidate(self);
            return 1;
        }
        return 0;
    default:
        if (egui_view_spin_button_clear_active_state(self, local))
        {
            egui_view_invalidate(self);
        }
        return egui_view_on_key_event(self, event);
    }
}

static int egui_view_spin_button_on_static_key_event(egui_view_t *self, egui_key_event_t *event)
{
    EGUI_LOCAL_INIT(egui_view_spin_button_t);
    EGUI_UNUSED(event);

    if (egui_view_spin_button_clear_active_state(self, local))
    {
        egui_view_invalidate(self);
    }
    return 1;
}
#endif

void egui_view_spin_button_override_static_preview_api(egui_view_t *self, egui_view_api_t *api)
{
    egui_view_copy_api(self, api);
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
    api->on_touch_event = egui_view_spin_button_on_static_touch_event;
#endif
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
    api->on_key_event = egui_view_spin_button_on_static_key_event;
#endif
}

const egui_view_api_t EGUI_VIEW_API_TABLE_NAME(egui_view_spin_button_t) = {
        .dispatch_touch_event = egui_view_dispatch_touch_event,
#if EGUI_CONFIG_FUNCTION_SUPPORT_TOUCH
        .on_touch_event = egui_view_spin_button_on_touch_event,
#else
        .on_touch_event = egui_view_on_touch_event,
#endif
        .on_intercept_touch_event = egui_view_on_intercept_touch_event,
        .compute_scroll = egui_view_compute_scroll,
        .calculate_layout = egui_view_calculate_layout,
        .request_layout = egui_view_request_layout,
        .draw = egui_view_draw,
        .on_attach_to_window = egui_view_on_attach_to_window,
        .on_draw = egui_view_spin_button_on_draw,
        .on_detach_from_window = egui_view_on_detach_from_window,
#if EGUI_CONFIG_FUNCTION_SUPPORT_KEY
        .dispatch_key_event = egui_view_dispatch_key_event,
        .on_key_event = egui_view_spin_button_on_key_event,
#endif
};

void egui_view_spin_button_init(egui_view_t *self)
{
    EGUI_INIT_LOCAL(egui_view_spin_button_t);

    egui_view_init(self, uicode_get_core());
    self->api = &EGUI_VIEW_API_TABLE_NAME(egui_view_spin_button_t);
    egui_view_set_padding_all(self, 2);
#if EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
    egui_view_set_focusable(self, true);
#endif

    local->on_value_changed = NULL;
    local->value_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->meta_font = (const egui_font_t *)EGUI_CONFIG_FONT_DEFAULT;
    local->label = NULL;
    local->suffix = NULL;
    local->helper = NULL;
    local->surface_color = HCW_COLOR_PANEL;
    local->field_color = HCW_COLOR_SURFACE;
    local->border_color = HCW_COLOR_BORDER_STRONG;
    local->text_color = HCW_COLOR_TEXT_STRONG;
    local->muted_text_color = HCW_COLOR_TEXT_SOFT;
    local->accent_color = HCW_COLOR_PRIMARY_DARK;
    local->value = 0;
    local->min_value = 0;
    local->max_value = 100;
    local->step = 1;
    local->large_step = 10;
    local->active_part = EGUI_VIEW_SPIN_BUTTON_PART_NONE;
    local->focus_part = EGUI_VIEW_SPIN_BUTTON_PART_INCREMENT;
    local->compact_mode = 0;
    local->read_only_mode = 0;
    local->value_buffer[0] = '\0';

    egui_view_set_view_name(self, "egui_view_spin_button");
}
