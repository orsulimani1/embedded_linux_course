DESCRIPTION = "Example recipe for hello yocto application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://07_cyclic_test.c"
S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} 07_cyclic_test.c -o 07_cyclic_test ${LDFLAGS}  -lrt
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 07_cyclic_test ${D}${bindir}/07_cyclic_test
}

FILES:${PN} = "${bindir}/07_cyclic_test"