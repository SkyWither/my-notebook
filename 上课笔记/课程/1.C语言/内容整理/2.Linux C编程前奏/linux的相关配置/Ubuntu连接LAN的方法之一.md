这里需要先给Ubuntu连接LAN，那怎么连接呢？
方法1：
sudo service network-manager stop
sudo rm /var/lib/NetworkManager/NetworkManager.state
sudo service network-manager start


方法2：
sudo vi /etc/NetworkManager/NetworkManager.conf
将managed=false改成managed=true
sudo service network-manager restart


虚拟机->啊网络适配器->移除，然后添加->网络适配器->桥接模式（如果没有效果，则选择NAT模式），确定。
