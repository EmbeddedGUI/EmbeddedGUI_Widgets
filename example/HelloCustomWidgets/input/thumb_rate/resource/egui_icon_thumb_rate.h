#ifndef _EGUI_ICON_THUMB_RATE_H_
#define _EGUI_ICON_THUMB_RATE_H_

#include "font/egui_font_std.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const egui_font_std_t egui_res_font_thumb_rate_icons_16_4;
extern const egui_font_std_t egui_res_font_thumb_rate_icons_20_4;
extern const egui_font_std_t egui_res_font_thumb_rate_icons_24_4;

#define EGUI_FONT_THUMB_RATE_ICON_16 ((const egui_font_t *)&egui_res_font_thumb_rate_icons_16_4)
#define EGUI_FONT_THUMB_RATE_ICON_20 ((const egui_font_t *)&egui_res_font_thumb_rate_icons_20_4)
#define EGUI_FONT_THUMB_RATE_ICON_24 ((const egui_font_t *)&egui_res_font_thumb_rate_icons_24_4)

#define EGUI_ICON_THUMB_RATE_DOWN_CODEPOINT 0xE8DBu
#define EGUI_ICON_THUMB_RATE_DOWN "\xEE\xA3\x9B"
#define EGUI_ICON_THUMB_RATE_UP_CODEPOINT 0xE8DCu
#define EGUI_ICON_THUMB_RATE_UP "\xEE\xA3\x9C"

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_ICON_THUMB_RATE_H_ */
