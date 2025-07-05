include $(TOPDIR)/rules.mk

PKG_NAME:=simple_mesh
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_MAINTAINER:=ddb65536 <ddb65536@example.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/simple_mesh
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Simple Mesh Network Implementation (MQTT-based)
  DEPENDS:=+libc +libubox +libubus +libblobmsg-json +libjson-c +libmosquitto +libuv
  PKG_FLAGS:=nonshared
endef

define Package/simple_mesh/description
  A simple mesh network implementation for OpenWrt routers using MQTT protocol.
  Provides automatic mesh network formation and management with encrypted communication.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LIBS="-luv -lmosquitto"
endef

define Package/simple_mesh/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/simple_mesh $(1)/usr/bin/

endef

$(eval $(call BuildPackage,simple_mesh)) 