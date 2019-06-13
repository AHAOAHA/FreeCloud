package freecloud

import (
	"FreeCloud/src/config"
	"FreeCloud/src/md5"
	"FreeCloud/src/rangedir"
	"fmt"
	"log"
	"os"
)
var err error


func (ctx *FLServer) ListHandler() {
	log.Printf("DirPath: %s\n", ctx.GetFileRealPath())

	dirStat, _ := os.Stat(ctx.GetFileRealPath())
	// 完善Header
	ctx.SetHeader("Content-Type", HTML_TYPE)
	ctx.SetHeader("Last-Modified", dirStat.ModTime().String())

	// 发送响应首行
	ctx.RspWriter.WriteHeader(200)

	// 获取当前文件夹下的文件
	FilesSlice, _ := rangedir.RangeDir(ctx.GetFileRealPath())
	refer := ctx.Request.Referer()

	// 组织html页面
	html_begin := fmt.Sprintf("<html><body><h2>%s</h2><hr><ol>", config.MyConfig.Html.Title)
	html_end := "</ol></body></html>"
	var html_mid string
	for _, v := range FilesSlice {	// 遍历当前文件夹下的文件
		fileReqPath := refer + ctx.GetFileRequestPath()
		if fileReqPath[len(fileReqPath) - 1] != '/' {
			fileReqPath = fileReqPath + "/" + v
		} else {
			fileReqPath = fileReqPath + v
		}

		log.Println("fileReqPath:", fileReqPath)
		fileStat, _ := os.Stat(PATH + fileReqPath)

		if fileStat.IsDir() {
			// 目录
			html_mid += fmt.Sprintf("<li><a href=%s>%s</a></li>", fileReqPath, fileStat.Name())
		} else {
			// 文件 计算md5值
			fileMD5, _ := md5.GetMD5(PATH + fileReqPath)
			html_mid += fmt.Sprintf("<li><a href=%s>%s</a><br>%s</li>", fileReqPath, fileStat.Name(), fileMD5)
		}
	}

	html := html_begin + html_mid +html_end
	// 写入正文部分 不判断错误
	_, err = ctx.RspWriter.Write([]byte(html))
}
