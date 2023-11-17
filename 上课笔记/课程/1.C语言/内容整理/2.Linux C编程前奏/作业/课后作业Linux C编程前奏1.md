# 1. 桌面系统运行在哪个终端？在虚拟机下如何打开其他终端？
Ubuntu默认的桌面系统运行在tty1终端
虚拟机下可以打开硬终端和软终端
1.桌面下按鼠标右键，然后选择Open Terminal
2.ctrl + alt + t
3.在桌面里选择Terminal的应用图标打开终端
4.ctrl + alt + F1~F7
[彻底理解linux的各种终端类型以及概念](https://blog.csdn.net/gdfhhj/article/details/87732985)
[[真终端]]：关于Ubuntu默认的桌面系统在哪个目录的笔记
[[Ubuntu打开真终端的快捷键]]
# 2. SHELL 是什么？
linux shell是Linux系统的**命令解释器**，还是内核的保护壳。
它**提供了一个接口**，允许用户通过键入命令来操纵系统。
shell通过读取、解析和执行命令，来完成诸如启动程序、移动文件、打印文本等任务。
它隐藏了操作系统的复杂性，为用户提供了一个简单易用的界面。
使用shell，我们可以**操控文件系统**、**管理程序**、**编写脚本**来实现自动化任务等。

# 3. 写出安装 xxxx 的命令。
sudo apt-get install xxxx
[[apt-get命令]]
[[APT软件包管理]]
# 4. Linux 中有 C 盘、D 盘等这些盘符的概念吗？它是如何管理分区的（分区与目录的关系）？
Linux中没有像Windows里那样的盘符概念。
Linux采用一种称为“**挂载点(mount point)**”的方式来管理分区。
每个分区都会挂载在文件系统的某个目录下，然后那个目录就可以像访问普通目录一样访问这个分区了。
例如,可以将一个分区挂载在/home目录下。这个分区的所有内容就都可以在/home目录访问到。可以将另一个分区挂载在/data下，这样这个分区的内容就可以通过/data目录访问。
根目录"/"自己也是一个分区。其他的一些常见挂载点还有：/boot(保存引导文件)、/usr(程序文件)、/var(变量数据)等。
通过mount和umount命令可以挂载和卸载分区。/etc/fstab文件中定义了分区信息和挂载点。
所以Linux的目录与分区不是严格对应的关系。多个分区可以挂载到同一个目录下，一个分区也可以只挂载其中的一个子目录。这种机制为Linux提供了非常灵活的分区和文件系统管理机制。
[[linux分区]]
# 5. 解释以下命令提示符：
root@ubuntu:/usr/include# 

username@hostname:pathname$
root - 当前登录的用户名是root。root用户是Linux系统的超级用户。
ubuntu - 系统的主机名是ubuntu。
/usr/include - 表示当前所在的工作目录是/usr/include。/usr是存放系统应用程序的目录，include子目录通常用于存放开发头文件。
\# - 表示当前用户是root。对于普通用户，提示符中会显示$，而不是#。
[[shell命令提示符相关内容]]
# 6. 请分三行输入命令：ls -l /usr/include
ls \
-l \
/usr/include
！这里需要注意-l之后要有空格
[[shell命令的一般格式]]
# 7. 请在一行中执行以下三个命令：
echo "Please input a number:"
read NUMBER
echo You input NUMBER = $NUMBER

echo "Please input a number:";read NUMBER;echo You input NUMBER = $NUMBER
！这里之后需要知道read命令和echo命令的作用
[[shell命令的一般格式]]
# 8. 请显示 /usr/share/file 下的详细内容，并解释这些内容

![[Pasted image 20231025000322.png]]
total 后面的数字 20 表示该目录下所有文件和子目录的大小总和为 20 个块
目录下内容：
	第一部分
		第一个字符表示文件系统的类型字符，其中d代表目录，l代表符号链接
		接下来每三个字符为一个部分，则
			第一部分代表：文件所有者的权限
			第二部分代表：文件所属组的权限
			第三部分代表：除文件所有者和所属组外的其他用户的权限
			其中，r对应的权限者可读，w代表对应的权限者可写，x代表对应的权限者可执行，如果没有对应的权限，则使用 - 来代替
	第二部分
		这个数字代表此目录所对应的硬链接个数，或者是目录下的子目录个数
	第三部分
		文件目录的所有者
	第四部分
		文件目录的所属组
	第五部分
		文件或目录的大小，以字节为单位
	第六部分
		最近修改时间
	第七部分
		文件或目录的名称
[[ls命令]]
# 9. 请显示家目录下除 . 和 .. 外其余全部内容
ls -A ~
![[Pasted image 20231025003436.png]]
[[ls命令]]
# 10. 请复制 /usr/include/stdio.h 文件到家目录
cp /usr/include/stdio.h ~
![[Pasted image 20231025003543.png]]
[[cp命令]]
# 11. 请复制 /home/farsight/vmware-tools-distrib 目录为 /home/farsight/vm
cp -r /home/farsight/vmware-tool-distrib /home/farsight/vm
![[Pasted image 20231025003714.png]]
[[cp命令]]
# 12. 请删除家目录下的文件 stdio.h
rm ~/stdio.h
[[rm命令]]
# 13. 请删除家目录下的目录 vm
rm -r ~/vm
[[rm命令]]
