// API函数统一以<method>_<route>命名
#include "../include/HttpServer.h"
#include "../include/HttpTool.h"
#include "../include/my_database.h"
#include <exception>
using namespace std;
// API的具体实现
HttpResponse POST_login(HttpRequest &req)
{ //要发送的JSON
    string user_name;
    string password;
    try
    {
        user_name = req.json["user_name"].as_string();
        password = req.json["password"].as_string();
    }
    catch (exception e)
    {
        return make_response_json(400, "请求格式不对");
    }

    bool password_correct = false;
    HttpResponse resp;
    //查找用户实体表项
    int user_id;
    if (req.current_user_id != 0)
    {
        resp = make_response_json(200, "您已经登录");
        return resp;
    }
    my_database p;
    p.connect();
    sprintf(p.sql, "select id,password_hash from UserEntity where user_name=\"%s\"", user_name.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(404, "该用户不存在");
    }
    else
    {
        string cmd = "echo -n " + password + " | md5sum", result;
        if (popen_cmd(cmd, result, 32 + 1) != -1)
        {
            // cout << result << endl;
            // cout << p.result_vector[0]["password_hash"] << endl;
            password_correct = (result == p.result_vector[0]["password_hash"]);
        }
        else
        {
            return make_response_json(500, "popen_err");
        }
    }
    //如果密码正确
    if (password_correct)
    {
        user_id = atoi(p.result_vector[0]["id"].c_str());
        resp = make_response_json(200);
        srand(time(NULL));
        string cmd = "echo -n " + user_name + to_string(rand()) + " | md5sum", result;
        if (popen_cmd(cmd, result, 32 + 1) != -1)
        {

            // 通过md5加密{user_name,rand}并保存在session
            // 对于每个用户的每个会话，这个md5值应该要一致
            session[user_id] = result;
            // cout << "Login session: " << endl;
            /*
            for (auto i : session)
            {
                cout << i.first << ":" << i.second << endl;
            }*/
            resp.setHeader("Set-Cookie: sessionid=" + to_string(user_id) + "|" + result + "; HttpOnly; Path=/;");
            //给client的set-cookie，没设置expires,即仅限于此次回话
        }
        else
        {
            return make_response_json(500, "popen_err");
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
{ //要发送的JSON
    string user_name;
    string password;
    try
    {
        user_name = req.json["user_name"].as_string();
        password = req.json["password"].as_string();
    }
    catch (exception e)
    {
        return make_response_json(400, "请求格式不对");
    }
    HttpResponse resp;
    //添加用户实体表项
    my_database p;
    p.connect();
    sprintf(p.sql, "select id from UserEntity where user_name=\"%s\"", user_name.c_str());
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
    }
    p.get();
    if (p.result_vector.size() > 0)
    {
        resp = make_response_json(400, "此用户名已被人使用");
    }
    else
    {
        string cmd = "echo -n " + password + " | md5sum", result;
        if (popen_cmd(cmd, result, 32 + 1) != -1)
        {

            sprintf(p.sql, "select max(id) as max_id from DirectoryEntity");
            if (p.execute() == -1)
            {
                return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
            }
            p.get();
            int max_id = atoi(p.result_vector[0]["max_id"].c_str()) + 1;
            sprintf(p.sql, "insert into DirectoryEntity(id,dname,parent_id) value (%d,\"%s\",%d)", max_id, "root", max_id);
            if (p.execute() == -1)
            {
                return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
            }
            sprintf(p.sql,
                    "insert into UserEntity(user_name,password_hash,root_dir_id) \
            value (\"%s\",\"%s\",%d)",
                    user_name.c_str(), result.c_str(), max_id);
            if (p.execute() == -1)
            {
                return make_response_json(500, "数据库查询出错,请联系管理员解决问题");
            }
            resp = make_response_json(200);
        }
        else
        {
            resp = make_response_json(500, "popen_err");
        }
    }
    p.disconnect();
    return resp;
}

HttpResponse GET_logout(HttpRequest &req)
{ //要发送的JSON
    HttpResponse resp = make_response_json(200);
    //从session中移除
    if (session.count(req.current_user_id))
        session.erase(req.current_user_id);
    resp = make_response_json(200);
    resp.setHeader("Set-Cookie: sessionid=; Expires=Thu, 01 Jan 1970 00:00:00 GMT; Max-Age=0; Path=/");
    return resp;
}
HttpResponse GET_myinfo(HttpRequest &req)
{
    if (req.current_user_id == 0)
    {
        return make_response_json(400, "当前用户未登录");
    }
    my_database p;
    p.connect();
    sprintf(p.sql, "select user_name from UserEntity where id=%d", req.current_user_id);
    if (p.execute() == -1)
    {
        return make_response_json(500, "数据库查询出现异常");
    }
    p.get();
    if (p.result_vector.size() == 0)
    {
        return make_response_json(400, "登录了不存在的用户");
    }
    return make_response_json(200, "当前用户信息为", "{\"user_name\":\"" + p.result_vector[0]["user_name"] + "\"}");
}
HttpResponse GET_get_identity(HttpRequest &req)
{
    return make_response_json(200, "当前用户登录情况为", "{\"login\":" + bool_to_string(req.current_user_id != 0) + '}');
}