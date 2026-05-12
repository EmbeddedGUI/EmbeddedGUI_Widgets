#ifndef _HCW_TEXT_CENTER_H_
#define _HCW_TEXT_CENTER_H_

#include "egui.h"

static uint8_t hcw_text_center_has_text(const char *text)
{
    return text != NULL && text[0] != '\0' ? 1 : 0;
}

static egui_dim_t hcw_text_center_round_div(int64_t numerator, int64_t denominator)
{
    if (denominator <= 0)
    {
        return 0;
    }

    if (numerator >= 0)
    {
        return (egui_dim_t)((numerator + denominator / 2) / denominator);
    }
    return (egui_dim_t)(-((-numerator + denominator / 2) / denominator));
}

static uint8_t hcw_text_center_get_font_pixel_alpha(const egui_font_std_info_t *font_info, const uint8_t *pixel_buffer, egui_dim_t width,
                                                    egui_dim_t x, egui_dim_t y)
{
    switch (font_info->font_bit_mode)
    {
#if EGUI_CONFIG_FUNCTION_FONT_FORMAT_1
    case 1:
    {
        const uint8_t *row = pixel_buffer + y * ((width + 7) >> 3);
        return (row[x >> 3] & (1 << (x & 0x07))) ? EGUI_ALPHA_100 : 0;
    }
#endif
#if EGUI_CONFIG_FUNCTION_FONT_FORMAT_2
    case 2:
    {
        const uint8_t *row = pixel_buffer + y * ((width + 3) >> 2);
        return egui_alpha_change_table_2[(row[x >> 2] >> ((x & 0x03) << 1)) & 0x03];
    }
#endif
#if EGUI_CONFIG_FUNCTION_FONT_FORMAT_4
    case 4:
    {
        const uint8_t *row = pixel_buffer + y * ((width + 1) >> 1);
        return egui_alpha_change_table_4[(row[x >> 1] >> ((x & 0x01) << 2)) & 0x0F];
    }
#endif
#if EGUI_CONFIG_FUNCTION_FONT_FORMAT_8
    case 8:
    {
        const uint8_t *row = pixel_buffer + y * width;
        return row[x];
    }
#endif
    default:
        break;
    }

    return 0;
}

static uint8_t hcw_text_center_add_glyph_center(const egui_font_std_info_t *font_info, const egui_font_std_char_descriptor_t *desc,
                                                int64_t *weight_sum, int64_t *weighted_y_sum)
{
    int64_t glyph_weight = 0;
    int64_t glyph_weighted_y = 0;
    egui_dim_t x;
    egui_dim_t y;

    if (font_info == NULL || desc == NULL || desc->box_h == 0)
    {
        return 0;
    }

    if (font_info->res_type == EGUI_RESOURCE_TYPE_INTERNAL && font_info->bitmap_codec == EGUI_FONT_STD_BITMAP_CODEC_RAW && font_info->pixel_buffer != NULL)
    {
        const uint8_t *pixel_buffer = font_info->pixel_buffer + desc->idx;

        for (y = 0; y < desc->box_h; ++y)
        {
            for (x = 0; x < desc->box_w; ++x)
            {
                uint8_t alpha = hcw_text_center_get_font_pixel_alpha(font_info, pixel_buffer, desc->box_w, x, y);

                if (alpha != 0)
                {
                    glyph_weight += alpha;
                    glyph_weighted_y += (int64_t)alpha * (desc->off_y + y);
                }
            }
        }

        if (glyph_weight > 0)
        {
            *weight_sum += glyph_weight;
            *weighted_y_sum += glyph_weighted_y;
            return 1;
        }
    }

    glyph_weight = desc->box_w > 0 ? desc->box_w : 1;
    for (y = 0; y < desc->box_h; ++y)
    {
        *weight_sum += glyph_weight;
        *weighted_y_sum += glyph_weight * (desc->off_y + y);
    }
    return 1;
}

static uint8_t hcw_text_center_get_stats(const egui_font_t *font, const char *text, int64_t *weight_sum, int64_t *weighted_y_sum)
{
    egui_canvas_t *canvas = &uicode_get_core()->canvas;
    const egui_font_std_info_t *font_info;
    egui_font_std_char_descriptor_t scratch;
    const egui_font_std_char_descriptor_t *desc;
    uint32_t utf8_code;
    int char_bytes;
    uint8_t has_ink = 0;

    if (font == NULL || font->res == NULL || !hcw_text_center_has_text(text))
    {
        return 0;
    }

    font_info = (const egui_font_std_info_t *)font->res;
    while (*text != '\0')
    {
        if (*text == '\r')
        {
            ++text;
            continue;
        }
        if (*text == '\n')
        {
            break;
        }

        char_bytes = egui_font_get_utf8_code_fast(text, &utf8_code);
        if (char_bytes <= 0)
        {
            break;
        }

        desc = egui_font_std_get_desc_fast_api(canvas, font_info, utf8_code, &scratch);
        if (hcw_text_center_add_glyph_center(font_info, desc, weight_sum, weighted_y_sum))
        {
            has_ink = 1;
        }
        text += char_bytes;
    }

    return has_ink;
}

static egui_dim_t hcw_text_center_get_delta(const egui_font_t *font, const char *text, const egui_region_t *region, uint8_t align)
{
    egui_dim_t line_height;
    egui_dim_t line_y;
    int64_t weight_sum = 0;
    int64_t weighted_y_sum = 0;
    int64_t numerator;
    int64_t denominator;

    if (region == NULL || region->size.height <= 0 || (align & EGUI_ALIGN_VMASK) != EGUI_ALIGN_VCENTER)
    {
        return 0;
    }
    if (!egui_font_std_try_get_line_height(font, &line_height))
    {
        return 0;
    }
    if (!hcw_text_center_get_stats(font, text, &weight_sum, &weighted_y_sum) || weight_sum <= 0)
    {
        return 0;
    }

    line_y = region->size.height > line_height ? (egui_dim_t)((region->size.height - line_height) >> 1) : 0;
    numerator = (int64_t)region->size.height * weight_sum - ((int64_t)line_y * 2 * weight_sum + weighted_y_sum * 2);
    denominator = weight_sum * 2;
    return hcw_text_center_round_div(numerator, denominator);
}

#endif /* _HCW_TEXT_CENTER_H_ */
