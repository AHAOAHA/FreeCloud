package freecloud

import (
	"bufio"
	"fmt"
	"io"
	"log"
	"md5"
	"os"
	//"md5"
)

const (
	//TODO
	BINARY_TYPE = ""
	HTML_TYPE = ""
)

func (ctx *FLServer)DownloadHandler() {
	Ranges := ctx.Request.Header.Get("Content-Range")
	log.Println("Content-Range:", Ranges)

	// 完善Header
	ctx.SetHeader("Content-Type", BINARY_TYPE)
	md5str, _ := md5.GetMD5(ctx.GetFilePath())
	ctx.SetHeader("Etag", md5str)
	ctx.SetHeader("Transfer-Encoding", "chunked")
	ctx.SetHeader("Connection", "Keep-Alive")
	ctx.SetHeader("Accept-Ranges", "bytes")
	fileInfo, _ := os.Stat(ctx.GetFilePath())
	ctx.SetHeader("Last-Modified", fileInfo.ModTime().String())

	// 写入状态码s
	ctx.WriteRspFirstHeader(206)

	// 写入body
	var pos uint64
	var prevPos uint64

	fileIO, _ := os.Open(ctx.GetFilePath())
	defer  func(){
		_ = fileIO.Close()
	}()
	Reader := bufio.NewReader(fileIO)

	for {
		buffer := make([]byte, 4096)
		n, err := Reader.Read(buffer)
		if err == io.EOF {
			log.Println("File Send Over")
			break
		} else {
			prevPos = pos
			pos += uint64(n)
			Range := fmt.Sprintf("bytes %d-%d/%d", prevPos, pos, fileInfo.Size())

			ctx.SetHeader("Content-Range", Range)
			_, _ = ctx.RspWriter.Write(buffer[:n])
		}
	}
}