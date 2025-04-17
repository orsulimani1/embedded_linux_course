SUMMARY = "GPIO LED Test Application using sysfs"
DESCRIPTION = "A simple application to control BeagleBone Black LEDs using GPIO sysfs interface"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://00_led.c"

S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} 00_led.c -o gpio-led-test
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 gpio-led-test ${D}${bindir}
}