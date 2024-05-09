include $(AT_SDK_HOME_SH)/tools/mk/function.mk
AT_SDK_HOME	= 	${AT_SDK_HOME_SH}
VERBOSE		=	yes
RTOS		= 	yes
DEBUG		=	no
CHIP		=	AT1K
PLATFORM 	= 	evb
FPU			=	yes
ITCM		=	yes
DTCM		=	yes
XIP			=	no
NPU			=	yes
PSRAM = yes
POSTLOAD	=	no
TARGET		=	$(notdir $(CURDIR))
APP_DIR		=	$(subst $(AT_SDK_HOME_SH),.,$(CURDIR))
APP_SRC		+=	$(subst $(AT_SDK_HOME_SH),,$(call wildcards,$(AT_SDK_HOME_SH)/$(APP_DIR),%.c))
APP_INC		=	$(addprefix -I,$(subst /$(TARGET),,${CURDIR}))
APP_INC		+= 	$(addprefix -I,$(call dirs,$(AT_SDK_HOME)/$(APP_DIR),%.h))

CFLAGS 		+=	-g

ifeq ($(CHIP), AT820)
# for AT820: enable static allocation for FreeRTOS, so DTCM can be used as NPU task stack
CFLAGS 		+=	-DAPP_HEAP_SIZE=24*1024 
CFLAGS		+=	-DconfigSUPPORT_STATIC_ALLOCATION=1
else
# for AT1K: DTCM can't be used as NPU task stack, because DTCM can't be accessed by NPU
CFLAGS 		+=	-DAPP_HEAP_SIZE=64*1024 
CFLAGS		+=	-DconfigSUPPORT_STATIC_ALLOCATION=0
endif

include $(AT_SDK_HOME_SH)/tools/mk/common.mk

