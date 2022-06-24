// API函数统一以<method>_<route>命名
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
        return make_response_json(400, "请求格式不对");
    }
    cout << index << endl;
    cout << file_md5 << endl;
    cout << fragment_md5 << endl;

    if (file_name.length() != 0) //有文件的情形中，文件内容可以为0，但文件名不可能为0
        file_exist = true;
    if (file_exist) //传了文件过来
    {
        cout << "传进来的碎片的文件名： " << file_name << endl; //可以跟client端约定
        // cout << "文件内容： " << file_fragment << endl;
        string savePath = "../pool/" + fragment_md5;
        if (file_exists(savePath) == false) //看看pool中文件不存在（或许可以同时在数据库里查询）
        {
            fstream myf_body(savePath, ios::out | ios::binary); //相对Main来说是上一级目录（要开机自启动可能要改成绝对路径）
            if (myf_body.good())
            {
                myf_body << file_fragment;
            }
            else
            {
                cout << "Can't open file!" << endl;
                return make_response_json(500, "存储路径错误");
            }
            myf_body.close();
            {
                string md5_correct; //服务器计算的碎块md5码
                string cmd = "md5sum " + savePath;
                if (popen_cmd(cmd, md5_correct, 32 + 1) != -1)
                {
                    if (md5_correct != fragment_md5) // md5码对不上，碎片可能传输有误
                    {
                        //删掉存着的文件
                        if (remove(savePath.c_str()) == 0)
                        {
                            cout << "删除成功" << endl;
                        }
                        else
                        {
                            cout << "删除失败" << endl;
                        }

                        return make_response_json(400, "碎块md5码错误");
                    }
                    else //完全正确，去数据库里找对应文件和碎块就行
                    {
                        //数据库操作
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
    // else //没传文件或已经处理完毕，说明这个碎片已经存在了，可用fragment_md5找碎片，然后更新数据库就行
    {
        //数据库操作
        cout << "碎片已存在" << endl;
        return make_response_json(200);
    }

    return make_response_json(200);
}