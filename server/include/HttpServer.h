#pragma once
#include <map>
#include <string>
#include <utility>  //pair

#include "HttpRequest.h"
#include "HttpResponse.h"

//ά��·�ɱ�, һ��method��route��Ӧ��һ������
class Routers {
    //·�ɱ�
    std::map<std::pair<std::string, std::string>, HttpResponse (*)(HttpRequest& req)> routers;

   public:
    //����·��
    void setRouter(const std::string method, const std::string route, HttpResponse func(HttpRequest& req));
    //��·�ɱ�������Ӧ
    HttpResponse getResponse(HttpRequest& req);

    //��ʼ��·�ɱ�
    void Init_routers();
};

//Ҫʵ�ֵ�API����
HttpResponse GET_test(HttpRequest& req);
HttpResponse POST_test_gzy(HttpRequest& req);
HttpResponse POST_test(HttpRequest& req);
HttpResponse GET_test(HttpRequest& req);
HttpResponse PUT_test(HttpRequest& req);
HttpResponse DELETE_test(HttpRequest& req);