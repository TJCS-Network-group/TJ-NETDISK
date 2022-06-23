#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
using namespace std;
HttpResponse GET_filesystem_get_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    int dir_id;
    if (req.params.find("dir_id") == req.params.end())
    {
        return make_response_json(400, "请求格式不对");
    }
    dir_id = atoi(req.params["dir_id"].c_str());
    my_database p;
    p.connect();
    sprintf(p.sql, "select FileDirectoryMap.id as id,FileDirectoryMap.fname as name,FileEntity.fsize as fsize\
     from FileDirectoryMap join FileEntity on FileEntity.id=FileDirectoryMap.fid where FileDirectoryMap.did=%d",
            dir_id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    map<string, string> d;
    vector<map<string, string>> data;
    for (size_t i = 0; i < p.result_vector.size(); i++)
    {
        map<string, string>().swap(d);
        d.insert(p.result_vector[i].begin(), p.result_vector[i].end());
        d["type"] = "0";
        data.push_back(d);
    }
    sprintf(p.sql, "select id,dname as name from DirectoryEntity where parent_id=%d and id!=%d", dir_id, dir_id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    for (size_t i = 0; i < p.result_vector.size(); i++)
    {
        map<string, string>().swap(d);
        d.insert(p.result_vector[i].begin(), p.result_vector[i].end());
        d["type"] = "1";
        // d["size"] = "0";
        data.push_back(d);
    }
    p.disconnect();
    string fin_data = "[";
    for (size_t i = 0; i < data.size(); i++)
    {
        fin_data += '{';
        for (auto j = data[i].begin(); j != data[i].end();)
        {
            fin_data += '\"' + j->first + "\":";
            if (j->first == "name")
            {
                fin_data += '\"' + j->second + '\"';
            }
            else
            {
                fin_data += j->second;
            }
            if (++j != data[i].end())
            {
                fin_data += ',';
            }
        }
        fin_data += '}';
        if (i < data.size() - 1)
        {
            fin_data += ',';
        }
    }
    fin_data += ']';
    p.disconnect();
    return make_response_json(200, "查询如下", fin_data);
}
HttpResponse GET_get_root_id(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    my_database p;
    p.connect();
    sprintf(p.sql, "select root_dir_id as root_id from UserEntity where id=%d", req.current_user_id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    string root_id = "{\"root_id\":" + p.result_vector[0]["root_id"] + '}';
    p.disconnect();
    return make_response_json(200, "查询结果如下", root_id);
}
HttpResponse PUT_filesystem_rename_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("did") == data.end() || data.find("dname") == data.end())
    {
        return make_response_json(400, "请求格式不对");
    }
    int did = data["did"].as_int();
    string new_name = data["dname"].as_string();
    my_database p;
    p.connect();
    sprintf(p.sql, "select root_dir_id from UserEntity where id=%d", req.current_user_id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    int current_root_id = atoi(p.result_vector[0]["root_dir_id"].c_str());
    sprintf(p.sql, "select parent_id,dname from DirectoryEntity where id=%d", did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(404, "尝试修改不存在的目录");
    }
    int parent_id = atoi(p.result_vector[0]["parent_id"].c_str());
    string dname = p.result_vector[0]["dname"];
    int child, parent;
    child = 0;
    parent = parent_id;
    while (child != parent)
    {
        child = parent;
        sprintf(p.sql, "select parent_id from DirectoryEntity where id=%d", child);
        if (p.execute() == -1)
        {
            return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
        }
        p.get();
        parent = atoi(p.result_vector[0]["parent_id"].c_str());
    }
    if (parent != current_root_id)
    {
        return make_response_json(400, "无法操作他人的文件");
    }
    if (parent_id == did)
    {
        return make_response_json(401, "用户无权修改根目录名");
    }
    if (dname == new_name)
    {
        return make_response_json(200, "原文件夹名无需修改");
    }
    sprintf(p.sql, "select * from DirectoryEntity where parent_id=%d and id!=%d and id!=%d and dname=\"%s\"",
            parent_id, parent_id, did, new_name.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    if (p.result_vector.size() != 0)
    {
        return make_response_json(400, "存在同名文件夹,修改失败");
    }
    sprintf(p.sql, "update DirectoryEntity set dname=\"%s\" where id=%d", new_name.c_str(), did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.disconnect();
    return make_response_json(200, "改名成功");
}