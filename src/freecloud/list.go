package freecloud

import (
	"config"
	"io/ioutil"
	"log"
	"os"
	"fmt"
)

func ListHandler(ctx *Context, FilePath string) {
	log.Printf("DirPath: %s", FilePath)
	UrlPath := FilePath[len(PATH):]

	ctx.RspWriter.WriteHeader(200)

	// 完善Header
	ctx.SetHeader("Content-Type", HTML_TYPE)
	ctx.SetHeader("xixi", "haha")

	// 写入body
	var DirStat []os.FileInfo
	DirStat, err = ioutil.ReadDir(FilePath)

	if err != nil {
		log.Fatal("[ListHandler +50]: unknown Fail")
	}

	// 写入标题部分 不判断错误
	_, err = ctx.RspWriter.Write([]byte(config.MyConfig.Html.Title))

	_, err = ctx.RspWriter.Write([]byte("<ul>"))

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
		content := fmt.Sprintf("<li><strong><a href='%s'>%s</a></strong></li>",UrlPath + v.Name(), v.Name())
		log.Println(v.Name(), "href:", UrlPath + v.Name())
		_, err = ctx.RspWriter.Write([]byte(content))
	}

	_, err = ctx.RspWriter.Write([]byte("</ul></html>"))
}