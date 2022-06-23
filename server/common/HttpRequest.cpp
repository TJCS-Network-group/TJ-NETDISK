//解析client端传入的HTTP请求
#include "../include/HttpRequest.h"

#include <iostream>
using namespace std;
// 解析请求报文
// resouce以'/'开头
// Method暂时只用这四种：GET POST PUT DELETE
// params只有当method为GET的时候才有值，body只解析JSON和from-data就行

void HttpRequest::Parse_request_header(const string _raw_http_request) {
    int lastcur = 0, cur = 0, line = 1, cnt = 0;
    //解析第一行
    while (true) {
        if (_raw_http_request[cur] == ' ') {
            if (cnt == 0) {
                method = _raw_http_request.substr(lastcur, cur - lastcur);
                lastcur = cur + 1;
                cnt++;
            } else if (cnt == 1) {
                string resource = _raw_http_request.substr(lastcur, cur - lastcur);
                // cout << "resource len:" << resource.length() << endl;
                lastcur = cur + 1;
                size_t pos = resource.find('?');
                if (pos != resource.npos) {
                    route = resource.substr(0, pos);
                    string params_str = resource.substr(pos + 1);  //之后的
                    string key, value;
                    bool find_key = true;
                    for (size_t i = 0; i < params_str.size(); ++i) {
                        if (params_str[i] == '&') {
                            find_key = true;
                            params.insert({key, value});
                            key.clear();
                            value.clear();
                        } else if (params_str[i] == '=') {
                            find_key = false;
                        } else {
                            if (find_key)
                                key.push_back(params_str[i]);
                            else {
                                value.push_back(params_str[i]);
                            }
                        }
                    }
                    params[key] = value;  //最后一项
                } else {
                    route = resource;
                }
                cnt++;
            }
        } else if (_raw_http_request[cur] == '\n' && _raw_http_request[cur - 1] == '\r') {  //换行
            http_version = _raw_http_request.substr(lastcur, cur - lastcur - 1);
            lastcur = cur + 1;  //下一行起始
            ++cur;
            cnt = 0;  //一行已经结束了
            break;    //这里只解析第一行
        }
        ++cur;
    }
    cout << lastcur << endl;
    cout << cur << endl;
    bool find_key = true;
    string key, value;

    while (true) {
        if (_raw_http_request[cur] == ':') {
            find_key = false;                               //开始找value
            ++cur;                                          //跳过冒号
            while (_raw_http_request[cur] == ' ') ++cur;    //跳过空格
        } else if (_raw_http_request.length() > cur + 3) {  //不越界
            if (_raw_http_request[cur] == '\r' && _raw_http_request[cur + 1] == '\n') {
                headers[key] = value;
                find_key = true;  //开始下一轮找key

                //下两个是否还是\r\n
                if (_raw_http_request[cur + 2] == '\r' && _raw_http_request[cur + 3] == '\n') {
                    cur += 4;
                    break;
                }

                cur += 2;
                key.clear();
                value.clear();
            }
        } else
            break;
        if (find_key == true) {
            key.push_back(_raw_http_request[cur]);
        } else {
            value.push_back(_raw_http_request[cur]);
        }
        ++cur;
    }
    origin_headers = _raw_http_request.substr(0, cur - 2);
    body = _raw_http_request.substr(cur);

    cout << "PARAMS:\n" << endl;
    for (auto i : params) {
        cout << i.first << ':' << i.second << endl;
    }
    cout << "HEADERS:\n" << endl;
    for (auto i : headers) {
        cout << i.first << ":" << i.second << endl;
    }
}
void HttpRequest::Parse_request_body() {
    if (headers.count("Content-Length") != 0) {
        string type = headers["Content-Type"];
        cout << type << endl;
        {
            size_t pos = type.find("application/json");
            cout << pos << endl;
            if (pos != type.npos) {  //找到了子串"application/json"，是json
                json = JSON(body);
                cout << "JSON:\n" << json << endl;
            }
        }
        {
            size_t pos = type.find("multipart/form-data");
            cout << pos << endl;
            if (pos != type.npos) {  //找到了子串"multipart/form-data"，是form-data
            }
        }
    }
}
