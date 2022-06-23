//����Client�˴����HTTP����
#pragma once
#include <iostream>
#include <map>
#include <string>

#include "./Json.h"
extern std::map<int, std::string> session;  // user_id : cookie��md5����
class HttpRequest {
   public:
    std::string http_version;                      // �󲿷�Ӧ�ö���1.1
    std::string route;                             // ��'/'��ͷ��ȥ��params
    std::string method;                            // ��ʱֻ�������֣�GET POST PUT DELETE
    std::map<std::string, std::string> params;     // GETʱ��resource��������
    std::map<std::string, std::string> headers;    // headers
    std::string origin_headers;                    //ԭʼheaders
    std::string body;                              // TypeΪJSON��form-dataʱbody������
    JSON json;                                     // body�к���jsonʱ��Ҫ
    std::map<std::string, std::string> form_data;  //�������ļ�������api�涨���ֶ��⣬��map�л��¼�һ��filename�ֶ�
    int current_user_id;
    bool disconnect;

    //����(����headers��)������
    void Parse_request_header(const std::string _raw_http_request);
    //����body���ݣ����У�
    void Parse_request_body();

    HttpRequest(const std::string _raw_http_request = "") { Parse_request_header(_raw_http_request); }
    void Concat_body(const std::string _body) { body += _body; }
    bool Read_body_over() {
        if (headers.count("Content-Length") != 0) {  //����length�ֶ���body�ĳ�����length�ĳ��Ȳ�һ�� ��Ϊ��û����body
            int len = atoi(headers["Content-Length"].c_str());
            // std::cout << "Content-Length: " << len << std::endl;
            // std::cout << "body len: " << body.length() << std::endl;
            if (body.length() < len) return false;
        }
        return true;
    }
    //��ȡԭʼ����
    std::string Get_origin_headers() { return origin_headers; }
    std::string Get_body() { return body; }
    void clear() {
        disconnect = false;
        current_user_id = 0;  // current_user_idΪ0����û��¼
        http_version.clear();
        route.clear();
        method.clear();
        params.clear();
        form_data.clear();
        json.clear();
        body.clear();
    }
    HttpRequest& operator=(const HttpRequest& t) {
        // �����Ը�ֵ ���
        if (this != &t) {
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
        }
        return *this;
    }
    ~HttpRequest() { clear(); }
};