// API函数统一以<method>_<route>命名
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
#include <exception>

using namespace std;

HttpResponse GET_download_fragment(HttpRequest &req)
{
    int fdid;
    int index;
    if (req.current_user_id == 0)
    {
        return make_response_json(400, "当前用户未登录");
    }
    try
    {
        fdid = atoi(req.params["fdid"].c_str());
        index = atoi(req.params["index"].c_str());
    }
    catch (exception e)
    {
        return make_response_json(400, "请求格式不对");
    }
    my_database p;
    p.connect();
    int check, dir_id;
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    sprintf(p.sql, "select FileFragmentEntity.MD5 as MD5,FileDirectoryMap.did as did from FileFragmentMap\
    join FileFragmentEntity on FileFragmentEntity.id=FileFragmentMap.fgid \
    join FileDirectoryMap on FileDirectoryMap.fid=FileFragmentMap.fid \
    where FileDirectoryMap.id = %d and FileFragmentMap.`index`= %d ",
            fdid, index);
    // cout << p.sql << endl;
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错");
    }
    p.get();
    // p.show();
    // cout << p.result_vector.size() << endl;
    if (p.result_vector.size() == 0)
    {
        return make_response_json(404, "该文件碎块不存在");
    }
    else if (p.result_vector.size() > 1)
    {
        return make_response_json(500, "数据库查询出错");
    }
    dir_id = atoi(p.result_vector[0]["did"].c_str());
    check = is_child(dir_id, current_root_id, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }
    if (!check && dir_id != current_root_id)
    {
        return make_response_json(400, "用户无权下载他人文件碎片");
    }
    string file_path = "../pool/" + p.result_vector[0]["MD5"];
    // string file_path = "./request_file/sjk.png"; //碎片的地址
    string file_value;

    if (FileToStr(file_path, file_value) == -1)
        return make_response_json(500, "pool中找不到文件");
    HttpResponse resp;                                                   //跨域
    resp.setHeader("Content-Type: application/octet-stream");            //直接传输二进制
    resp.setHeader("Content-Length: " + to_string(file_value.length())); //长度
    resp.setHeader("Access-Control-Allow-Origin: http://" + IP);
    resp.setHeader("Access-Control-Allow-Headers: X-Requested-With,Content-Type");
    resp.setHeader("Access-Control-Allow-Methods: PUT,POST,GET,DELETE,OPTIONS");
    resp.setHeader("Access-Control-Allow-Credentials: true");
    resp.setBody(file_value);
    return resp;
}
