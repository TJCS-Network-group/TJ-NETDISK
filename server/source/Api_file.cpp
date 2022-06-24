#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
#include <set>
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
    if (data.find("fid") == data.end() || data.find("fname") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    string new_name = data["fname"].as_string();
    int id = data["fid"].as_int();
    my_database p;
    p.connect();
    sprintf(p.sql, "select did,fname from FileDirectoryMap where id=%d", id);
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
    auto k = p.result_vector;
    int did = atoi(p.result_vector[0]["did"].c_str());
    string fname = p.result_vector[0]["fname"];
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
    sprintf(p.sql, "update FileDirectoryMap set fname=\"%s\" where id=%d",
            new_name.c_str(), id);
    cout << p.sql << endl;
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.disconnect();
    return make_response_json(200, "�����ɹ�");
}
HttpResponse GET_upload_allocation(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(404, "��ǰ�û�δ��¼");
    }
    if (req.params.find("md5") == req.params.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    string md5 = req.params["md5"];
    string data;
    // check(md5);
    my_database p;
    p.connect();
    sprintf(p.sql, "select fsize,next_index,is_complete from FileEntity where md5=\"%s\"", md5.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        if (req.params.find("size") == req.params.end())
        {
            return make_response_json(400, "���ļ���С�޷��½���Ŀ");
        }
        int fsize = atoi(req.params["size"].c_str());
        if (fsize < 0)
        {
            return make_response_json(400, "�����ڸ�����С���ļ�");
        }
        sprintf(p.sql, "insert into FileEntity(MD5,fsize,link_num,next_index,is_complete)\
         value (\"%s\",%d,%d,%d,%d)",
                md5.c_str(), fsize, 1, 0, 0);
        if (p.execute() == -1)
        {
            return make_response_json(500, "���ݿ��ѯ�½�,����ϵ����Ա�������");
        }
        data += "{\"next_index\":0}";
        return make_response_json(200, "��Ҫ����һ��Ϊ", data);
    }
    else if (p.result_vector.size() == 1)
    {
        if (req.params.find("size") != req.params.end())
        {
            if (atoi(req.params["size"].c_str()) != atoi(p.result_vector[0]["fsize"].c_str()))
            {
                return make_response_json(400, "�ļ���Сָ������");
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
                return make_response_json(500, "���ݿ��ѯʧ��,����ϵ����Ա�������");
            }
            p.get();
            cout << "has upload:" << p.result_vector.size();
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
                return make_response_json(500, "���ݿ���³���,����ϵϵͳ����Ա");
            }
        }
        else
        {
            data += "{\"next_index\":-1}";
            return make_response_json(200, "��Ҫ����һ��Ϊ", data);
        }
    }
    p.disconnect();
    return make_response_json(200, "��Ҫ����һ��Ϊ", data);
}