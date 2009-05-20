#!/usr/bin/make
#
# Copyright (C) 2007, 2008 Stefan Weil
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Sinus
  NAME:=Sinus 154 DSL Basic SE
  PACKAGES:=kmod-acx-mac80211 kmod-sangam-atm-annex-b
  #~ PACKAGES:=kmod-acx kmod-sangam-atm-annex-b
endef

define Profile/Sinus/Description
	Package set compatible with hardware using Texas Instruments WiFi cards
endef
$(eval $(call Profile,Sinus))
