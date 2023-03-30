//ά��·�ɱ�
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
//����·��
#include <iostream>
using namespace std;
void Routers::setRouter(const std::string method, const std::string route, HttpResponse func(HttpRequest &req))
{
    std::pair<std::string, std::string> teppair = {method, route};
    // cout << teppair.first << endl;
    // cout << teppair.second << endl;
    routers[teppair] = func;
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

    HttpResponse resp; //����
    resp.setHeader("Access-Control-Allow-Origin: http://" + IP);
    resp.setHeader("Access-Control-Allow-Headers: X-Requested-With,Content-Type");
    resp.setHeader("Access-Control-Allow-Methods: PUT,POST,GET,DELETE,OPTIONS");
    resp.setHeader("Access-Control-Allow-Credentials: true");
    resp.setHeader("Content-Type: application/json;charset=GBK"); // json&gbk
    resp.setBody(send_json);
    resp.setHeader("Content-Length: " + to_string(send_json.length())); //����
    return resp;
}

//��·�ɱ���������Ӧ
HttpResponse Routers::getResponse(HttpRequest &req) const
{
    for (auto iter : routers) //·�ɱ�������ע��·��
    {

        if (req.method == iter.first.first && req.route.find(iter.first.second) != req.route.npos)
        {
            return iter.second(req); //����request������response
        }
    }
    return make_response_json(404, "·�ɱ���δ�ҵ���·��"); //û�ҵ�
}