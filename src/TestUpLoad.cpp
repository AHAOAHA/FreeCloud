/*************************************************************************
*File Name: TestUpLOad.cpp
*Author: AHAOAHA
*mail: ahaoaha_@outlook.com
*Created Time: Sat 23 Feb 2019 03:09:10 PM CST
 ************************************************************************/
#include "Utils.hpp"
#include <string.h>

enum _boundry_type {
  BOUNDRY_NO = 0, 
  BOUNDRY_FIRST,
  BOUNDRY_MIDDLE,
  BOUNDRY_LAST,
  BOUNDRY_BAK,
};

class UpLoad {
  private:
    int _file_fd; //文件描述符
    int64_t content_len; //表示正文的长度
    std::string _first_boundry;
    std::string _middle_boundry;
    std::string _last_boundry;
    std::string _file_name;

  private:
    int MatchBoundry(char* buf, int blen, int& boundry_pos) {
      if(memcmp(buf, _first_boundry.c_str(), _first_boundry.length())) {
        return BOUNDRY_FIRST;
      }

      boundry_pos = 0;

      for(int i = 0; i < blen; ++i) {
        //如果剩余的字符长度大于boundry的长度
        //就进行全匹配
        if((blen - i) > _middle_boundry.length()) {
          if(!memcmp(buf+i, _middle_boundry.c_str(), _middle_boundry.length())) {
            return BOUNDRY_MIDDLE;
          }
          else if(!memcmp(buf+i, _last_boundry.c_str(), _last_boundry.length())) {
            return BOUNDRY_LAST;
          }
        }

        else {
          int cmp_len = (blen - i) > _middle_boundry.length() ? _middle_boundry.length() : (blen - i);
          if(!memcmp(buf + i, _last_boundry.c_str(), cmp_len)) {
            return BOUNDRY_BAK;
          }
        }
      }
      return BOUNDRY_NO;
    }
    
      bool GetFileName(char* buf, int& content_pos) {
        char* ptr = NULL;
        ptr = strstr(buf, "\r\n\r\n");
        if(ptr == NULL) {
          cerr << "getfilename fail" << endl;
          return false;
        }

        content_pos = (ptr - buf) + 4;
        std::string header;
        header.assign(buf, ptr - buf);

        std::string file_sep = "filename=\"";
        size_t pos = header.find(file_sep);
        if(pos == std::string::npos) {
          return false;
        }

        _file_name = header.substr(pos + file_sep.length());
        pos = _file_name.find("\"");
        if(pos = std::string::npos) {
          return false;
        }
        
        _file_name.erase(pos);

        cerr << _file_name << endl;
        return true;
      }

      bool CreateFile() {
        umask(0);
        _file_fd = open(_file_name.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0664);
        if(_file_fd < 0) {
          cerr << "open fail" << endl;
          return false;
        }
        return true;
      }

      bool CloseFile() {
        if(_file_fd != -1) {
          close(_file_fd);
          _file_fd = -1;
        }
        return true;
      }

      bool WriteFile(char* buf, int len) {
        if(_file_fd != -1) {
          write(_file_fd, buf, len);
          return true;
        }
        return false;
      }
  public:
    UpLoad(): _file_fd(-1) 
    {}

    bool InitUpLoadInfo() {
      char* ptr = getenv("Content-Length");
      if(ptr == NULL) {
        cerr << "content-length zero" << endl;
        return false;
      }
      content_len = Utils::StrToNum(ptr);

      std::string boundry_sep = "boundry=";
      std::string content_type = ptr;

      size_t pos = content_type.find(boundry_sep);
      if(pos == std::string::npos) {
        cerr << "have no boundry" << endl;
        return false;
      }
      std::string boundry = content_type.substr(pos + boundry_sep.length());

      _first_boundry = "--" + boundry;
      _middle_boundry = "\r\n" + _first_boundry + "\r\n";
      _last_boundry = "\r\n" + _first_boundry + "--";
      return true;
    }

    bool ProcessUpload() {
      int64_t tlen = 0;
      int64_t blen = 0;

      char buf[MAX_BUFF] = {0};

      while(tlen < content_len) {
        int len = read(0, buf+blen, MAX_BUFF - blen);
        blen += len;
        int boundry_pos = 0;
        int content_pos = 0;

        int flag = MatchBoundry(buf, blen, boundry_pos);
        if(flag == BOUNDRY_FIRST) {
          //1. 从boundry中获取文件名
          //2. 如果获取信息成功，则创建文件并打开文件
          //3. 将头部信息从buf中移除，其余进行下一步匹配
          if(GetFileName(buf, content_pos)) {
            CreateFile();
            blen -= content_len;
            memmove(buf, buf+content_pos, blen);
          }
          else {
            blen -= _first_boundry.length();
            memmove(buf, buf+boundry_pos, len);
          }
        }

        while(1) {
          flag = MatchBoundry(buf, blen, boundry_pos);
          if(flag != BOUNDRY_MIDDLE) {
            break;
          }
          //匹配middle_boundry成功
          WriteFile(buf, boundry_pos);
          CloseFile();
          blen -= boundry_pos;
          memmove(buf, buf+boundry_pos, blen);
          if(GetFileName(buf, content_pos)) {
            CreateFile();
            blen -= content_pos;
            memmove(buf, buf + content_pos, blen);
          }
          else {
            if(content_pos == 0) {
              break;
            }
            blen -= +_middle_boundry.length();
            memmove(buf, buf+content_pos, blen);
          }
        }

        flag = MatchBoundry(buf, blen, boundry_pos);
        if(flag == BOUNDRY_LAST) {
          WriteFile(buf, boundry_pos);
          blen -= boundry_pos;
          memmove(buf, buf + boundry_pos, blen);
          return true;
        }

        flag = MatchBoundry(buf, blen, boundry_pos);
        if(flag == BOUNDRY_NO) {
          WriteFile(buf, blen);
          blen = 0;
        }

        blen += len;
      }

      return false;
    }
};

int main()
{
  cerr << "upload run" << endl;
  UpLoad upload;
  if(upload.InitUpLoadInfo() == false) {
    //组织页面
    return -1;
  }

  if(upload.ProcessUpload() == false) {
    //组织页面
    return -1;
  }

  return 0;
}
