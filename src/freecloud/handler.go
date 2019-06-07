package freecloud

import (
	"config"

	"os"
)


type CONTENT_TYPE string


var PATH = config.MyConfig.Service.RootPath

func (ctx *FLServer) ListAndDownLoad() {
	RelFilePath := ctx.Request.URL.Path
	FilePath := PATH + RelFilePath
	FileStat, err := os.Stat(FilePath)

	if err != nil {
		// 文件不存在或存在一些未知问题
		ctx.Handler404()
		return
	}

	if FileStat.IsDir() {
		// 建立列表请求
		ctx.ListHandler()
		return
	}

	// 下载请求
	ctx.DownloadHandler()
}