/*************************************************************************
*File Name: Utils.hpp
*Author: AHAOAHA
*mail: ahaoaha_@outlook.com
*Created Time: 2019年02月19日 星期二 14时43分51秒
 ************************************************************************/
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <sys/wait.h>
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
#define MAX_BUFF 4096

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
      close(_cli_sock);
    }

    void Run() {
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

    static int64_t StrToNum(std::string str) {
      return strtol(str.c_str(), NULL, 10);
    }

    static void TimeToGMT(time_t& t, std::string& date_gmt) {
      char tmp[128] = {0};
      struct tm* date_tm;
      date_tm = gmtime(&t);
      strftime(tmp, 127, "%a, %d %b %Y %H:%M:%S GMT", date_tm);
      date_gmt = tmp;
    }

    static void NumToStr(size_t num, std::string& tm_str) {
      char buf[128] = {0};
      snprintf(buf, 127, "%d", num);
      tm_str = buf;
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
    std::string _etag; 
    std::string _fsize;

  private:
    bool ProcessCGI(const RequestInfo& req_info) {
      //使用外部程序完成CGI请求
      //将HTTP头信息和正文全部交给子进程处理
      //使用环境变量传递头信息
      //使用管道传递正文
      //使用管道接受返回信息
      int in[2];
      int out[2];
      if(pipe(in) || pipe(out)) {
        //创建管道失败
        ErrHandler("500");
        return false;
      }

      pid_t pid = fork();
      if(pid < 0) {
        //创建子进程失败
        ErrHandler("500");
        return false;
      }
      if(pid == 0) {
        //子进程
        //使用环境变量传递头信息
        setenv("METHOD", req_info._method.c_str(), 1);  //请求方法
        setenv("VERSION", req_info._version.c_str(), 1);  //协议版本
        setenv("PATH_INFO", req_info._path_info.c_str(), 1);  //路径信息
        setenv("QUERY_STRING", req_info._query_string.c_str(), 1); //请求字符串
        for(auto it = req_info._hdr_list.begin(); it != req_info._hdr_list.end(); ++it) {
          setenv(it->first.c_str(), it->second.c_str(), 1);
        }
        close(in[1]); //关闭in的读端
        close(out[0]);  //关闭out的写端
        dup2(in[0], 0); //将in[0]重定向至子进程的标准输入
        dup2(out[1], 1);  //重定向out[1]至子进程的标准输出
        execl(req_info._path_phys.c_str(), req_info._path_phys.c_str(), NULL);
        exit(0);
      }

      //父进程
      close(in[0]);
      close(out[1]);
      //通过管道将正文传递给子进程
      //Content-length记录正文的长度
      auto it = req_info._hdr_list.find("Content-Length");

      if(it != req_info._hdr_list.end()) {
        char buf[MAX_BUFF] = {0};
        //正文长度
        int64_t content_len = Utils::StrToNum(it->second);
        int tlen = 0; //content_length - tlen 表示剩余的正文长度
        while(tlen < content_len) {
          int len = MAX_BUFF > (content_len - tlen) ? (content_len - tlen) : MAX_BUFF;

          int rlen = recv(_cli_sock, buf, len, 0);
          if(rlen <= 0) {
            //返回错误信息,content-length与实际正文长度不匹配
            //TODO
            return false;
          }
          if(write(in[1], buf, rlen) < 0) {
            return false;
          }
          tlen += len;  //tlen记录已经接受的正文长度
        }
      }
      else{
        //请求中不包含content-length字段
        //TODO
      }

      // 通过out管道返回子进程的处理结果直到返回0
      // 组织响应Http数据，并返回给客户端
      std::string rsp_hdr;
      // 组织头部信息
      rsp_hdr = "HTTP/1.1 200 OK\r\n";
      rsp_hdr += "Content-Type: text/html\r\n";
      std::string date_gmt;
      time_t t = time(NULL);
      Utils::TimeToGMT(t, date_gmt);
      rsp_hdr += "Date: ";
      rsp_hdr += date_gmt;
      rsp_hdr += "\r\n\r\n";
      SendData(rsp_hdr);  //传递头信息

      while(1) {  //为了检测管道写端关闭
        char buf[MAX_BUFF] = {0};
        int rlen = read(out[0], buf, MAX_BUFF);
        if(rlen == 0) {
          break;
        }

        send(_cli_sock, buf, MAX_BUFF, 0);
      }

      std::string rsp_body;
      rsp_body = "<html><body><h1>success!</h1></body></html>";
      SendData(rsp_body);
      return true;
    }
    bool CGIHandler(const RequestInfo& req_info) {
      return ProcessCGI(req_info);
    }
    bool ProcessFile(const std::string& file_path) {
      std::string file_hdr;

      OrganizeFileHdr(file_path, file_hdr);

      SendData(file_hdr);
      SendFile(file_path);

      return true;
    }
    bool ProcessList(const std::string& file_path_info, const std::string& file_path_phys) {
      std::string list_hdr;
      std::string list_body;
      OrganizeListHdr(list_hdr, file_path_phys);
      OrganizeListBody(list_body, file_path_info, file_path_phys);
       
      SendData(list_hdr);
      SendData(list_body);
      return true;
    }
    bool FileHandler(std::string file_path_info, std::string file_path_phys, struct stat & file_st) {
      //判断目标资源是文件还是目录
      if(file_st.st_mode & S_IFDIR) {
        //是一个目录
        if(file_path_info.back() != '/') {
          file_path_info += "/";
        }
        if(file_path_phys.back() != '/') {
          file_path_phys += "/";
        }
        if(ProcessList(file_path_info, file_path_phys) == false) {
          return false;
        }
        return true;
      }

      if(ProcessFile(file_path_phys) == false) {
        return false;
      }
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
      file_hdr += "Content-Type: application/octet-stream\r\n";
      file_hdr += "Connection: close\r\n";

      //std::string file_type;
      //OrganizeFileType(file_name, file_type);
      //file_hdr += file_type;
      file_hdr += "\r\n";
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
    bool OrganizeListBody(std::string& list_body,const std::string& file_path_info, const std::string& file_path_phys) {
      std::string path_phys = file_path_phys;
      std::string path_info = file_path_info;
      struct dirent **p_dirent = NULL;
      int num = scandir(path_phys.c_str(), &p_dirent, 0, alphasort);

      list_body += "<html><body><h1>Index:/";
      list_body += "</h1>";
      list_body += "<form action='/upload' method='post' ";
      list_body += "enctype='multipart/form-data'>";
      list_body += "<input type='file' name='FileUpload' />";
      list_body += "<input type='submit' value='upload' /></form>";
      list_body += "<hr /><ol>";

      for(int i =0; i < num; ++i) {
        list_body += "<li>";
        //当前文件文件文件全路径
        std::string file = path_phys + p_dirent[i]->d_name;
        struct stat st;
        if(stat(file.c_str(), &st) < 0) {
          continue;
        }

        //std::string mtime;
        list_body += "<strong><a href='";
        list_body += file_path_info.c_str();
        list_body += p_dirent[i]->d_name;

        list_body += "'>";
        list_body += p_dirent[i]->d_name;
        list_body += "</a></strong>";
        std::string mtime;
        Utils::TimeToGMT(st.st_ctime, mtime);

        std::string sz_str;
        Utils::NumToStr(st.st_size, sz_str);
        list_body += "<br><small>";
        list_body += mtime;
        list_body += " size: ";
        list_body += sz_str.c_str();
        list_body += " bytes";
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
        CGIHandler(req_info);
        return true;
      }

      //判断请求资源是否存在
      std::string file_path = req_info._path_phys;
      
      struct stat file_st;
      if(stat(file_path.c_str(), &file_st) < 0) {
        //请求资源不存在，返回404页面
        ErrHandler("404");
        return true;
      }

      FileHandler(req_info._path_info, req_info._path_phys, file_st);
      return true;
    }
};

#endif
