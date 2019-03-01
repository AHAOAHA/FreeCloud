/*************************************************************************
*File Name: HttpServer.hpp
*Author: AHAOAHA
*mail: ahaoaha_@outlook.com
*Created Time: 2019年02月19日 星期二 14时42分36秒
 ************************************************************************/
#ifndef __HTTP_SERVER_HPP__
#define __HTTP_SERVER_HPP__

#include "Utils.hpp"
#include "ThreadPool.hpp"

#define MAX_CLIENTNUM 5

class HttpServer {
  private:
    int _serv_sock; //服务器端套接字
    ThreadPool _thr;  //线程池
  private:
    bool HttpServerInit(uint16_t port) {
      _thr.ThreadInit();
      _serv_sock = socket(AF_INET, SOCK_STREAM, 0);
      if(_serv_sock < 0) {
        cerr << "socket error" << endl;
        return false;
      }
      //设置服务器立即重启
     int opt = 1;
     setsockopt(_serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int));

      sockaddr_in serv_addr;
      bzero(&serv_addr, sizeof(sockaddr_in));
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(port);
      serv_addr.sin_addr.s_addr = INADDR_ANY;

      if(bind(_serv_sock, (sockaddr*)&serv_addr, sizeof(sockaddr_in)) < 0) {
        cerr << "bind error" << endl;
        return false;
      }

      if(listen(_serv_sock, MAX_CLIENTNUM) < 0) {
        cerr << "listen error" << endl;
        return false;
      }
     return true; 
    }
  public:
    HttpServer():_serv_sock(-1)
    {}

    void Start(uint16_t port, handler_t handler) {
      if(HttpServerInit(port) == false) {
        return;
      }

      while(1) {
        sockaddr_in cli_addr;
        socklen_t len = sizeof(cli_addr);
        int cli_sock = accept(_serv_sock, (sockaddr*)&cli_addr, &len);
        if(cli_sock < 0) {
          cerr << "accept error" << endl;
          continue;
        }
        //创建HttpTask
        HttpTask* tt = new HttpTask(cli_sock, handler);
        _thr.PushTask(tt);
      }
    }
};

bool handler(int sock) {
  std::string hdr;  //保存接受到的头部信息
  HttpRequest req;
  HttpResponse rsp;
  RequestInfo req_info;
  req.HttpRequestInit(sock);
  //接受http请求头
  if(req.FetchHttpHeader() == false) {
    goto end;
  }
  //req.ShowHeader();
  //解析头部信息
  if(req.ParseHttpHeader() == false) {
    goto end;
  }

  req.GetRequestInfo(req_info);

  rsp.HttpResponseInit(sock, req_info);
  if(rsp.ResponseHandler() == false) {
    goto end;
  }


  return true;

end:
  return false;
}
#endif
