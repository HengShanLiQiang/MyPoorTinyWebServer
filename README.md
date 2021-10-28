# MyPoorTinyWebServer
### Linux下C++轻量级Web服务器
借鉴了游双老师《Linux高性能服务器编程》一书
* 使用**半同步/半反应堆线程池**的并发模型
* 使用**状态机**解析HTTP请求报文，目前仅支持解析**GET**请求

压力测试器使用：

``./stress_test 192.168.153.130 1234 10``

更新：
第二次迭代 2021/10/18：修复了一堆bug，可以运行，但webbench测试很多failed，暂不明原因

第2.5次迭代 2021/10/28：添加注释、添加一个简单的压力测试器、增加一个html页面
