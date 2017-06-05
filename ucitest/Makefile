
include $(TOPDIR)/rules.mk

PKG_NAME:=ucitest
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

define Package/ucitest/Default
  SECTION:=XXX
  CATEGORY:=XXX
  TITLE:=ucitest application
  DEPENDS:= +libuci +libubox +libubus
endef

define Package/ucitest
$(call Package/ucitest/Default)
  TITLE:=uci Daemon
endef

define Package/ucitest/description
	my ucitest application
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)	
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS+= -ljson-c -lubus -lubox -luci

define Build/Compile
	$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include -std=gnu99" LDFLAGS="$(TARGET_LDFLAGS)" \
		$(MAKE_FLAGS) $(MAKE) -C $(PKG_BUILD_DIR)
endef

define Package/ucitest/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ucitestd $(1)/usr/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,ucitest))
$(eval $(call BuildPackage,ucitest-led))

