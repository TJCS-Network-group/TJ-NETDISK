// API函数统一以<method>_<route>命名
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
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
        return make_response_json(400, "请求格式不对");
    }
    cout << file_id << index << endl;
    my_database p;
    p.connect();
    sprintf(p.sql, "select FileFragmentEntity.MD5 as MD5 from FileFragmentMap \
    join FileFragmentEntity on FileFragmentEntity.id=FileFragmentMap.fgid \
    where FileFragmentMap.fid=%d and FileFragmentMap.`index`=%d",
            file_id, index);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(404, "该文件碎块不存在");
    }
    else if (p.result_vector.size() > 1)
    {
        return make_response_json(500, "数据库查询出错");
    }
    string file_path = "../pool/" + p.result_vector[0]["MD5"];
    // string file_path = "./request_file/sjk.png"; //碎片的地址
    string file_value;

    if (FileToStr(file_path, file_value) == -1)
        return make_response_json(500, "pool中找不到文件");
    HttpResponse resp;
    resp.setHeader("Content-Type: application/octet-stream");            //直接传输二进制
    resp.setHeader("Content-Length: " + to_string(file_value.length())); //长度
    resp.setBody(file_value);
    return resp;
}
