#ifndef _APP_EGUI_CONFIG_H_
#define _APP_EGUI_CONFIG_H_

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS
#define EGUI_CONFIG_FUNCTION_SUPPORT_FOCUS 1
#endif

#ifndef EGUI_CONFIG_FUNCTION_SUPPORT_MASK
#define EGUI_CONFIG_FUNCTION_SUPPORT_MASK 1
#endif

#ifndef EGUI_CONFIG_FUNCTION_SUPPORT_LAYER
#define EGUI_CONFIG_FUNCTION_SUPPORT_LAYER 1
#endif

#ifndef EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING
#define EGUI_CONFIG_FUNCTION_SUPPORT_MARGIN_PADDING 0
#endif

#ifndef EGUI_CONFIG_SCREEN_WIDTH
#define EGUI_CONFIG_SCREEN_WIDTH 480
#endif

#ifndef EGUI_CONFIG_SCREEN_HEIGHT
#define EGUI_CONFIG_SCREEN_HEIGHT 480
#endif

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* _APP_EGUI_CONFIG_H_ */
