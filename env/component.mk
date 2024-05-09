ifeq ($(LIBANSAI_ZX), yes)
LIB_ANSAI_ZX_DIR	=	./audio-lib/ansai-zx/libansai_zx-$(LIBANSAI_ZX_VERSION)
APP_INC				+=	$(addprefix -I,$(call dirs,$(AT_SDK_HOME)/$(LIB_ANSAI_ZX_DIR),%.h))
USERLIB_LDFLAGS		+=	-L$(AT_SDK_HOME_SH)/$(LIB_ANSAI_ZX_DIR)
ALG_MODULES			+=	-lansai_zx
CFLAGS 				+=	-DLIBANSAI_ZX_VER=$(LIBANSAI_ZX_VERSION)
else
LIB_ANSAI_ZX_DIR	=	./audio-lib/ansai-zx/libansai_zx-20230906
APP_INC				+=	$(addprefix -I,$(call dirs,$(AT_SDK_HOME)/$(LIB_ANSAI_ZX_DIR),%.h))
CFLAGS 				+=	-DLIBANSAI_ZX_VER=20230906
endif
