#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
#include <stack>
using namespace std;
HttpResponse GET_filesystem_get_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    int dir_id;
    if (req.params.find("dir_id") == req.params.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    dir_id = atoi(req.params["dir_id"].c_str());
    my_database p;
    p.connect();
    sprintf(p.sql, "select FileDirectoryMap.id as id,FileDirectoryMap.fname as name,FileEntity.fsize as fsize\
     from FileDirectoryMap join FileEntity on FileEntity.id=FileDirectoryMap.fid where FileDirectoryMap.did=%d",
            dir_id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
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
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
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
    return make_response_json(200, "��ѯ����", fin_data);
}
HttpResponse GET_get_root_id(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    string message;
    int root_id = get_root_id_by_user(req.current_user_id, message);
    if (root_id < 0)
    {
        return make_response_json(-root_id, message);
    }
    string data = "{\"root_id\":" + to_string(root_id) + '}';
    return make_response_json(200, "��ѯ�������", data);
}
HttpResponse PUT_filesystem_rename_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("did") == data.end() || data.find("dname") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
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
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(404, "�����޸Ĳ����ڵ�Ŀ¼");
    }
    int parent_id = atoi(p.result_vector[0]["parent_id"].c_str());
    string dname = p.result_vector[0]["dname"];
    int check = is_child(parent_id, current_root_id, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }

    if (!check && current_root_id != parent_id)
    {
        return make_response_json(400, "�޷��������˵��ļ���");
    }
    if (current_root_id == did)
    {
        return make_response_json(401, "�û���Ȩ�޸ĸ�Ŀ¼��");
    }
    sprintf(p.sql, "select dname from DirectoryEntity where parent_id=%d and id!=%d and id!=%d",
            parent_id, parent_id, did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
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
        return make_response_json(200, "���ļ�������ԭ�ļ�����ͬ��������ļ��д��ڳ�ͻ,�޸ı�ȡ��");
    }
    if (dname == fin_name)
    {
        return make_response_json(200, "���ļ�������ԭ�ļ�����ͬ��������ļ��д��ڳ�ͻ,�޸ı�ȡ��");
    }
    sprintf(p.sql, "update DirectoryEntity set dname=\"%s\" where id=%d", fin_name.c_str(), did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.disconnect();
    message = "���ļ�����Ϊ" + fin_name;
    return make_response_json(200, message);
}

HttpResponse POST_filesystem_create_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("pid") == data.end() || data.find("dname") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
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
        return make_response_json(400, "�����ڱ��˵��ļ������½��ļ���");
    }
    my_database p;
    p.connect();
    sprintf(p.sql, "select dname from DirectoryEntity where parent_id=%d and id!=%d", pid, pid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա���");
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
        return make_response_json(500, "���ݿ�������,����ϵ����Ա���");
    }
    set<string>().swap(names);
    p.disconnect();
    message = "�½��ļ��гɹ�,���ļ�����Ϊ" + fin_name;
    return make_response_json(200, message);
}
HttpResponse POST_share_move_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("pid") == data.end() || data.find("did") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    int pid = data["pid"].as_int();
    int did = data["did"].as_int();
    if (pid == did)
    {
        return make_response_json(400, "���ɽ��ļ����ƶ����Լ���");
    }
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    if (did == current_root_id)
    {
        return make_response_json(400, "�����ƶ����ļ���");
    }
    int d_check = is_child(did, current_root_id, message);
    if (d_check < 0)
    {
        return make_response_json(-d_check, message);
    }
    if (!d_check && did != current_root_id)
    {
        return make_response_json(400, "�����ƶ����˵��ļ���");
    }
    int p_check = is_child(pid, current_root_id, message);
    if (p_check < 0)
    {
        return make_response_json(-p_check, message);
    }
    if (!p_check && pid != current_root_id)
    {
        return make_response_json(400, "�����ƶ������˵��ļ���");
    }
    int check = is_child(pid, did, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }
    else if (check == 1)
    {
        return make_response_json(400, "���ɽ��ļ����ƶ��������ļ�����");
    }
    my_database p;
    p.connect();
    sprintf(p.sql, "select dname,parent_id from DirectoryEntity where id=%d", did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա���");
    }
    p.get();
    int parent_id = atoi(p.result_vector[0]["parent_id"].c_str());
    if (parent_id == pid)
    {
        return make_response_json(200, "�ļ��������ƶ�");
    }
    int statusCode = 0;
    string fin_name = get_dir_new_name(did, pid, statusCode);
    if (statusCode > 0)
    {
        return make_response_json(statusCode, fin_name);
    }
    sprintf(p.sql, "update DirectoryEntity set parent_id=%d,dname=\"%s\" where id=%d", pid, fin_name.c_str(), did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ���³���,����ϵϵͳ����Ա");
    }
    p.disconnect();
    message = "�ƶ��ļ��гɹ�,���ļ�����Ϊ" + fin_name;
    return make_response_json(200, message);
}
HttpResponse POST_share_copy_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("pid") == data.end() || data.find("did") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    int pid = data["pid"].as_int();
    int did = data["did"].as_int();
    if (pid == did)
    {
        return make_response_json(400, "���ɽ��ļ��и��Ƶ��Լ���");
    }
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    if (did == current_root_id)
    {
        return make_response_json(400, "���ɸ��Ƹ��ļ���");
    }
    int d_check = is_child(did, current_root_id, message);
    if (d_check < 0)
    {
        return make_response_json(-d_check, message);
    }
    if (!d_check && did != current_root_id)
    {
        return make_response_json(400, "���ɸ������˵��ļ���");
    }
    int p_check = is_child(pid, current_root_id, message);
    if (p_check < 0)
    {
        return make_response_json(-p_check, message);
    }
    if (!p_check && pid != current_root_id)
    {
        return make_response_json(400, "���ɸ��������˵��ļ���");
    }
    int check = is_child(pid, did, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }
    else if (check == 1)
    {
        return make_response_json(400, "���ɽ��ļ��и��Ƶ������ļ�����");
    }
    int now, statusCode, fid;
    vector<int> floor, next;
    stack<int> dir_now, new_insert;
    stack<vector<int>> dir_tree;
    string fin_name;
    floor.push_back(did);
    new_insert.push(pid);
    dir_tree.push(floor);
    dir_now.push(0);
    vector<int>().swap(floor);
    my_database p;
    p.connect();
    while (!dir_now.empty())
    {
        now = dir_now.top();
        floor = dir_tree.top();
        if (now == floor.size())
        {
            dir_now.pop();
            dir_tree.pop();
            new_insert.pop();
        }
        else
        {
            dir_now.pop();
            dir_now.push(now + 1);
            fin_name = get_dir_new_name(floor[now], new_insert.top(), statusCode);
            if (statusCode > 0)
            {
                return make_response_json(statusCode, fin_name);
            }
            sprintf(p.sql, "insert into DirectoryEntity(dname,parent_id) value (\"%s\",%d)",
                    fin_name.c_str(), new_insert.top());
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ�������");
            }
            sprintf(p.sql, "select LAST_INSERT_ID() as id");
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ��ѯ����");
            }
            p.get();
            new_insert.push(atoi(p.result_vector[0]["id"].c_str()));
            sprintf(p.sql, "select id from DirectoryEntity where parent_id=%d and id!=%d", floor[now], floor[now]);
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ��ѯ����");
            }
            p.get();
            for (size_t i = 0; i < p.result_vector.size(); i++)
            {
                next.push_back(atoi(p.result_vector[i]["id"].c_str()));
            }
            dir_tree.push(next);
            vector<int>().swap(next);
            dir_now.push(0);
            sprintf(p.sql, "select fid,fname from FileDirectoryMap where did=%d", floor[now]);
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ��ѯ����");
            }
            p.get();
            for (size_t i = 0; i < p.result_vector.size(); i++)
            {
                fid = atoi(p.result_vector[i]["fid"].c_str());
                fin_name = p.result_vector[i]["fname"];
                sprintf(p.sql, "insert into FileDirectoryMap(fid,did,fname) value (%d,%d,\"%s\");",
                        fid, new_insert.top(), fin_name.c_str());
                if (p.execute() == -1)
                {
                    return make_response_json(500, "���ݿ�����ʧ��,����ϵ����Ա");
                }
                //Ӧ�ÿ������񣿻��������ݿ�д��������
                sprintf(p.sql, "update FileEntity set link_num=link_num+1 where id=%d", fid);
                if (p.execute() == -1)
                {
                    return make_response_json(500, "���ݿ��޸�ʧ��,����ϵ����Ա");
                }
            }
        }
    }
    return make_response_json(200, "�������");
}

HttpResponse DEL_remove_dir(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("dir_id") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    int did = data["dir_id"].as_int();
    if (did == current_root_id)
    {
        return make_response_json(400, "�û���Ȩɾ����Ŀ¼");
    }
    int check = is_child(did, current_root_id, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }
    if (!check)
    {
        return make_response_json(400, "�û���Ȩɾ������Ŀ¼");
    }
    my_database p;
    p.connect();
    set<int> all_del_dirs, now_del_dirs, next_del_dirs;
    all_del_dirs.insert(did);
    now_del_dirs.insert(did);
    while (!now_del_dirs.empty())
    {
        for (set<int>::iterator it = now_del_dirs.begin(); it != now_del_dirs.end(); it++)
        {
            sprintf(p.sql, "select id from DirectoryEntity where parent_id=%d and id!=%d", *it, *it);
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ��ѯ����");
            }
            p.get();
            for (size_t i = 0; i < p.result_vector.size(); i++)
            {
                next_del_dirs.insert(atoi(p.result_vector[i]["id"].c_str()));
            }
            sprintf(p.sql, "select id from FileDirectoryMap where did=%d", *it);
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ��ѯ����");
            }
            p.get();
            for (size_t i = 0; i < p.result_vector.size(); i++)
            {
                check = remove_file(atoi(p.result_vector[i]["id"].c_str()), current_root_id, true, message);
                if (check < 0)
                {
                    return make_response_json(-check, message);
                }
            }
        }
        all_del_dirs.insert(next_del_dirs.begin(), next_del_dirs.end());
        now_del_dirs.swap(next_del_dirs);
        set<int>().swap(next_del_dirs);
    }
    for (set<int>::iterator it = all_del_dirs.begin(); it != all_del_dirs.end(); it++)
    {
        sprintf(p.sql, "delete from DirectoryEntity where id=%d", *it);
        if (p.execute() == -1)
        {
            return make_response_json(500, "���ݿ�ɾ������");
        }
    }
    return make_response_json(200, "ɾ���ɹ�");
}
HttpResponse GET_share_get_dir_tree(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    cout << "root:" << current_root_id << endl;
    int now, statusCode, fid;
    vector<pair<int, string>> floor, next;
    stack<int> dir_now;
    stack<vector<pair<int, string>>> dir_tree;
    floor.push_back(make_pair(current_root_id, "root"));
    dir_tree.push(floor);
    dir_now.push(0);
    vector<pair<int, string>>().swap(floor);
    string data = "";
    my_database p;
    p.connect();
    while (!dir_now.empty())
    {
        now = dir_now.top();
        floor = dir_tree.top();
        if (now == floor.size())
        {
            dir_now.pop();
            dir_tree.pop();
            if (dir_now.size() > 0)
            {
                data += "]}";
            }
        }
        else
        {
            if (now > 0)
            {
                data += ',';
            }
            data += "{\"label\":\"" + floor[now].second + "\",\"did\":" + to_string(floor[now].first) + "\"isopen\":true" + ",\"children\":[";
            dir_now.pop();
            dir_now.push(now + 1);
            sprintf(p.sql, "select id,dname from DirectoryEntity where parent_id=%d and id!=%d", floor[now].first, floor[now].first);
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ��ѯ����");
            }
            p.get();
            for (size_t i = 0; i < p.result_vector.size(); i++)
            {
                next.push_back(make_pair(atoi(p.result_vector[i]["id"].c_str()), p.result_vector[i]["dname"]));
            }
            dir_tree.push(next);
            vector<pair<int, string>>().swap(next);
            dir_now.push(0);
        }
    }
    cout << data << endl;
    return make_response_json(200, "����Ŀ¼����", data);
}