//����client�˴����HTTP����
#include "../include/HttpRequest.h"

#include <iostream>
using namespace std;
// ����������
// resouce��'/'��ͷ
// Method��ʱֻ�������֣�GET POST PUT DELETE
// paramsֻ�е�methodΪGET��ʱ�����ֵ��bodyֻ����JSON����
void HttpRequest::Parse_request(const string _raw_http_request) {
    int lastcur = 0, cur = 0, line = 1, cnt = 0;
    while (true) {
        if (_raw_http_request[cur] == ' ') {
            if (line == 1) {
                if (cnt == 0) {
                    method = _raw_http_request.substr(lastcur, cur - lastcur);
                    lastcur = cur + 1;
                    cnt++;
                } else if (cnt == 1) {
                    cout << lastcur << "    " << cur << endl;
                    string resource = _raw_http_request.substr(lastcur, cur - lastcur);
                    lastcur = cur + 1;
                    size_t pos = resource.find('?');
                    cout << pos << "    " << resource.npos << endl;
                    cout << resource << endl;
                    if (pos != resource.npos) {
                        route = resource.substr(0, pos);
                        params = resource.substr(pos);  //֮���
                    } else {
                        route = resource;
                    }
                    cnt++;
                }
            }
        } else if (_raw_http_request[cur] == '\n') {
            if (line == 1) {
                http_version = _raw_http_request.substr(lastcur, cur - lastcur);
                lastcur = cur + 1;
                cnt = 0;  //һ���Ѿ�������
                line++;
                break;  //�����Ȳ�����body
            }
        }
        ++cur;
    }
}