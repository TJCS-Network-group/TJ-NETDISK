// API����ͳһ��<method>_<route>����
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include <exception>

using namespace std;

HttpResponse GET_download_fragment(HttpRequest &req)
{
    int file_id;
    int index;
    try
    {
        file_id = atoi(req.params["file_id"].c_str());
        index = atoi(req.params["index"].c_str());
    }
    catch (exception e)
    {
        return make_response_json(400, "�����ʽ����");
    }
    cout << file_id << index << endl;
    string file_path = "./request_file/sjk.png"; //��Ƭ�ĵ�ַ
    string file_value = FileToStr(file_path);
    HttpResponse resp;
    resp.setHeader("Content-Type: application/octet-stream");
    resp.setHeader("Content-Length: " + to_string(file_value.length())); //����
    resp.setBody(file_value);
    return resp;
}
