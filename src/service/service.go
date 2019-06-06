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
	err = http.ListenAndServe(config.MyConfig.Service.Addr, freecloud.Context{})
	if err != nil {
		log.Panic(err)
	}
}



