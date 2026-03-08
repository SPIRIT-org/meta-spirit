FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
FILESEXTRAPATHS:prepend := "${THISDIR}/files/${MACHINE}:"

# Only use the dtsi for the spirit-phone-cm5 target
SRC_URI:append:spirit-phone-cm5 = " \
	file://spirit-phone-cm5.dtsi \
        file://bcm2712-spirit-phone-cm5.dts \
        file://panel-wd5ea5f01-boe.c \
        file://0001-add-BOE-WD5ea5f01-panel-driver-to-kernel.patch \
	"

do_patch:append:spirit-phone-cm5() {
    # Copy custom dtsi into kernel DTS tree
    cp ${WORKDIR}/spirit-phone-cm5.dtsi \
       ${S}/arch/arm64/boot/dts/broadcom/
    cp ${WORKDIR}/bcm2712-spirit-phone-cm5.dts \
       ${S}/arch/arm64/boot/dts/broadcom/
    cp ${WORKDIR}/panel-wd5ea5f01-boe.c \
       ${S}/drivers/gpu/drm/panel/
}

KERNEL_DEVICETREE:append:spirit-phone-cm5 = " broadcom/bcm2712-spirit-phone-cm5.dtb"

# Equivalent of CONFIG_DRM_PANEL_BOE_WD5EAF01=y
KERNEL_CONFIG_ENABLE:append:spirit-phone-cm5 = " DRM_PANEL_BOE_WD5EAF01"
