zos-core
========
安装前须知！！！
系统配置：
建议配置：双核、80G硬盘、8G内存
最小配置：双核、40G硬盘、4G内存

我们建议将ZOS Core在Ubuntu 16.04 LTS（64位）上构建
注意：	1.ZOS Core需要使用64位操作系统来构建，并且不能在32位系统上构建;
		2.ZOS CORE使用的boost和openssl的版本比较低，容易引起版本冲突，建议安装在个人目录下，
具体参照下面boost和openssl安装的步骤

#建立依赖关系：
##更新依赖库
sudo apt update

##安装git
sudo apt install git

##安装cmake
sudo apt install cmake

##安装makefile生成工具
sudo apt install autoconf automake libtool

##安装openssl（需要1.0.x版本）
sudo wget https://www.openssl.org/source/openssl-1.0.2o.tar.gz
tar -zxvf openssl-1.1.0e.tar.gz
cd openssl-1.0.2o
./config --prefix=/home/zos/zoscorelib/openssl_1_0_2(不能使用【～】需要使用绝对路径；/home/zos是用户目录)
make
make install

##安装boost库的依赖库
sudo apt install mpi-default-dev libicu-dev python-dev libbz2-dev zlib1g-dev python-gtk2 doxygen libssl-dev

##安装boost库（注意：ZOS CORE只能在boost_1_57_0下编译）
sudo wget http://sourceforge.net/projects/boost/files/boost/1.57.0/boost_1_57_0.tar.gz
tar -zvxf boost_1_57_0.tar.gz
cd boots_1_57_0
./bootstrap.sh  --prefix=/home/zos/zoscorelib/boost_1_57_0(不能使用【～】需要使用绝对路径；/home/zos是用户目录)
./b2
./b2 -a install

##修改用户目录下环境变量系统文件.bashrc
增加如下内容：
export OPENSSL_ROOT_DIR=/home/zos/zoscorelib/openssl_1_0_2
export OPENSSL_INCLUDE_DIR=/home/zos/zoscorelib/openssl_1_0_2/include
export OPENSSL_CRYPTO_LIBRARY=/home/zos/zoscorelib/openssl_1_0_2/lib

export BOOST_ROOT=/home/zos/zoscorelib/boost_1_57_0
export BOOST_LIBRARYDIR=/home/zos/zoscorelib/boost_1_57_0/lib
export BOOST_INCLUDEDIR=/home/zos/zoscorelib/boost_1_57_0/include

##修改完.bashrc文件后需要重新打开Terminal，以保证环境变量生效

#构建脚本
git clone https://github.com/zosnet/zos-core.git
cd zos-core
sh build.sh

#启动节点
##下载
sudo wget https://download.zos.io/node/ubuntu/zos_node

##启动节点
chmod 777 zos_node
./zos_node --rpc-endpoint "127.0.0.1:8190"

等待节点数据同步完成

#启动命令钱包
##下载
sudo wget https://download.zos.io/node/ubuntu/zos_cli

##启动cli命令行钱包

chmod 777 zos_cli
./zos_cli --chain-id="0d3bc516d8ad70c30ce07322297af7005b2d09075def0005b789226a1a9c1823" -s ws://127.0.0.1:8190

初次启动后，新钱包需要设定钱包密码
set_password xxxxxxxx

解锁钱包
unlock xxxxxxxx

然后就可以使用cli命令了


#容易出现的几种错误解决方法
##在安装软件包时出现Could not get lock /var/lib/dpkg/lock-frontend
说明说明之前使用apt时出现异常，没有正常关闭，还在运行。
解决方法：使用ps和gerp查找apt的pid，并使用kill杀掉
.ps -e|grep apt
执行上述命令后会显示类似如下
1436	?	00:00:00	apt.systemd.dai
1459	?	00:00:00	apt.syatemd.dai
然后执行
.sudo kill 1436
.sudo kill 1459
如果仍未解决可以手动删除/var/lib/dpkg/lock
.rm -rf /var/lib/dpgk/lock

##在安装依赖库的过程中，根据网络情况会发生错误，可以切换网络重复执行几次，直到正确为止

##上诉过程中需要下载几个比较大的安装包如boost，可以自行下载后上传到服务器，为了提供更好的体验给用户，ZOS将为您提供下载地址

##boost版本查询：
.dpkg -S /usr/include/boost/version.hpp

##编译过程中如发生CMake不能找到BOOST库的错误，可以使用下面的命令重新安装cmake
.sudo apt-get install cmake libblkid-dev e2fslibs-dev libboost-all-dev libaudit-dev


