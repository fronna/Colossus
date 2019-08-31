#include <iostream>
#include "ProtocolUtil.hpp"

static void Usage(std::string proc)
{
    std::cout<<"Usage: "<<proc<<" "<<"port"<<std::endl;
}

int main(int argc, char *argv[])
{
  //1.创建searcher对象，并初始化
  searcher::Searcher s;
  bool ret = s.Init("../../data/tmp/raw_input");
  if(!ret) {
    std::cout << "Searcher Init Failed" << std::endl;
    return 1;
  }
  //2.创建http服务器
  using namespace httplib;
    if(argc!=2)
    {
        Usage(argv[0]);
        exit(1);
    }
    HttpServer *ser=new HttpServer(atoi(argv[1]));
    ser->InitServer();
    ser->Start();

    // /searche?query=filesystem
    Http_Request *rq=new Http_Request;
    Http_Response *rsp=new Http_Response;
    std::string query = rq->GetParam();
    std::string result;
    s.Search(query, &result);
    rsp->response_text = result; 
    return 0;
}
