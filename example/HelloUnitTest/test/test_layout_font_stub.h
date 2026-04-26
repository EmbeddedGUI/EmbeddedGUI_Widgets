#ifndef _TEST_LAYOUT_FONT_STUB_H_
#define _TEST_LAYOUT_FONT_STUB_H_

#define EGUI_TEST_LAYOUT_FONT_HEIGHT 8
#define EGUI_TEST_LAYOUT_FONT_ADV    3

static int egui_test_layout_font_draw_string(const egui_font_t *self, egui_canvas_t *canvas, const void *string, egui_dim_t x, egui_dim_t y,
                                             egui_color_t color, egui_alpha_t alpha)
{
    EGUI_UNUSED(self);
    EGUI_UNUSED(canvas);
    EGUI_UNUSED(string);
    EGUI_UNUSED(x);
    EGUI_UNUSED(y);
    EGUI_UNUSED(color);
    EGUI_UNUSED(alpha);
    return 0;
}

static int egui_test_layout_font_get_str_size(const egui_font_t *self, const void *string, uint8_t is_multi_line, egui_dim_t line_space,
                                              egui_dim_t *width, egui_dim_t *height)
{
    const char *cursor = (const char *)string;
    egui_dim_t max_width = 0;
    egui_dim_t line_width = 0;
    egui_dim_t line_height = EGUI_TEST_LAYOUT_FONT_HEIGHT;

    EGUI_UNUSED(self);

    if (cursor == NULL)
    {
        if (width != NULL)
        {
            *width = 0;
        }
        if (height != NULL)
        {
            *height = 0;
        }
        return -1;
    }

    while (*cursor != '\0')
    {
        if (*cursor == '\r')
        {
            cursor++;
            continue;
        }
        if (*cursor == '\n')
        {
            if (!is_multi_line)
            {
                break;
            }
            if (max_width < line_width)
            {
                max_width = line_width;
            }
            line_width = 0;
            line_height += EGUI_TEST_LAYOUT_FONT_HEIGHT + line_space;
            cursor++;
            continue;
        }

        line_width += EGUI_TEST_LAYOUT_FONT_ADV;
        cursor++;
    }

    if (max_width < line_width)
    {
        max_width = line_width;
    }

    if (width != NULL)
    {
        if (*width > 0 && max_width > *width)
        {
            max_width = *width;
        }
        *width = max_width;
    }
    if (height != NULL)
    {
        *height = line_height;
    }
    return 0;
}

static const egui_font_api_t egui_test_layout_font_api = {
        .draw_string = egui_test_layout_font_draw_string,
        .get_str_size = egui_test_layout_font_get_str_size,
};

static const egui_font_t egui_test_layout_font = {
        .res = NULL,
        .api = &egui_test_layout_font_api,
};

static const egui_font_t *egui_test_layout_get_font(void)
{
    return &egui_test_layout_font;
}

#endif /* _TEST_LAYOUT_FONT_STUB_H_ */
