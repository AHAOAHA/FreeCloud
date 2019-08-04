/*************************************************************************
*File Name: main.cpp
*Author: AHAOAHA
*mail: ahaoaha_@outlook.com
*Created Time: 2019年02月19日 星期二 16时48分45秒
 ************************************************************************/
#include "HttpServer.hpp"


int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    cout << "./server [port]" << endl;
    return -1;
  }
  HttpServer serv;
  serv.Start(atoi(argv[1]), handler);
  return 0;
}
