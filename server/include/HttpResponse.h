// Response类，用于向client返回HTTP响应报文
#pragma once
#include <string>
class HttpResponse
{
    std::string header, body;
    static const std::string HTTP_VERSION;

public:
    void setStatus(const std::string status = "200 OK");
    void setHeader(const std::string _header = "");
    void setBody(const std::string _body = "");
    void clear()
    {
        header.clear();
        body.clear();
    }
    std::string getMessage() { return header + "\r\n" + body; }

    // HttpResponse() {}
    //我们server端默认返回200 OK了，在API标准接口处statusCode再指定别的
    HttpResponse(const std::string status = "200 OK") { setStatus(status); }
    ~HttpResponse() { clear(); }
    HttpResponse &operator=(const HttpResponse &t)
    {
        // 避免自赋值 深拷贝
        if (this != &t)
        {
            body = t.body;
            header = t.header;
        }
        return *this;
    }
};
