#include "../include/HttpServer.h"

//初始化路由表
void Routers::Init_routers()
{
    setRouter("POST", "/api/login", POST_login);
    setRouter("GET", "/api//logout", GET_logout);
    setRouter("POST", "/api//register", POST_register);
}
