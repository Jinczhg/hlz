ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

include $(MKFILES_ROOT)/qmacros.mk

IS_DEBUG_BUILD:=$(filter g, $(VARIANT_LIST))
DBG_LIBSUFFIX := $(if $(IS_DEBUG_BUILD),_g)
DBG_DIRSUFFIX := $(if $(IS_DEBUG_BUILD),-debug)

CCFLAGS += $(if $(IS_DEBUG_BUILD), -O0)

# You can change the NAME line to rename the .so file, but don't change
# SONAME_DLL--the WFD driver is linked against a specific SONAME.
SONAME_DLL=$(IMAGE_PREF_SO)wfdcfg$(IMAGE_SUFF_SO).0
NAME=$(IMAGE_PREF_SO)wfdcfg-$(PROJECT)


INSTALLDIR:=/usr/lib/graphics/omap4430$(DBG_DIRSUFFIX)

# Install on other wfd variant that might use the generic wfdcfg lib
POST_INSTALL += $(CP_HOST) $(INSTALL_ROOT_DLL)/$(INSTALLDIR)/$(NAME)$(DBG_LIBSUFFIX)$(IMAGE_SUFF_SO) $(INSTALL_ROOT_DLL)/usr/lib/graphics/omap4460$(DBG_DIRSUFFIX)/;
POST_INSTALL += $(CP_HOST) $(INSTALL_ROOT_DLL)/$(INSTALLDIR)/$(NAME)$(DBG_LIBSUFFIX)$(IMAGE_SUFF_SO) $(INSTALL_ROOT_DLL)/usr/lib/graphics/omap5430$(DBG_DIRSUFFIX)/;
POST_INSTALL += $(CP_HOST) $(INSTALL_ROOT_DLL)/$(INSTALLDIR)/$(NAME)$(DBG_LIBSUFFIX)$(IMAGE_SUFF_SO) $(INSTALL_ROOT_DLL)/usr/lib/graphics/omap5430ES2_0$(DBG_DIRSUFFIX)/;
POST_INSTALL += $(CP_HOST) $(INSTALL_ROOT_DLL)/$(INSTALLDIR)/$(NAME)$(DBG_LIBSUFFIX)$(IMAGE_SUFF_SO) $(INSTALL_ROOT_DLL)/usr/lib/graphics/jacinto6$(DBG_DIRSUFFIX)/;

DESC=Returns a list of display timings and extensions for the generic implementation of omap4-5-j6.

define PINFO
PINFO DESCRIPTION = "$(DESC)"
endef

include $(MKFILES_ROOT)/qtargets.mk
