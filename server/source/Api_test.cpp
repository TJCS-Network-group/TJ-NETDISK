// API����ͳһ��<method>_<route>����
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
using namespace std;

/*
    //һ����򵥵�������ʵ��
    string send_json =
        "{\
\"data\": {}, \
\"message\": \"�����ʽ����\", \
\"statusCode\": 400, \
\"success\": false}\
";

    string send_str =
        "HTTP/1.1 200 OK\r\n\
Content-Type: application/json\r\n\
Content-Length: \
" + to_string(send_json.length()) +
        "\r\n\n";
    string resp=send_str+send_json;
*/

// API�ľ���ʵ��,����ֱ����ʾһ�ֵ���tool��make_response_json����
HttpResponse GET_test(HttpRequest& req) { return make_response_json(200, "sucess", "{\"test\":\"hello\"}"); }
HttpResponse POST_test(HttpRequest& req) { return make_response_json(400); }
HttpResponse PUT_test(HttpRequest& req) { return make_response_json(404); }
HttpResponse DELETE_test(HttpRequest& req) { return make_response_json(403); }
HttpResponse POST_test_gzy(HttpRequest& req) { return make_response_json(); }