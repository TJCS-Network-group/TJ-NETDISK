// Response类成员函数实现，用于向client返回HTTP响应报文
#include "../include/HttpResponse.h"
using namespace std;
const string HttpResponse::HTTP_VERSION = "HTTP/1.1";

void HttpResponse::setStatus(const string status) { header = HTTP_VERSION + " " + status + "\r\n"; }
void HttpResponse::setHeader(const string _header) { header += _header + "\r\n"; }
void HttpResponse::setBody(const string _body) { body += _body; }
