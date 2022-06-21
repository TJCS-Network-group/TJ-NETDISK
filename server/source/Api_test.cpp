// API函数统一以<method>_<route>命名
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
using namespace std;

/*
    //一个最简单的请求报文实现
    string send_json =
        "{\
\"data\": {}, \
\"message\": \"请求格式错误\", \
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

// API的具体实现,这里直接演示一手调用tool的make_response_json函数
HttpResponse GET_test(HttpRequest& req) { return make_response_json(200, "sucess", "{\"test\":\"hello\"}"); }
HttpResponse POST_test(HttpRequest& req) { return make_response_json(400); }
HttpResponse PUT_test(HttpRequest& req) { return make_response_json(404); }
HttpResponse DELETE_test(HttpRequest& req) { return make_response_json(403); }
HttpResponse POST_test_gzy(HttpRequest& req) { return make_response_json(); }