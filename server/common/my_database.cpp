#include "../include/my_database.h"
#include "../include/HttpTool.h"
#include <iostream> // cin,cout等
#include <iomanip>  // setw等
#include <cstdlib>
using namespace std;
const char *my_database::host = "localhost";

const char *my_database::user = "root";
const char *my_database::passwd = "root123";
const char *my_database::db = "pan";
const unsigned int my_database::port = 0;
const char *my_database::unix_socket = nullptr;
const unsigned long my_database::clientflag = 0;
const char *my_database::character_set_name = "gbk";
my_database::my_database()
{
    mysql = nullptr;
    result = nullptr;
    row = nullptr;
}
my_database::~my_database()
{
    vector<map<string, string>>().swap(result_vector);
    if (mysql != nullptr)
    {
        mysql_free_result(result);
        mysql_close(mysql);
    }
    mysql = nullptr;
    result = nullptr;
    row = nullptr;
}

void my_database::connect()
{
    if (mysql == nullptr)
    {
        mysql = mysql_init(nullptr);
        if (mysql == nullptr)
        {
            cout << "mysql_init failed" << endl;
            exit(-1);
        }
    }
    if (mysql_real_connect(mysql, host, user, passwd, db, port, unix_socket, clientflag) == nullptr)
    {
        cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        exit(-1);
    }
    if (mysql_set_character_set(mysql, character_set_name))
    {
        cout << "set character " << character_set_name << " failed" << endl;
        exit(-1);
    }
}
void my_database::disconnect()
{
    if (mysql != nullptr)
    {
        mysql_free_result(result);
        mysql_close(mysql);
    }
    vector<map<string, string>>().swap(result_vector);
    result = nullptr;
    row = nullptr;
    mysql = nullptr;
}
int my_database::execute()
{
    vector<map<string, string>>().swap(result_vector);
    if (mysql_query(mysql, sql))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        return -1;
    }
    return 0;
}
int my_database::get()
{
    vector<map<string, string>>().swap(result_vector);
    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        exit(-1);
    }
    map<string, string> s;
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        map<string, string>().swap(s);
        for (int i = 0; i < int(result->field_count); i++)
        {
            s[(result->fields + i)->name] = row[i];
        }
        result_vector.push_back(s);
    }
    return 0;
}
void my_database::show()
{
    for (size_t i = 0; i < result_vector.size(); i++)
    {
        for (auto j = result_vector[i].begin(); j != result_vector[i].end(); j++)
        {
            cout << j->first << ":" << j->second << endl;
        }
    }
}
int get_root_id_by_user(int user_id, string &message)
{
    my_database p;
    p.connect();
    sprintf(p.sql, "select root_dir_id as root_id from UserEntity where id=%d", user_id);
    if (p.execute() == -1)
    {
        message = "数据库查询出现错误";
        return -500;
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        message = "该用户不存在";
        return -404;
    }
    int root_id = atoi(p.result_vector[0]["root_id"].c_str());
    p.disconnect();
    message = "查询结果如下";
    return root_id;
}
int is_child(int first, int second, string &message)
{
    if (first == second)
    {
        return 0;
    }
    my_database p;
    p.connect();
    int result = 0;
    int parent = first, child = 0;
    while (parent != child)
    {
        child = parent;
        sprintf(p.sql, "select parent_id from DirectoryEntity where id=%d", child);
        if (p.execute() == -1)
        {
            message = "数据库查询出错,请联系系统管理员";
            result = -500;
            break;
        }
        p.get();
        if (p.result_vector.size() == 0)
        {
            message = "正在访问不存在的文件夹";
            result = -400;
            break;
        }
        parent = atoi(p.result_vector[0]["parent_id"].c_str());
        if (parent == second)
        {
            result = 1;
            break;
        }
    }
    p.disconnect();
    return result;
}
int remove_file(int file_dir_id, int current_root_id, bool is_checked, string &message)
{
    my_database p;
    p.connect();
    sprintf(p.sql, "select fid,did from FileDirectoryMap where id=%d", file_dir_id);
    if (p.execute() == -1)
    {
        message = "数据查询出错";
        return -500;
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        message = "试图删除不存在的文件";
        return -400;
    }
    int file_id = atoi(p.result_vector[0]["fid"].c_str());
    int did = atoi(p.result_vector[0]["did"].c_str());
    int check;
    if (!is_checked)
    {
        check = is_child(did, current_root_id, message);
        if (check < 0)
        {
            return check;
        }
        if (!check && did != current_root_id)
        {
            message = "不可删除他人的文件";
            return -400;
        }
    }
    sprintf(p.sql, "delete from FileDirectoryMap where id=%d", file_dir_id);
    if (p.execute() == -1)
    {
        message = "数据库删除出现错误";
        return -500;
    }
    sprintf(p.sql, "select link_num from FileEntity where id=%d", file_id);
    if (p.execute() == -1)
    {
        message = "数据库查询出现问题";
        return -500;
    }
    p.get();
    int link_num = atoi(p.result_vector[0]["link_num"].c_str());
    int sub_link_num, sub_id;
    string sub_path;
    if (link_num == 0)
    {
        //?
    }
    else if (link_num > 1)
    {
        sprintf(p.sql, "update FileEntity set link_num=link_num-1 where id=%d", file_id);
        if (p.execute() == -1)
        {
            message = "数据库更新出现问题";
            return -500;
        }
    }
    else
    {
        sprintf(p.sql, "select FileFragmentEntity.id as id,FileFragmentEntity.MD5 as MD5,FileFragmentEntity.link_num as link_num\
         from FileFragmentMap join FileFragmentEntity on FileFragmentEntity.id=FileFragmentMap.fgid\
            where FileFragmentMap.fid=%d",
                file_id);
        if (p.execute() == -1)
        {
            message = "数据库查询出现问题";
            return -500;
        }
        p.get();
        vector<map<string, string>> result = p.result_vector;
        sprintf(p.sql, "delete from FileFragmentMap where fid=%d", file_id);
        if (p.execute() == -1)
        {
            message = "数据库删除出现问题";
            return -500;
        }
        for (size_t i = 0; i < result.size(); i++)
        {
            sub_link_num = atoi(result[i]["link_num"].c_str());
            sub_id = atoi(result[i]["id"].c_str());
            if (sub_link_num > 1)
            {
                sprintf(p.sql, "update FileFragmentEntity set link_num=link_num-1 where id=%d", sub_id);
                if (p.execute() == -1)
                {
                    message = "数据库更新出现问题";
                    return -500;
                }
            }
            else
            {
                sprintf(p.sql, "delete from FileFragmentEntity where id=%d", sub_id);
                if (p.execute() == -1)
                {
                    message = "数据库删除出现问题";
                    return -500;
                }
                sub_path = "../pool/" + result[i]["MD5"];
                if (file_exists(sub_path))
                {
                    remove(sub_path.c_str());
                }
                else
                {
                    message = "数据库存储了不应存在的文件碎片" + sub_path;
                    return -500;
                }
            }
        }
        sprintf(p.sql, "delete from FileEntity where id=%d", file_id);
        if (p.execute() == -1)
        {
            message = "数据库删除出现问题";
            return -500;
        }
    }
    message = "删除成功";
    p.disconnect();
    return 0;
}
string get_dir_new_name(int did, int pid, int &statusCode)
{

    int num = 0;
    my_database p;
    p.connect();
    if (did == pid)
    {
        sprintf(p.sql, "select parent_id from DirectoryEntity where id=%d", did);
        if (p.execute() == -1)
        {
            statusCode = 500;
            return "数据库查询错误";
        }
        p.get();
        pid = atoi(p.result_vector[0]["parent_id"].c_str());
    }
    sprintf(p.sql, "select dname from DirectoryEntity where id=%d", did);
    if (p.execute() == -1)
    {
        statusCode = 500;
        return "数据库查询错误";
    }
    p.get();
    string dname = p.result_vector[0]["dname"], fin_name = dname;
    sprintf(p.sql, "select dname from DirectoryEntity where parent_id=%d and id!=%d", pid, pid);
    if (p.execute() == -1)
    {
        statusCode = 500;
        return "数据库查询出错,请联系管理员检查";
    }
    p.get();
    set<string> names;
    for (size_t i = 0; i < p.result_vector.size(); i++)
    {
        names.insert(p.result_vector[i]["dname"]);
    }
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
    statusCode = 0;
    p.disconnect();
    return fin_name;
}
string get_file_new_name(string fname, int pid, int &statusCode)
{
    my_database p;
    p.connect();
    set<string> names;
    sprintf(p.sql, "select fname from FileDirectoryMap where did=%d", pid);
    if (p.execute() == -1)
    {
        statusCode = 500;
        return "数据库查询出错";
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
    set<string>().swap(names);
    statusCode = 0;
    return fin_name;
}