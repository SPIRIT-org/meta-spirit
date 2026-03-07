FILESEXTRAPATHS:prepend := "${THISDIR}/files/${MACHINE}:"

# Only use the dtsi for the spirit-phone-cm5 target
SRC_URI:append:spirit-phone-cm5 = " \
	file://spirit-phone-cm5.dtsi \
        file://bcm2712-spirit-phone-cm5.dts \
	"

do_configure:append:spirit-phone-cm5() {
    # Copy custom dtsi into kernel DTS tree
    cp ${WORKDIR}/spirit-phone-cm5.dtsi \
       ${S}/arch/arm64/boot/dts/broadcom/
    cp ${WORKDIR}/bcm2712-spirit-phone-cm5.dts \
       ${S}/arch/arm64/boot/dts/broadcom/
}

KERNEL_DEVICETREE:append:spirit-phone-cm5 = " broadcom/bcm2712-spirit-phone-cm5.dtb"
