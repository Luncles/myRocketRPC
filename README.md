### 日志模块

> 1.日志级别
> 2. 打印到文件，支持日期命名，以及日志的滚动。
> 3. c 格式化风控
> 4. 线程安全

#### LogLevel:

> Debug
> Info
> Error

#### LogEvent:

> 文件名、行号
> MsgNo
> 进程号
> Thread id
> 日期，以及时间。精确到 ms
> 自定义消息

#### 日志格式

> [Level]\\t
>
> [%y-%m-%d %H:%M:%s.%ms]\t
>
> [pid:thread_id]\t
>
> [file_name:line][%msg]\t

#### Logger 日志器

> 1.提供打印日志的方法
>
> 2.设置日志输出的路径

### 定时器模块

#### timer_event：定时任务

> std::function<void()> myTask
>
> int64_t myArriveTime;     // 定时时间:ms
>
> int64_t myInterval;       // 时间间隔:ms
> bool myIsRepeated{false}; // 是否要重复定时器
> bool myIsCanceled{false}; // 是否要取消定时器

#### timer：定时器容器类

是timer_event的集合，继承于fd_event类

> 添加定时器任务到定时器表中
>
> voidAddTimerEvent(TimerEvent::myTimerEventPtrtimeEvent);
>
> voidDeleteTimerEvent(TimerEvent::myTimerEventPtrtimeEvent);
>
> // 触发定时器事件，统一事件源，当发生IO事件后，eventloop会执行这个事件处理器，执行对应的TimerEvent的回调函数
>
> voidOnTimer();
>
> voidResetTimerArriveTime();
>
>
> // 管理定时器容器：multimap，底层是红黑树，会根据触发时间对timer event进行排序
>
>     std::multimap<int64_t, TimerEvent::myTimerEventPtr>myTimerEventMap;


### IO线程

创建一个IO 线程，他会帮我们执行：

1. 创建一个新线程（pthread_create）
2. 在新线程里面 创建一个 EventLoop，完成初始化
3. 开启 loop
