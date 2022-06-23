#include "../include/HttpServer.h"

//初始化路由表
void Routers::Init_routers()
{
    setRouter("POST", "/api/login", POST_login);
    setRouter("GET", "/api/logout", GET_logout);
    setRouter("POST", "/api/register", POST_register);
    setRouter("GET", "/api/filesystem/get_dir", GET_filesystem_get_dir);
    setRouter("GET", "/api/get_root_id", GET_get_root_id);
    setRouter("GET", "/api/file_exist", GET_file_exist);
    // setRouter("GET","/api/file_fragment_exist",GET_file_fragment);
}
