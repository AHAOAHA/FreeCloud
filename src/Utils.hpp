/*************************************************************************
*File Name: Utils.hpp
*Author: AHAOAHA
*mail: ahaoaha_@outlook.com
*Created Time: 2019年02月19日 星期二 14时43分51秒
 ************************************************************************/
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <sys/sendfile.h>
#include <dirent.h>
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
#include <fcntl.h>


#define MAX_HEADERSIZE 4096

using std::cout;
using std::endl;
using std::cerr;

std::unordered_map<std::string, std::string> ErrorCode({ \
    {"200", "OK"}, \
    {"400", "Bad Request"}, \
    {"404", "Not Found"}, \
    {"414", "Request-URL Too Long"}, \
    {"500", "Internal Server Error"} \
    });
std::unordered_map<std::string, std::string> FileType({ \
    {".txt", "text/plain"}, \
    {".doc", "application/msword"}, \
    {".html", "text/html"}, \
    {".htx", "text/html"}, \
    {".ico", "image/x-icon"}, \
    {".img", "application/x-img"}, \
    {".java", "java/*"} \
    });

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
  public:
    int _cli_sock;  //记录客户端sock
    handler_t _task_handler;  //处理方法
  public:
    HttpTask(int sock, handler_t handler): _cli_sock(sock), _task_handler(handler)  //创建线程池中的任务 
    {}

    ~HttpTask()
    {
      cout << "task over" << endl;
      close(_cli_sock);
    }

    void Run() {
      cout << "task run success" << endl;
      _task_handler(_cli_sock);
    }

};

class Utils {
  public:
    static void HdrCutByStr(const std::string hdr, const std::string cut_str, std::vector<std::string> &v_info) {
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

    static void TimeToGMT(time_t& t, std::string& date_gmt) {
      char tmp[128] = {0};
      struct tm* date_tm;
      date_tm = gmtime(&t);
      strftime(tmp, 127, "%a, %d %b %Y %H:%M:%S GMT", date_tm);
      date_gmt = tmp;
    }
};

class HttpRequest {
  private:
    int _cli_sock;
    RequestInfo _req_info;
    std::string _hdr;
  public:
    HttpRequest(): _cli_sock(-1) 
    {}

    void HttpRequestInit(int sock)
    {
      _cli_sock = sock;
    }

    bool FetchHttpHeader() {
      cout << "fetch begin" << endl;
      while(1) {
        char buf[MAX_HEADERSIZE] = {0};
        int ret = recv(_cli_sock, buf, MAX_HEADERSIZE, MSG_PEEK); //MSG_PEEK指从接受缓冲区中查看数据，但不会拿走数据
        if(ret < 0) { // <0 代表读取出错
          if(errno == EINTR || errno == EAGAIN) {
            continue;
          }
          return false;
        }

        char* ptr = strstr(buf, "\r\n\r\n");
        if(ptr == NULL) {
          if(ret >= MAX_HEADERSIZE) {
            cerr << "header too long" <<endl;
            return false;
          }
          else if(ret == 0) {
            return false;
          }
        }
        else {
          int hdr_lenth = ptr - buf;
          _hdr.assign(buf, hdr_lenth);
          recv(_cli_sock, buf, hdr_lenth + 4, 0);
          break;
        }
      }
      cout << "fetch req success" << endl;
      return true;
    }

    bool ParseHttpHeader() {
      std::vector<std::string> v_info;  //将头部的信息存储进vector中
      Utils::HdrCutByStr(_hdr, "\r\n", v_info); //将头部信息切割并存储进vector

      std::vector<std::string> v_first_line;
      Utils::HdrCutByStr(v_info[0], " ", v_first_line);  //将首行的信息保存在v_first_line
      _req_info._method = v_first_line[0];
      _req_info._version = v_first_line[2];
      std::vector<std::string> v_path_query;
      Utils::HdrCutByStr(v_first_line[1], "?", v_path_query);
      if(v_path_query.size() == 2)
      {
        _req_info._query_string = v_path_query[1];
      }
      _req_info._path_info = v_path_query[0];
      _req_info._path_phys = "./www";
      _req_info._path_phys += _req_info._path_info;

      //将头部信息存储进键值对
      std::vector<std::string> v_hdr_pair;
      for(size_t i = 1; i < v_info.size(); ++i)
      {
        Utils::HdrCutByStr(v_info[i], ": ", v_hdr_pair);
        _req_info._hdr_list[v_hdr_pair[0]] = v_hdr_pair[1];
        v_hdr_pair.resize(0);
      }
      cout << "prase req success" << endl;
      return true;
    }

    void ShowHeader()
    {
      cout << "header["<<_hdr <<"]"<< endl << endl << endl;
    }

    void GetRequestInfo(RequestInfo& req_info) {
      req_info._hdr_list = _req_info._hdr_list;
      req_info._method = _req_info._method;
      req_info._path_info = _req_info._path_info;
      req_info._path_phys = _req_info._path_phys;
      req_info._query_string = _req_info._query_string;
      req_info._st = _req_info._st;
      req_info._version = _req_info._version;
    }

};

class HttpResponse {
  private:
    int _cli_sock;
    std::string _stag;
    std::string _mtime; //最后一次修改时间
    std::string _cont_len;

  private:
    bool ProcessCGI(const RequestInfo& req_info);
    bool CGIHandler(const RequestInfo& req_info);
    bool ProcessFile(const std::string& file_path) {
      std::string file_hdr;

      OrganizeFileHdr(file_path, file_hdr);

      SendData(file_hdr);
      SendFile(file_path);

      return true;
    }
    bool ProcessList(const std::string& file_path) {
      std::string list_hdr;
      std::string list_body;

      OrganizeListHdr(list_hdr, file_path);
      OrganizeListBody(list_body, file_path);
      
      SendData(list_hdr);
      SendData(list_body);
      return true;
    }
    bool FileHandler(std::string file_path, struct stat & file_st) {
      //判断目标资源是文件还是目录
      if(file_st.st_mode & S_IFDIR) {
        //是一个目录
        if(file_path.back() != '/') {
          file_path += "/";
        }
        if(ProcessList(file_path) == false) {
          return false;
        }
        cout << "ListHandler success" << endl;
        return true;
      }

      if(ProcessFile(file_path) == false) {
        return false;
      }
      cout << "FileHandler success" << endl;
      return true;
    }
    bool ErrHandler(const std::string &err_code) {
      //组织err头部信息
      std::string err_hdr;
      std::string err_body;

      OrganizeErrHdr(err_code, err_hdr);
      OrganizeErrBody(err_code, err_body);

      SendData(err_hdr);
      SendData(err_body);
      return true;
    }

  private:
    bool SendFile(const std::string& file_path) {
      int file_fd = open(file_path.c_str(), O_RDONLY);
      if(file_fd < 0) {
        //TODO
      }
      struct stat st;
      stat(file_path.c_str(), &st);
      if(sendfile(_cli_sock, file_fd, NULL, st.st_size) < 0) {
        return false;
      }
      return true;
    }
    bool OrganizeFileHdr(const std::string& file_name, std::string& file_hdr) {
      file_hdr += "HTTP/1.1 200 OK\r\n";
      file_hdr += "Date: ";
      std::string date_gmt;
      time_t t = time(NULL);
      Utils::TimeToGMT(t, date_gmt);
      file_hdr += date_gmt;
      file_hdr += "\r\n";
      file_hdr += "Content-Type: ";
      std::string file_type;
      OrganizeFileType(file_name, file_type);
      file_hdr += file_type;
      file_hdr += "\r\n\r\n";
    }
    void OrganizeFileType(const std::string& file_name, std::string& file_type) {
      size_t pos = 0;
      pos = file_name.rfind(".");
      if(pos == std::string::npos) {
        file_type = "application/octet-stream";
        return;
      }

      std::string suffix;
      suffix.assign(file_name, pos, file_name.size() - 1);
      auto it = FileType.begin();
      for(it = FileType.begin(); it != FileType.end(); ++it)
      {
        if(it->first == suffix)
        {
          break;
        }
      }

      if(it != FileType.end()) {
        file_type = it->second;
      }
      else {
        file_type = "application/octet-stream";
      }
    }
    bool OrganizeListHdr(std::string& list_hdr, const std::string& file_path) {
      list_hdr = "HTTP/1.1 200 OK\r\n";
      list_hdr += "Date: ";
      std::string date_gmt;
      time_t t = time(NULL);
      Utils::TimeToGMT(t, date_gmt);
      list_hdr += date_gmt;
      list_hdr += "\r\n";
      list_hdr += "Content-Type: text/html\r\n\r\n";
      return true;
    }
    bool OrganizeListBody(std::string& list_body, const std::string& file_path) {
      std::string path = file_path;
      struct dirent **p_dirent = NULL;
      int num = scandir(path.c_str(), &p_dirent, 0, alphasort);

      list_body += "<html><body><h1>Index:/";
      list_body += "</h1>";
      list_body += "<hr /><ol>";

      for(int i =0; i < num; ++i) {
        list_body += "<li>";
        //当前文件文件文件全路径
        std::string file = path + p_dirent[i]->d_name;
        struct stat st;
        if(stat(file.c_str(), &st) < 0) {
          continue;
        }

        //std::string mtime;
        list_body += "<strong><a href='";
        list_body += p_dirent[i]->d_name;
        list_body += "'>";
        list_body += p_dirent[i]->d_name;
        list_body += "</a></strong>";
        std::string mtime;
        Utils::TimeToGMT(st.st_ctime, mtime);
        list_body += "<br><small>";
        list_body += mtime;
        list_body += "</small>";
        list_body += "</li><br>";
        
      }
      list_body += "</ol></body></html>";
      return true;
    }
    void OrganizeErrHdr(const std::string &err_code, std::string& err_hdr) {
      //首行
      err_hdr += "HTTP/1.1";
      err_hdr += " ";
      err_hdr += err_code;
      err_hdr += " ";
      err_hdr += ErrorCode[err_code];
      err_hdr += "\r\n";

      //Content-Type
      err_hdr += "Content-Type: text/html\r\n";
      //Date标签信息
      std::string date_gmt;
      time_t t = time(NULL);
      Utils::TimeToGMT(t, date_gmt);
      err_hdr += "Date: ";
      err_hdr += date_gmt;
      err_hdr += "\r\n\r\n";

      
    }

    void OrganizeErrBody(const std::string &err_code, std::string& err_body) {
      err_body += "<html><body><h1>";
      err_body += err_code;
      err_body += "</h1><h2>";
      err_body += ErrorCode[err_code];
      err_body += "</h2></body></html>";
    }

    bool SendData(std::string data) {
      if(send(_cli_sock, data.c_str(), data.size(), 0) == -1) {
        return false;
      }
      return true;
    }

  public:
    HttpResponse(): _cli_sock(-1)
    {}

    bool HttpResponseInit(int sock) {
      _cli_sock = sock;
      return true;
    }

    bool ResponseHandler(const RequestInfo& req_info) {
      //判断是否为CGI请求
      if((req_info._method == "GET" && !req_info._query_string.empty()) || \
          req_info._method == "POST") {
        //处理CGI请求
        //CGIHandler(req_info);
        return true;
      }

      //判断请求资源是否存在
      std::string file_path = req_info._path_phys;
      struct stat file_st;
      if(stat(file_path.c_str(), &file_st) < 0) {
        //请求资源不存在，返回404页面
        ErrHandler("404");
        cout << "errhandler success" << endl;
        return true;
      }

      FileHandler(req_info._path_phys, file_st);
      return true;
    }
};
/*

void MakeErrPage(std::string& err_page, const std::string& err_code) {
  err_page += "";
}

void MakeErrHdr(std::string& err_hdr, const std::string& err_code) {
  err_hdr += "HTTP/1.1 "; //组织首行
  err_hdr += err_hdr;
  err_hdr += " ";
  err_hdr += ErrorCode[err_code];
  err_hdr += "\r\n";

  std::string date_gmt; //组织Date标签信息
  Utils::TimeToGMT(date_gmt);
  err_hdr += date_gmt;
  err_hdr += "\r\n";
}

bool SendErrorPage(int sock, const std::string err_code) {
  std::string err_page;
  std::string err_hdr;
  MakeErrHdr(err_hdr, err_code);
  

  MakeErrPage(err_page, err_code);
}

bool FileRequest(int sock, const RequestInfo& hdr_info) {
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

*/
#endif
