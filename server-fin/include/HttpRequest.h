//解析Client端传入的HTTP请求
#pragma once
#include <iostream>
#include <map>
#include <string>

#include "./Json.h"
extern std::map<int, std::string> session; // user_id : cookie的md5部分
class HttpRequest
{
public:
    std::string http_version;                     // 大部分应该都是1.1
    std::string route;                            // 以'/'开头，去掉params
    std::string method;                           // 暂时只用这四种：GET POST PUT DELETE
    std::map<std::string, std::string> params;    // GET时从resource解析出来
    std::map<std::string, std::string> headers;   // headers
    std::string origin_headers;                   //原始headers
    std::string body;                             // Type为JSON或form-data时body的内容
    JSON json;                                    // body中含有json时需要
    std::map<std::string, std::string> form_data; //若含有文件，除了api规定的字段外，此map中会新加一个filename字段
    std::string clientIP;
    int clientPort;
    int current_user_id;
    bool disconnect;
    bool wait_header;
    //解析(包含headers的)请求报文
    void Parse_request_header(const std::string _raw_http_request);
    //解析body内容（若有）
    void Parse_request_body();

    HttpRequest(const std::string _raw_http_request = "")
    {
        clear();
        Parse_request_header(_raw_http_request);
    }
    void Concat_body(const std::string _body) { body += _body; }
    void Set_IP(const char *ip, const int Port)
    {
        clientIP = ip;
        clientPort = Port;
    }
    bool Read_body_over()
    {
        if (headers.count("content-length") != 0)
        { //存在length字段且body的长度与length的长度不一致 认为还没读完body
            int len = atoi(headers["content-length"].c_str());
            // std::cout << "content-length: " << len << std::endl;
            // std::cout << "body len: " << body.length() << std::endl;
            if (body.length() < len)
                return false;
        }
        return true;
    }
    int Get_body_len() //获取此次报文body的长度, 不一定要等body传输完毕
    {
        if (wait_header)
            return -1;
        else
        {
            if (headers.count("content-length") != 0)
                return atoi(headers["content-length"].c_str());
            else
                return 0;
        }
    }
    int Get_request_len() //获取此次报文的总长度
    {
        if (wait_header)
            return -1;
        else
        {
            return origin_headers.length() + 2 + Get_body_len();
        }
    }
    //获取原始数据
    std::string Get_origin_headers() { return origin_headers; }
    std::string Get_body() { return body; }
    void clear()
    {
        disconnect = false;
        wait_header = true;
        current_user_id = 0; // current_user_id为0代表没登录
        http_version.clear();
        route.clear();
        method.clear();
        params.clear();
        form_data.clear();
        json.clear();
        body.clear();
        headers.clear();
        origin_headers.clear();
    }
    HttpRequest &operator=(const HttpRequest &t)
    {
        // 避免自赋值 深拷贝
        if (this != &t)
        {
            wait_header = wait_header;
            http_version = t.http_version;
            route = t.route;
            method = t.method;
            params = t.params;
            headers = t.headers;
            origin_headers = t.origin_headers;
            body = t.body;
            json = t.json;
            form_data = t.form_data;
            disconnect = t.disconnect;
            current_user_id = t.current_user_id;
        }
        return *this;
    }
    ~HttpRequest() { clear(); }
};

// HttpRequest http_recv_request(int sockfd);
