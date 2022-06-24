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
    cout << index << endl;
    cout << file_md5 << endl;
    cout << fragment_md5 << endl;

    if (file_name.length() != 0) //���ļ��������У��ļ����ݿ���Ϊ0�����ļ���������Ϊ0
        file_exist = true;
    if (file_exist) //�����ļ�����
    {
        cout << "����������Ƭ���ļ����� " << file_name << endl; //���Ը�client��Լ��
        // cout << "�ļ����ݣ� " << file_fragment << endl;
        string savePath = "../pool/" + fragment_md5;
        if (file_exists(savePath) == false) //����pool���ļ������ڣ��������ͬʱ�����ݿ����ѯ��
        {
            fstream myf_body(savePath, ios::out | ios::binary); //���Main��˵����һ��Ŀ¼��Ҫ��������������Ҫ�ĳɾ���·����
            if (myf_body.good())
            {
                myf_body << file_fragment;
            }
            else
            {
                cout << "Can't open file!" << endl;
                return make_response_json(500, "�洢·������");
            }
            myf_body.close();
            {
                string md5_correct; //��������������md5��
                string cmd = "md5sum " + savePath;
                if (popen_cmd(cmd, md5_correct, 32 + 1) != -1)
                {
                    if (md5_correct != fragment_md5) // md5��Բ��ϣ���Ƭ���ܴ�������
                    {
                        //ɾ�����ŵ��ļ�
                        if (remove(savePath.c_str()) == 0)
                        {
                            cout << "ɾ���ɹ�" << endl;
                        }
                        else
                        {
                            cout << "ɾ��ʧ��" << endl;
                        }

                        return make_response_json(400, "���md5�����");
                    }
                    else //��ȫ��ȷ��ȥ���ݿ����Ҷ�Ӧ�ļ���������
                    {
                        //���ݿ����
                        return make_response_json(200);
                    }
                }
                else
                {
                    return make_response_json(500, "popen_err");
                }
            }
        }
    }
    // else //û���ļ����Ѿ�������ϣ�˵�������Ƭ�Ѿ������ˣ�����fragment_md5����Ƭ��Ȼ��������ݿ����
    {
        //���ݿ����
        cout << "��Ƭ�Ѵ���" << endl;
        return make_response_json(200);
    }

    return make_response_json(200);
}