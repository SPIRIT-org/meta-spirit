FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

# Only use the dtsi for the spirit-phone-cm5 target
SRC_URI:append:spirit-phone-cm5 = " \
	file://spirit-phone-cm5.dtsi \
	file://spirit-phone-power.dtsi \
	file://0001-include-spirit-phone-cm5-dtsi.patch \
	"

do_configure:append:spirit-phone-cm5() {
    # Copy custom dtsi into kernel DTS tree
    cp ${WORKDIR}/spirit-phone-cm5.dtsi \
       ${S}/arch/arm64/boot/dts/broadcom/
	 cp ${WORKDIR}/spirit-phone-power.dtsi \
		 ${S}/arch/arm64/boot/dts/broadcom/
}
	
KERNEL_DEVICETREE:append:spirit-phone-cm5 = " broadcom/bcm2712-rpi-5-b.dtb"
