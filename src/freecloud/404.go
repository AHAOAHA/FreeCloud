package freecloud

func (ctx *FLServer) Handler404() {
	ctx.RspWriter.WriteHeader(404)

	// 设置Header

	// 设置404页面

}