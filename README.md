# yocto_workshop
Yocto workshop


# Prerequisites
----------------

1. 65 Gbytes of free disk space

2. Runs a supported Linux distribution (i.e. recent releases of Fedora, openSUSE, CentOS, Debian, or Ubuntu). WSL on windows 10 kerlnel does not have support for yocto use virtual machine

3. 	Git 1.8.3.1 or greater
	tar 1.27 or greater
	Python 3.4.0 or greater.
	

4. install needed packeges for ubuntu, for other distros use the equivilent packege manager 
	sudo apt-get install -y gawk wget git-core diffstat unzip texinfo gcc-multilib build-essential chrpath socat cpio python python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev pylint3 xterm python3-subunit zstd liblz4-tool file locales libacl1


(*) Install the required packages for Yocto to Work from
        https://www.yoctoproject.org/docs/latest/ref-manual/ref-manual.html#ubuntu-packages


# to start
in the repo level run these commands

1. git submodule update --init --recursive
2. source ./init_yocto.sh
3. bitbake core-image-minimal

it can take a while 