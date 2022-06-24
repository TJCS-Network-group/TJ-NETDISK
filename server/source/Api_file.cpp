#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
#include <set>
using namespace std;
HttpResponse GET_file_exist(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    if (req.params.find("md5") == req.params.end())
    {
        return make_response_json(400, "请求格式错误");
    }
    string md5 = req.params["md5"];
    my_database p;
    p.connect();
    sprintf(p.sql, "select * from FileEntity where md5=\"%s\"", md5.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    string result = "{\"is_exist\":";
    if (p.result_vector.size() == 0)
    {
        result += "false";
    }
    else if (p.result_vector.size() == 1)
    {
        result += "true";
    }
    else
    {
        return make_response_json(500, "数据库中文件md5相撞,请联系管理员进行处理");
    }
    result += '}';
    return make_response_json(200, "查询结果如下", result);
}
HttpResponse GET_file_fragment(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    if (req.params.find("md5") == req.params.end())
    {
        return make_response_json(400, "请求格式错误");
    }
    string md5 = req.params["md5"];
    my_database p;
    p.connect();
    sprintf(p.sql, "select * from FileFragmentEntity where md5=\"%s\"", md5.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    string result = "{\"is_exist\":";
    if (p.result_vector.size() == 0)
    {
        result += "false";
    }
    else if (p.result_vector.size() == 1)
    {
        result += "true";
    }
    else
    {
        return make_response_json(500, "数据库中文件块md5相撞,请联系管理员进行处理");
    }
    result += '}';
    return make_response_json(200, "查询结果如下", result);
}
HttpResponse PUT_filesystem_rename_file(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("fid") == data.end() || data.find("fname") == data.end())
    {
        return make_response_json(400, "请求格式不对");
    }
    string new_name = data["fname"].as_string();
    int id = data["fid"].as_int();
    my_database p;
    p.connect();
    sprintf(p.sql, "select did,fname from FileDirectoryMap where id=%d", id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(400, "正在修改不存在的文件");
    }
    int did = atoi(p.result_vector[0]["did"].c_str());
    string fname = p.result_vector[0]["fname"];
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    int parent = get_root_id_by_did(did, message);
    if (parent < 0)
    {
        return make_response_json(-parent, message);
    }
    if (parent != current_root_id)
    {
        return make_response_json(400, "无法操作他人的文件");
    }
    sprintf(p.sql, "select fname from FileDirectoryMap where did=%d and id!=%d",
            did, id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    set<string> names;
    for (size_t i = 0; i < p.result_vector.size(); i++)
    {
        names.insert(p.result_vector[i]["fname"]);
    }
    int num = 0;
    string fin_name = new_name;
    size_t x = new_name.find_last_of(".");
    while (true)
    {
        if (names.find(fin_name) != names.end())
        {
            num += 1;
            fin_name = new_name;
            if (x == string::npos)
            {
                fin_name += '_' + to_string(num);
            }
            else
            {
                fin_name.insert(x, "_" + to_string(num));
            }
        }
        else
        {
            break;
        }
    }
    if (fin_name == fname)
    {
        return make_response_json(200, "新文件夹名和原文件名相同或和其他文件夹存在冲突,修改被取消");
    }
    sprintf(p.sql, "update FileDirectoryMap set fname=\"%s\" where id=%d",
            fin_name.c_str(), id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.disconnect();
    set<string>().swap(names);
    message = "文件改名成功,新名为" + fin_name;
    return make_response_json(200, message);
}
HttpResponse GET_upload_allocation(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    if (req.params.find("md5") == req.params.end())
    {
        return make_response_json(400, "请求格式错误");
    }
    string md5 = req.params["md5"];
    string data;
    // check(md5);
    my_database p;
    p.connect();
    sprintf(p.sql, "select fsize,next_index,is_complete from FileEntity where md5=\"%s\"", md5.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        if (req.params.find("size") == req.params.end())
        {
            return make_response_json(400, "无文件大小无法新建项目");
        }
        int fsize = atoi(req.params["size"].c_str());
        if (fsize < 0)
        {
            return make_response_json(400, "不存在负数大小的文件");
        }
        sprintf(p.sql, "insert into FileEntity(MD5,fsize,link_num,next_index,is_complete)\
         value (\"%s\",%d,%d,%d,%d)",
                md5.c_str(), fsize, 1, 0, 0);
        if (p.execute() == -1)
        {
            return make_response_json(500, "数据库查询新建,请联系管理员解决问题");
        }
        data += "{\"next_index\":0}";
        return make_response_json(200, "需要的下一块为", data);
    }
    else if (p.result_vector.size() == 1)
    {
        if (req.params.find("size") != req.params.end())
        {
            if (atoi(req.params["size"].c_str()) != atoi(p.result_vector[0]["fsize"].c_str()))
            {
                return make_response_json(400, "文件大小指定错误");
            }
        }
        if (atoi(p.result_vector[0]["is_complete"].c_str()) == 0)
        {
            // check()
            data += "{\"next_index\":" + p.result_vector[0]["next_index"] + '}';
            int max_index = atoi(p.result_vector[0]["fsize"].c_str()) / FRAGMENT_SIZE + 1;
            int now_index = atoi(p.result_vector[0]["next_index"].c_str());
            int next_index = -1;
            set<int> indexs;
            for (int i = 0; i < max_index; i++)
            {
                indexs.insert(i);
            }
            sprintf(p.sql, "select `index` from FileFragmentMap \
            join FileEntity on FileEntity.id=FileFragmentMap.fid where FileEntity.md5=\"%s\" and `index` > %d",
                    md5.c_str());
            if (p.execute() == -1)
            {
                return make_response_json(500, "数据库查询失败,请联系管理员解决问题");
            }
            p.get();
            for (size_t i = 0; i < p.result_vector.size(); i++)
            {
                indexs.erase(atoi(p.result_vector[i]["index"].c_str()));
            }

            for (auto it = indexs.begin(); it != indexs.end(); it++)
            {
                if (*it > now_index)
                {
                    next_index = *it;
                    break;
                }
            }
            if (next_index == -1)
            {
                next_index = *indexs.begin();
            }
            sprintf(p.sql, "update FileEntity set next_index=%d where md5=\"%s\"", next_index, md5.c_str());
            if (p.execute() == -1)
            {
                return make_response_json(500, "数据库更新出错,请联系系统管理员");
            }
        }
        else
        {
            data += "{\"next_index\":-1}";
            return make_response_json(200, "需要的下一块为", data);
        }
    }
    p.disconnect();
    return make_response_json(200, "需要的下一块为", data);
}
HttpResponse POST_share_move_file(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("fdid") == data.end() || data.find("pid") == data.end())
    {
        return make_response_json(400, "请求格式不对");
    }
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    int fdid = data["fdid"].as_int();
    int pid = data["pid"].as_int();
    my_database p;
    p.connect();
    sprintf(p.sql, "select did,fname from FileDirectoryMap where id=%d", fdid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(400, "移动了不存在的文件");
    }
    int f_in_did = atoi(p.result_vector[0]["did"].c_str());
    string fname = p.result_vector[0]["fname"];
    int f_root_id = get_root_id_by_did(f_in_did, message);
    if (f_root_id < 0)
    {
        return make_response_json(-f_root_id, message);
    }
    if (f_root_id != current_root_id)
    {
        return make_response_json(400, "不可移动他人的文件");
    }
    int d_root_id = get_root_id_by_did(pid, message);
    if (d_root_id < 0)
    {
        return make_response_json(-d_root_id, message);
    }
    if (d_root_id != current_root_id)
    {
        return make_response_json(400, "不可移动到他人的文件夹");
    }
    if (pid == f_in_did)
    {
        return make_response_json(200, "文件无需移动");
    }
    set<string> names;
    sprintf(p.sql, "select fname from FileDirectoryMap where did=%d", pid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错");
    }
    p.get();
    for (size_t i = 0; i < p.result_vector.size(); i++)
    {
        names.insert(p.result_vector[i]["fname"]);
    }
    int num = 0;
    string fin_name = fname;
    size_t x = fname.find_last_of(".");
    while (true)
    {
        if (names.find(fin_name) != names.end())
        {
            num += 1;
            fin_name = fname;
            if (x == string::npos)
            {
                fin_name += '_' + to_string(num);
            }
            else
            {
                fin_name.insert(x, "_" + to_string(num));
            }
        }
        else
        {
            break;
        }
    }
    sprintf(p.sql, "update FileDirectoryMap set did=%d,fname=\"%s\" where id=%d", pid, fin_name.c_str(), fdid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库修改失败,请联系管理员");
    }
    p.disconnect();
    message = "移动成功,新文件名为" + fin_name;
    return make_response_json(200, message);
}
HttpResponse POST_share_copy_file(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "当前用户未登录");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("fdid") == data.end() || data.find("pid") == data.end())
    {
        return make_response_json(400, "请求格式不对");
    }
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    int fdid = data["fdid"].as_int();
    int pid = data["pid"].as_int();
    my_database p;
    p.connect();
    sprintf(p.sql, "select fid,did,fname from FileDirectoryMap where id=%d", fdid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(400, "移动了不存在的文件");
    }
    int fid = atoi(p.result_vector[0]["fid"].c_str());
    int f_in_did = atoi(p.result_vector[0]["did"].c_str());
    string fname = p.result_vector[0]["fname"];
    int f_root_id = get_root_id_by_did(f_in_did, message);
    if (f_root_id < 0)
    {
        return make_response_json(-f_root_id, message);
    }
    if (f_root_id != current_root_id)
    {
        return make_response_json(400, "不可移动他人的文件");
    }
    int d_root_id = get_root_id_by_did(pid, message);
    if (d_root_id < 0)
    {
        return make_response_json(-d_root_id, message);
    }
    if (d_root_id != current_root_id)
    {
        return make_response_json(400, "不可移动到他人的文件夹");
    }
    set<string> names;
    sprintf(p.sql, "select fname from FileDirectoryMap where did=%d", pid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错");
    }
    p.get();
    for (size_t i = 0; i < p.result_vector.size(); i++)
    {
        names.insert(p.result_vector[i]["fname"]);
    }
    int num = 0;
    string fin_name = fname;
    size_t x = fname.find_last_of(".");
    while (true)
    {
        if (names.find(fin_name) != names.end())
        {
            num += 1;
            fin_name = fname;
            if (x == string::npos)
            {
                fin_name += '_' + to_string(num);
            }
            else
            {
                fin_name.insert(x, "_" + to_string(num));
            }
        }
        else
        {
            break;
        }
    }
    sprintf(p.sql, "insert into FileDirectoryMap(fid,did,fname) value (%d,%d,\"%s\");", fid, pid, fin_name.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库新增失败,请联系管理员");
    }
    //应该开启事务？或者是数据库写触发器？
    sprintf(p.sql, "update FileEntity set link_num=link_num+1 where id=%d", fid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库修改失败,请联系管理员");
    }
    p.disconnect();
    message = "复制成功,新文件名为" + fin_name;
    return make_response_json(200, message);
}