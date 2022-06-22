//����Client�˴����HTTP����
#pragma once
#include <map>
#include <string>

#include "./Json.h"
class HttpRequestHeader {
    std::string raw_http_request;  // socket���ܵ��ĵ�һ���ı�
   public:
    std::string http_version;                   // �󲿷�Ӧ�ö���1.1
    std::string route;                          // ��'/'��ͷ��ȥ��params
    std::string method;                         // ��ʱֻ�������֣�GET POST PUT DELETE
    std::map<std::string, std::string> params;  // GETʱ��resource��������
    std::map<std::string, std::string> header;  // header
    //����������
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
    std::string http_version;                   // �󲿷�Ӧ�ö���1.1
    std::string route;                          // ��'/'��ͷ��ȥ��params
    std::string method;                         // ��ʱֻ�������֣�GET POST PUT DELETE
    std::map<std::string, std::string> params;  // GETʱ��resource��������
    std::map<std::string, std::string> header;  // header
    // std::string body;                              // TypeΪJSON��form-dataʱbody������
    JSON json;                                     // body�к���jsonʱ��Ҫ
    std::map<std::string, std::string> form_data;  //�������ļ�����map�л��¼�һ��filename:xxx

    //����(����header��)������
    void Parse_request_header(const std::string _raw_http_request);

    HttpRequest(const std::string _raw_http_request) { Parse_request_header(_raw_http_request); }

    //����body���ݣ����У�
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