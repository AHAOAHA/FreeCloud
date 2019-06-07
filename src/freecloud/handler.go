package freecloud

import (
	//"bufio"

	"config"

	"os"
)

var err error

type CONTENT_TYPE string

const (
	BINARY_TYPE = "application/octet-stream"
	HTML_TYPE = "text/html"
)

var PATH = config.MyConfig.Service.RootPath

func ListAndDownLoad(ctx *Context) {
	RelFilePath := ctx.Request.URL.Path
	FilePath := PATH + RelFilePath
	FileStat, err := os.Stat(FilePath)

	if err != nil {
		// 文件不存在或存在一些未知问题
		Handler404(ctx)
		return
	}

	if FileStat.IsDir() {
		// 建立列表请求
		ListHandler(ctx, FilePath)
		return
	}

	// 下载请求
	DownloadHandler(ctx, FilePath)
}
