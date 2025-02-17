DESCRIPTION = "Example recipe for hello yocto application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

OE_TERMINAL = "tmux"

SRC_URI = "file://hello_yocto.c"
S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} hello_yocto.c -o hello_yocto ${LDFLAGS}
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 hello_yocto ${D}${bindir}/hello_yocto
}

FILES_${PN} = "${bindir}/hello_yocto"