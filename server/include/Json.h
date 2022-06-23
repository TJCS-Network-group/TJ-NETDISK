#pragma once
/*
��ʼΪһ����client�˵õ���string

Json�﷨��
string->JSON
JSON->map<string,JSON>
JSON->vector<JSON>
JSON->int
JSON->bool
JSON->string
Ҫʵ�ֵĺ��Ĺ���Ϊǰ����

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
                raw_json.push_back(i);  //ȥ���س��ո���ַ�
            }
        }
    }
    friend std::ostream &operator<<(std::ostream &os, const JSON &a);  //���������
    JSON operator[](const std::string &key);                           //����
    JSON operator[](int index);                                        //����
    std::map<std::string, JSON> as_map();
    std::vector<JSON> as_vector();
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