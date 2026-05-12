

#include "font/egui_font_std.h"

// clang-format off



/**
 * Font size: 16
 * Font bit size: 4
 * TTF file: MaterialSymbolsOutlined-Regular.ttf
 * options: -i MaterialSymbolsOutlined-Regular.ttf -n rating_control_icons -p 16 -s 4 -ext 0 --var-coords 1,0,16,500 -t rating_control_icons_text.txt
 */


/**
 * Total character count: 1 Supported icons:
 * - star (U+E838)
 */

static const uint8_t egui_res_font_rating_control_icons_16_4_pixel_buffer[] = {


    /* Glyph for icon "star" U+E838 0x00eea0b8 */
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xee, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xff, 0x06, 0x00, 0x00,
    0x00, 0x31, 0xd4, 0xff, 0x4d, 0x13, 0x00, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
    0x20, 0xfe, 0xff, 0xff, 0xff, 0xef, 0x02, 0x00, 0xc1, 0xff, 0xff, 0xff, 0x1c, 0x00,
    0x00, 0x20, 0xff, 0xff, 0xff, 0x02, 0x00, 0x00, 0x50, 0xff, 0xff, 0xff, 0x05, 0x00,
    0x00, 0x90, 0xff, 0xbb, 0xff, 0x09, 0x00, 0x00, 0xd0, 0x4d, 0x00, 0xd4, 0x0d, 0x00,
    0x00, 0x70, 0x00, 0x00, 0x00, 0x07, 0x00,

};

static const egui_font_std_char_descriptor_t egui_res_font_rating_control_icons_16_4_char_array[] = {

    {.idx=     0, .size=    91, .box_w= 14, .box_h= 13, .adv= 16, .off_x=  1, .off_y=  3}, /* icon "star" U+E838 0x00eea0b8 */
};
static const egui_font_std_code_descriptor_t egui_res_font_rating_control_icons_16_4_code_array[] = {

    {.code=0x00eea0b8}, /* star */
};



static const egui_font_std_info_t egui_res_font_rating_control_icons_16_4_info = {
    .font_size = 16,
    .font_bit_mode = 4,
    .height = 20,
    .res_type = EGUI_RESOURCE_TYPE_INTERNAL,
    .bitmap_codec = EGUI_FONT_STD_BITMAP_CODEC_RAW,
    .count = 1,
    .code_array = egui_res_font_rating_control_icons_16_4_code_array,
    .char_array = (void *)egui_res_font_rating_control_icons_16_4_char_array,
    .pixel_buffer = (void *)egui_res_font_rating_control_icons_16_4_pixel_buffer,
};

extern const egui_font_std_t egui_res_font_rating_control_icons_16_4;
EGUI_FONT_SUB_DEFINE_CONST(egui_font_std_t, egui_res_font_rating_control_icons_16_4, &egui_res_font_rating_control_icons_16_4_info);




// clang-format on


