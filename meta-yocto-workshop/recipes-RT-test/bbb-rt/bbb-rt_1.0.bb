DESCRIPTION = "Example recipe for hello yocto application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://08_bbb_rt.c"
S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} 08_bbb_rt.c -o 08_bbb_rt ${LDFLAGS}  -lrt
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 08_bbb_rt ${D}${bindir}/08_bbb_rt
}

FILES:${PN} = "${bindir}/08_bbb_rt"