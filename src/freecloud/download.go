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

func DownloadHandler(ctx *FLServer, FilePath string) {
	log.Printf("FilePath: %s", FilePath)
	ctx.RspWriter.WriteHeader(200)

	// 完善Header
	ctx.SetHeader("Content-Type", BINARY_TYPE)
	md5str, _ := md5.GetMD5(FilePath)
	ctx.SetHeader("Etag", md5str)
	//ctx.SetHeader("Transfer-Encoding", "chunked")
	//ctx.SetHeader("Connection", "Keep-Alive")
	//ctx.SetHeader("Accept-Ranges", "bytes")


	// 写入body
	var pos uint64
	var prevPos uint64

	FileInfo, _ := os.Stat(FilePath)

	fileIO, _ := os.Open(FilePath)
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
			Range := fmt.Sprintf("bytes %d-%d/%d", prevPos, pos, FileInfo.Size())

			ctx.SetHeader("Content-Range", Range)
			_, _ = ctx.RspWriter.Write(buffer[:n])
		}
	}
}