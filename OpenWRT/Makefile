include $(TOPDIR)/rules.mk

PKG_NAME:=Agentx1
PKG_VERSION:=0.5
PKG_RELEASE:=$(PKG_SOURCE_VERSION)

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libpthread

include $(INCLUDE_DIR)/package.mk

define Package/Agentx1
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Agentx1
	URL:=https://github.com/Droid-MAX/agentx1
	MAINTAINER:=CrazyBoyFeng<https://bitbucket.org/CrazyBoyFeng/agentx1>
	DEPENDS:=+libpthread
endef

define Package/Agentx1/description
	For the Ruijie private 802.1X network authentication protocol for two-way proxy.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r ../* $(PKG_BUILD_DIR)
endef

define Package/Agentx1/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/agentx1 $(1)/usr/bin
endef

$(eval $(call BuildPackage,Agentx1))
