//解析client端传入的HTTP请求
#include "../include/HttpRequest.h"
#include "../include/HttpTool.h"
#include <fstream>
#include <iostream>
using namespace std;
// 解析请求报文
// resouce以'/'开头
// Method暂时只用这四种：GET POST PUT DELETE
// params只有当method为GET的时候才有值，body只解析JSON和from-data就行

std::map<int, std::string> session; // user_id : cookie的md5部分

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
    int lastcur = 0, cur = 0, cnt = 0;
    //解析第一行
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

                    string params_str = resource.substr(pos + 1); //之后的
                    if (Parse_params(params_str, params) == -1)
                    {
                        cerr << "popen fail.Params 未解析成功" << endl;
                        // exit(-1);
                    }
                }
                else
                {
                    route = resource;
                }
                cnt++;
            }
        }
        else if (_raw_http_request[cur] == '\n' && _raw_http_request[cur - 1] == '\r')
        { //换行
            http_version = _raw_http_request.substr(lastcur, cur - lastcur - 1);
            lastcur = cur + 1; //下一行起始
            ++cur;
            cnt = 0; //一行已经结束了
            break;   //这里只解析第一行
        }
        ++cur;
    }

    bool find_key = true;
    string key, value;

    while (true)
    {
        if (_raw_http_request[cur] == ':')
        {
            find_key = false; //开始找value
            ++cur;            //跳过冒号
            while (_raw_http_request[cur] == ' ')
                ++cur; //跳过空格
        }
        else if (_raw_http_request.length() > cur + 3)
        { //不越界
            if (_raw_http_request[cur] == '\r' && _raw_http_request[cur + 1] == '\n')
            {
                headers[key] = value;
                find_key = true; //开始下一轮找key

                //下两个是否还是\r\n
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

    cout << "PARAMS:" << endl;
    for (auto i : params)
    {
        cout << i.first << ':' << i.second << endl;
    }
    /*
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
    if (headers.count("Cookie") != 0)
    { //有cookie
        string cookie = headers["Cookie"];
        string current_user_str;
        // cookie结构： remember_token=1951705|{md5};
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
            current_user_id = atoi(current_user_str.c_str()); // current_user_id为0代表没登录

            if (session.count(current_user_id) == 0 || md5 != session[current_user_id])
            { //与session中的表项对比
                cout << "cookie错误！" << endl;
                current_user_id = 0;
            }
        }
    }
    cout << "current_user_id:" << current_user_id << endl;
}
int find_name_form_data(string &content, string &find_name, string &result)
{
    size_t find_name_pos = content.find(find_name);
    if (find_name_pos == content.npos)
        return -1;
    size_t name_pos_begin = find_name_pos + find_name.length();
    size_t name_pos_end = content.find("\"", name_pos_begin);
    if (name_pos_end == content.npos)
        return -1;
    result = content.substr(name_pos_begin, name_pos_end - name_pos_begin);
    return 0;
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
            { //找到了子串"application/json"，是json
                json = JSON(body);
                cout << "JSON:\n"
                     << json << endl;
            }
        }
        {
            size_t pos = type.find("multipart/form-data");
            if (pos != type.npos)
            { //找到了子串"multipart/form-data"，是form-data
                string boundary = "boundary=";
                pos = type.find(boundary);
                if (pos != type.npos)
                {
                    pos += boundary.size();
                    boundary = "--" + type.substr(pos);
                    // cout << boundary << endl;
                    size_t next_pos = 0;
                    string key, value, filename;
                    while (true)
                    {
                        pos = body.find(boundary, next_pos); //一般来说一定能找到
                        if (pos == body.npos)
                            break;
                        pos += boundary.length() + 2;        //跳过\r\n
                        next_pos = body.find(boundary, pos); //下一个
                        if (next_pos == body.npos)
                            break;
                        string content = body.substr(pos, next_pos - pos);
                        cout << content << endl;
                        size_t line_1_pos = content.find("\r\n");
                        string content_line_1 = content.substr(0, line_1_pos);

                        string find_filename = "filename=\"";
                        if (find_name_form_data(content_line_1, find_filename, filename) != -1) //是个文件
                        {
                            form_data["filename"] = To_gbk(filename);
                            string find_name = "name=\"";
                            if (find_name_form_data(content_line_1, find_name, key) == -1)
                                break;
                            for (size_t i = line_1_pos; i < content.size(); ++i)
                            {
                                if (content[i] == '\n' && content[i - 1] == '\r' && content[i - 2] == '\n' && content[i - 3] == '\r')
                                {
                                    size_t begin_pos = i + 1;
                                    size_t value_len = content.length() - begin_pos - 2; //最后还有一个\r\n
                                    string value = content.substr(begin_pos, value_len);
                                    form_data[To_gbk(key)] = value;
                                    /*
                                    cout << "文件大小: " << value.size() << endl;
                                    fstream myf_body("./request_file/" + filename, ios::out | ios::binary);
                                    if (myf_body.good())
                                    {
                                        myf_body << value;
                                    }
                                    else
                                    {
                                        cout << "Can't open file!" << endl;
                                    }
                                    myf_body.close();
                                    */
                                }
                            }
                        }
                        else
                        {
                            string find_name = "name=\"";
                            if (find_name_form_data(content_line_1, find_name, key) == -1)
                                break;
                            string value = content.substr(line_1_pos + 4, content.length() - (line_1_pos + 4) - 2); //开头两个\r\n 最后还有一个\r\n
                            form_data[To_gbk(key)] = To_gbk(value);
                        }
                    }
                }
            }
        }
        {
            size_t pos = type.find("application/x-www-form-urlencoded");
            if (pos != type.npos) //找到了子串"application/x-www-form-urlencoded"，是无文件的form
            {
                if (Parse_params(body, form_data) == -1)
                {
                    cerr << "popen fail.Params 未解析成功" << endl;
                    // exit(-1);
                }
            }
        }
    }
}
