APP ?= HelloCustomWidgets
PORT ?= pc

SDK_ROOT ?= sdk/EmbeddedGUI
OUTPUT_PATH ?= $(CURDIR)/output
OBJROOT_PATH ?= $(OUTPUT_PATH)
EGUI_APP_ROOT_PATH ?= ../../example
SIBLING_EMSDK := $(CURDIR)/../EmbeddedGUI/tools/emsdk

ifeq ($(strip $(EMSDK_PATH)),)
ifneq ($(wildcard $(SIBLING_EMSDK)),)
EMSDK_PATH := $(SIBLING_EMSDK)
endif
else
ifeq ($(wildcard $(EMSDK_PATH)),)
ifneq ($(wildcard $(SIBLING_EMSDK)),)
EMSDK_PATH := $(SIBLING_EMSDK)
endif
endif
endif

FORWARD_VARS := \
	APP \
	APP_SUB \
	PORT \
	BITS \
	COMPILE_OPT_LEVEL \
	COMPILE_DEBUG \
	USER_CFLAGS \
	EMSDK_PATH \
	TARGET \
	OUTPUT_PATH \
	OBJROOT_PATH \
	EGUI_APP_ROOT_PATH \
	APP_OBJ_SUFFIX \
	V \
	QEMU_PATH \
	ARM_GCC_PATH \
	CPU_ARCH

define forward_arg
$(if $(filter OUTPUT_PATH OBJROOT_PATH,$(1)),$(1)="$(abspath $($(1)))",$(if $(filter EMSDK_PATH,$(1)),$(1)=$(subst \,/,$($(1))),$(1)="$($(1))"))
endef

FORWARD_ARGS := $(foreach var,$(FORWARD_VARS),$(if $($(var)),$(call forward_arg,$(var))))

.PHONY: all clean run resource resource_refresh

all clean run resource resource_refresh:
	$(MAKE) -C $(SDK_ROOT) $@ $(FORWARD_ARGS)

%:
	$(MAKE) -C $(SDK_ROOT) $@ $(FORWARD_ARGS)
