//����Client�˴����HTTP����
#pragma once
#include <iostream>
#include <map>
#include <string>

#include "./Json.h"
extern std::map<int, std::string> session; // user_id : cookie��md5����
class HttpRequest
{
public:
    std::string http_version;                     // �󲿷�Ӧ�ö���1.1
    std::string route;                            // ��'/'��ͷ��ȥ��params
    std::string method;                           // ��ʱֻ�������֣�GET POST PUT DELETE
    std::map<std::string, std::string> params;    // GETʱ��resource��������
    std::map<std::string, std::string> headers;   // headers
    std::string origin_headers;                   //ԭʼheaders
    std::string body;                             // TypeΪJSON��form-dataʱbody������
    JSON json;                                    // body�к���jsonʱ��Ҫ
    std::map<std::string, std::string> form_data; //�������ļ�������api�涨���ֶ��⣬��map�л��¼�һ��filename�ֶ�
    string clientIP;
    int clientPort;
    int current_user_id;
    bool disconnect;
    bool wait_header;
    //����(����headers��)������
    void Parse_request_header(const std::string _raw_http_request);
    //����body���ݣ����У�
    void Parse_request_body();

    HttpRequest(const std::string _raw_http_request = "")
    {
        clear();
        Parse_request_header(_raw_http_request);
    }
    void Concat_body(const std::string _body) { body += _body; }
    void Set_IP(const char *ip, int Port)
    {
        clientIP = ip;
        clientPort = Port;
    }
    bool Read_body_over()
    {
        if (headers.count("content-length") != 0)
        { //����length�ֶ���body�ĳ�����length�ĳ��Ȳ�һ�� ��Ϊ��û����body
            int len = atoi(headers["content-length"].c_str());
            // std::cout << "content-length: " << len << std::endl;
            // std::cout << "body len: " << body.length() << std::endl;
            if (body.length() < len)
                return false;
        }
        return true;
    }
    int Get_body_len() //��ȡ�˴α���body�ĳ���, ��һ��Ҫ��body�������
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
    int Get_request_len() //��ȡ�˴α��ĵ��ܳ���
    {
        if (wait_header)
            return -1;
        else
        {
            return origin_headers.length() + 2 + Get_body_len();
        }
    }
    //��ȡԭʼ����
    std::string Get_origin_headers() { return origin_headers; }
    std::string Get_body() { return body; }
    void clear()
    {
        disconnect = false;
        wait_header = true;
        current_user_id = 0; // current_user_idΪ0����û��¼
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
        // �����Ը�ֵ ���
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
