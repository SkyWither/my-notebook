linux分区的目录结构
	linux分区可以挂载在某个目录下
	windows每个分区是一个盘
	linux没有盘符概念

介绍挂载linux分区的相关概念
![[Pasted image 20231024144511.png]]
介绍挂载概念
![[Pasted image 20231024144711.png]]

这里需要知道
Linux中分区从属于目录，Windows中目录从属于分区。
的区别

分区查看命令：
![[Pasted image 20231024144845.png]]
![[Pasted image 20231024145252.png]]

介绍设备文件：/dev/sda
linux中dev的sda指的是硬盘;dev是device的缩写

df命令可以查看每个文件系统挂载到哪里
![[Pasted image 20231024145015.png]]
![[Pasted image 20231024145049.png]]
![[Pasted image 20231024145333.png]]


介绍了光驱的设备文件：/dev/sr0
