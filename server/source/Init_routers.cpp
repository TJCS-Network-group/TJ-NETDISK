#include "../include/HttpServer.h"

//��ʼ��·�ɱ�
void Routers::Init_routers()
{
    setRouter("POST", "/api/login", POST_login);
    setRouter("GET", "/api//logout", GET_logout);
    setRouter("POST", "/api//register", POST_register);
}
