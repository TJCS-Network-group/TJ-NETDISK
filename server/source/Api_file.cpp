#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
using namespace std;
HttpResponse GET_file_exist(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    if (req.params.find("md5") == req.params.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    string md5 = req.params["md5"];
    my_database p;
    p.connect();
    sprintf(p.sql, "select * from FileEntity where md5=\"%s\"", md5.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
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
        return make_response_json(500, "���ݿ����ļ�md5��ײ,����ϵ����Ա���д���");
    }
    result += '}';
    return make_response_json(200, "��ѯ�������", result);
}
HttpResponse GET_file_fragment(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    if (req.params.find("md5") == req.params.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    string md5 = req.params["md5"];
    my_database p;
    p.connect();
    sprintf(p.sql, "select * from FileFragmentEntity where md5=\"%s\"", md5.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
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
        return make_response_json(500, "���ݿ����ļ���md5��ײ,����ϵ����Ա���д���");
    }
    result += '}';
    return make_response_json(200, "��ѯ�������", result);
}
HttpResponse PUT_filesystem_rename_file(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("fid") == data.end() || data.find("fname") == data.end() || data.find("did") == data.end() || data.find("new_name") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    string fname = data["fname"].as_string(), new_name = data["new_name"].as_string();
    int fid = data["fid"].as_int(), did = data["did"].as_int();
    my_database p;
    p.connect();
    sprintf(p.sql, "select * from FileDirectoryMap where fid=%d and did=%d and fname=\"%s\"", fid, did, fname.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    cout << p.sql << endl;
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(400, "�����޸Ĳ����ڵ��ļ�");
    }
    sprintf(p.sql, "select root_dir_id from UserEntity where id=%d", req.current_user_id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.get();
    int current_root_id = atoi(p.result_vector[0]["root_dir_id"].c_str());
    int child, parent;
    child = 0;
    parent = did;
    while (child != parent)
    {
        child = parent;
        sprintf(p.sql, "select parent_id from DirectoryEntity where id=%d", child);
        if (p.execute() == -1)
        {
            return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
        }
        p.get();
        parent = atoi(p.result_vector[0]["parent_id"].c_str());
    }
    if (parent != current_root_id)
    {
        return make_response_json(400, "�޷��������˵��ļ�");
    }
    if (new_name == fname)
    {
        return make_response_json(200, "ԭ�ļ������޸ĺ�����,�����޸�");
    }
    sprintf(p.sql, "select * from FileDirectoryMap where did=%d and fname=\"%s\"",
            did, new_name.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.get();
    if (p.result_vector.size() != 0)
    {
        return make_response_json(400, "���������ļ�,�޸ı�ȡ��");
    }
    sprintf(p.sql, "update FileDirectoryMap set fname=\"%s\" where did=%d and fid=%d and fname=\"%s\"",
            new_name.c_str(), did, fid, fname.c_str());
    cout << p.sql << endl;
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.disconnect();
    return make_response_json(200, "�����ɹ�");
}