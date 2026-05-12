EGUI_CODE_SRC		+= $(EGUI_APP_PATH)

EGUI_CODE_INCLUDE	+= $(EGUI_APP_ROOT_PATH)
EGUI_CODE_INCLUDE	+= $(EGUI_APP_PATH)
EGUI_CODE_INCLUDE	+= $(EGUI_PATH)/widget


# select the sub app
ifeq ($(origin APP_SUB), undefined)
APP_SUB := showcase
endif

EGUI_APP_SUB_PATH := $(EGUI_APP_PATH)/$(APP_SUB)

# Each sub-app has its own obj directory (/ replaced with _)
APP_OBJ_SUFFIX := HelloCustomWidgets_$(subst /,_,$(APP_SUB))

ifeq ($(APP_SUB),showcase)
HCW_WIDGET_DIRS := $(sort $(dir $(wildcard $(EGUI_APP_PATH)/*/*/test.c)))
HCW_WIDGET_SRC_FILES := $(filter-out %/test.c,$(wildcard $(EGUI_APP_PATH)/*/*/*.c))
HCW_WIDGET_SRC_FILES := $(filter-out $(EGUI_APP_SUB_PATH)/generated/%,$(HCW_WIDGET_SRC_FILES))
HCW_WIDGET_SRC_FILES := $(filter-out $(EGUI_APP_PATH)/layout/items_repeater/egui_view_items_repeater.c,$(HCW_WIDGET_SRC_FILES))

EGUI_CODE_SRC		+= $(EGUI_APP_SUB_PATH)
EGUI_CODE_SRC		+= $(EGUI_APP_SUB_PATH)/generated
EGUI_CODE_SRC		+= $(EGUI_APP_PATH)/display/bitmap_icon/resource/img
EGUI_CODE_SRC		+= $(EGUI_APP_PATH)/display/font_icon/resource/font
EGUI_CODE_SRC		+= $(EGUI_APP_PATH)/display/image_icon/resource/img
EGUI_CODE_SRC		+= $(EGUI_APP_PATH)/display/symbol_icon/resource/font
EGUI_CODE_SRC		+= $(EGUI_APP_PATH)/input/rating_control/resource/font
EGUI_CODE_SRC		+= $(EGUI_APP_PATH)/input/thumb_rate/resource/font
EGUI_CODE_SRC_FILES += $(HCW_WIDGET_SRC_FILES)

EGUI_CODE_INCLUDE	+= $(EGUI_APP_SUB_PATH)
EGUI_CODE_INCLUDE	+= $(EGUI_APP_SUB_PATH)/generated
EGUI_CODE_INCLUDE	+= $(HCW_WIDGET_DIRS)
EGUI_CODE_INCLUDE	+= $(EGUI_APP_PATH)/display/bitmap_icon
EGUI_CODE_INCLUDE	+= $(EGUI_APP_PATH)/display/bitmap_icon/resource
EGUI_CODE_INCLUDE	+= $(EGUI_APP_PATH)/display/font_icon/resource
EGUI_CODE_INCLUDE	+= $(EGUI_APP_PATH)/display/image_icon
EGUI_CODE_INCLUDE	+= $(EGUI_APP_PATH)/display/image_icon/resource
EGUI_CODE_INCLUDE	+= $(EGUI_APP_PATH)/display/symbol_icon/resource
else
EGUI_CODE_SRC		+= $(EGUI_APP_SUB_PATH)
EGUI_CODE_SRC		+= $(EGUI_APP_SUB_PATH)/resource
EGUI_CODE_SRC		+= $(EGUI_APP_SUB_PATH)/resource/img
EGUI_CODE_SRC		+= $(EGUI_APP_SUB_PATH)/resource/font

EGUI_CODE_INCLUDE	+= $(EGUI_APP_SUB_PATH)
EGUI_CODE_INCLUDE	+= $(EGUI_APP_SUB_PATH)/resource

EGUI_APP_RESOURCE_PATH ?= $(EGUI_APP_SUB_PATH)/resource
endif
