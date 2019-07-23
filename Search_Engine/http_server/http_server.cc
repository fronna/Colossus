#include "httplib.h"
#include <iostream>
#include "../searcher/searcher.h"

int main() {
  //1. 创建searcher对象，并初始化
  searcher::Searcher s;
  bool ret = s.Init("../data/tmp/raw_input");
  if(!ret) {
    std::cout << "Searcher Init failed" << std::endl;
    return 1;
  }
  //2. 创建http服务器
  using namespace httplib;
  Server server;
  // /search?query=filesystem
  server.Get("/search", [&s] (const Request& req, Response& res){
      std::string query = req.get_param_value("query");
      std::string result;
      s.Search(query, &result);
      res.set_content(result, "text/plain");
      });
  server.set_base_dir("./wwwroot");
  server.listen("0.0.0.0", 8888);
  return 0;
}
