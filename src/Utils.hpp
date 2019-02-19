/*************************************************************************
*File Name: Utils.hpp
*Author: AHAOAHA
*mail: ahaoaha_@outlook.com
*Created Time: 2019年02月19日 星期二 14时43分51秒
 ************************************************************************/
#pragma once
#include <string>
#include <unordered_map>
#include <sys/stat.h>
#include <iostream>

using std::cout;
using std::endl;
using std::cerr;

typedef bool (*handler_t)(int sock);
class RequestInfo { //记录http解析出来的请求信息
  public:
    std::string _method;  //请求方法
    std::string _version; //协议版本
    std::string _path_info; //请求的资源路径
    std::string _path_phys; //请求的资源在服务器中的实际路径
    std::string _query_string;  //查询字符串
    std::unordered_map<std::string, std::string> _hdr_list; //整个头部信息的键值对
    struct stat _st;  //获取文件信息
};

class HttpTask {
  private:
    int _cli_sock;  //记录客户端sock
    handler_t _task_handler;  //处理方法
  public:
    HttpTask(int sock, handler_t handler): _cli_sock(sock), _task_handler(handler)  //创建线程池中的任务 
    {}

    void Run()
    {
      _task_handler(_cli_sock);
    }

};

