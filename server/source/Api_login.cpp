// API����ͳһ��<method>_<route>����
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
using namespace std;
// API�ľ���ʵ��
HttpResponse POST_login(HttpRequest& req) {  //Ҫ���͵�JSON
    string user_name = req.json["user_name"].as_string();
    string password = req.json["password"].as_string();
    bool password_correct = true;
    //�����û�ʵ�����
    int user_id = 1;

    HttpResponse resp;
    //���������ȷ
    if (password_correct) {
        resp = make_response_json(200);

        srand(time(NULL));
        string cmd = "echo " + user_name + to_string(rand()) + " | md5sum";
        cout << cmd << endl;
        FILE* fp = popen(cmd.c_str(), "r");
        if (fp != NULL) {
            char buf[33];
            string md5 = fgets(buf, 33, fp);
            pclose(fp);

            // ͨ��md5����{user_name,rand}��������session
            // ����ÿ���û���ÿ���Ự�����md5ֵӦ��Ҫһ��
            cout << md5 << endl;
            session[user_id] = md5;
            cout << "Login session: " << endl;
            for (auto i : session) {
                cout << i.first << ":" << i.second << endl;
            }
            resp.setHeader("Set-Cookie: user_id=" + to_string(user_id) + "|" + md5);
            //��client��set-cookie��û����expires,�������ڴ˴λػ�
        }
    } else {
        resp = make_response_json(401);
    }
    return resp;
}

HttpResponse POST_register(HttpRequest& req) {  //Ҫ���͵�JSON
    string user_name = req.json["user_name"].as_string();
    string password = req.json["password"].as_string();
    HttpResponse resp = make_response_json(200);
    //����û�ʵ�����

    resp = make_response_json(200);
    return resp;
}

HttpResponse GET_logout(HttpRequest& req) {  //Ҫ���͵�JSON
    HttpResponse resp = make_response_json(200);
    //��session���Ƴ�
    if (session.count(req.current_user_id)) session.erase(req.current_user_id);
    resp = make_response_json(200);
    return resp;
}