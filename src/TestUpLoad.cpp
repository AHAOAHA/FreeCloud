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
    int MatchBoundry(char* buf, const int& blen, int& boundry_pos) {
      if(!memcmp(buf, _first_boundry.c_str(), _first_boundry.length())) {
        boundry_pos = 0;
        return BOUNDRY_FIRST;
      }


      for(int i = 0; i < blen; ++i) {
        //如果剩余的字符长度大于boundry的长度
        //就进行全匹配
        if((blen - i) > _middle_boundry.length()) {
          if(!memcmp(buf+i, _middle_boundry.c_str(), _middle_boundry.length())) {
            boundry_pos = i;
            return BOUNDRY_MIDDLE;
          }
          else if(!memcmp(buf+i, _last_boundry.c_str(), _last_boundry.length())) {
            boundry_pos = i;
            return BOUNDRY_LAST;
          }
        }

        else {
          int cmp_len = (blen - i) > _middle_boundry.length() ? _middle_boundry.length() : (blen - i);
          if(!memcmp(buf + i, _last_boundry.c_str(), cmp_len)) {
            boundry_pos = i;
            return BOUNDRY_BAK;
          }
        }
      }

      boundry_pos = blen;

      return BOUNDRY_NO;
    }
    
      bool GetFileName(char* buf, int& content_pos) {
        char* ptr = NULL;
        ptr = strstr(buf, "\r\n\r\n");
        if(ptr == NULL) {
          cerr << "getfilename fail" << endl;
          return false;
        }

        cerr << "get filename" << endl;
        content_pos = (ptr - buf) + 4;
        std::string header;
        header.assign(buf, ptr - buf);

        std::string file_sep = "filename=\"";
        size_t pos = header.find(file_sep);
        if(pos == std::string::npos) {
          return false;
        }

        _file_name = header.substr(pos + file_sep.length());
        
        pos = _file_name.find('\"');
        if(pos == std::string::npos) {
          return false;
        }
        
        _file_name.erase(pos);
        char* ptr_path = getenv("Referer");
        std::string prev_url = ptr_path;

        pos = prev_url.find("://");
        if(pos == std::string::npos) {
          //Referer中链接不完整
          return false;
        }

        prev_url.erase(pos, 3);
        cerr << prev_url << endl;
        pos = prev_url.find('/');
        
        std::string prev_path = prev_url.substr(pos);
        prev_path = "./www" + prev_path;
        _file_name = prev_path + _file_name;

        cerr << "path_info: " << _file_name;

        return true;
      }

      bool CreateFile() {
        cerr << "create file" << endl;
        umask(0);
        _file_fd = open(_file_name.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0664);
        if(_file_fd < 0) {
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
        return false;
      }
      content_len = Utils::StrToNum(ptr);

      std::string boundry_sep = "boundary=";
      std::string content_type = getenv("Content-Type");

      size_t pos = content_type.find(boundry_sep);
      if(pos == std::string::npos) {
        return false;
      }
      std::string boundry = content_type.substr(pos + boundry_sep.length());

      _first_boundry = "--" + boundry;
      _middle_boundry = "\r\n" + _first_boundry + "\r\n";
      _last_boundry = "\r\n" + _first_boundry + "--";
      return true;
    }

    bool ProcessUpload() {
      int64_t tlen = 0; //记录当前已经写入的字节数
      int64_t blen = 0; //记录当前buff中的字节数

      int boundry_pos = 0;  //记录当前boundry的位置
      int content_pos = 0;  //记录当前正文的位置

      char buf[MAX_BUFF] = {0};

      while(tlen < content_len) {
        int len = read(0, buf + blen, MAX_BUFF - blen);
        blen += len;  //blen为上一次缓冲区中的内容加上本次所读取的内容
        buf[blen] = '\0'; //将缓冲区中可用的内容组织成C形式字符串

        int flag = MatchBoundry(buf, blen, boundry_pos);
        if(flag == BOUNDRY_FIRST) {
          cerr << "get first boundry" << endl;
          //说明当前boundry是文件的开始
          blen -= boundry_pos;
          memmove(buf, buf + boundry_pos, blen); //将读取到的boundry从缓冲区中拿走
          boundry_pos = blen;
          buf[blen] = '\0';
          if(GetFileName(buf, content_pos)) {
            blen -= content_pos;
            memmove(buf, buf + content_pos, blen);
            content_pos = blen;
            buf[blen] = '\0';
            CreateFile();
          }
          else {
            //该段落中没有filename
            //TODO
          }
        }

        while(1) {
          flag = MatchBoundry(buf, blen, boundry_pos);
          if(flag == BOUNDRY_MIDDLE) {
            //找到第一个文件的结尾
            WriteFile(buf, boundry_pos);
            blen -= boundry_pos;
            memmove(buf, buf + boundry_pos, blen);
            boundry_pos = blen;
            buf[blen] = '\0';
            CloseFile();
            if(GetFileName(buf, content_pos)) {
              cout << "file create" << endl;
              CreateFile();
              blen -= content_pos;
              memmove(buf, buf + content_pos, blen);
              content_pos = blen;
              buf[blen] = '\0';
            }
            else {
              //未找到对应文件名
              //TODO
            }
          }
          else {
            break;
          }
        }

        flag = MatchBoundry(buf, blen, boundry_pos);
        if(flag == BOUNDRY_LAST) {
          cerr << "------ get last boundry" << endl;
          WriteFile(buf, boundry_pos);
          CloseFile();
          return true;
        }

        flag = MatchBoundry(buf, blen, boundry_pos);
        if(flag == BOUNDRY_NO) {
          WriteFile(buf, blen);
          blen = 0;
        }

        flag = MatchBoundry(buf, blen, boundry_pos);
        if(flag == BOUNDRY_BAK) {
          //将该条boundry之前的内容写入文件
          //取出buf中，boundry之前的内容
          WriteFile(buf, boundry_pos);
          blen -= boundry_pos;
          memmove(buf, buf + boundry_pos, blen);
          boundry_pos = blen;
          buf[blen] = '\0';
        }
        tlen += len;
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

  cerr << "init success" << endl;

  if(upload.ProcessUpload() == false) {
    //组织页面
    return -1;
  }

  cerr << "process success" << endl;


  return 0;
}
