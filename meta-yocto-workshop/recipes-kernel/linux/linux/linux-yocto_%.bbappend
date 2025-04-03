FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://gpio-sysfs.cfg"
KERNEL_CONFIG_FRAGMENTS += "${WORKDIR}/gpio-sysfs.cfg"

# Make linux-yocto compatible with beaglebone-yocto
COMPATIBLE_MACHINE:beaglebone-yocto = "beaglebone-yocto"