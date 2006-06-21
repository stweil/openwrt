ifneq ($(strip $(PKG_CAT)),)
  ifeq ($(PKG_CAT),unzip)
    UNPACK=unzip -d $(PKG_BUILD_DIR) $(DL_DIR)/$(PKG_SOURCE)
  else
    UNPACK=$(PKG_CAT) $(DL_DIR)/$(PKG_SOURCE) | tar -C $(PKG_BUILD_DIR)/.. $(TAR_OPTIONS) -
  endif
  define Build/Prepare/Default
  	$(UNPACK)
	@if [ -d ./patches ]; then \
		$(PATCH) $(PKG_BUILD_DIR) ./patches; \
	fi
  endef
endif

define Build/Prepare
  $(call Build/Prepare/Default)
endef

define Build/Configure/Default
	@(cd $(PKG_BUILD_DIR)/$(3); \
	[ -x configure ] && \
		$(2) \
		CPPFLAGS="-I$(STAGING_DIR)/host/include" \
		LDFLAGS="-L$(STAGING_DIR)/host/lib" \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--program-prefix="" \
		--program-suffix="" \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share \
		--localstatedir=/var \
		--mandir=/usr/man \
		--infodir=/usr/info \
		$(DISABLE_NLS) \
		$(1); \
		true; \
	)
endef

define Build/Configure
  $(call Build/Configure/Default)
endef

define Build/Compile/Default
	$(MAKE) -C $(PKG_BUILD_DIR) $(1)
endef

define Build/Compile
  $(call Build/Compile/Default)
endef

		
ifneq ($(strip $(PKG_SOURCE)),)
  source: $(DL_DIR)/$(PKG_SOURCE)

  $(DL_DIR)/$(PKG_SOURCE):
	mkdir -p $(DL_DIR)
	$(SCRIPT_DIR)/download.pl "$(DL_DIR)" "$(PKG_SOURCE)" "$(PKG_MD5SUM)" $(PKG_SOURCE_URL)

  $(PKG_BUILD_DIR)/.prepared: $(DL_DIR)/$(PKG_SOURCE)
endif

define HostBuild
  $(PKG_BUILD_DIR)/.prepared:
	@-rm -rf $(PKG_BUILD_DIR)
	@mkdir -p $(PKG_BUILD_DIR)
	$(call Build/Prepare)
	touch $$@

  $(PKG_BUILD_DIR)/.configured: $(PKG_BUILD_DIR)/.prepared
	$(call Build/Configure)
	touch $$@

  $(PKG_BUILD_DIR)/.built: $(PKG_BUILD_DIR)/.configured
	$(call Build/Compile)
	touch $$@

  $(STAGING_DIR)/stampfiles/.host_$(PKG_NAME)-installed: $(PKG_BUILD_DIR)/.built
	$(call Build/Install)
	touch $$@
	
  ifdef Build/Install
    install-targets: $(STAGING_DIR)/stampfiles/.host_$(PKG_NAME)-installed
  endif

  package-clean: FORCE
	$(call Build/Clean)
	$(call Build/Uninstall)
	rm -f $(STAGING_DIR)/stampfiles/.host_$(PKG_NAME)-installed

  source:
  prepare: $(PKG_BUILD_DIR)/.prepared
  configure: $(PKG_BUILD_DIR)/.configured

  compile-targets: $(PKG_BUILD_DIR)/.built
  compile: compile-targets

  install-targets:
  install: install-targets

  clean-targets:
  clean: FORCE
	@$(MAKE) clean-targets
	$(call Build/Clean)
	rm -rf $(PKG_BUILD_DIR)

endef
