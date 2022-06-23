#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include "../include/HttpServer.h"
using namespace std;

//�����ļ�ת��string
string FileToStr(string filepath)
{
    fstream f(filepath, ios::in | ios::binary);
    if (f.good())
    {
        f.unsetf(ios::skipws); // �ر�inputFile�ĺ��Կո��־,�����ļ��еı����ո�
        istream_iterator<char> iter(f);
        string s(iter, istream_iterator<char>());
        return s;
    }
    else
    {
        cout << "Can't open file!Srcfile path error!" << endl;
        exit(EXIT_FAILURE);
    }
}

//ת��form-data��ʽ
string MapToString(map<string, string> data)
{
    string res = "";
    map<string, string>::iterator iter;
    for (iter = data.begin(); iter != data.end(); ++iter)
    {
        res += iter->first + "=" + iter->second + "&";
    }
    res.erase(res.end() - 1);
    return res;
}

// boolתstring
string bool_to_string(bool _success)
{
    if (_success)
        return "true";
    else
        return "false";
}

//��׼API�ӿ��µĴ��䷽��, ����dataֱ����string����
HttpResponse make_response_json(int _statusCode, string _message, string _data, bool _success)
{
    if (_statusCode / 100 == 2)
    {
        _success = true;
        if (_message == "")
            _message = "success";
    }
    else
    {
        _success = false;
        if (_message == "")
            _message = "fail";
    }
    //Ҫ���͵�JSON
    string send_json =
        "{\
\"data\": " +
        _data +
        ", \
\"message\": \"" +
        _message +
        "\", \
\"statusCode\": " +
        to_string(_statusCode) +
        ", \
\"success\": " +
        bool_to_string(_success) +
        "}\
";

    HttpResponse resp;
    resp.setHeader("Content-Type: application/json;charset=GBK"); // json&gbk
    resp.setBody(send_json);
    resp.setHeader("Content-Length: " + to_string(send_json.length())); //����
    return resp;
}
