package service

import (
	"config"
	"freecloud"
	"log"
	"net/http"
)

var err error

const (

)

func Run() {
	// 创建serv
	mux := http.NewServeMux()
	// 注册根路由 默认路由
	mux.Handle("/", &freecloud.FLServer{})

	err = http.ListenAndServe(config.MyConfig.Service.Addr, mux)
	if err != nil {
		log.Panic(err)
	}
}



