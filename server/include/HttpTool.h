//各种工具函数
#pragma once
#include "HttpServer.h"
using namespace std;
HttpResponse make_response_json(int _statusCode = 200, string _message = "", string _data = "{}", bool _success = true);
string FileToStr(string filepath);