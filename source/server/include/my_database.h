#include <cstdio>
#include <map>
#include <mysql/mysql.h> // mysql特有
#include <set>
#include <string>
#include <vector>
using namespace std;
struct database_variable
{
    string host;
    string user;
    string passwd;
    string db;
};

extern database_variable my_database_variable;
class my_database
{
protected:
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;
    string host;
    string user;
    string passwd;
    string db;
    static const unsigned int port;
    static const char *unix_socket;
    static const unsigned long clientflag;
    static const char *character_set_name;

public:
    char sql[1024];
    vector<map<string, string>>
        result_vector;
    my_database();
    void connect();
    int execute();
    int get();
    void show();
    void disconnect();
    ~my_database();
};
int get_root_id_by_user(int user_id, string &message);
//表明first是不是second的子文件夹
int is_child(int first, int second, string &message);
int remove_file(int file_dir_id, int current_root_id, bool is_checked, string &message);
string get_dir_new_name(int did, int pid, int &statusCode);
string get_file_new_name(string fname, int pid, int &statusCode);
int get_tree(int did, string &message, bool search_file = false);