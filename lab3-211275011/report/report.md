- 姓名：李润泽
- 学号：211275011
- 邮箱：211275011@smail.nju.edu.cn



## 实验进度

完成了所有任务

## 实验结果

![image-20240511200612948](C:\Users\86150\AppData\Roaming\Typora\typora-user-images\image-20240511200612948.png)



## 实验代码修改位置

- kernel/irqHandle.c 添加了timerHandle，syscallFork，syscallSleep， syscallExit等中断处理函数，以及syscallHandle。
- lib/syscall.c，完成了对上述中断处理函数的封装：fork()，sleep(),exit()。

