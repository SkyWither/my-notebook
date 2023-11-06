sudo visudo

找到%sudo ALL=(ALL:ALL) ALL
然后添加：
用户名 ALL=(ALL) NOPASSWD:ALL
保存并退出
