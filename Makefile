include $(TOPDIR)/rules.mk

PKG_NAME:=Agentx1
PKG_VERSION:=5
PKG_RELEASE:=$(PKG_SOURCE_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libpthread

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Agentx1
	URL:=https://github.com/Droid-MAX/agentx1
	MAINTAINER:=CrazyBoyFeng<https://bitbucket.org/CrazyBoyFeng/agentx1>
	DEPENDS:=+libpthread
endef

define Package/$(PKG_NAME)/description
	For the Ruijie private 802.1X network authentication protocol for two-way proxy.
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

define Package/$(PKG_NAME)/postinst
#!/bin/sh
SRC=/etc/rc.local
BAK=/etc/rc.local.bak
install(){
	sed -i '/exit/i\/etc/init.d/agentx1 restart' $SRC
}
main(){
	if [ ! -f $BAK ]; then
		cp $SRC $BAK
		install
	else
		install
	fi
}
main
endef

define Package/$(PKG_NAME)/prerm
#!/bin/sh
SRC=/etc/rc.local
BAK=/etc/rc.local.bak
restore(){
	cp -f $BAK $SRC
	rm -rf $BAK
}
main(){
	if [ ! -f $BAK ]; then
		sed -i '/agentx1/d' $SRC
	else
		restore
	fi
}
main
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
