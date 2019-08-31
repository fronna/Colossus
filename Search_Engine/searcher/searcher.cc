#include "searcher.h"
#include<iostream>
#include<fstream>
#include<algorithm>
#include<jsoncpp/json/json.h>
#include "../common/util.hpp"

namespace searcher {

  /////////////////////////////////////////////////// 
  // 以下代码是索引模块的实现
  /////////////////////////////////////////////////// 

  const char* const DICT_PATH = "/home/lua/Proc/ColossusProject/colossus/Search_Engine/jieba_dict/jieba.dict.utf8";
  const char* const HMM_PATH = "/home/lua/Proc/ColossusProject/colossus/Search_Engine/jieba_dict/hmm_model.utf8";
  const char* const USER_DICT_PATH = "/home/lua/Proc/ColossusProject/colossus/Search_Engine/jieba_dict/user.dict.utf8";
  const char* const IDF_PATH = "/home/lua/Proc/ColossusProject/colossus/Search_Engine/jieba_dict/idf.utf8";
  const char* const STOP_WORD_PATH = "/home/lua/Proc/ColossusProject/colossus/Search_Engine/jieba_dict/stop_words.utf8";

  Index::Index() : jieba_(DICT_PATH,
      HMM_PATH, USER_DICT_PATH,  IDF_PATH, STOP_WORD_PATH) {
  }

  const DocInfo* Index::GetDocInfo(uint64_t doc_id) {
    if(doc_id >= forward_index_.size()) {
      return NULL;
    } 
    return &forward_index_[doc_id];
  }
  const InvertedList* Index::GetInvertedList(const std::string& key) const {
    auto pos = inverted_index_.find(key);
    if(pos == inverted_index_.end()) {
      //没找到
      return NULL;
    }
    //unordered_map迭代器指向的数据类型是什么
    return &pos->second;
  }

  bool Index::Build(const std::string& input_path) {
    std::cout << "Index Build Start!" << std::endl;
    //1. 按行读取文件内容（每一行对应一个文档）
    std::ifstream file(input_path.c_str());
    if(!file.is_open()) {
      std::cout << "input_path open failed! input_path=" << input_path << std::endl;
      return false;
    }
    std::string line;  //每次按行读出的这个内容不包含结尾的"\n"
    while(std::getline(file, line)) {
      //2. 构造DocInfo对象，更新正排索引数据；对读到的一行文件进行解析，
      //得到DocInfo对象再插入vector
      const DocInfo* doc_info = BuildForward(line);
      //3. 更新倒排索引数据
      BuildInverted(*doc_info);
      if(doc_info->doc_id % 100 == 0)
      {
        std::cout << "Build doc_id=" << doc_info->doc_id << std::endl;
      }
    }
    std::cout << "Index Build Finish!" << std::endl;
    file.close();
    return true;
  }

  const DocInfo* Index::BuildForward(const std::string& line) {
    //1.对这一行内容进行切分(\3)
    std::vector<std::string> tokens; //存放切分结果
    // strtok破坏原字符串
    // stringstream比较麻烦
    // 借助boost进行切分
    StringUtil::Split(line, &tokens, "\3");
    if(tokens.size() != 3) {
      std::cout << "tokens not ok" << std::endl;
      return NULL;
    }
    //2.构造一个DocInfo对象
    DocInfo doc_info;
    doc_info.doc_id = forward_index_.size();
    doc_info.title = tokens[0];
    doc_info.url = tokens[1];
    doc_info.content = tokens[2];
    //3.把这个对象插入到正排索引中
    forward_index_.push_back(doc_info);
    return &forward_index_.back();
  }

  void Index::BuildInverted(const DocInfo& doc_info) {
    //1.先对doc_info进行分词，对正文和标题分词
    std::vector<std::string> title_tokens;
    CutWord(doc_info.title, &title_tokens);
    std::vector<std::string> content_tokens;
    CutWord(doc_info.content, &content_tokens);
    //2.对doc_info中的标题和正文进行词频统计
    //   当前词在标题中出现几次，在正文中出现几次
    struct WordCnt {
      int title_cnt;
      int content_cnt;
    };
    //用一个hash表完成词频的统计
    std::unordered_map<std::string, WordCnt> word_cnt;
    for(std::string word : title_tokens) {
      //假设正文中出现hello ,HELLO，应该算一个词出现两次
      //统计词频时忽略大小写
      boost::to_lower(word);
      ++word_cnt[word].title_cnt;
    }
    for(std::string word : content_tokens) {
      boost::to_lower(word);
      ++word_cnt[word].content_cnt;
    }
    //3. 遍历分词结果，在倒排索引中查找
    //word_pair的类型是std::pair，我们j在这里是把std::pair作为value用在了std::unordered_map里
    for(const auto& word_pair : word_cnt) {
      Weight weight;
      weight.doc_id = doc_info.doc_id; 
      weight.weight = 10 * word_pair.second.title_cnt 
        + word_pair.second.content_cnt;
      weight.key = word_pair.first;  //把这个词顺便记录到Weight中
      //4.如果该分词结果在倒排索引中不存在，就构建新的键值对
      //5. 如果该分词结果在倒排中存在，找到对应的值(vector)，
      //构建一个新的Weight对象插入到vector中
      InvertedList& inverted_index = inverted_index_[word_pair.first];
      inverted_index.push_back(weight);
    } 
    return;
  }

  void Index::CutWord(const std::string& input,
      std::vector<std::string>* output){
    jieba_.CutForSearch(input, *output);
  }


  ///////////////////////////////////////////////////
  //以下代码是搜索模块的实现
  /////////////////////////////////////////////// 

  bool Searcher::Init(const std::string& input_path) {
    return index_->Build(input_path);
  }

  bool Searcher::Search(const std::string& query, std::string* json_result) {
    //1. [分词] 对查询词进行分词
    std::vector<std::string> tokens;
    index_->CutWord(query, &tokens);
    //2. [触发] 针对分词结果查倒排索引，找到哪些文档是具有相关性的
    std::vector<Weight> all_token_result;
    for(std::string word : tokens) {
      boost::to_lower(word);
      auto* inverted_list = index_->GetInvertedList(word);
      if(inverted_list == NULL) {
        //不能因为某个分词的结果在索引中不存在就影响到其他的分词结果的查询
        continue;
      }
      //此处进一步的改进是考虑不同的分词结果对应相同文档id的情况，需要进行去重，
      //和权重合并，实现思路类似于合并有序链表
      all_token_result.insert(all_token_result.end(),
          inverted_list->begin(), inverted_list->end());
    }
    //3. [排序] 把这些结果按照一定规则排序
    //sort第三个参数可以使用 仿函数/函数指针/lambda表达式
    //lambda表达式是一个匿名函数
    std::sort(all_token_result.begin(), all_token_result.end(), 
              [](const Weight& w1, const Weight& w2){
        return w1.weight > w2.weight;
        }); 
    //4. [构造结果] 查正排，扎到每个搜索结果的标题，正文，url
    //预期构造成的结果：
    //[
    //  {
    //        "title": "标题",
    //        "desc": "描述",
    //        "url":"url"
    //  }
    //]
    Json::Value results;   //表示所有搜索结果的JSON对象
    for(const auto& weight : all_token_result) {
      const auto* doc_info = index_->GetDocInfo(weight.doc_id);
      if(doc_info == NULL) {
        continue;
      }
      //如何构造成 JSON结构，用第三方库来实现 jsoncpp
      Json::Value result;   //表示一条搜索结果的JSON对象
      result["title"] = doc_info->title;
      result["url"] = doc_info->url;
      result["desc"] = GetDesc(doc_info->content, weight.key);
      results.append(result);
    }
    //借助 jsoncpp 能够快速地完成JSON对象和字符串的转换
    Json::FastWriter writer;
    *json_result = writer.write(results);
    return true;
  }

  std::string Searcher::GetDesc(const std::string& content, const std::string& key) 
  {
    //描述是正文的一部分，描述最好要包含查询词
    //1. 先在正文中查找一下这个词的位置
    size_t pos = content.find(key);
    if(pos == std::string::npos) {
      // 该词在正文中不存在(合理的， 有可能只在标题中出现)
      // 此时直接从开头截取一段即可
      if(content.size() < 160) {
        return content;
      } else {
        return content.substr(0, 160) + "...";
      }
    }
    //2. 以该位置为基准位置，往前截取一部分字符串，往后截取一部分字符串
    //以该位置为基准，往前截取60个字节，往后截取100个字节
    size_t beg = pos < 60 ? 0 : pos - 60;
    if(beg + 160 >= content.size()) {
      //beg之后的长度不足160，就把剩余的内容都算作描述
      return content.substr(beg);
    } else {
      return content.substr(beg, 160) + "...";
    }
  } 
} //end searcher

int main() {
  //1.创建searcher对象，并初始化
  searcher::Searcher s;
  bool ret = s.Init("../../../data/tmp/raw_input");
  if(!ret) {
    std::cout << "Searcher Init failed" << std::endl;
    return 1;
  }
  return 0;
}
