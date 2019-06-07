package freecloud

import (
	"log"
	"net/http"
)

type FLServer struct {
	//context.Context

	Request *http.Request
	RspWriter http.ResponseWriter

	FilePath string // 请求文件的相对目录
}

func (fc *FLServer) ServeHTTP(rspWriter http.ResponseWriter,Req *http.Request) {
	fc.Request = Req
	fc.RspWriter = rspWriter
	fc.FilePath = Req.URL.Path

	log.Printf("Get a request, req: %s", Req.URL.String())
	Method := Req.Method
	log.Printf("Method: %s", Method)
	switch Method {
	case "GET":
		fc.ListAndDownLoad()
	case "POST":
		fc.UpLoadCgi()
	default:
	}
}

// 设置一个header
func (fc *FLServer) SetHeader(k , v string) {
	fc.RspWriter.Header().Set(k, v)
}

func (fc *FLServer) WriteRspFirstHeader(retcode int) {
	fc.RspWriter.WriteHeader(retcode)
}

func (fc *FLServer) GetFilePath() string {
	return fc.FilePath
}
