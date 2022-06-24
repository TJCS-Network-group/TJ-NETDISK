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
    string message;
    int root_id = get_root_id_by_user(req.current_user_id, message);
    if (root_id < 0)
    {
        return make_response_json(-root_id, message);
    }
    string data = "{\"root_id\":" + to_string(root_id) + '}';
    return make_response_json(200, "查询结果如下", data);
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
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    my_database p;
    p.connect();
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
    int check = is_child(current_root_id, parent_id, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }

    if (!check)
    {
        return make_response_json(400, "无法操作他人的文件夹");
    }
    if (current_root_id == did)
    {
        return make_response_json(401, "用户无权修改根目录名");
    }
    sprintf(p.sql, "select dname from DirectoryEntity where parent_id=%d and id!=%d and id!=%d",
            parent_id, parent_id, did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    set<string> names;
    for (size_t i = 0; i < p.result_vector.size(); i++)
    {
        names.insert(p.result_vector[i]["dname"]);
    }
    int num = 0;
    string fin_name = new_name;
    while (true)
    {
        if (names.find(fin_name) != names.end())
        {
            num += 1;
            fin_name = new_name + '_' + to_string(num);
        }
        else
            break;
    }
    if (dname == fin_name)
    {
        return make_response_json(200, "新文件夹名和原文件名相同或和其他文件夹存在冲突,修改被取消");
    }
    sprintf(p.sql, "update DirectoryEntity set dname=\"%s\" where id=%d", fin_name.c_str(), did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.disconnect();
    set<string>().swap(names);
    message = "新文件夹名为" + fin_name;
    return make_response_json(200, message);
}

HttpResponse POST_filesystem_create_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("pid") == data.end() || data.find("dname") == data.end())
    {
        return make_response_json(400, "请求格式不对");
    }
    int pid = data["pid"].as_int();
    string dname = data["dname"].as_string();
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    int check = is_child(pid, current_root_id, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }
    if (!check && pid != current_root_id)
    {
        return make_response_json(400, "不可在别人的文件夹下新建文件夹");
    }
    my_database p;
    p.connect();
    sprintf(p.sql, "select dname from DirectoryEntity where parent_id=%d and id!=%d", pid, pid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员检查");
    }
    p.get();
    set<string> names;
    for (size_t i = 0; i < p.result_vector.size(); i++)
    {
        names.insert(p.result_vector[i]["dname"]);
    }
    int num = 0;
    string fin_name = dname;
    while (true)
    {
        if (names.find(fin_name) != names.end())
        {
            num += 1;
            fin_name = dname + '_' + to_string(num);
        }
        else
            break;
    }
    sprintf(p.sql, "insert into DirectoryEntity(dname,parent_id) value (\"%s\",%d)", fin_name.c_str(), pid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库插入出错,请联系管理员检查");
    }
    set<string>().swap(names);
    p.disconnect();
    message = "新建文件夹成功,新文件夹名为" + fin_name;
    return make_response_json(200, message);
}
HttpResponse POST_share_move_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("pid") == data.end() || data.find("did") == data.end())
    {
        return make_response_json(400, "请求格式不对");
    }
    int pid = data["pid"].as_int();
    int did = data["did"].as_int();
    if (pid == did)
    {
        return make_response_json(400, "不可将文件夹移动到自己内");
    }
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    if (did == current_root_id)
    {
        return make_response_json(400, "不可移动根文件夹");
    }
    int d_check = is_child(did, current_root_id, message);
    if (d_check < 0)
    {
        return make_response_json(-d_check, message);
    }
    if (!d_check && did != current_root_id)
    {
        return make_response_json(400, "不可移动他人的文件夹");
    }
    int p_check = is_child(pid, current_root_id, message);
    if (p_check < 0)
    {
        return make_response_json(-p_check, message);
    }
    if (!p_check && pid != current_root_id)
    {
        return make_response_json(400, "不可移动至他人的文件夹");
    }
    int check = is_child(pid, did, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }
    else if (check == 1)
    {
        return make_response_json(400, "不可将文件夹移动到其子文件夹下");
    }
    my_database p;
    p.connect();
    sprintf(p.sql, "select dname,parent_id from DirectoryEntity where id=%d", did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员检查");
    }
    p.get();
    int parent_id = atoi(p.result_vector[0]["parent_id"].c_str());
    if (parent_id == pid)
    {
        return make_response_json(200, "文件夹无需移动");
    }
    string dname = p.result_vector[0]["dname"];
    sprintf(p.sql, "select dname from DirectoryEntity where parent_id=%d and id!=%d", pid, pid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员检查");
    }
    p.get();
    set<string> names;
    for (size_t i = 0; i < p.result_vector.size(); i++)
    {
        names.insert(p.result_vector[i]["dname"]);
    }
    int num = 0;
    string fin_name = dname;
    while (true)
    {
        if (names.find(fin_name) != names.end())
        {
            num += 1;
            fin_name = dname + '_' + to_string(num);
        }
        else
            break;
    }
    sprintf(p.sql, "update DirectoryEntity set parent_id=%d,dname=\"%s\" where id=%d", pid, fin_name.c_str(), did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库更新出错,请联系系统管理员");
    }
    p.disconnect();
    message = "移动文件夹成功,新文件夹名为" + fin_name;
    return make_response_json(200, message);
}