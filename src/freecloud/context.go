package freecloud

import (
	"context"
	"log"
	"net/http"
)

type Context struct {
	context.Context
	Request *http.Request
	RspWriter http.ResponseWriter
}

func (fc Context) ServeHTTP(rspWriter http.ResponseWriter,Req *http.Request) {
	log.Printf("Get a request, req: %s", Req.URL.String())
	tmpctx, cancel := context.WithCancel(context.Background())
	ctx := &Context{
		Context: tmpctx,
		Request: Req,
		RspWriter: rspWriter,
	}
	defer cancel()
	Method := Req.Method
	log.Printf("Method: %s", Method)
	switch Method {
	case "GET":
		ListAndDownLoad(ctx)
	default:
	}
}

// 一次仅设置一个header
func (fc Context) SetHeader(k interface{}, v interface{}) {
	h := fc.RspWriter.Header()
	var vSlince []string
	switch v.(type) {
	case string: vSlince = append(vSlince, v.(string))
	case []string: vSlince = v.([]string)
	}
	h[k.(string)] = vSlince
}

