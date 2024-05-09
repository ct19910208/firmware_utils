 # !!! Default indent width are 4 spaces. To follow the existed code pattern is the thumb of rule. !!!
# !!! TAB is not recommended. If used, please keep in mind to set your editor's tab width to 4 spaces. !!!
# !!! In VIM, please use command "set ts=4". !!!

# Global configurations
VERBOSE 			?=	yes
RTOS 				?=	yes
CHIP 				?=  AT1K
PLATFORM 			?=	evb	
CLUSTER				?=	no
CLUSTER_HEAP		?=	no
ITCM				?=	no
DTCM				?=	no
FULLMASK    		?=	yes
CONVERT     		?=	no
FPU         		?=	yes
TRACE               ?=  no
PSRAM				?=  no
PSRAM_HEAP			?=  no
LIBAUDIO			?=  no
LIBIRCTRL			?=	no
LIBCOMPUTE			?=	no
CXXSUPPORT			?=	yes
CXX_RTTI			?=	yes
CXX_EXCEPTION		?=	no
RELEASE				?=	no
MAKEFLAGS			+=	-j
CODE_IN_DDR			?=	yes
XIP					?=	no
NPU					?=	no
COMPUTE_MATH		?=	yes
AUDIO				?=	yes
AUDIO_PPL			?=	no

# Directory containing built objects
BUILDTOPDIR			=	./BUILD
BUILDSCRIPT_DIR 	?= 	./tools/mk
ifeq ($(CHIP), AT1K)
BUILDDIR   			= 	./$(BUILDTOPDIR)/$(CHIP)/GCC_RISCV
BUILDLIBDIR   		= 	./$(BUILDTOPDIR)/$(CHIP)/AR_$(ARCH_NAME)
ARCH_NAME			=RISC-V
SOC_NAME			=at1k
else ifeq ($(CHIP), AT820)
BUILDDIR   			= 	./$(BUILDTOPDIR)/$(CHIP)/GCC_RISCV
BUILDLIBDIR   		= 	./$(BUILDTOPDIR)/$(CHIP)/AR_$(ARCH_NAME)
ARCH_NAME			=RISC-V
SOC_NAME			=at820
else ifeq ($(CHIP), AT5K)
BUILDDIR   			= 	./$(BUILDTOPDIR)/$(CHIP)/GCC_ARM
BUILDLIBDIR   		= 	./$(BUILDTOPDIR)/$(CHIP)/AR_$(ARCH_NAME)
ARCH_NAME			=ARM
SOC_NAME			=at5k
ITCM				=no
DTCM				=no
else ifeq ($(CHIP), AT5050)
BUILDDIR   			= 	./$(BUILDTOPDIR)/$(CHIP)/GCC_ARM
BUILDLIBDIR   		= 	./$(BUILDTOPDIR)/$(CHIP)/AR_$(ARCH_NAME)
ARCH_NAME			=ARM
SOC_NAME			=at5050
ITCM				=no
DTCM				=no
endif


# The toolchain main config.
ifneq ($(findstring $(CHIP), AT1K AT820), )
CROSS_COMPILE 		?= 	riscv32-unknown-elf-
else ifeq ($(CHIP), AT5K)
CROSS_COMPILE 		?= 	arm-none-eabi-
else ifeq ($(CHIP), AT5050)
CROSS_COMPILE 		?= 	arm-none-eabi-
endif

CC 					= 	$(CROSS_COMPILE)gcc
CXX         		= 	$(CROSS_COMPILE)g++
AR         			= 	$(CROSS_COMPILE)ar
LD         			= 	$(CROSS_COMPILE)ld
OBJCOPY 			= 	$(CROSS_COMPILE)objcopy
OBJDUMP 			= 	$(CROSS_COMPILE)objdump
READELF 			= 	$(CROSS_COMPILE)readelf

ifeq ($(CHIP), AT1K)
ifeq ($(FPU), yes)
MCU_RISCV_ARCH 		?= 	rv32imfcxpulpv2
MCU_RISCV_ABI 		?= 	ilp32f
else
MCU_RISCV_ARCH 		?= 	rv32imcxpulpv2
MCU_RISCV_ABI 		?= 	ilp32
endif
CROSS_COMPILE		?=	riscv32-unknown-elf-
CFLAGS 				+= 	-DCONFIG_CORE_TM500 -DCONFIG_AT1K
ifeq ($(FULLMASK), yes)
CFLAGS 				+= 	-DCONFIG_AT1K_MP
ASMFLAGS			+=	-DCONFIG_AT1K_MP
endif
#fence instruction now is treated as UND with TM500
CFLAGS 				+= 	-DCONFIG_CORE_TM500_WA_FENCE
ASMFLAGS			+=	-DCONFIG_CORE_TM500 -DCONFIG_AT1K
ASMFLAGS			+=	-DCONFIG_CORE_TM500_WA_FENCE
else ifeq ($(CHIP), AT820)
ifeq ($(FPU), yes)
MCU_RISCV_ARCH 		?= 	rv32imfcxpulpv2
MCU_RISCV_ABI 		?= 	ilp32f
else
MCU_RISCV_ARCH 		?= 	rv32imcxpulpv2
MCU_RISCV_ABI 		?= 	ilp32
endif
CROSS_COMPILE		?=	riscv32-unknown-elf-
CFLAGS 				+= 	-DCONFIG_CORE_TM500 -DCONFIG_AT820
#fence instruction now is treated as UND with TM500
CFLAGS 				+= 	-DCONFIG_CORE_TM500_WA_FENCE
ASMFLAGS			+=	-DCONFIG_CORE_TM500 -DCONFIG_AT820
ASMFLAGS			+=	-DCONFIG_CORE_TM500_WA_FENCE
else ifeq ($(CHIP), AT5K)
CFLAGS              +=  -DCONFIG_AT5K
MCU_ARM_ARCH 		?= 	armv7-a
MCU_ARM_ABI 		?= 	aapcs-linux
MCU_ARM_FPU			?=  vfpv4-d16
else ifeq ($(CHIP), AT5050)
CFLAGS              +=  -DCONFIG_AT5050
MCU_ARM_ARCH 		?= 	armv7-a
MCU_ARM_ABI 		?= 	aapcs-linux
MCU_ARM_FPU			?=  vfpv4-d16
endif

ifeq ($(PLATFORM), fpga)
CFLAGS  	+= 	-DFPGA_BOARD
ASMFLAGS	+=	-DFPGA_BOARD
endif


# for SOC layer/OS level common support
ifeq ($(FPU), yes)
CFLAGS				+=	-DCONFIG_WITH_FPU
ASMFLAGS			+=	-DCONFIG_WITH_FPU
endif

ifeq ($(CONVERT), yes)
CFLAGS				+=	-DCONFIG_WITH_CONVERT
ASMFLAGS			+=	-DCONFIG_WITH_CONVERT
endif

ifeq ($(TRACE), yes)
CFLAGS				+=	-DCONFIG_TRACE -DCONFIG_TOOL_RESERVE
ASMFLAGS			+=	-DCONFIG_TRACE -DCONFIG_TOOL_RESERVE
endif

ifeq ($(PSRAM), yes)
CFLAGS				+=	-DCONFIG_WITH_PSRAM
ASMFLAGS			+=	-DCONFIG_WITH_PSRAM
ifeq ($(PSRAM_HEAP), yes)
CFLAGS 				+= 	-DCONFIG_PSRAM_HEAP
ASMFLAGS			+=	-DCONFIG_PSRAM_HEAP
endif
endif

ifeq ($(POSTLOAD), yes)
CFLAGS				+=	-DCONFIG_POSTLOAD
ASMFLAGS			+=	-DCONFIG_POSTLOAD
endif

ifneq ($(CLUSTER), no)
CFLAGS 				+= 	-DCONFIG_WITH_CLUSTER
ASMFLAGS			+=	-DCONFIG_WITH_CLUSTER
CFLAGS 				+= 	-DCC_OPS
ifeq ($(CLUSTER_HEAP), yes)
CFLAGS 				+= 	-DCONFIG_CLUSTER_HEAP
ASMFLAGS			+=	-DCONFIG_CLUSTER_HEAP
endif
endif

ifneq ($(NPU), no)
CFLAGS 				+= 	-DCONFIG_WITH_NPU
ASMFLAGS			+=	-DCONFIG_WITH_NPU
endif

# ITCM related configruations
ifeq ($(ITCM), yes)
ifeq ($(CLUSTER), yes)
CFLAGS 				+= 	-DCONFIG_L1ITCM 
ASMFLAGS			+= 	-DCONFIG_L1ITCM 
else
CFLAGS 				+= 	-DCONFIG_L1ITCM -DCONFIG_FAST_VECTORS
ASMFLAGS			+= 	-DCONFIG_L1ITCM -DCONFIG_FAST_VECTORS
endif
endif

# DTCM related configruations
ifeq ($(DTCM), yes)
CFLAGS 				+= 	-DCONFIG_L1DTCM
ASMFLAGS			+= 	-DCONFIG_L1DTCM 
endif

#Middleware Compute FFT configruations
ifeq ($(COMPUTE), yes)
CFLAGS 				+= 	-DCONFIG_COMPUTE
endif

ifneq ($(findstring $(CHIP), AT1K AT820),)
LINKER_TEMPLATE		= 	$(AT_SDK_HOME_SH)/$(BUILDSCRIPT_DIR)/$(CHIP)_$(PLATFORM).lds
else ifneq ($(findstring $(CHIP), AT5K AT5050),)
LINKER_TEMPLATE		= 	$(AT_SDK_HOME_SH)/$(BUILDSCRIPT_DIR)/$(CHIP)_$(PLATFORM)_cmsis.lds
ifeq ($(CODE_IN_DDR), no)
else
CFLAGS				+= 	-DCONFIG_CODE_IN_DDR
endif
endif

LINKER_SCRIPT 		= 	$(AT_SDK_HOME_SH)/$(BUILDSCRIPT_DIR)/link.lds
LINK_DEPS 			+= 	$(LINKER_SCRIPT)

# for Optimization and debug options
ifeq ($(DEBUG), yes)
GCCFLAGS 			+= 	-g -O0 -DDEBUG
ASMFLAGS			+= 	-g -O0 -DDEBUG
else
GCCFLAGS 			+=  -O1 
ASMFLAGS			+=  -O1 
endif

ifeq ($(CHIP), AT820)
ifeq ($(XIP), yes)
CFLAGS		+=	-DCONFIG_XIP_EN
ASMFLAGS	+=	-DCONFIG_XIP_EN
ifeq ($(FLASH_PROGRAM_WITH_XIP),yes)
CFLAGS		+=	-DCONFIG_FLASH_PROGRAM_WITH_XIP
ASMFLAGS	+=	-DCONFIG_FLASH_PROGRAM_WITH_XIP
endif
endif
endif

#toolchain specific flags
ifneq ($(findstring $(CHIP), AT1K AT820),)
GCCFLAGS 				+=  -mnohwloop
# "-fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables" is supposed to remove .eh_frame meanwhile is not working.
GCCFLAGS 				+= 	-nodefaultlibs -nostdlib #-nostdinc
# For sign extension failure. 
GCCFLAGS				+=	-fno-expensive-optimizations
# For O3 enablement
GCCFLAGS				+=	-fno-tree-loop-vectorize
# For strict alignment
GCCFLAGS 				+= -mstrict-align

# Compilation warning message setting
WARNINGGLOGAL		= 	-Wall -Wextra -Wfatal-errors
## loose warning messages, will be opened in future.
override WARNINGLOOSE 	+= 	-Wno-unused-parameter \
						-Wno-unused-function \
						-Wno-unused-variable \
						-Wno-unused-but-set-variable \
						-Wno-unused-value \
						-Wno-missing-field-initializers \
						-Wno-sign-compare
						#-Wno-implicit-function-declaration\
						#-Wno-format \
						#-Wno-implicit-int \
					
GCCFLAGS			+=	$(WARNINGGLOGAL) $(WARNINGLOOSE)
GCCFLAGS			+= 	-Wstack-usage=0x800
GCCFLAGS			+= 	-mcmodel=medany  -ffunction-sections -fdata-sections
GCCFLAGS			+= 	-ffreestanding -fno-builtin # bypass compiler impacts

CFLAGS 				+= 	$(GCCFLAGS)
CFLAGS				+=	-std=gnu99
CFLAGS				+=	-fno-builtin-printf -fno-builtin-malloc # bypass compiler impact in fine granularity

ifeq ($(CXXSUPPORT), yes)
CPPFLAGS			+=	$(GCCFLAGS)
CPPFLAGS      		+= 	$(GCCFLAGS)                       \
						-std=c++11                        \
						-x c++                            \
						-fstrict-enums                    \
						-fno-use-cxa-atexit               \
						-fno-use-cxa-get-exception-ptr    \
						-fno-nonansi-builtins             \
						-fno-threadsafe-statics           \
						-fno-enforce-eh-specs             \
						-ftemplate-depth-64               
ifeq ($(CXX_RTTI), no)
CPPFLAGS			+=	-fno-rtti
else
CPPFLAGS 			+=	-DCONFIG_CXX_RTTI
endif
ifeq ($(CXX_EXCEPTIONS), no)
CPPFLAGS			+=	-fno-exceptions
else
CPPFLAGS			+=	-fexceptions
CPPFLAGS 			+=	-DCONFIG_CXX_EXCEPTIONS
endif
endif						

ASMFLAGS   			+= 	-x assembler-with-cpp -march=$(MCU_RISCV_ARCH) -mabi=$(MCU_RISCV_ABI) $(CLFAGS)
MCU_CFLAGS 			+= 	-march=$(MCU_RISCV_ARCH) -mabi=$(MCU_RISCV_ABI) $(CFLAGS) 
MCU_CPPFLAGS 		+= 	-march=$(MCU_RISCV_ARCH) -mabi=$(MCU_RISCV_ABI) $(CPPFLAGS) 

# LDFLAGS should be first appear in this file. If there is LDFLAGS defined outside, then the line below is ignored.
#LDFLAGS     		?= 	-T$(AT_SDK_HOME_SH)/$(BOARD_DIR)/$(PLATFORM)/$(CHIP)_$(PLATFORM).lds
LDFLAGS     		?= 	-T$(LINKER_SCRIPT)
LDFLAGS 			+= 	-static -Wl,--gc-sections -Wl,--check-sections --stats -Wl,--print-memory-usage -Wl,-Map=$@.map #-Wl,--print-gc-sections
LDFLAGS				+=	-Wl,--noinhibit-exec
LDFLAGS				+= 	-march=$(MCU_RISCV_ARCH) -mabi=$(MCU_RISCV_ABI)

#else ifeq ($(CHIP), AT5K)
else ifneq ($(findstring $(CHIP), AT5K AT5050),)
WARNINGGLOGAL		= 	-Wall -Wextra -Werror=date-time
## loose warning messages, will be opened in future.
override WARNINGLOOSE 	+= 	-Wno-unused-parameter \
						-Wno-unused-function \
						-Wno-unused-variable \
						-Wno-unused-but-set-variable \
						-Wno-unused-value \
						-Wno-missing-field-initializers \
						-Wno-sign-compare \
						-Wno-format-security \
						-Wno-empty-body \
						-Wno-enum-conversion \
						-Wno-format-nonliteral 
						#-Wno-implicit-function-declaration\
						#-Wno-format \
						#-Wno-implicit-int \

CFLAGS 				+=	-D__KERNEL__   \
						-marm -mno-thumb-interwork  \
						-mno-unaligned-access -ffunction-sections -fdata-sections -fno-common \
						-ffixed-r9 -pipe \
						-fno-builtin -ffreestanding -fno-short-wchar \
						-fno-stack-protector -fno-delete-null-pointer-checks \
						-fstack-usage -fshort-enums
CFLAGS				+=	$(WARNINGGLOGAL) $(WARNINGLOOSE)
CFLAGS 				+= 	$(GCCFLAGS)

ifeq ($(FPU), yes)
CFLAGS 					+= -mfloat-abi=softfp
else
CFLAGS 					+= -mfloat-abi=soft
endif

ASMFLAGS   			+= 	-x assembler-with-cpp -march=$(MCU_ARM_ARCH) -mabi=$(MCU_ARM_ABI)
ASMFLAGS   			+= 	-D__ASSEMBLY__

ifeq ($(FPU), yes)
MCU_CFLAGS 			+=  -mfpu=$(MCU_ARM_FPU)
endif
MCU_CFLAGS 			+= 	-march=$(MCU_ARM_ARCH) -mabi=$(MCU_ARM_ABI) $(CFLAGS)

LDFLAGS     		?= 	-T$(LINKER_SCRIPT)
LDFLAGS 			+= 	-static -Wl,--gc-sections -Wl,--check-sections --stats -Wl,--print-memory-usage -Wl,-Map=$@.map #-Wl,--print-gc-sections
LDFLAGS				+= 	-march=$(MCU_ARM_ARCH) -mabi=$(MCU_ARM_ABI)
endif


# For global common dir
ARCH_DIR 			= 	./device/core/$(ARCH_NAME)
DEVICE_DIR 			= 	./device/
DRIVER_DIR 			= 	./device/peripheral
CC_DIR 				= 	./device/cluster
CLUSTER_DIR			=	./cluster

TOP_INC				=	-I$(AT_SDK_HOME_SH)/

ifeq ($(NPU), yes)
NPU_SDK_HOME_SH		=	$(AT_SDK_HOME_SH)

ifeq ($(RELEASE), no)
ifneq ($(wildcard $(AT_SDK_HOME_SH)/ts-npu-sdk/sourceme-lin.sh),)
NPU_SDK_DIR			=	./ts-npu-sdk
NPU_ROLE			=	peripheral
else
NPU_SDK_DIR			=	./device/npu
SYSLIBS += -L$(AT_SDK_HOME_SH)/$(NPU_SDK_DIR) -lnpu -u _cc_start
endif
else
NPU_SDK_DIR			=	./device/npu
SYSLIBS += -L$(AT_SDK_HOME_SH)/$(NPU_SDK_DIR) -lnpu -u _cc_start
endif

ifneq ($(findstring $(CHIP), AT1K AT820), )
include $(NPU_SDK_HOME_SH)/$(NPU_SDK_DIR)/tools/mk/npu-sdk-component.mk
endif

endif

#specific libs
ifeq ($(CLUSTER), yes)
ifeq ($(RELEASE), yes)
SYSLIBS += -L$(AT_SDK_HOME_SH)/$(CC_DIR) -lcc -u _cc_start
endif
endif

ifeq ($(LIBAUDIO), yes)
SYSLIBS += -L$(AT_SDK_HOME_SH)/middleware/audio -laudio
endif

ifeq ($(LIBIRCTRL), yes)
SYSLIBS += -L$(AT_SDK_HOME_SH)/middleware/irctrl -lirctrl
endif

ifeq ($(LIBVIDEO), yes)
SYSLIBS += -L$(AT_SDK_HOME_SH)/middleware/video -lvideo
endif

ifeq ($(LIBVIDEO), yes)
SYSLIBS += -L$(AT_SDK_HOME_SH)/middleware/video -lvideo
endif

ifeq ($(LIBCOMPUTE), yes)
SYSLIBS += -L$(AT_SDK_HOME_SH)/middleware/compute -lcompute
endif

ifeq ($(LIBOPUS), yes)
SYSLIBS += -L$(AT_SDK_HOME_SH)/middleware/opus -lopus
endif

ifeq ($(LIBOPENCV), yes)
SYSLIBS += -L$(AT_SDK_HOME_SH)/middleware/opencv -lopencv
endif

ifneq ($(strip $(ALG_MODULES))x, x)
ALG_MODULE_LDFLAGS = -Wl,--whole-archive -Wl,--start-group $(ALG_MODULES) -Wl,--end-group -Wl,--no-whole-archive
USERLIB_LDFLAGS += $(ALG_MODULE_LDFLAGS)
endif

LIBS 				?= 	-nostartfiles $(USERLIB_LDFLAGS) $(SYSLIBS) -lstdc++ -lm -lc -lgcc -nostdlib  -nodefaultlibs

CPUDIR				?= $(BUILDDIR)/device/core/$(ARCH_NAME)/src
LDFLAGS				+= 	-nostdlib \
						-DCPUDIR=$(CPUDIR)
#For boot src
include $(AT_SDK_HOME_SH)/$(ARCH_DIR)/../core-component.mk
BOOT_SRC			= 	$(CORE_SRC)
BOOT_INC			= 	$(CORE_INC)
BOOT_ASM_SRC		= 	$(CORE_ASM_SRC)

#FOR device part src
DEVICE_INC			= 	-I$(AT_SDK_HOME_SH)/$(DEVICE_DIR)/$(SOC_NAME)/include \
						-I$(AT_SDK_HOME_SH)/$(DEVICE_DIR)
DEVICE_SRC_DIR  	= 	$(DEVICE_DIR)/$(SOC_NAME)

DEVICE_SRC      	= 	$(subst $(AT_SDK_HOME_SH),,$(call wildcards,$(AT_SDK_HOME_SH)/$(DEVICE_SRC_DIR),%.c))

#FOR Driver part src
DRIVER_INC			= 	-I$(AT_SDK_HOME_SH)/$(DRIVER_DIR)/include

# DRIVER_SRC will declear here
include $(AT_SDK_HOME_SH)/$(DRIVER_DIR)/drv-component.mk

#for cluster part
ifeq ($(CLUSTER), yes)
CC_INC				= 	-I$(AT_SDK_HOME_SH)/$(CC_DIR)/include
ifeq ($(RELEASE), no)
CLUSTER_INC			= 	-I$(AT_SDK_HOME_SH)/$(CLUSTER_DIR)/include
else
CLUSTER_INC			= 	
endif
else
CC_INC				=
CLUSTER_INC			= 
endif

INCLUDES			+= 	$(TOP_INC) $(ARCH_INC) $(DRIVER_INC) $(DEVICE_INC) $(CC_INC) $(CLUSTER_INC) $(BOOT_INC) $(NPU_INTERFACE_INC)

ifneq ($(findstring $(CHIP), AT1K AT820),)
ifeq ($(RELEASE), no)
CLUSTER_ASM_DIR  	= 	$(CLUSTER_DIR)/AT1K
CLUSTER_ASM_SRC		= 	$(subst $(AT_SDK_HOME_SH),,$(call wildcards,$(AT_SDK_HOME_SH)/$(CLUSTER_ASM_DIR),%.S))
CLUSTER_SRC			= 	$(subst $(AT_SDK_HOME_SH),,$(call wildcards,$(AT_SDK_HOME_SH)/$(CLUSTER_DIR),%.c))
endif
else
CLUSTER_ASM_SRC		=
CLUSTER_SRC			=
endif

BOOT_ASM_OBJ		= 	$(patsubst %.S, $(BUILDDIR)/%.o, $(BOOT_ASM_SRC))
BOOT_OBJ      		= 	$(patsubst %.c, $(BUILDDIR)/%.o, $(BOOT_SRC))
DRIVER_OBJ      	= 	$(patsubst %.c, $(BUILDDIR)/%.o, $(DRIVER_SRC))
DEVICE_OBJ      	= 	$(patsubst %.c, $(BUILDDIR)/%.o, $(DEVICE_SRC))
ifeq ($(CLUSTER), yes)
ifeq ($(RELEASE), no)
CLUSTER_ASM_OBJ		= 	$(patsubst %.S, $(BUILDDIR)/%.o, $(CLUSTER_ASM_SRC))
CLUSTER_OBJ			= 	$(patsubst %.c, $(BUILDDIR)/%.o, $(CLUSTER_SRC))
else
CLUSTER_ASM_OBJ		=
CLUSTER_OBJ			=
endif
else
CLUSTER_ASM_OBJ		=
CLUSTER_OBJ			=
endif

# For RTOS part
RTOS_DIR 			= 	./rtos
PORT_DIR 			= 	./rtos/portable/$(ARCH_NAME)/
ifeq ($(RTOS), yes)
INC_PATH			+= 	$(AT_SDK_HOME_SH)/$(RTOS_DIR)/include \
						$(AT_SDK_HOME_SH)/$(PORT_DIR)

INCLUDES    		+= 	$(addprefix -I,$(call dirs,$(INC_PATH),%.h))
RTOS_PORT_ASM_SRC	= 	$(subst $(AT_SDK_HOME_SH),,$(call wildcards,$(AT_SDK_HOME_SH)/$(PORT_DIR),%.S))
RTOS_DIR_X			=	$(subst ./,,$(RTOS_DIR))
RTOS_SRC        	= 	$(RTOS_DIR_X)/list.c \
                  		$(RTOS_DIR_X)/queue.c \
                  		$(RTOS_DIR_X)/tasks.c \
                  		$(RTOS_DIR_X)/timers.c \
                  		$(RTOS_DIR_X)/event_groups.c \
                  		$(RTOS_DIR_X)/stream_buffer.c
RTOS_PORT_SRC   	= 	$(subst $(AT_SDK_HOME_SH),,$(call wildcards,$(AT_SDK_HOME_SH)/$(PORT_DIR),%.c))

RTOS_OBJ        	= 	$(patsubst %.c, $(BUILDDIR)/%.o, $(RTOS_SRC))
RTOS_PORT_ASM_OBJ   = 	$(patsubst %.S, $(BUILDDIR)/%.o, $(RTOS_PORT_ASM_SRC))
RTOS_PORT_OBJ		= 	$(patsubst %.c, $(BUILDDIR)/%.o, $(RTOS_PORT_SRC))
CFLAGS    			+= 	-DCONFIG_FREERTOS
ASMFLAGS    		+= 	-DCONFIG_FREERTOS
endif

# MIDDLE-WARE
# As middle ware mostly may be ported from 3rd party, the namespace is not easy to control, we use explicit definition below.
MIDWARE_DIR 		=	./middleware
include $(AT_SDK_HOME_SH)/$(MIDWARE_DIR)/component.mk
CFLAGS          	+=	$(MID_CFLAGS)
CPPFLAGS			+=	$(MID_CPPFLAGS)
ASMFLAGS        	+=	$(MID_ASMFLAGS)
LDFLAGS         	+=	$(MID_LDFLAGS)
INCLUDES        	+=	$(MID_INCLUDES)
MID_OBJ        		=	$(patsubst %.c, $(BUILDDIR)/%.o, $(MID_SRC))
MID_CPP_OBJ    		=	$(patsubst %.cpp, $(BUILDDIR)/%.o, $(MID_SRC_CPP))
CPP_INCLUDES        +=   $(MID_CPP_INCLUDES) 

# For gdbscript part
ifneq ($(findstring $(CHIP), AT1K AT820),)
GDB_SCRIPT_TEMPLATE		= 	$(AT_SDK_HOME_SH)/tools/gen_gdbscript/$(CHIP).gdb
#else ifeq ($(CHIP), AT5K)
else ifneq ($(findstring $(CHIP), AT5K AT5050),)
ifeq ($(CODE_IN_DDR), no)
GDB_SCRIPT_TEMPLATE		= 	$(AT_SDK_HOME_SH)/tools/gen_gdbscript/$(CHIP).gdb
else
GDB_SCRIPT_TEMPLATE		= 	$(AT_SDK_HOME_SH)/tools/gen_gdbscript/$(CHIP)_DDR.gdb
endif
endif

GDB_SCRIPT 		= 	$(AT_SDK_HOME_SH)/$(APP_DIR)/load.gdb

# Application part
APP_OBJ				= 	$(patsubst %.c, $(BUILDDIR)/%.o, $(APP_SRC))
APP_CC_OBJ			= 	$(patsubst %.c, $(BUILDDIR)/%.o, $(APP_CC_SRC))
#CPP part
APP_CPP_OBJ			=	$(patsubst %.cpp, $(BUILDDIR)/%.o, $(APP_CPP_SRC))
INCLUDES			+=	$(APP_INC)
CPP_INCLUDES		+=	$(APP_CPP_INC)

# Final 
CPP_INCLUDES		+=	$(INCLUDES)

# Final objects
ASM_OBJS        	= 	$(BOOT_ASM_OBJ) $(RTOS_PORT_ASM_OBJ) $(MID_ASM_OBJ) $(CLUSTER_ASM_OBJ) $(NPU_SDK_ASM_OBJ)
C_OBJS          	= 	$(APP_OBJ) $(APP_CC_OBJ) $(MID_OBJ) $(RTOS_OBJ) $(RTOS_PORT_OBJ) $(DRIVER_OBJ) $(DEVICE_OBJ) $(CLUSTER_OBJ) $(BOOT_OBJ) $(NPU_SDK_C_OBJ)
CPP_OBJS			=	$(MID_CPP_OBJ) $(APP_CPP_OBJ)
OBJS            	= 	$(ASM_OBJS) $(C_OBJS) $(CPP_OBJS)
BIN             	= 	$(TARGET)
BIN_DUMP           	= 	$(TARGET).dump

OBJS_DUMP       	= $(patsubst %.o, %.dump, $(OBJS))
OBJDUMP_OPT 		= -S -D -l -f

# Enable incremental compilation (header dependecies part)
DEPS				=	$(call wildcards,$(AT_SDK_HOME_SH)/$(APP_DIR)/$(BUILDDIR),%.d)

# Makefile targets :
# Build objects (*.o) and associated dependecies (*.d) with disassembly (*.dump).
#------------------------------------------

all::   dir $(LINK_DEPS) $(GDB_SCRIPT) $(OBJS) $(BIN) #$(BIN_DUMP)
dir:
	@mkdir -p $(AT_SDK_HOME_SH)/$(APP_DIR)/$(BUILDDIR)

# Assembly compilation section.
$(ASM_OBJS): $(BUILDDIR)/%.o: $(AT_SDK_HOME_SH)/%.S
	@echo "    SS  $(<F)"
	@mkdir -p $(dir $@)
	@$(CC) $(ASMFLAGS) $(CFLAGS) $(INCLUDES) -MD -MF $(basename $@).d -c -o $@ $<

# C Source code compilation section.
$(C_OBJS): $(BUILDDIR)/%.o : $(AT_SDK_HOME_SH)/%.c
	@echo "    CC  $(<F)"
	@mkdir -p $(dir $@)
	@$(CC) $(MCU_CFLAGS) $(INCLUDES) -MD -MF $(basename $@).d -c -o $@ $<

# C Source code compilation section.
$(CPP_OBJS): $(BUILDDIR)/%.o : $(AT_SDK_HOME_SH)/%.cpp
	@echo "    CX  $(<F)"
	@mkdir -p $(dir $@)
	@$(CXX) $(MCU_CPPFLAGS) $(CPP_INCLUDES) -MD -MF $(basename $@).d -c -o $@ $<

$(BIN): $(OBJS) $(EXTERNAL_OBJ)
	@$(CC) -MMD -MP -o $@ $(LDFLAGS) $+ $(LIBS) $(LIBSFLAGS) $(STRIP)

$(BIN_DUMP): $(BIN)
	@echo "    OBJDUMP  $(<F) > $(@F)"
	@$(OBJDUMP) $(OBJDUMP_OPT) $< > $@ 

$(LINKER_SCRIPT)::
	@$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -I./link -x assembler-with-cpp -nostdinc -undef -E -P $(LINKER_TEMPLATE) -o $@

$(LIB_OBJS): $(BUILDLIBDIR)/%.o : $(AT_SDK_HOME_SH)/%.c
	@echo "    CC  $(<F)"
	@mkdir -p $(dir $@)	
	@$(CC) $(MCU_CFLAGS) $(LIB_CFLAGS) $(LIB_INCLUES) $(INCLUDES) -MD -MF $(basename $@).d -c -o $@ $<

$(LIB_CPP_OBJS): $(BUILDLIBDIR)/%.o : $(AT_SDK_HOME_SH)/%.cpp
	@echo "    CX  $(<F)"
	@mkdir -p $(dir $@)
	@$(CXX) $(MCU_CPPFLAGS) $(LIB_CPPFLAGS) $(LIB_CPP_INCLUES) $(INCLUDES) -MD -MF $(basename $@).d -c -o $@ $<

$(GDB_SCRIPT)::
	@cp $(GDB_SCRIPT_TEMPLATE) $@
	@sed -i -e "s/filename/$(TARGET)/g" $@

-include $(DEPS)

lib: $(LIB_OBJS) $(LIB_CPP_OBJS)
	@echo $(LIB_DIR)
	@echo $(LIB_SRC)
	@echo $(LIB_CPP_SRC)
	@echo $(LIB_OBJS)
	@echo $(LIB_CPP_OBJS)
	@$(AR) -rc $(LIB_TARGET) $(LIB_OBJS) $(LIB_CPP_OBJS)

ifeq ($(CHIP), AT1K)
ifeq ($(SBL), yes)
TAIL_NAME=_sbl
else
TAIL_NAME=
endif
image:
ifeq ($(POSTLOAD), yes)
	@$(AT_SDK_HOME_SH)/tools/gen_image_at1k/gen_code_image$(TAIL_NAME).sh $(AT_SDK_HOME_SH)/$(APP_DIR)/$(TARGET) $(AT_SDK_HOME_SH)/tools/gen_image_at1k
else
	@echo "Error, please set POSTLOAD=yes in Makefile"
endif

burn:
	@$(AT_SDK_HOME_SH)/tools/gen_image_at1k/burn_image$(TAIL_NAME).sh $(AT_SDK_HOME_SH)/$(APP_DIR)/$(TARGET) $(AT_SDK_HOME_SH)/tools/gen_image_at1k

read_flash:
	@$(AT_SDK_HOME_SH)/tools/gen_image_at1k/read_flash.sh $(AT_SDK_HOME_SH)/$(APP_DIR)/$(TARGET) $(AT_SDK_HOME_SH)/tools/gen_image_at1k
else ifeq ($(CHIP), AT5K)
image:
	@$(AT_SDK_HOME_SH)/tools/gen_image_at5k/gen_code_image.sh $(AT_SDK_HOME_SH)/$(APP_DIR)/$(TARGET) $(AT_SDK_HOME_SH)/tools/gen_image_at5k

burn:
	@$(AT_SDK_HOME_SH)/tools/gen_image_at5k/burn_image.sh $(AT_SDK_HOME_SH)/$(APP_DIR)/$(TARGET) $(AT_SDK_HOME_SH)/tools/gen_image_at5k

read_flash:
	@$(AT_SDK_HOME_SH)/tools/gen_image_at5k/read_flash.sh $(AT_SDK_HOME_SH)/$(APP_DIR)/$(TARGET) $(AT_SDK_HOME_SH)/tools/gen_image_at5k
else ifeq ($(CHIP), AT820)
image:
	@$(AT_SDK_HOME_SH)/tools/gen_image_at820/gen_code_image.sh $(AT_SDK_HOME_SH)/$(APP_DIR)/$(TARGET) $(AT_SDK_HOME_SH)/tools/gen_image_at820

burn:
	@$(AT_SDK_HOME_SH)/tools/gen_image_at820/burn_image.sh $(AT_SDK_HOME_SH)/$(APP_DIR)/$(TARGET) $(AT_SDK_HOME_SH)/tools/gen_image_at820

read_flash:
	@$(AT_SDK_HOME_SH)/tools/gen_image_at820/read_flash.sh $(AT_SDK_HOME_SH)/$(APP_DIR)/$(TARGET) $(AT_SDK_HOME_SH)/tools/gen_image_at820
endif

clean::
	@rm -rf $(BUILDTOPDIR) $(BIN) $(BIN).map $(BIN_DUMP) $(LINKER_SCRIPT) $(GDB_SCRIPT) -rf
	@rm -rf $(BIN).*bin $(BIN).*img *.ini ota-sbl.img sbl-text.bin sbl-data.bin

.PHONY: clean dir
