 
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=ar7
BOARDNAME:=TI AR7
FEATURES:=squashfs jffs2 atm

LINUX_VERSION:=2.6.27.29

#~ LINUX_VERSION:=2.6.23.17
#~ LINUX_VERSION:=2.6.24.7
#~ LINUX_VERSION:=2.6.25.20
#~ LINUX_VERSION:=2.6.26.8
#~ LINUX_VERSION:=2.6.27.29
#~ LINUX_VERSION:=2.6.28.10
#~ LINUX_VERSION:=2.6.29.4
LINUX_VERSION:=2.6.30.4

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES+= kmod-pppoa ppp-mod-pppoa linux-atm atm-tools br2684ctl \
		kmod-acx

define Target/Description
	Build firmware images for TI AR7 based routers.
endef

$(eval $(call BuildTarget))
