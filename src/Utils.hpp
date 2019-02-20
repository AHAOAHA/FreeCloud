/*************************************************************************
*File Name: Utils.hpp
*Author: AHAOAHA
*mail: ahaoaha_@outlook.com
*Created Time: 2019年02月19日 星期二 14时43分51秒
 ************************************************************************/
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>
#include <unordered_map>
#include <sys/stat.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <queue>
#include <arpa/inet.h>
#include <unordered_map>

#define MAX_BUFSIZE 4096

using std::cout;
using std::endl;
using std::cerr;

std::unordered_map<std::string, std::string> ErrorCode({ \
    {"200", " OK\r\n"}, \
    {"400", " Bad Request\r\n"}, \
    {"404", " Not Found\r\n"}, \
    {"414", " Request-URL Too Long\r\n"}, \
    {"500", " Internal Server Error\r\n"}});

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


bool FetchHttpHeader(int sock, std::string& hdr)
{
  char buf[MAX_BUFSIZE] = {0};
  while(1) {
    int ret = recv(sock, buf, MAX_BUFSIZE, MSG_PEEK); //MSG_PEEK指从接受缓冲区中查看数据，但不会拿走数据
    if(ret < 0) { // <0 代表读取出错
      if(errno == EINTR || errno == EAGAIN)
      {
        continue;
      }
      return false;
    }

    char* ptr = strstr(buf, "\r\n\r\n");
    if(ptr == NULL) {
      if(ret >= MAX_BUFSIZE)
      {
        cerr << "header too long" <<endl;
        return false;
      }
    }
    else {
      int hdr_lenth = ptr - buf;
      hdr.assign(buf, hdr_lenth);
      recv(sock, buf, hdr_lenth + 4, 0);
      break;
    }
  }
  return true;
}

void HdrCutByStr(const std::string hdr, const std::string cut_str, std::vector<std::string> &v_info) {
  size_t start = 0;
  size_t pos = 0;
  std::string val;
  while(pos != std::string::npos) {
    pos = hdr.find(cut_str, start);
    val.assign(hdr, start, pos - start);
    v_info.push_back(val);
    val.resize(0);
    start = pos + cut_str.size();
  }
}


bool ParseHttpHeader(std::string hdr, RequestInfo& hdr_info) {
  std::vector<std::string> v_info;  //将头部的信息存储进vector中
  HdrCutByStr(hdr, "\r\n", v_info); //将头部信息切割并存储进vector
  
  std::vector<std::string> v_first_line;
  HdrCutByStr(v_info[0], " ", v_first_line);  //将首行的信息保存在v_first_line
  hdr_info._method = v_first_line[0];
  hdr_info._version = v_first_line[2];
  std::vector<std::string> v_path_query;
  HdrCutByStr(v_first_line[1], "?", v_path_query);
  if(v_path_query.size() == 2)
  {
    hdr_info._query_string = v_path_query[1];
  }
  hdr_info._path_info = v_path_query[0];
  hdr_info._path_phys = ".";
  hdr_info._path_phys += hdr_info._path_info;

  //将头部信息存储进键值对
  std::vector<std::string> v_hdr_pair;
  for(size_t i = 1; i < v_info.size(); ++i)
  {
    HdrCutByStr(v_info[i], ": ", v_hdr_pair);
    hdr_info._hdr_list[v_hdr_pair[0]] = v_hdr_pair[1];
    v_hdr_pair.resize(0);
  }
  return true;
}
void MakeErrPage(std::string& err_page, const std::string& err_code) {
  err_page += "";
}

bool SendErrorPage(int sock, const std::string err_code) {
  std::string err_page;
  std::string err_hdr;
  err_hdr += "HTTP/1.1 ";
  err_hdr += err_code;
  err_hdr += ErrorCode[err_code];

  MakeErrPage(err_page, err_code);
}

bool FileRequest(int sock, const RequestInfo& hdr_info)
{
  //判断是文件请求还是列表请求
  std::string path = hdr_info._path_phys;
  struct stat file_st;
  if(stat(hdr_info._path_phys.c_str(), &file_st) == -1) {
    //说明文件不存在，返回404页面
    
    SendErrorPage(sock, "404");
    return false;
  }

  //判断是文件还是目录
  if(file_st.st_mode & S_IFDIR) {
    //说明是目录
    //ListProcess();
  }

  //FileProcess();
  //TODO
  return true;
}

bool handler(int sock)
{
  std::string hdr;  //保存接受到的头部信息
  RequestInfo hdr_info; //保存解析出来的http头信息
  //接受http请求头
  if(FetchHttpHeader(sock, hdr) == false) {
    goto end;
  }
  //解析头部信息
  if(ParseHttpHeader(hdr, hdr_info) == false) {
    goto end;
  }

  //判断请求是否为CGI请求
  if((hdr_info._method == "GET" && hdr_info._query_string.empty()) ||
      hdr_info._method == "POST") {
    //处理CGI请求
  }

  else {
    //处理非CGI请求
    FileRequest(sock, hdr_info);  
  }

  close(sock);
  return true;
end:
  close(sock);
  return false;
}


#endif
