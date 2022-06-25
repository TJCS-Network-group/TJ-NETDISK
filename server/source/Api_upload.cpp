// API����ͳһ��<method>_<route>����
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
#include <cstdio>
#include <exception>
#include <fstream>
using namespace std;

bool file_exists(const std::string &name)
{
    ifstream f(name.c_str());
    bool res = f.good();
    f.close();
    return res;
}

HttpResponse POST_upload_fragment(HttpRequest &req)
{
    if (req.current_user_id == 0) //û��¼
    {
        return make_response_json(401, "���ȵ�¼");
    }
    string file_md5, fragment_md5, file_fragment, file_name;
    int index;
    bool file_exist = false;
    try
    {
        index = atoi(req.form_data["index"].c_str());
        file_md5 = req.form_data["file_md5"];
        fragment_md5 = req.form_data["fragment_md5"];
        if (req.form_data.count("file_fragment") != 0)
            file_fragment = req.form_data["file_fragment"];
        if (req.form_data.count("filename") != 0)
            file_name = req.form_data["filename"];
    }
    catch (exception e)
    {
        return make_response_json(400, "�����ʽ����");
    }

    bool file_fragment_in_database = true;

    //���ݿ����
    my_database create_link;
    create_link.connect();
    sprintf(create_link.sql, "select id from FileFragmentEntity where MD5=\"%s\"", fragment_md5.c_str());
    // cout << create_link.sql << endl;
    if (create_link.execute() == -1)
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    create_link.get();
    if (create_link.result_vector.size() == 0)
        file_fragment_in_database = false;

    sprintf(create_link.sql, "select id from FileEntity where MD5=\"%s\"", file_md5.c_str());
    // cout << create_link.sql << endl;
    if (create_link.execute() == -1)
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    create_link.get();
    if (create_link.result_vector.size() == 0)
        return make_response_json(500, "���ݿ�����в����ڸ���Ƭ���ڵ��ļ�������upload_allocate�ӿ�");
    int file_id = atoi(create_link.result_vector[0]["id"].c_str());

    if (file_name.length() != 0) //���ļ��������У��ļ����ݿ���Ϊ0�����ļ���������Ϊ0
        file_exist = true;
    if (file_exist) //�����ļ�����
    {
        cout << "����������Ƭ���ļ����� " << file_name << endl; //���Ը�client��Լ��
        // cout << "�ļ����ݣ� " << file_fragment << endl;
        string savePath = "../pool/" + fragment_md5;
        if (file_exists(savePath) == false && file_fragment_in_database)
            return make_response_json(500, "�ļ���Ƭ�������ݿⲻһ�£���Ƭ���и�md5������ݿ�û��");
        else if (file_exists(savePath) && file_fragment_in_database == false)
            return make_response_json(500, "�ļ���Ƭ�������ݿⲻһ�£���Ƭ���޸�md5������ݿ���");
        else if (file_exists(savePath) == false && file_fragment_in_database == false) //����pool�����ݿ����ļ�������
        {
            fstream myf_body(savePath, ios::out | ios::binary); //���Main��˵����һ��Ŀ¼��Ҫ��������������Ҫ�ĳɾ���·����
            if (myf_body.good())
                myf_body << file_fragment;
            else
                return make_response_json(500, "�洢·����������pool����·��");
            myf_body.close();
            {
                string md5_correct; //��������������md5��
                string cmd = "md5sum " + savePath;
                if (popen_cmd(cmd, md5_correct, 32 + 1) != -1)
                {
                    if (md5_correct != fragment_md5) // md5��Բ��ϣ���Ƭ���ܴ�������
                    {
                        //ɾ�����ŵ��ļ�
                        if (remove(savePath.c_str()) != 0)
                            return make_response_json(500, "���md5����󣬴������ɾ��ʧ��");
                        return make_response_json(400, "�������md5�������˸���������ݼ�����Ľ����һ�£��������δ�洢�����ݿ�δ����");
                    }
                    else //��������ȫ��ȷ��ȥ���ݿ����Ҷ�Ӧ�ļ���������
                    {
                        sprintf(create_link.sql, "insert into FileFragmentEntity(MD5,fgsize,link_num) value(\"%s\",%d,%d)",
                                md5_correct.c_str(), file_fragment.size(), 0); // 0������ͳһִ��++
                        // cout << create_link.sql << endl;
                        if (create_link.execute() == -1)
                            return make_response_json(500, "���ݿ�������,����ϵ����Ա�������");
                    }
                }
                else
                    return make_response_json(500, "popen_err");
            }
        }
        // else
        //     cout << "��Ƭ�ļ��Ѵ���,���跴���ϴ�" << endl;
    }
    // else //û���ļ����Ѿ�������ϣ�˵�������Ƭ�Ѿ������ˣ�����fragment_md5����Ƭ��Ȼ��������ݿ����
    {
        //���ݿ����
        sprintf(create_link.sql, "select id from FileFragmentEntity where MD5=\"%s\"", fragment_md5.c_str());
        // cout << create_link.sql << endl;
        if (create_link.execute() == -1)
            return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
        create_link.get();
        if (create_link.result_vector.size() == 0)
            return make_response_json(500, "���ݿ�����в����ڸ���Ƭ");
        int fgid = atoi(create_link.result_vector[0]["id"].c_str());

        sprintf(create_link.sql, "select id from FileEntity where MD5=\"%s\"", file_md5.c_str());
        // cout << create_link.sql << endl;
        if (create_link.execute() == -1)
            return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
        create_link.get();
        if (create_link.result_vector.size() == 0)
            return make_response_json(500, "���ݿ�����в����ڸ���Ƭ���ڵ��ļ�");
        int file_id = atoi(create_link.result_vector[0]["id"].c_str());

        sprintf(create_link.sql, "select id from FileFragmentMap where fid=%d and `index`=%d and fgid=%d", file_id, index, fgid);
        // cout << create_link.sql << endl;
        if (create_link.execute() == -1)
            return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
        create_link.get();
        if (create_link.result_vector.size() == 0)
        {
            sprintf(create_link.sql, "insert into FileFragmentMap(fid,`index`,fgid) value(%d,%d,%d)", file_id, index, fgid);
            // cout << create_link.sql << endl;
            if (create_link.execute() == -1)
                return make_response_json(500, "���ݿ�������,����ϵ����Ա�������");
            sprintf(create_link.sql, "update FileFragmentEntity set link_num=link_num+1 where id=%d", fgid);
            // cout << create_link.sql << endl;
            if (create_link.execute() == -1)
                return make_response_json(500, "���ݿ��޸ĳ���,����ϵ����Ա�������");
        }
        else
            return make_response_json(200, "���Ѿ��ϴ������ļ���Ƭ��Ŷ");
    }

    return make_response_json(200);
}
bool check_file_name_legal(string file_name)
{
    if (file_name == "")
        return false;
    char illegal_chars[10] = {'\\', '/', '*', '?', '\"', '<', '>', '|'};
    for (char ch : illegal_chars)
    {
        if (file_name.find(ch) != file_name.npos)
            return false;
    }
    return true;
}

HttpResponse POST_upload_file(HttpRequest &req)
{
    if (req.current_user_id == 0) //û��¼
    {
        return make_response_json(401, "���ȵ�¼");
    }
    string file_md5, file_name;
    int parent_dir_id;

    try
    {
        file_md5 = req.json["md5"].as_string();
        parent_dir_id = req.json["parent_dir_id"].as_int();
        file_name = req.json["file_name"].as_string();
    }
    catch (exception e)
    {
        return make_response_json(400, "�����ʽ����");
    }

    //���ݿ����
    my_database add_map;
    add_map.connect();
    //����ļ�
    sprintf(add_map.sql, "select is_complete,id from FileEntity where MD5=\"%s\"", file_md5.c_str());
    // cout << add_map.sql << endl;
    if (add_map.execute() == -1)
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    add_map.get();
    if (add_map.result_vector.size() == 0)
        return make_response_json(500, "���ݿ�����в����ڸ��ļ�������upload_allocate�ӿ�");
    int file_id = atoi(add_map.result_vector[0]["id"].c_str());
    cout << "is_complete: " << atoi(add_map.result_vector[0]["is_complete"].c_str());
    bool is_complete = atoi(add_map.result_vector[0]["is_complete"].c_str());

    //���Ŀ¼
    sprintf(add_map.sql, "select id from DirectoryEntity where parent_id=\"%d\"", parent_dir_id);
    // cout << add_map.sql << endl;
    if (add_map.execute() == -1)
        return make_response_json(500, "���ݿ��ѯ����,����ϵ����Ա�������");
    add_map.get();
    if (add_map.result_vector.size() == 0)
        return make_response_json(500, "���ݿ�����в����ڸ�Ŀ¼");
    string message = "";
    int current_user_root_id = get_root_id_by_user(req.current_user_id, message);
    if (current_user_root_id < 0)
    {
        return make_response_json(-current_user_root_id, message);
    }
    else
    {
        int status = is_child(parent_dir_id, current_user_root_id, message);
        if (current_user_root_id == parent_dir_id || status > 0) //�Ǹ��ӹ�ϵ���߾��ǵ�ǰ�û���Ŀ¼�������·���
            ;
        else if (status < 0) //��ѯ����
            return make_response_json(-status, message);
        else //�����ļ���ѹ��û�й�ϵ�������ڲ�ͬ�û�
            return make_response_json(401, "���޷����������û����ļ���");
    }

    //����ļ���
    if (check_file_name_legal(file_name) == false)
        return make_response_json(400, "�ļ�������");

    if (is_complete)
    {
        sprintf(add_map.sql, "insert into `FileDirectoryMap`(`fid`,`did`,`fname`) value(%d,%d,%s)", file_id, parent_dir_id, file_name.c_str());
        if (add_map.execute() == -1)
            return make_response_json(500, "���ݿ�������,����ϵ����Ա�������");
        sprintf(add_map.sql, "update `FileEntity` set link_num=link_num+1 where id=%d", file_id);
        if (add_map.execute() == -1)
            return make_response_json(500, "���ݿ��޸ĳ���,����ϵ����Ա�������");
    }
    else
    {
        return make_response_json(400, "�ļ���δ�����ϴ��������ϴ���Ƭ��");
    }
    return make_response_json();
}
