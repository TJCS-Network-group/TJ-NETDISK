//解析Client端传入的HTTP请求
#pragma once
#include <string>
class HttpRequest {
    std::string raw_http_request;  // socket接受到的文本
   public:
    std::string http_version;  // 大部分应该都是1.1
    std::string route;         // 以'/'开头，去掉params
    std::string method;        // 暂时只用这四种：GET POST PUT DELETE
    std::string params;        // GET时从resource解析出来
    std::string body;          // JSON或form-data
    //解析请求报文
    void Parse_request(const std::string _raw_http_request);

    HttpRequest(const std::string _raw_http_request) { Parse_request(_raw_http_request); }

    //
    void clear() {
        raw_http_request.clear();
        http_version.clear();
        route.clear();
        method.clear();
        params.clear();
        body.clear();
    }
    ~HttpRequest() { clear(); }
};