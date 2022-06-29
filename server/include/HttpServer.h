#pragma once
#include <map>
#include <string>
#include <utility> //pair

#include "HttpRequest.h"
#include "HttpResponse.h"

//维护路由表, 一对method和route对应着一个函数
class Routers
{
    //路由表
    std::map<std::pair<std::string, std::string>, HttpResponse (*)(HttpRequest &req)> routers;

public:
    //设置路由
    void setRouter(const std::string method, const std::string route, HttpResponse func(HttpRequest &req));
    //找路由表，返回响应
    HttpResponse getResponse(HttpRequest &req) const;
    Routers &operator=(const Routers &t)
    {
        if (this != &t)
            routers = t.routers;
        return *this;
    }
    //初始化路由表
    void Init_routers();
};
HttpResponse make_response_json(int _statusCode = 200, std::string _message = "", std::string _data = "{}", bool _success = true);

//要实现的API函数
HttpResponse POST_login(HttpRequest &req);
HttpResponse POST_register(HttpRequest &req);
HttpResponse GET_logout(HttpRequest &req);
HttpResponse GET_filesystem_get_dir(HttpRequest &req);
HttpResponse GET_get_root_id(HttpRequest &req);
HttpResponse GET_file_exist(HttpRequest &req);
HttpResponse GET_file_fragment(HttpRequest &req);
HttpResponse PUT_filesystem_rename_file(HttpRequest &req);
HttpResponse PUT_filesystem_rename_dir(HttpRequest &req);
HttpResponse GET_upload_allocation(HttpRequest &req);
HttpResponse GET_download_fragment(HttpRequest &req);
HttpResponse POST_filesystem_create_dir(HttpRequest &req);
HttpResponse POST_upload_fragment(HttpRequest &req);
HttpResponse POST_upload_file(HttpRequest &req);
HttpResponse POST_share_move_file(HttpRequest &req);
HttpResponse POST_share_copy_file(HttpRequest &req);
HttpResponse POST_share_move_dir(HttpRequest &req);
HttpResponse POST_share_copy_dir(HttpRequest &req);
HttpResponse DEL_remove_file(HttpRequest &req);
HttpResponse DEL_remove_dir(HttpRequest &req);
HttpResponse GET_share_get_dir_tree(HttpRequest &req);
HttpResponse GET_filesystem_get_tree(HttpRequest &req);
HttpResponse GET_myinfo(HttpRequest &req);
HttpResponse GET_get_identity(HttpRequest &req);
