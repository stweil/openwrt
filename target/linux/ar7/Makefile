#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=ar7
BOARDNAME:=TI AR7
FEATURES:=squashfs atm low_mem
MAINTAINER:=Florian Fainelli <florian@openwrt.org>
SUBTARGETS:=generic ac49x

KERNEL_PATCHVER:=3.18

#~ LINUX_VERSION:=2.4.37.9
#~ LINUX_VERSION:=2.6.27.49
#~ LINUX_VERSION:=2.6.31.14
#~ LINUX_VERSION:=2.6.32.27
#~ LINUX_VERSION:=2.6.33.7
#~ LINUX_VERSION:=2.6.34.2
#~ LINUX_VERSION:=2.6.35.9
#~ LINUX_VERSION:=2.6.36.2
#~ LINUX_VERSION:=2.6.37

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES+= swconfig

define Target/Description
	Build firmware images for TI AR7 based routers.
endef

$(eval $(call BuildTarget))
