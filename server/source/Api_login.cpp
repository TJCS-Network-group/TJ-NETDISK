// API����ͳһ��<method>_<route>����
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
using namespace std;
// API�ľ���ʵ��
HttpResponse POST_login(HttpRequest &req)
{ //Ҫ���͵�JSON
    string user_name = req.json["user_name"].as_string();
    string password = req.json["password"].as_string();
    cout << "current_user:" << req.current_user_id << endl;
    bool password_correct;
    HttpResponse resp;
    //�����û�ʵ�����
    int user_id;
    if (req.current_user_id != 0)
    {
        resp = make_response_json(200, "���Ѿ���¼");
        return resp;
    }
    my_database p;
    p.connect();
    sprintf(p.sql, "select id,password_hash from UserEntity where user_name=\"%s\"", user_name.c_str());
    cout << p.sql << endl;
    p.execute();
    p.get();
    if (p.result_vector.size() == 0)
    {
        resp = make_response_json(404, "���û�������");
    }
    else
    {
        string cmd = "echo " + password + " | md5sum";
        cout << cmd << endl;
        FILE *fp = popen(cmd.c_str(), "r");
        if (fp != NULL)
        {
            char buf[33];
            string md5 = fgets(buf, 33, fp);
            pclose(fp);
            cout << md5 << endl;
            cout << p.result_vector[0]["password_hash"] << endl;
            password_correct = (md5 == p.result_vector[0]["password_hash"]);
        }
        else
        {
        }
    }
    //���������ȷ
    if (password_correct)
    {
        cout << p.result_vector[0]["id"].c_str() << endl;
        user_id = atoi(p.result_vector[0]["id"].c_str());
        cout << user_id << endl;
        resp = make_response_json(200);

        srand(time(NULL));
        string cmd = "echo " + user_name + to_string(rand()) + " | md5sum";
        cout << cmd << endl;
        FILE *fp = popen(cmd.c_str(), "r");
        if (fp != NULL)
        {
            char buf[33];
            string md5 = fgets(buf, 33, fp);
            pclose(fp);

            // ͨ��md5����{user_name,rand}��������session
            // ����ÿ���û���ÿ���Ự�����md5ֵӦ��Ҫһ��
            cout << md5 << endl;
            session[user_id] = md5;
            cout << "Login session: " << endl;
            for (auto i : session)
            {
                cout << i.first << ":" << i.second << endl;
            }
            resp.setHeader("Set-Cookie: remember_token=" + to_string(user_id) + "|" + md5);
            //��client��set-cookie��û����expires,�������ڴ˴λػ�
        }
    }
    else
    {
        resp = make_response_json(401);
    }
    p.disconnect();
    return resp;
}

HttpResponse POST_register(HttpRequest &req)
{ //Ҫ���͵�JSON
    string user_name = req.json["user_name"].as_string();
    string password = req.json["password"].as_string();
    HttpResponse resp;
    //����û�ʵ�����
    my_database p;
    p.connect();
    sprintf(p.sql, "select id from UserEntity where user_name=\"%s\"", user_name.c_str());
    p.execute();
    p.get();
    if (p.result_vector.size() > 0)
    {
        resp = make_response_json(400, "���û����ѱ���ʹ��");
    }
    else
    {
        string cmd = "echo " + password + " | md5sum";
        cout << cmd << endl;
        FILE *fp = popen(cmd.c_str(), "r");
        if (fp != NULL)
        {
            char buf[33];
            string md5 = fgets(buf, 33, fp);
            pclose(fp);
            cout << md5 << endl;
            //
            sprintf(p.sql, "select max(id) as max_id from DirectoryEntity");
            p.execute();
            p.get();
            int max_id = atoi(p.result_vector[0]["max_id"].c_str()) + 1;
            sprintf(p.sql, "insert into DirectoryEntity(id,dname,parent_id) value (%d,\"%s\",%d)", max_id, "root", max_id);
            p.execute();
            sprintf(p.sql,
                    "insert into UserEntity(user_name,password_hash,root_dir_id) \
            value (\"%s\",\"%s\",%d)",
                    user_name.c_str(), md5.c_str(), max_id);
            p.execute();
            resp = make_response_json(200);
        }
        else
        {
            resp = make_response_json(500);
        }
    }
    p.disconnect();
    return resp;
}

HttpResponse GET_logout(HttpRequest &req)
{ //Ҫ���͵�JSON
    HttpResponse resp = make_response_json(200);
    //��session���Ƴ�
    if (session.count(req.current_user_id))
        session.erase(req.current_user_id);
    resp = make_response_json(200);
    resp.setHeader("Set-Cookie: remember_token=; Expires=Thu, 01 Jan 1970 00:00:00 GMT; Max-Age=0; Path=/");
    return resp;
}