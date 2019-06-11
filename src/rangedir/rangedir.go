/*
 * Author: ahaoozhang
 * Date: 2019/6/10 20:48
 * Desc: 遍历文件夹下的文件，包括 . / ..
 */


package rangedir

/*
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#define FileSize 1024
#define NameSize 256

char* FileName[FileSize];

int ListFile(char* file) {
	struct dirent **p_dirent = NULL;
	int num = scandir(file, &p_dirent, 0, alphasort);
	int i = 0;
	for(i = 0; i < num; i++) {
		FileName[i] = (char*)malloc(sizeof(char) * NameSize);
		strcpy(FileName[i], p_dirent[i]->d_name);
	}
	return num;
}
*/
import "C"
import (
	"fmt"
	"log"
	"unsafe"
)

func RangeDir(DirPath string) (FileSlice []string, err error) {
	var num C.int
	num = C.ListFile(C.CString(DirPath))
	fmt.Println(num)

	// 将C语言数组转换为slice
	for i := 0; i < int(num); i++ {
		FileSlice = append(FileSlice, C.GoString(C.FileName[i]))
		C.free(unsafe.Pointer(C.FileName[i]))
	}

	log.Println(FileSlice)
	return FileSlice, nil
}

