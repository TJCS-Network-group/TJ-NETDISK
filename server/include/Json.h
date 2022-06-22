#pragma once
/*
��ʼΪһ����client�˵õ���string
Json�﷨��

string->map<string,string>
string->vector<string>
string->int
string->bool

Ҫʵ�ֵĺ��Ĺ���Ϊǰ����

*/
#include <iostream>
#include <map>
#include <string>
#include <vector>
struct JSON {
    std::string raw_json;

    JSON(std::string s = "") {
        for (char i : s) {
            if (i != ' ' && i != '\n' && i != '\t' && i != '\r') {
                raw_json.push_back(i);  //ȥ���س��ո���ַ�
            }
        }
    }

    std::string as_string() { return raw_json.substr(1, raw_json.size() - 2); };  //��Ҫ��ȥ��˫����
    int as_int() { return atoi(raw_json.c_str()); };
    // float as_float();
    bool as_bool() {
        if (raw_json == "true")
            return true;
        else
            return false;
    };
};