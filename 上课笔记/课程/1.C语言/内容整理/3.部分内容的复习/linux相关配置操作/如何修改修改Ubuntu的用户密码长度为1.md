sudo vim /etc/pam.d/common-password

找到password requisite pam_pwquality.so
在后面加 minlen=1

然后使用sudo passwd 用户名 来修改密码