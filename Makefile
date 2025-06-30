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
  DEPENDS:=+libc +libubox +libubus +libblobmsg-json +libjson-c +libmosquitto +libssl +libcrypto +libuv
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
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mesh_controller $(1)/usr/bin/
	
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/wifi-mesh.init $(1)/etc/init.d/wifi-mesh
	
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/wifi-mesh.config $(1)/etc/config/wifi-mesh
	
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/20-wifi-mesh $(1)/etc/hotplug.d/iface/
	
	$(INSTALL_DIR) $(1)/etc/ssl/certs
	$(INSTALL_DATA) ./files/ca.crt $(1)/etc/ssl/certs/
	
	$(INSTALL_DIR) $(1)/etc/ssl/private
	$(INSTALL_DATA) ./files/client.crt $(1)/etc/ssl/private/
	$(INSTALL_DATA) ./files/client.key $(1)/etc/ssl/private/
endef

$(eval $(call BuildPackage,simple_mesh)) 