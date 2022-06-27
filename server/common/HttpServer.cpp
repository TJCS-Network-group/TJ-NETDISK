//维护路由表
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
//设置路由
#include <iostream>
using namespace std;
void Routers::setRouter(const std::string method, const std::string route, HttpResponse func(HttpRequest &req))
{
    std::pair<std::string, std::string> teppair = {method, route};
    // cout << teppair.first << endl;
    // cout << teppair.second << endl;
    routers[teppair] = func;
}

//标准API接口下的传输方法, 这里data直接用string存了
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
    //要发送的JSON
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
    resp.setHeader("Content-Length: " + to_string(send_json.length())); //长度
    return resp;
}

//找路由表，返回响应
HttpResponse Routers::getResponse(HttpRequest &req)
{
    for (auto iter : routers) //路由表里找已注册路由
    {
        // cout << req.method << endl;
        // cout << iter.first.first << endl;
        // cout << req.route << endl;
        // cout << iter.first.second << endl;
        if (req.method == iter.first.first && req.route.find(iter.first.second) != req.route.npos)
        {
            return iter.second(req); //传入request；传出response
        }
    }
    //没找到
    return make_response_json(404);
}
