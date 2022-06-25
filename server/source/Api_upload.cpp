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
    if (req.current_user_id == 0) //没登录
    {
        return make_response_json(401, "请先登录");
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
        return make_response_json(400, "请求格式不对");
    }

    bool file_fragment_in_database = true;

    //数据库操作
    my_database create_link;
    create_link.connect();
    sprintf(create_link.sql, "select id from FileFragmentEntity where MD5=\"%s\"", fragment_md5.c_str());
    // cout << create_link.sql << endl;
    if (create_link.execute() == -1)
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    create_link.get();
    if (create_link.result_vector.size() == 0)
        file_fragment_in_database = false;

    sprintf(create_link.sql, "select id from FileEntity where MD5=\"%s\"", file_md5.c_str());
    // cout << create_link.sql << endl;
    if (create_link.execute() == -1)
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    create_link.get();
    if (create_link.result_vector.size() == 0)
        return make_response_json(500, "数据库表项中不存在该碎片所在的文件！请检查upload_allocate接口");
    int file_id = atoi(create_link.result_vector[0]["id"].c_str());

    if (file_name.length() != 0) //有文件的情形中，文件内容可以为0，但文件名不可能为0
        file_exist = true;
    if (file_exist) //传了文件过来
    {
        cout << "传进来的碎片的文件名： " << file_name << endl; //可以跟client端约定
        // cout << "文件内容： " << file_fragment << endl;
        string savePath = "../pool/" + fragment_md5;
        if (file_exists(savePath) == false && file_fragment_in_database)
            return make_response_json(500, "文件碎片池与数据库不一致！碎片池有该md5码而数据库没有");
        else if (file_exists(savePath) && file_fragment_in_database == false)
            return make_response_json(500, "文件碎片池与数据库不一致！碎片池无该md5码而数据库有");
        else if (file_exists(savePath) == false && file_fragment_in_database == false) //看看pool和数据库中文件不存在
        {
            fstream myf_body(savePath, ios::out | ios::binary); //相对Main来说是上一级目录（要开机自启动可能要改成绝对路径）
            if (myf_body.good())
                myf_body << file_fragment;
            else
                return make_response_json(500, "存储路径错误，请检查pool所在路径");
            myf_body.close();
            {
                string md5_correct; //服务器计算的碎块md5码
                string cmd = "md5sum " + savePath;
                if (popen_cmd(cmd, md5_correct, 32 + 1) != -1)
                {
                    if (md5_correct != fragment_md5) // md5码对不上，碎片可能传输有误
                    {
                        //删掉存着的文件
                        if (remove(savePath.c_str()) != 0)
                            return make_response_json(500, "碎块md5码错误，错误碎块删除失败");
                        return make_response_json(400, "传入碎块md5码与服务端根据碎块内容计算出的结果不一致，错误碎块未存储，数据库未更新");
                    }
                    else //添加碎块完全正确，去数据库里找对应文件和碎块就行
                    {
                        sprintf(create_link.sql, "insert into FileFragmentEntity(MD5,fgsize,link_num) value(\"%s\",%d,%d)",
                                md5_correct.c_str(), file_fragment.size(), 0); // 0，后面统一执行++
                        // cout << create_link.sql << endl;
                        if (create_link.execute() == -1)
                            return make_response_json(500, "数据库插入出错,请联系管理员解决问题");
                    }
                }
                else
                    return make_response_json(500, "popen_err");
            }
        }
        // else
        //     cout << "碎片文件已存在,无需反复上传" << endl;
    }
    // else //没传文件或已经处理完毕，说明这个碎片已经存在了，可用fragment_md5找碎片，然后更新数据库就行
    {
        //数据库操作
        sprintf(create_link.sql, "select id from FileFragmentEntity where MD5=\"%s\"", fragment_md5.c_str());
        // cout << create_link.sql << endl;
        if (create_link.execute() == -1)
            return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
        create_link.get();
        if (create_link.result_vector.size() == 0)
            return make_response_json(500, "数据库表项中不存在该碎片");
        int fgid = atoi(create_link.result_vector[0]["id"].c_str());

        sprintf(create_link.sql, "select id from FileEntity where MD5=\"%s\"", file_md5.c_str());
        // cout << create_link.sql << endl;
        if (create_link.execute() == -1)
            return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
        create_link.get();
        if (create_link.result_vector.size() == 0)
            return make_response_json(500, "数据库表项中不存在该碎片所在的文件");
        int file_id = atoi(create_link.result_vector[0]["id"].c_str());

        sprintf(create_link.sql, "select id from FileFragmentMap where fid=%d and `index`=%d and fgid=%d", file_id, index, fgid);
        // cout << create_link.sql << endl;
        if (create_link.execute() == -1)
            return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
        create_link.get();
        if (create_link.result_vector.size() == 0)
        {
            sprintf(create_link.sql, "insert into FileFragmentMap(fid,`index`,fgid) value(%d,%d,%d)", file_id, index, fgid);
            // cout << create_link.sql << endl;
            if (create_link.execute() == -1)
                return make_response_json(500, "数据库插入出错,请联系管理员解决问题");
            sprintf(create_link.sql, "update FileFragmentEntity set link_num=link_num+1 where id=%d", fgid);
            // cout << create_link.sql << endl;
            if (create_link.execute() == -1)
                return make_response_json(500, "数据库修改出错,请联系管理员解决问题");
        }
        else
            return make_response_json(200, "您已经上传过该文件碎片了哦");
    }

    return make_response_json(200);
}
