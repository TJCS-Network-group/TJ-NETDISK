#include <mysql/mysql.h> // mysql����
#include <map>
#include <cstdio>
#include <vector>
#include <string>
using namespace std;
class my_database
{
protected:
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;
    static const char *host;
    static const char *user;
    static const char *passwd;
    static const char *db;
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