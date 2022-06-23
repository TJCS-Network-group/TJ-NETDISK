#include <mysql/mysql.h> // mysqlÃÿ”–
#include <map>
#include <vector>
#include <string>
using namespace std;
class my_database
{
protected:
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;

public:
    vector<map<string, string>> result_vector;
    my_database();
    void connect(const char *host, const char *user, const char *passwd, const char *db,
                 unsigned int port, const char *unix_socket, unsigned long clientflag);
    int set_character(const char *character_set_name);
    int execute(const char *query);
    int get();
    void show();
    void disconnect();
    ~my_database();
};