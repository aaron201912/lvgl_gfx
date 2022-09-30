#
# Makefile
#
#CC := gcc
#CC := aarch64-linux-gnu-gcc
CC := arm-linux-gnueabihf-gcc
LVGL_DIR ?= .
LVGL_DIR_NAME ?= lvgl

#WARNINGS = -Werror -Wall -Wextra \
#           -Wshadow -Wundef -Wmaybe-uninitialized -Wmissing-prototypes -Wpointer-arith -Wuninitialized \
#           -Wunreachable-code -Wreturn-type -Wmultichar -Wformat-security -Wdouble-promotion -Wclobbered -Wdeprecated  \
#           -Wempty-body -Wstack-usage=2048 \
#           -Wtype-limits -Wsizeof-pointer-memaccess

#-Wno-unused-value -Wno-unused-parameter 
OPTIMIZATION ?= -O3 -mfpu=neon #-g -fno-omit-frame-pointer 

ALKAID_PROJ := /users/sz.chen/workspace/SDK_Release/TAKOYAKI_DLS00V050/project

include $(ALKAID_PROJ)/configs/current.configs

CFLAGS ?= -I$(LVGL_DIR)/ $(DEFINES) $(WARNINGS) $(OPTIMIZATION)
CFLAGS += -I$(LVGL_DIR)/$(LVGL_DIR_NAME)
CFLAGS += -I$(LVGL_DIR)/lv_drivers/indev/
CFLAGS += -I$(LVGL_DIR)/lvgl/demos/
CFLAGS += -I$(LVGL_DIR)/lv_porting_sstar/
CFLAGS += -I$(LVGL_DIR)/squareline_proj

CFLAGS += -I$(PROJ_ROOT)/release/include/

CFLAGS += -DCHIP_$(CHIP)

ifeq ($(CHIP),i2m)
LDFLAGS += -L $(ALKAID_PROJ)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/dynamic
LDFLAGS += -L $(ALKAID_PROJ)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/ex_libs/dynamic
CFLAGS += -I./lv_porting_sstar/panel
else
LDFLAGS += -L $(ALKAID_PROJ)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/dynamic
LDFLAGS += -L $(ALKAID_PROJ)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/ex_libs/dynamic
LDFLAGS += -L $(ALKAID_PROJ)/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic
endif

LDFLAGS += -lpthread
LDFLAGS += -lcam_os_wrapper -lcam_fs_wrapper -lmi_sys -lmi_common -lmi_panel -lmi_disp -lmi_gfx -lm 

BIN ?= demo

#Collect the files to compile

include lvgl/lvgl.mk
include lv_drivers/lv_drivers.mk
include lv_porting_sstar/lv_porting_sstar.mk
include squareline_proj/squareline_proj.mk

CSRCS += main.c

BUILD_DIR := $(LVGL_DIR)/build/
OBJ_DIR = $(BUILD_DIR)/obj/
BIN_DIR = $(BUILD_DIR)/bin/

OBJEXT ?= .o
AOBJS = $(addprefix $(OBJ_DIR),$(ASRCS:.S=$(OBJEXT)))
COBJS = $(addprefix $(OBJ_DIR),$(CSRCS:.c=$(OBJEXT)))
DEPS = $(addprefix $(OBJ_DIR),$(CSRCS:.c=.d))

SRCS = $(ASRCS) $(CSRCS)
OBJS = $(AOBJS) $(COBJS)

.PHONY: all env clean default
all: env default

-include $(DEPS)

env:
	@echo "Building env......."
	@if [ ! -d "$(BUILD_DIR)" ]; then mkdir -p $(BUILD_DIR); fi
	@if [ ! -d "$(OBJ_DIR)" ]; then mkdir -p $(OBJ_DIR); fi
	@if [ ! -d "$(BIN_DIR)" ]; then mkdir -p $(BIN_DIR); fi

$(OBJ_DIR)main.o: main.c
	@echo "Compiling $(LVGL_DIR)/$<"
	@$(CC)  $(CFLAGS) -c $< -o $@

$(OBJ_DIR)%.o: %.c
	@echo "Compiling $<"
	@mkdir -p $(OBJ_DIR)$(dir $<)
	@$(CC)  $(CFLAGS) -MMD -c $< -o $@

default: $(OBJS)
	@echo "Linking $(BIN)"
	@$(CC) -o $(BIN_DIR)$(BIN) $(OBJS) $(LDFLAGS)

clean:
	rm build -r

