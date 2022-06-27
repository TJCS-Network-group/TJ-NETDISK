//解析client端传入的HTTP请求
#include "../include/HttpRequest.h"
#include "../include/HttpTool.h"
#include <cerrno>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
using namespace std;

const int TIME_OUT = 5; // 5s的TIME_OUT时间

// 解析请求报文
// resouce以'/'开头
// Method暂时只用这四种：GET POST PUT DELETE
// params只有当method为GET的时候才有值，body只解析JSON和from-data就行

std::map<int, std::string> session; // user_id : cookie的md5部分

void HttpRequest::Parse_request_header(const string _raw_http_request)
{
    bool flag = true;
    time_t past_time;
    time(&past_time);
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
    while (flag)
    {
        time_t cur_time;
        time(&cur_time);
        if (cur_time - past_time >= TIME_OUT)
        {
            disconnect = true;
            flag = false;
            return;
        }

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

    while (flag)
    {
        time_t cur_time;
        time(&cur_time);
        if (cur_time - past_time >= TIME_OUT)
        {
            // disconnect = true;
            flag = false;
            // return;//没有header也不是不行
        }

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
    }*/
    // cout << "SESSION: " << endl;
    // for (auto i : session)
    //{
    //     cout << i.first << ": " << i.second << endl;
    // }
    if (headers.count("Cookie") != 0)
    { //有cookie
        string cookie = headers["Cookie"];
        string current_user_str;
        // cookie结构： sessionid=1951705|{md5};
        string sub_str = "sessionid=";
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
            string md5 = cookie.substr(cur + 1, 32); //我们用MD5码 默认为32字节长度
            // cout << md5 << endl;
            current_user_id = atoi(current_user_str.c_str()); // current_user_id为0代表没登录

            if (session.count(current_user_id) == 0 || md5 != session[current_user_id])
            { //与session中的表项对比
                cout << "cookie错误！" << endl;
                current_user_id = 0;
            }
        }
    }
    // cout << "current_user_id:" << current_user_id << endl;
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
    bool flag = true;
    time_t past_time;
    time(&past_time);
    if (headers.count("Content-Type") != 0)
    {
        string type = headers["Content-Type"];
        // cout << type << endl;
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
                    while (flag)
                    {
                        time_t cur_time;
                        time(&cur_time);
                        if (cur_time - past_time >= TIME_OUT)
                        {
                            disconnect = true;
                            flag = false;
                            return; //没有body也不是不行
                        }

                        pos = body.find(boundary, next_pos); //一般来说一定能找到
                        if (pos == body.npos)
                            break;
                        pos += boundary.length() + 2;        //跳过\r\n
                        next_pos = body.find(boundary, pos); //下一个
                        if (next_pos == body.npos)
                            break;
                        string content = body.substr(pos, next_pos - pos);
                        // cout << content << endl;
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

const int BUFFER_SIZE = 20000;
char buf[BUFFER_SIZE]; //接收传过来的http request请求，暂时用2万字节存，可能不够大，要注意一下

//不停的收 直到断开连接或者收到完整HTTP报文 此后应该设置超时退出条件
HttpRequest http_recv_request(int sockfd)
{ //传入accept的socketfd
    memset(buf, 0, BUFFER_SIZE);
    int len = recv(sockfd, buf, BUFFER_SIZE, 0); //第一次接受数据，把所有header找到

    string client_http_request(buf, len);
    // cout << client_http_request << endl;
    //创建一个HttpRequest对象解析第一次发过来的原报文（包含完整header, 但可能不包含完整body）
    HttpRequest new_request(client_http_request);
    time_t past_time;
    time(&past_time);
    while (new_request.Read_body_over() == false) //收到Content-Length没达到body值
    {

        time_t cur_time;
        time(&cur_time);
        if (cur_time - past_time >= TIME_OUT)
        {
            new_request.disconnect = true;
            return new_request; //直接不等之后的body了
        }

        memset(buf, 0, BUFFER_SIZE);
        int len = recv(sockfd, buf, BUFFER_SIZE, 0); //接受数据
        if (len < 0 && errno != EAGAIN)
        {
            cout << errno << endl
                 << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }
        else if (len == 0) // disconnect
        {
            break;
        }
        else if (len > 0)
        {
            //把buf数据copy到body中
            string tep_str(buf, len);
            new_request.Concat_body(tep_str);
        }
    }
    if (len != 0)
    {
        //创建一个HttpRequest对象解析原报文，需加入超时和try机制
        new_request.Parse_request_body(); //解析body
    }
    return new_request;
}
