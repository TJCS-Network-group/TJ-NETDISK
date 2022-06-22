//解析Client端传入的HTTP请求
#pragma once
#include <map>
#include <string>

#include "./Json.h"
class HttpRequestHeader {
    std::string raw_http_request;  // socket接受到的第一段文本
   public:
    std::string http_version;                   // 大部分应该都是1.1
    std::string route;                          // 以'/'开头，去掉params
    std::string method;                         // 暂时只用这四种：GET POST PUT DELETE
    std::map<std::string, std::string> params;  // GET时从resource解析出来
    std::map<std::string, std::string> header;  // header
    //解析请求报文
    void Parse_request(const std::string _raw_http_request);

    HttpRequestHeader(const std::string _raw_http_request) { Parse_request(_raw_http_request); }

    //
    void clear() {
        raw_http_request.clear();
        http_version.clear();
        route.clear();
        method.clear();
        params.clear();
        header.clear();
    }
    ~HttpRequestHeader() { clear(); }
};

class HttpRequest {
   public:
    std::string http_version;                   // 大部分应该都是1.1
    std::string route;                          // 以'/'开头，去掉params
    std::string method;                         // 暂时只用这四种：GET POST PUT DELETE
    std::map<std::string, std::string> params;  // GET时从resource解析出来
    std::map<std::string, std::string> header;  // header
    // std::string body;                              // Type为JSON或form-data时body的内容
    JSON json;                                     // body中含有json时需要
    std::map<std::string, std::string> form_data;  //若含有文件，此map中会新加一个filename:xxx

    //解析(包含header的)请求报文
    void Parse_request_header(const std::string _raw_http_request);

    HttpRequest(const std::string _raw_http_request) { Parse_request_header(_raw_http_request); }

    //解析body内容（若有）
    void Parse_request_body(const std::string body);
    //
    void clear() {
        http_version.clear();
        route.clear();
        method.clear();
        params.clear();
        form_data.clear();
        json.clear();
    }
    ~HttpRequest() { clear(); }
};