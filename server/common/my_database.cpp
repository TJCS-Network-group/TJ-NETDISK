#include "../include/my_database.h"
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