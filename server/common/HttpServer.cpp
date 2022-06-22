//维护路由表
#include "../include/HttpServer.h"

#include "../include/HttpTool.h"
//设置路由
#include <iostream>
using namespace std;
void Routers::setRouter(const std::string method, const std::string route, HttpResponse func(HttpRequest& req)) {
    std::pair<std::string, std::string> teppair = {method, route};
    // cout << teppair.first << endl;
    // cout << teppair.second << endl;
    routers[teppair] = func;
}

//找路由表，返回响应
HttpResponse Routers::getResponse(HttpRequest& req) {
    for (auto iter : routers) {  //路由表里找已注册路由
        // cout << req.method << endl;
        // cout << iter.first.first << endl;
        // cout << req.route << endl;
        // cout << iter.first.second << endl;
        if (req.method == iter.first.first && req.route == iter.first.second) {
            return iter.second(req);  //传入request；传出response
        }
    }
    //没找到
    return make_response_json(404);
}
