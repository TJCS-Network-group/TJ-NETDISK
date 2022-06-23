// API函数统一以<method>_<route>命名
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
using namespace std;
// API的具体实现
HttpResponse POST_login(HttpRequest& req) {  //要发送的JSON
    string user_name = req.json["user_name"].as_string();
    string password = req.json["password"].as_string();
    bool password_correct = true;
    //查找用户实体表项
    int user_id = 1;

    HttpResponse resp;
    //如果密码正确
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

            // 通过md5加密{user_name,rand}并保存在session
            // 对于每个用户的每个会话，这个md5值应该要一致
            cout << md5 << endl;
            session[user_id] = md5;
            cout << "Login session: " << endl;
            for (auto i : session) {
                cout << i.first << ":" << i.second << endl;
            }
            resp.setHeader("Set-Cookie: user_id=" + to_string(user_id) + "|" + md5);
            //给client的set-cookie，没设置expires,即仅限于此次回话
        }
    } else {
        resp = make_response_json(401);
    }
    return resp;
}

HttpResponse POST_register(HttpRequest& req) {  //要发送的JSON
    string user_name = req.json["user_name"].as_string();
    string password = req.json["password"].as_string();
    HttpResponse resp = make_response_json(200);
    //添加用户实体表项

    resp = make_response_json(200);
    return resp;
}

HttpResponse GET_logout(HttpRequest& req) {  //要发送的JSON
    HttpResponse resp = make_response_json(200);
    //从session中移除
    if (session.count(req.current_user_id)) session.erase(req.current_user_id);
    resp = make_response_json(200);
    return resp;
}