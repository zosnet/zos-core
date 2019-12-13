zos-core
========
Before you install!!! 
System configuration: 
Recommended configuration: dual core, 80G hard disk, 8G memory Minimum configuration: dual core, 40G hard disk, 4G memory

We recommend building ZOS Core on Ubuntu 16.04 LTS (64-bit) Note: 1. ZOS Core needs to be built with a 64-bit operating system and cannot be built on 32-bit systems; 2. ZOS CORE uses a lower version of boost and openssl, easy to cause version conflict, it is recommended to install in the personal directory, refer to the following steps of boost and openssl installation.

#establish dependencies:
##Update the Dependency Library
sudo apt update

##Install git
sudo apt install git

##Install cmake
sudo apt cmake

##Install makefile generation tool
sudo apt autoconf automake libtool

##Install openssl (requires 1.0.x version) sudo wget https://www.openssl.org/source/openssl-1.0.2o.tar.gz tar -zxvf openssl-1.1.0e.tar.gz cd openssl-1.0 .2o ./config --prefix = / home / zos / zoscorelib / openssl_1_0_2 (cannot use [～] requires absolute path; / home / zos is the user directory) make make install

##Install the dependent libraries of the boost library sudo apt install mpi-default-dev libicu-dev python-dev libbz2-dev zlib1g-dev python-gtk2 doxygen libssl-dev

##Install the boost library (note: ZOS CORE can only be compiled under boost_1_57_0) sudo wget http://sourceforge.net/projects/boost/files/boost/1.57.0/boost_1_57_0.tar.gz tar -zvxf boost_1_57_0.tar .gz cd boots_1_57_0 ./bootstrap.sh --prefix = / home / zos / zoscorelib / boost_1_57_0 (Cannot use [～] requires an absolute path; / home / zos is the user directory) ./b2 ./b2 -a install

##To modify the environment variable system file in the user directory.bashrc add the following:
export OPENSSL_ROOT_DIR=/home/zos/zoscorelib/openssl_1_0_2
export OPENSSL_INCLUDE_DIR=/home/zos/zoscorelib/openssl_1_0_2/include
export OPENSSL_CRYPTO_LIBRARY=/home/zos/zoscorelib/openssl_1_0_2/lib

export BOOST_ROOT=/home/zos/zoscorelib/boost_1_57_0
export BOOST_LIBRARYDIR=/home/zos/zoscorelib/boost_1_57_0/lib
export BOOST_INCLUDEDIR=/home/zos/zoscorelib/boost_1_57_0/include
##After modifying the .bashrc file, you need to reopen Terminal to ensure that the variables take effect.

#writing the script:
git clone https://github.com/zosnet/zos-core.git
cd zos-core
sh build.sh

#starting up the nodes
##Download
sudo wget https://download.zos.io/node/ubuntu/zos_node

##Starting the nodes
chmod 777 zos_node
./zos_node --rpc-endpoint "127.0.0.1:8190"

Please wait for node data synchronization to complete

#launch the command wallet
##Download
sudo wget https://download.zos.io/node/ubuntu/zos_cli

##Launch cli command line wallet

chmod 777 zos_cli
./zos_cli --chain-id="0d3bc516d8ad70c30ce07322297af7005b2d09075def0005b789226a1a9c1823" -s ws://127.0.0.1:8190

After the initial launch, the new wallet needs to be set with the wallet password
set_password xxxxxxxx

Unlock the wallet
unlock xxxxxxxx

And then you can use the cli command.


#ways to solve several error resolution

##Could not get lock / var / lib / dpkg / lock-frontend when installing software packages. Description: An exception occurred when using apt before, it was not closed normally, and it is still running. Solution: Use ps and gerp to find the pid of apt, and use kill to kill. Ps -e | grep apt. After executing the above command, it will display something like the following: 1436? 00:00:00 apt.systemd.dai 1459? 00:00: 00 apt.syatemd.dai then execute
.sudo kill 1436
.sudo kill 1459

If still unresolved, manually remove /var/lib/ DPKG /lock

##During the installation of dependent libraries, errors will occur according to the network situation. You can switch the network and repeat the execution several times until it is correct.

##The appeal process requires downloading several large installation packages, such as boost, which can be downloaded and uploaded to the server. To provide a better experience for users, ZOS will provide you with the download address

##boost Version query:
.dpkg -S /usr/include/boost/version.hpp

##If CMake fails to find the BOOST library during compilation, you can reinstall it using the following command
cmake
.sudo apt-get install cmake libblkid-dev e2fslibs-dev libboost-all-dev libaudit-dev



