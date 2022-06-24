#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
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
    int parent = get_root_id_by_did(parent_id, message);
    if (parent < 0)
    {
        return make_response_json(-parent, message);
    }
    if (parent != current_root_id)
    {
        return make_response_json(400, "�޷��������˵��ļ���");
    }
    if (parent_id == did)
    {
        return make_response_json(401, "�û���Ȩ�޸ĸ�Ŀ¼��");
    }
    if (dname == new_name)
    {
        return make_response_json(200, "ԭ�ļ����������޸�");
    }
    sprintf(p.sql, "select * from DirectoryEntity where parent_id=%d and id!=%d and id!=%d and dname=\"%s\"",
            parent_id, parent_id, did, new_name.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.get();
    if (p.result_vector.size() != 0)
    {
        return make_response_json(400, "����ͬ���ļ���,�޸�ʧ��");
    }
    sprintf(p.sql, "update DirectoryEntity set dname=\"%s\" where id=%d", new_name.c_str(), did);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.disconnect();
    return make_response_json(200, "�����ɹ�");
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
    int parent = get_root_id_by_did(pid, message);
    if (parent < 0)
    {
        return make_response_json(-parent, message);
    }
    if (parent != current_root_id)
    {
        return make_response_json(400, "�����ڱ��˵��ļ������½��ļ���");
    }
    my_database p;
    p.connect();
    sprintf(p.sql, "select * from DirectoryEntity where parent_id=%d and dname=\"%s\"", pid, dname.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ�������,����ϵ����Ա���");
    }
    p.get();
    if (p.result_vector.size() != 0)
    {
        return make_response_json(400, "�Ѵ���ͬ���ļ���,����ʧ��");
    }
    sprintf(p.sql, "insert into DirectoryEntity(dname,parent_id) value (\"%s\",%d)", dname.c_str(), pid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ�������,����ϵ����Ա���");
    }
    p.disconnect();
    return make_response_json(200, "�½��ļ��гɹ�");
}
