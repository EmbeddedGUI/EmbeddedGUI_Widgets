APP ?= HelloCustomWidgets
PORT ?= pc
PYTHON ?= python
CATEGORY ?=
CI_TIMEOUT ?=
CI_COMPILE_CASE_JOBS ?=
CI_RUNTIME_JOBS ?=
SKIP_UNIT_TESTS ?=

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

CI_ARGS := $(if $(filter-out all ALL,$(strip $(CATEGORY))),--category $(CATEGORY),)
CI_ARGS += $(if $(filter 32,$(strip $(BITS))),--bits32,)
CI_ARGS += $(if $(strip $(CI_TIMEOUT)),--timeout $(CI_TIMEOUT),)
CI_ARGS += $(if $(strip $(CI_COMPILE_CASE_JOBS)),--compile-case-jobs $(CI_COMPILE_CASE_JOBS),)
CI_ARGS += $(if $(strip $(CI_RUNTIME_JOBS)),--runtime-jobs $(CI_RUNTIME_JOBS),)
CI_ARGS += $(if $(filter 1 true TRUE yes YES,$(strip $(SKIP_UNIT_TESTS))),--skip-unit-tests,)

.PHONY: all clean run resource resource_refresh ci

all clean run resource resource_refresh:
	$(MAKE) -C $(SDK_ROOT) $@ $(FORWARD_ARGS)

ci:
	$(PYTHON) scripts/ci_local_check.py $(CI_ARGS)

%:
	$(MAKE) -C $(SDK_ROOT) $@ $(FORWARD_ARGS)
