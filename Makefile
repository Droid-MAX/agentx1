include $(TOPDIR)/rules.mk

PKG_NAME:=Agentx1
PKG_VERSION:=5
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libpthread

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Agentx1
	MAINTAINER:=CrazyBoyFeng<https://bitbucket.org/CrazyBoyFeng/agentx1>
	DEPENDS:=+libpthread
endef

define Package/$(PKG_NAME)/description
	Ruijie 802.1X Network Authentication Protocol Bidirectional Proxy Tool.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r src/* $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install
	cp -r root/* $(1)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/agentx1 $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
