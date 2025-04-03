require recipes-kernel/linux/linux-yocto.inc

COMPATIBLE_MACHINE = "beaglebone-yocto"

LINUX_VERSION ?= "5.15.120"
SRCREV_machine ?= "74328ae91abb636b50975771b5e4da781fec78e3"
SRCREV_meta ?= "f0b575cdf95358b23f6a108788b4dfeeeebce98c"

# Use the same source tree as linux-yocto
SRC_URI = "git://git.yoctoproject.org/linux-yocto.git;name=machine;branch=v5.15/standard; \
           git://git.yoctoproject.org/yocto-kernel-cache;type=kmeta;name=meta;branch=yocto-5.15;destsuffix=${KMETA} \
           file://gpio-sysfs.cfg"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

LINUX_KERNEL_TYPE = "standard"

# This is for the beaglebone - same as in your current config
KMACHINE:beaglebone-yocto = "beaglebone"

SRC_URI += "file://gpio-sysfs.cfg"
KERNEL_CONFIG_FRAGMENTS += "${WORKDIR}/gpio-sysfs.cfg"