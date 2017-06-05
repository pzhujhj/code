#
# Copyright (C) 2008-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=myubus
PKG_VERSION:=1.0
PKG_RELEASE:=
PKG_REV:=

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/nls.mk

define Package/myubus
  SECTION:=xxx
  CATEGORY:=xxx
  DEPENDS:=+libubox +libubus
  TITLE:=ubus api of myubus
endef

define Package/myubus/description
	myubus service daemon
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include -I$(ICONV_CFLAGS) 
TARGET_LDFLAGS+= -ljson-c -lubus -lubox -luci -L$(ICONV_LDFLAGS) -liconv -lblobmsg_json 

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/*  $(PKG_BUILD_DIR)/
endef


define Package/myubus/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/myubus $(1)/usr/sbin
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,myubus))
