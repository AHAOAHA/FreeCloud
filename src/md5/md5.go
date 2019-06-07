package md5

import (
	"bufio"
	"crypto/md5"
	"io"
	"fmt"
	"os"
)

// 该文件提供接口计算出文件的md5值

func GetMD5(FilePath string) (checksum string, err error){
	if FileInfo , err := os.Stat(FilePath); err != nil {
		return "", err
	} else if FileInfo.IsDir() {
		return "", err
	}

	FileReader, err := os.Open(FilePath)
	if err != nil {
		return "", err
	}
	defer FileReader.Close()

	hash := md5.New()

	for buf, reader := make([]byte, 4096), bufio.NewReader(FileReader) ; ; {
		n, err := reader.Read(buf)
		if err != nil {
			if err == io.EOF {
				break
			}

			return "", err
		}

		hash.Write(buf[:n])
	}

	checksum = fmt.Sprintf("%x", hash.Sum(nil))
	fmt.Println("md5:", checksum)
	return
}
