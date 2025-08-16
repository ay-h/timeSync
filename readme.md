	一个自动从同步系统时间的小程序，自动从ntp服务器获取时间并更新系统时间。

配置文件说明:

```json
{
    "isShowWindow": false,  // 启动的时候是否显示主窗口，不显示则缩小到任务栏
    "ntpServer": "time.nist.gov",  // ntp 服务器地址,不设置的话默认使用阿里的
    "systemTimeCheckInterval": 120, // 检测间隔(秒)，多少秒对比一次本地时间与服务器时间
    "timeOffset": 5   // 时间差(秒)，当本地时间与服务器时达到设置的时间差则更新本地时间
}
```

