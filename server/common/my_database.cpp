#include "../include/my_database.h"
#include <iostream> // cin,coutµÈ
#include <iomanip>  // setwµÈ
#include <cstdlib>
using namespace std;
my_database::my_database()
{
    this->mysql = nullptr;
    this->result = nullptr;
    this->row = nullptr;
}
my_database::~my_database()
{
    if (this->mysql != nullptr)
    {
        mysql_free_result(this->result);
        mysql_close(this->mysql);
    }
    this->mysql = nullptr;
    this->result = nullptr;
    this->row = nullptr;
}

void my_database::connect(const char *host, const char *user, const char *passwd, const char *db,
                          unsigned int port, const char *unix_socket, unsigned long clientflag)
{
    if (this->mysql == nullptr)
    {
        this->mysql = mysql_init(nullptr);
        if (this->mysql == nullptr)
        {
            cout << "mysql_init failed" << endl;
            exit(-1);
        }
    }
    if (mysql_real_connect(this->mysql, host, user, passwd, db, port, unix_socket, clientflag) == nullptr)
    {
        cout << "mysql_real_connect failed(" << mysql_error(this->mysql) << ")" << endl;
        exit(-1);
    }
    cout << "connect " << db << " success" << endl;
}
void my_database::disconnect()
{
    if (this->mysql != nullptr)
    {
        mysql_free_result(this->result);
        mysql_close(this->mysql);
    }
    this->result = nullptr;
    this->row = nullptr;
    this->mysql = nullptr;
}
int my_database::set_character(const char *character_set_name)
{
    if (mysql_set_character_set(this->mysql, character_set_name))
    {
        cout << "set character " << character_set_name << " failed" << endl;
        exit(-1);
    }
    return 0;
}
int my_database::execute(const char *query)
{
    vector<map<string, string>>().swap(this->result_vector);
    if (mysql_query(this->mysql, query))
    {
        cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
        exit(-1);
    }
    return 0;
}
int my_database::get()
{
    vector<map<string, string>>().swap(this->result_vector);
    if ((result = mysql_store_result(mysql)) == NULL)
    {
        cout << "mysql_store_result failed" << endl;
        exit(-1);
    }
    map<string, string> s;
    while ((this->row = mysql_fetch_row(this->result)) != NULL)
    {
        map<string, string>().swap(s);
        for (int i = 0; i < int(this->result->field_count); i++)
        {
            s[(this->result->fields + i)->name] = this->row[i];
        }
        result_vector.push_back(s);
    }
    return 0;
}
void my_database::show()
{
    for (size_t i = 0; i < this->result_vector.size(); i++)
    {
        for (auto j = result_vector[i].begin(); j != result_vector[i].end(); j++)
        {
            cout << j->first << ":" << j->second << endl;
        }
    }
}