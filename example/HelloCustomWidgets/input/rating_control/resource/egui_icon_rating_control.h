#ifndef _EGUI_ICON_RATING_CONTROL_H_
#define _EGUI_ICON_RATING_CONTROL_H_

#include "font/egui_font_std.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const egui_font_std_t egui_res_font_rating_control_icons_16_4;
extern const egui_font_std_t egui_res_font_rating_control_icons_20_4;
extern const egui_font_std_t egui_res_font_rating_control_icons_24_4;

#define EGUI_FONT_RATING_CONTROL_ICON_16 ((const egui_font_t *)&egui_res_font_rating_control_icons_16_4)
#define EGUI_FONT_RATING_CONTROL_ICON_20 ((const egui_font_t *)&egui_res_font_rating_control_icons_20_4)
#define EGUI_FONT_RATING_CONTROL_ICON_24 ((const egui_font_t *)&egui_res_font_rating_control_icons_24_4)

#define EGUI_ICON_RATING_CONTROL_STAR_CODEPOINT 0xE838u
#define EGUI_ICON_RATING_CONTROL_STAR           "\xEE\xA0\xB8"

#ifdef __cplusplus
}
#endif

#endif /* _EGUI_ICON_RATING_CONTROL_H_ */
