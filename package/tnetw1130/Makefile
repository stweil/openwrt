#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

# TODO:
# We use a private configuration if CONFIG_TARGET_ar7_Sinus is defined
# (any other private trigger would have worked).
# Normal user won't need this private configuration.
# It is only used for tnetw1130 development.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=tnetw1130
PKG_REV:=master
PKG_VERSION:=git
PKG_RELEASE:=1

ifdef CONFIG_TARGET_ar7_Sinus
PKG_GIT_URL:=ssh://weil@git.berlios.de/gitroot/ar7-firmware/tnetw1130.git
else
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=git://git.berlios.de/ar7-firmware/tnetw1130.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
endif

include $(INCLUDE_DIR)/package.mk

define KernelPackage/tnetw1130
  SUBMENU:=Wireless Drivers
  TITLE:=TNETW1130 Mac80211 driver
  DEPENDS:=@LINUX_2_6 @PCI_SUPPORT||TARGET_ar7 +kmod-mac80211 @BROKEN
  FILES:=$(PKG_BUILD_DIR)/tnetw1130.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,50,tnetw1130)
endef

define KernelPackage/tnetw1130/description
	Driver for tnetw1330 based cards (Mac80211 version)
endef

C16_FW:=tiacx111c16
C19_FW:=tiacx111c19

ifdef CONFIG_TARGET_ar7_Sinus
define Build/Prepare
	cd $(PKG_BUILD_DIR) && \
	git init && \
	git remote add origin $(PKG_GIT_URL) && \
	git config branch.master.remote origin && \
	git config branch.master.merge refs/heads/master && \
	git pull
endef
endif

define Download/tiacx111c16
  FILE:=$(C16_FW)
  URL:=http://teknoraver.net/software/acx-mac80211/
  MD5SUM:=7026826460376f6b174f9225bd7781b9
endef
$(eval $(call Download,tiacx111c16))

define Download/tiacx111c19
  FILE:=$(C19_FW)
  URL:=http://ipkg.k1k2.de/files/
  MD5SUM:=a1fa9681e297b4e36e257090fc12265a
endef
$(eval $(call Download,tiacx111c19))


PKG_EXTRA_KCONFIG:= \
	CONFIG_TNETW1130=m

PKG_EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(PKG_EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(PKG_EXTRA_KCONFIG)))) \

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		$(PKG_EXTRA_KCONFIG) \
		EXTRA_CFLAGS="$(PKG_EXTRA_CFLAGS)" \
		LINUXINCLUDE="-I$(STAGING_DIR)/usr/include/mac80211 -I$(LINUX_DIR)/include \
			-include linux/autoconf.h" \
		V="$(V)" \
		modules
endef

define Build/Configure
endef

define KernelPackage/tnetw1130/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(DL_DIR)/$(C16_FW) $(1)/lib/firmware/
	$(INSTALL_DATA) $(DL_DIR)/$(C19_FW) $(1)/lib/firmware/
endef

$(eval $(call KernelPackage,tnetw1130))
