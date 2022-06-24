// Response�࣬������client����HTTP��Ӧ����
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
    //����server��Ĭ�Ϸ���200 OK�ˣ���API��׼�ӿڴ�statusCode��ָ�����
    HttpResponse(const std::string status = "200 OK") { setStatus(status); }
    ~HttpResponse() { clear(); }
    HttpResponse &operator=(const HttpResponse &t)
    {
        // �����Ը�ֵ ���
        if (this != &t)
        {
            body = t.body;
            header = t.header;
        }
        return *this;
    }
};
