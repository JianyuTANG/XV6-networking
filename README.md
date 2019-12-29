# SS.THU-Xv6
清华大学《操作系统》课程实验 Xv6 系统 | by 软件学院七字班

分支说明：
  networking-feature 组长 唐建宇

## 进度：

- [x] 将基本内容整合进六字班版本并可以正确运行，整合了arptest命令以及icmp发包协议
- [x] 修改了e1000网卡驱动，解决了中断不触发的问题
- [x] 完成了ipconfig命令
- [x] 实现了socket支持
- [x] 完成了应用层的sockettest命令
- [x] 完成了ping命令



## 测试方法

- ipconfig

  - 直接执行 ipconfig
  - 修改本机ip ipconfig inet 10.0.2.15
  - 修改网关ip ipconfig gateway 10.0.2.2
- arptest

  直接执行 arptest 10.0.2.2
  
- ping

  在host机启动python ping_server_simulator.py，在qemu中输入ping 10.0.2.2 即可
  
- sockettest

  在host机启动python server.py，在qemu中输入ping content 即可，content为一段不含空格字符串
