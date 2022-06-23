#pragma once
/*
初始为一个从client端得到的string

Json语法：
string->JSON
JSON->map<string,JSON>
JSON->vector<JSON>
JSON->int
JSON->bool
JSON->string
要实现的核心功能为前两条

*/
#include <iostream>
#include <map>
#include <string>
#include <vector>
struct JSON {
    std::string raw_json;
    void clear() { raw_json.clear(); }
    JSON(std::string s = "") {
        for (char i : s) {
            if (i != ' ' && i != '\n' && i != '\t' && i != '\r') {
                raw_json.push_back(i);  //去除回车空格等字符
            }
        }
    }
    friend std::ostream &operator<<(std::ostream &os, const JSON &a);  //重载输出流
    JSON operator[](const std::string &key);                           //重载
    JSON operator[](int index);                                        //重载
    std::map<std::string, JSON> as_map();
    std::vector<JSON> as_vector();
    std::string as_string() { return raw_json.substr(1, raw_json.size() - 2); };  //主要是去除双引号
    int as_int() { return atoi(raw_json.c_str()); };
    // float as_float();
    bool as_bool() {
        if (raw_json == "true")
            return true;
        else
            return false;
    };
};