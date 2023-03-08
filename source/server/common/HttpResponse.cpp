// Response���Ա����ʵ�֣�������client����HTTP��Ӧ����
#include "../include/HttpResponse.h"
using namespace std;
const string HttpResponse::HTTP_VERSION = "HTTP/1.1";

void HttpResponse::setStatus(const string status) { header = HTTP_VERSION + " " + status + "\r\n"; }
void HttpResponse::setHeader(const string _header) { header += _header + "\r\n"; }
void HttpResponse::setBody(const string _body) { body += _body; }
