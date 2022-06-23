//����client�˴����HTTP����
#include "../include/HttpRequest.h"

#include <iostream>
using namespace std;
// ����������
// resouce��'/'��ͷ
// Method��ʱֻ�������֣�GET POST PUT DELETE
// paramsֻ�е�methodΪGET��ʱ�����ֵ��bodyֻ����JSON��from-data����

std::map<int, std::string> session; // user_id : cookie��md5����

void HttpRequest::Parse_request_header(const string _raw_http_request)
{
    // cout << _raw_http_request << endl;
    if (_raw_http_request.length() == 0)
    {
        disconnect = true;
        return;
    }
    else
        disconnect = false;
    int lastcur = 0, cur = 0, line = 1, cnt = 0;
    //������һ��
    while (true)
    {
        if (_raw_http_request[cur] == ' ')
        {
            if (cnt == 0)
            {
                method = _raw_http_request.substr(lastcur, cur - lastcur);
                lastcur = cur + 1;
                cnt++;
            }
            else if (cnt == 1)
            {
                string resource = _raw_http_request.substr(lastcur, cur - lastcur);
                // cout << "resource len:" << resource.length() << endl;
                lastcur = cur + 1;
                size_t pos = resource.find('?');
                if (pos != resource.npos)
                {
                    route = resource.substr(0, pos);
                    string params_str = resource.substr(pos + 1); //֮���
                    string key, value;
                    bool find_key = true;
                    for (size_t i = 0; i < params_str.size(); ++i)
                    {
                        if (params_str[i] == '&')
                        {
                            find_key = true;
                            params.insert({key, value});
                            key.clear();
                            value.clear();
                        }
                        else if (params_str[i] == '=')
                        {
                            find_key = false;
                        }
                        else
                        {
                            if (find_key)
                                key.push_back(params_str[i]);
                            else
                            {
                                value.push_back(params_str[i]);
                            }
                        }
                    }
                    params[key] = value; //���һ��
                }
                else
                {
                    route = resource;
                }
                cnt++;
            }
        }
        else if (_raw_http_request[cur] == '\n' && _raw_http_request[cur - 1] == '\r')
        { //����
            http_version = _raw_http_request.substr(lastcur, cur - lastcur - 1);
            lastcur = cur + 1; //��һ����ʼ
            ++cur;
            cnt = 0; //һ���Ѿ�������
            break;   //����ֻ������һ��
        }
        ++cur;
    }

    bool find_key = true;
    string key, value;

    while (true)
    {
        if (_raw_http_request[cur] == ':')
        {
            find_key = false; //��ʼ��value
            ++cur;            //����ð��
            while (_raw_http_request[cur] == ' ')
                ++cur; //�����ո�
        }
        else if (_raw_http_request.length() > cur + 3)
        { //��Խ��
            if (_raw_http_request[cur] == '\r' && _raw_http_request[cur + 1] == '\n')
            {
                headers[key] = value;
                find_key = true; //��ʼ��һ����key

                //�������Ƿ���\r\n
                if (_raw_http_request[cur + 2] == '\r' && _raw_http_request[cur + 3] == '\n')
                {
                    cur += 4;
                    break;
                }

                cur += 2;
                key.clear();
                value.clear();
            }
        }
        else
            break;
        if (find_key == true)
        {
            key.push_back(_raw_http_request[cur]);
        }
        else
        {
            value.push_back(_raw_http_request[cur]);
        }
        ++cur;
    }
    origin_headers = _raw_http_request.substr(0, cur - 2);
    body = _raw_http_request.substr(cur);
    // cout << body << endl;
    /*
    cout << "PARAMS:" << endl;
    for (auto i : params)
    {
        cout << i.first << ':' << i.second << endl;
    }
    cout << "HEADERS:" << endl;
    for (auto i : headers)
    {
        cout << i.first << ":" << i.second << endl;
    }
    cout << "SESSION: " << endl;
    for (auto i : session)
    {
        cout << i.first << ": " << i.second << endl;
    }*/
    cout << current_user_id << endl;
    if (headers.count("Cookie") != 0)
    { //��cookie
        string cookie = headers["Cookie"];
        string current_user_str;
        // cookie�ṹ�� remember_token=1951705|{md5};
        string sub_str = "remember_token=";
        size_t pos = cookie.find(sub_str);
        if (pos != cookie.npos)
        {
            int cur = pos + sub_str.length();
            while (cur < cookie.length())
            {
                if (cookie[cur] == '|')
                    break;
                current_user_str.push_back(cookie[cur]);
                ++cur;
            }
            string md5 = cookie.substr(cur + 1);
            current_user_id = atoi(current_user_str.c_str()); // current_user_idΪ0����û��¼

            if (session.count(current_user_id) == 0 || md5 != session[current_user_id])
            { //��session�еı���Ա�
                cout << "cookie����" << endl;
                current_user_id = 0;
            }
            cout << "current_user_id:" << current_user_id << endl;
        }
    }
}

void HttpRequest::Parse_request_body()
{
    if (headers.count("Content-Type") != 0)
    {
        string type = headers["Content-Type"];
        cout << type << endl;
        {
            size_t pos = type.find("application/json");
            if (pos != type.npos)
            { //�ҵ����Ӵ�"application/json"����json
                json = JSON(body);
                cout << "JSON:\n"
                     << json << endl;
            }
        }
        {
            size_t pos = type.find("multipart/form-data");
            if (pos != type.npos)
            { //�ҵ����Ӵ�"multipart/form-data"����form-data
            }
        }
    }
}
