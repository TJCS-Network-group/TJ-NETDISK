//����Client�˴����HTTP����
#pragma once
#include <string>
class HttpRequest {
    std::string raw_http_request;  // socket���ܵ����ı�
   public:
    std::string http_version;  // �󲿷�Ӧ�ö���1.1
    std::string route;         // ��'/'��ͷ��ȥ��params
    std::string method;        // ��ʱֻ�������֣�GET POST PUT DELETE
    std::string params;        // GETʱ��resource��������
    std::string body;          // TypeΪJSON��form-dataʱbody������
    int Content_Length;        // body�ĳ���
    std::string Content_Type;  // JSON��form-data
    //����������
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