package freecloud

import (
	"context"
	"log"
	"net/http"
)

type FLServer struct {
	context.Context



	Request *http.Request
	RspWriter http.ResponseWriter
}

func (fc *FLServer) ServeHTTP(rspWriter http.ResponseWriter,Req *http.Request) {
	log.Printf("Get a request, req: %s", Req.URL.String())
	tmpctx, cancel := context.WithCancel(context.Background())
	ctx := &FLServer{
		Context: tmpctx,
		Request: Req,
		RspWriter: rspWriter,
	}
	rspWriter.Header().Set("Content-Length", "10000")
	defer cancel()
	Method := Req.Method
	log.Printf("Method: %s", Method)
	switch Method {
	case "GET":
		ListAndDownLoad(ctx)
	default:
	}
}

// 设置一个header
func (fc *FLServer) SetHeader(k , v string) {
	fc.RspWriter.Header().Set(k, v)
}

