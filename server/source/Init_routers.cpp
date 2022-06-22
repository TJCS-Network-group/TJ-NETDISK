#include "../include/HttpServer.h"

//初始化路由表
void Routers::Init_routers() {
    setRouter("GET", "/test", GET_test);
    setRouter("POST", "/test", POST_test);
    setRouter("DELETE", "/test", DELETE_test);
    setRouter("PUT", "/test", PUT_test);
    setRouter("POST", "/test_gzy", POST_test_gzy);
}
