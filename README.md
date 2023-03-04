# 一个可以动态挂载/卸载的Linux内核服务器
- 服务端是`./kernelServer`下的内容
- 客户端是`clientSocket.c`
- 服务端的编译文件是`kernelServer.ko`
- 客户端是`client`
- `Makefile`文件中包含Linux内核的源码位置，使用时注意自行进行更改
```sh
KERNELDIR := /root/study/LinuxDriver/Linux_5_7_9Kernel/linux-5.7.9
```
- 上述编译使用的内核源码的版本为`Linux-5.7.9`