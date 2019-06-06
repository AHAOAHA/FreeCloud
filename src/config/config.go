package config

// 读取配置文件
import (
	"github.com/BurntSushi/toml"
	"log"
)

// ../conf/config.toml 配置文件路径
const ConfigFilePath = "../conf/config.toml"


type ServiceConf struct {
	Addr string
	NetWork string
}

type HtmlInfo struct {
	Title string
}
type Config struct {
	// 服务器监听配置
	Service ServiceConf
	Html HtmlInfo

}

var MyConfig Config

func init() {
	if _, err := toml.DecodeFile(ConfigFilePath, &MyConfig); err != nil {
		// 配置文件解析失败则panic
		log.Fatal("MyConfig Decode Fail, errcode: %d", err)
	}
	// 配置文件解析成功
}
