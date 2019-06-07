package freecloud

import (
	"config"
	"fmt"
	"io/ioutil"
	"log"
	"os"
)
var err error


func (ctx *FLServer) ListHandler() {
	log.Printf("DirPath: %s", ctx.GetFilePath())

	dirStat, _ := os.Stat(ctx.GetFilePath())
	// 完善Header
	ctx.SetHeader("Content-Type", HTML_TYPE)
	ctx.SetHeader("Last-Modified", dirStat.ModTime().String())

	// 发送响应首行
	ctx.RspWriter.WriteHeader(200)

	// 写入body
	var DirStat []os.FileInfo
	DirStat, err = ioutil.ReadDir(ctx.GetFilePath())

	if err != nil {
		log.Fatal("[ListHandler +50]: unknown Fail")
	}

	// 写入标题部分 不判断错误
	_, err = ctx.RspWriter.Write([]byte(config.MyConfig.Html.Title))

	_, err = ctx.RspWriter.Write([]byte("<ol>"))

	log.Println("DirStat:", DirStat)

	// 写入列表部分
	refer := ctx.Request.Referer()
	if len(refer) > 0 {
		if refer[len(refer) - 1] == '/' {
			refer = refer[:len(refer)-1]
		}
	}
	log.Println("Refer:", refer)

	for _, v := range DirStat {
		content := fmt.Sprintf("<li><strong><a href='%s'>%s</a></strong></li>",ctx.GetFilePath() + v.Name(), v.Name())
		log.Println(v.Name(), "href:", ctx.GetFilePath() + v.Name())
		_, err = ctx.RspWriter.Write([]byte(content))
	}

	_, err = ctx.RspWriter.Write([]byte("</ol></html>"))
}