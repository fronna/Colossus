### FalconSearchEngine

> 实现了一个基于boost文档的在线搜索网站

#### 开发环境
> Centos7，gcc 4.9.2，jsoncpp，CppJieba分词，cpp-httplib，Boost库

#### 项目描述
> 实现了一个基于boost文档的在线搜索网站。搜索引擎部分有三个模块，数据处理(对HTML去标签)、索引(构造正排索引和倒排索引)、搜索(根据关键词的出现频率进行降序显示)；四个步骤，分词、触发、排序、返回搜索结果；网页部分使用 vue.js 接收来自服务器的JSON格式的搜索结果，并使用html，CSS对页面进行渲染，用户体验良好
