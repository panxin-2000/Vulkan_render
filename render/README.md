
# 简单的写一写计划
如果可以的话，render 下的第一级目录，是外部能够调用的接口

## 各个目录中的内容
- render_common : 基础的数据类型的定义
- render_component : common 中的内容或组合后 添加 entity 作为第一个参数
- render_thread : 和跨线程相关的函数都放置在这里
- shader : 放置着色器文件
- vulkan_code : 主要是底层 vulkan API 的调用
- 单个的文件,不清楚,需要整理归类

vulkan_code 中 作为最底层，其中不能保护其他文件夹中的内容，比如 entity 