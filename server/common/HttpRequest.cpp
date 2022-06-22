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
    while (true) {  //解析第一行
        if (_raw_http_request[cur] == ' ') {
            if (cnt == 0) {
                method = _raw_http_request.substr(lastcur, cur - lastcur);
                lastcur = cur + 1;
                cnt++;
            } else if (cnt == 1) {
                string resource = _raw_http_request.substr(lastcur, cur - lastcur);
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
                    // for (auto i : params) {
                    //     cout << i.first << ':' << i.second << endl;
                    // }
                } else {
                    route = resource;
                }
                cnt++;
            }
        } else if (_raw_http_request[cur] == '\n') {
            http_version = _raw_http_request.substr(lastcur, cur - lastcur);
            lastcur = cur + 1;
            cnt = 0;  //一行已经结束了
            break;    //这里只解析第一行
        }
        ++cur;
    }
}