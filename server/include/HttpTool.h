//���ֹ��ߺ���
#pragma once
#include <map>
#include <string>
std::string FileToStr(std::string filepath);
std::string bool_to_string(bool _success);
std::string To_gbk(std::string utf8_str);
int popen_cmd(std::string cmd, std::string &string, const int max_len);
int Parse_params(const std::string &params_str, std::map<std::string, std::string> &params);
