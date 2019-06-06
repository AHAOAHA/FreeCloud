package freecloud

import (
	"bufio"
	"io"

	//"bufio"
	"config"
	"fmt"
	//"io"
	"io/ioutil"
	"log"
	"os"
)

var err error

type CONTENT_TYPE string

const (
	BINARY_TYPE = "application/octet-stream"
	HTML_TYPE = "text/html"
)

const PATH = "../wwwroot"

func ListAndDownLoad(ctx *Context) {
	RelFilePath := ctx.Request.URL.Path
	FilePath := PATH + RelFilePath
	log.Printf("DirPath: %s", FilePath)
	FileStat, err := os.Stat(FilePath)

	if err != nil {
		// 文件不存在或存在一些未知问题
		// 404 TODO
		ctx.RspWriter.WriteHeader(404)
		return
	}

	if FileStat.IsDir() == true {
		// 建立列表请求
		ListHandler(ctx, FilePath)
		return
	}

	// 下载请求
	DownloadHandler(ctx, FilePath)
}

func unknowFail(ctx *Context) {
	_, err = ctx.RspWriter.Write([]byte("Unknown Fail"))
}

func ListHandler(ctx *Context, FilePath string) {
	ctx.RspWriter.WriteHeader(200)

	// 完善Header
	ctx.SetHeader("Content-Type", HTML_TYPE)

	// 写入body
	var DirStat []os.FileInfo
	DirStat, err = ioutil.ReadDir(FilePath)
	if err != nil {
		unknowFail(ctx)
		log.Fatal("[ListHandler +50]: unknown Fail")
	}

	// 写入标题部分 不判断错误
	_, err = ctx.RspWriter.Write([]byte(config.MyConfig.Html.Title))

	_, err = ctx.RspWriter.Write([]byte("<ul>"))
	// 写入列表部分
	refer := ctx.Request.Referer()
	log.Println("Refer:", refer)
	for _, v := range DirStat {
		content := fmt.Sprintf("<li><a href='%s'>%s</a></li>",refer+ctx.Request.URL.Path[1:]+"/"+v.Name(), v.Name())
		log.Println(v.Name(), "href:", refer+ctx.Request.URL.Path[1:]+"/"+v.Name())
		_, err = ctx.RspWriter.Write([]byte(content))
	}
	_, err = ctx.RspWriter.Write([]byte("</ul></html>"))
}

func DownloadHandler(ctx *Context, FilePath string) {
	log.Printf("FilePath: %s", FilePath)
	ctx.RspWriter.WriteHeader(200)

	// 完善Header
	ctx.SetHeader("Content-Type", BINARY_TYPE)

	// 写入body
	fileIO, _ := os.Open(FilePath)
	Reader := bufio.NewReader(fileIO)

	for {
		buffer := make([]byte, 4096)
		n, err := Reader.Read(buffer)
		if err == io.EOF {
			log.Println("File Send Over")
			break
		} else {
			_, _ = ctx.RspWriter.Write(buffer[:n])
		}
	}
}