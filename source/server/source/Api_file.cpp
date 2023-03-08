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
        if (atoi(p.result_vector[0]["is_complete"].c_str()) == 1)
            result += "true";
        else
            result += "false";
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
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(400, "�����޸Ĳ����ڵ��ļ�");
    }
    int did = atoi(p.result_vector[0]["did"].c_str());
    string fname = p.result_vector[0]["fname"];
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    int check = is_child(did, current_root_id, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }
    if (!check && did != current_root_id)
    {
        return make_response_json(400, "�޷��������˵��ļ�");
    }
    int statusCode = 0;
    string fin_name = get_file_new_name(new_name, did, statusCode);
    if (statusCode)
    {
        return make_response_json(statusCode, fin_name);
    }
    if (fin_name == fname)
    {
        return make_response_json(200, "���ļ�������ԭ�ļ�����ͬ��������ļ��д��ڳ�ͻ,�޸ı�ȡ��");
    }
    sprintf(p.sql, "update FileDirectoryMap set fname=\"%s\" where id=%d",
            fin_name.c_str(), id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    }
    p.disconnect();
    message = "�ļ������ɹ�,����Ϊ" + fin_name;
    return make_response_json(200, message);
}
HttpResponse GET_upload_allocation(HttpRequest &req)
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
        int next_index = fsize > FRAGMENT_SIZE;
        sprintf(p.sql, "insert into FileEntity(MD5,fsize,link_num,next_index,is_complete)\
         value (\"%s\",%d,%d,%d,%d)",
                md5.c_str(), fsize, 0, next_index, 0); // link����Ϊ1���ϴ���ɺ������Ϊ0
        if (p.execute() == -1)
        {
            return make_response_json(500, "���ݿ��½�����,����ϵ����Ա�������");
        }
        data += "{\"next_index\":0,\"num\":0}";
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
            // data += "{\"next_index\":" + p.result_vector[0]["next_index"] + '}';
            int max_index = atoi(p.result_vector[0]["fsize"].c_str()) / FRAGMENT_SIZE + 1;
            int now_index = atoi(p.result_vector[0]["next_index"].c_str());
            int next_index = -1;
            set<int> indexs;
            for (int i = 0; i < max_index; i++)
            {
                indexs.insert(i);
            }
            sprintf(p.sql, "select `index` from FileFragmentMap \
            join FileEntity on FileEntity.id=FileFragmentMap.fid where FileEntity.md5=\"%s\"",
                    md5.c_str());
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ��ѯʧ��,����ϵ����Ա�������");
            }
            p.get();
            int num = p.result_vector.size();
            for (size_t i = 0; i < p.result_vector.size(); i++)
            {
                // cout << "has index:" << atoi(p.result_vector[i]["index"].c_str()) << endl;
                indexs.erase(atoi(p.result_vector[i]["index"].c_str()));
            }
            if (indexs.find(now_index) == indexs.end())
            {
                for (auto it = indexs.begin(); it != indexs.end(); it++)
                {
                    if (*it > now_index)
                    {
                        next_index = *it;
                        break;
                    }
                }
                if (next_index == -1 && indexs.size() > 0)
                {
                    next_index = *indexs.begin();
                }
                now_index = next_index;
                next_index = -1;
            }
            data += "{\"next_index\":" + to_string(now_index) + ",\"num\":" + to_string(num) + '}';
            for (auto it = indexs.begin(); it != indexs.end(); it++)
            {
                if (*it > now_index)
                {
                    next_index = *it;
                    break;
                }
            }
            if (next_index == -1 && indexs.size() > 0)
            {
                next_index = *indexs.begin();
            }
            // cout << "next_index:" << next_index << endl;
            if (next_index != -1)
            {
                sprintf(p.sql, "update FileEntity set next_index=%d where md5=\"%s\"", next_index, md5.c_str());
            }
            else
            {
                sprintf(p.sql, "update FileEntity set next_index=%d,is_complete=1 where md5=\"%s\"", next_index, md5.c_str());
            }
            // cout << p.sql << endl;
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ���³���,����ϵϵͳ����Ա");
            }
        }
        else
        {
            sprintf(p.sql, "select count(*) as num from FileFragmentMap join FileEntity on FileEntity.id=FileFragmentMap.fid\
             where FileEntity.MD5=\"%s\"",
                    md5.c_str());
            if (p.execute() == -1)
            {
                return make_response_json(500, "���ݿ��ѯ����");
            }
            p.get();
            data += "{\"next_index\":-1,\"num\":" + p.result_vector[0]["num"] + '}';
            return make_response_json(200, "��Ҫ����һ��Ϊ", data);
        }
    }
    p.disconnect();
    return make_response_json(200, "��Ҫ����һ��Ϊ", data);
}
HttpResponse POST_share_move_file(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("fdid") == data.end() || data.find("pid") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
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
        return make_response_json(500, "���ݿ��ѯ����");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(400, "�ƶ��˲����ڵ��ļ�");
    }
    int f_in_did = atoi(p.result_vector[0]["did"].c_str());
    string fname = p.result_vector[0]["fname"];
    int f_check = is_child(f_in_did, current_root_id, message);
    if (f_check < 0)
    {
        return make_response_json(-f_check, message);
    }
    if (!f_check && f_in_did != current_root_id)
    {
        return make_response_json(400, "�����ƶ����˵��ļ�");
    }
    int d_check = is_child(pid, current_root_id, message);
    if (d_check < 0)
    {
        return make_response_json(-d_check, message);
    }
    if (!d_check && pid != current_root_id)
    {
        return make_response_json(400, "�����ƶ������˵��ļ���");
    }
    if (pid == f_in_did)
    {
        return make_response_json(200, "�ļ������ƶ�");
    }
    int statusCode = 0;
    string fin_name = get_file_new_name(fname, pid, statusCode);
    if (statusCode)
    {
        return make_response_json(statusCode, fin_name);
    }
    sprintf(p.sql, "update FileDirectoryMap set did=%d,fname=\"%s\" where id=%d", pid, fin_name.c_str(), fdid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��޸�ʧ��,����ϵ����Ա");
    }
    p.disconnect();
    message = "�ƶ��ɹ�,���ļ���Ϊ" + fin_name;
    return make_response_json(200, message);
}
HttpResponse POST_share_copy_file(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("fdid") == data.end() || data.find("pid") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
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
        return make_response_json(500, "���ݿ��ѯ����");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(400, "�ƶ��˲����ڵ��ļ�");
    }
    int fid = atoi(p.result_vector[0]["fid"].c_str());
    int f_in_did = atoi(p.result_vector[0]["did"].c_str());
    string fname = p.result_vector[0]["fname"];
    int f_check = is_child(f_in_did, current_root_id, message);
    if (f_check < 0)
    {
        return make_response_json(-f_check, message);
    }
    if (!f_check && f_in_did != current_root_id)
    {
        return make_response_json(400, "�����ƶ����˵��ļ�");
    }
    int d_check = is_child(pid, current_root_id, message);
    if (d_check < 0)
    {
        return make_response_json(-d_check, message);
    }
    if (!d_check && pid != current_root_id)
    {
        return make_response_json(400, "�����ƶ������˵��ļ���");
    }
    int statusCode = 0;
    string fin_name = get_file_new_name(fname, pid, statusCode);
    if (statusCode)
    {
        return make_response_json(statusCode, fin_name);
    }
    sprintf(p.sql, "insert into FileDirectoryMap(fid,did,fname) value (%d,%d,\"%s\");", fid, pid, fin_name.c_str());
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
    p.disconnect();
    message = "���Ƴɹ�,���ļ���Ϊ" + fin_name;
    return make_response_json(200, message);
}
HttpResponse DEL_remove_file(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    map<string, JSON> data = req.json.as_map();
    if (data.find("fdid") == data.end())
    {
        return make_response_json(400, "�����ʽ����");
    }

    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    int fdid = data["fdid"].as_int();
    int check = remove_file(fdid, current_root_id, false, message);
    if (check < 0)
    {
        return make_response_json(-check, message);
    }
    return make_response_json(200, "ɾ���ɹ�");
}
HttpResponse GET_filesystem_get_md5code(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(401, "��ǰ�û�δ��¼");
    }
    if (req.params.find("fdid") == req.params.end())
    {
        return make_response_json(400, "�����ʽ����");
    }
    string message;
    int current_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_root_id < 0)
    {
        return make_response_json(-current_root_id, message);
    }
    int fdid = atoi(req.params["fdid"].c_str());
    my_database p;
    p.connect();
    sprintf(p.sql, "select FileEntity.MD5 as MD5, FileDirectoryMap.did as did from FileDirectoryMap\
     join FileEntity on FileEntity.id=FileDirectoryMap.fid  where FileDirectoryMap.id=%d",
            fdid);
    if (p.execute() == -1)
    {
        return make_response_json(500, "���ݿ��ѯ����");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(400, "��ѯ�˲����ڵ��ļ�");
    }
    string MD5 = p.result_vector[0]["MD5"];
    int did = atoi(p.result_vector[0]["did"].c_str());
    int f_check = is_child(did, current_root_id, message);
    if (f_check < 0)
    {
        return make_response_json(-f_check, message);
    }
    if (!f_check && did != current_root_id)
    {
        return make_response_json(400, "���ɲ鿴���˵��ļ�");
    }
    string data = "{\"MD5\":\"" + MD5 + "\"}";
    return make_response_json(200, "��ѯ�������", data);
}