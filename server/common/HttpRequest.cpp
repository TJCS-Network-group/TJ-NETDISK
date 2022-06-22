//����client�˴����HTTP����
#include "../include/HttpRequest.h"

#include <iostream>
using namespace std;
// ����������
// resouce��'/'��ͷ
// Method��ʱֻ�������֣�GET POST PUT DELETE
// paramsֻ�е�methodΪGET��ʱ�����ֵ��bodyֻ����JSON��from-data����

void HttpRequest::Parse_request_header(const string _raw_http_request) {
    int lastcur = 0, cur = 0, line = 1, cnt = 0;
    while (true) {  //������һ��
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
                    string params_str = resource.substr(pos + 1);  //֮���
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
                    params[key] = value;  //���һ��
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
            cnt = 0;  //һ���Ѿ�������
            break;    //����ֻ������һ��
        }
        ++cur;
    }
}