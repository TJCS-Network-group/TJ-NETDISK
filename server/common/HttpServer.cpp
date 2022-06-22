//ά��·�ɱ�
#include "../include/HttpServer.h"

#include "../include/HttpTool.h"
//����·��
#include <iostream>
using namespace std;
void Routers::setRouter(const std::string method, const std::string route, HttpResponse func(HttpRequest& req)) {
    std::pair<std::string, std::string> teppair = {method, route};
    // cout << teppair.first << endl;
    // cout << teppair.second << endl;
    routers[teppair] = func;
}

//��·�ɱ�������Ӧ
HttpResponse Routers::getResponse(HttpRequest& req) {
    for (auto iter : routers) {  //·�ɱ�������ע��·��
        // cout << req.method << endl;
        // cout << iter.first.first << endl;
        // cout << req.route << endl;
        // cout << iter.first.second << endl;
        if (req.method == iter.first.first && req.route == iter.first.second) {
            return iter.second(req);  //����request������response
        }
    }
    //û�ҵ�
    return make_response_json(404);
}
