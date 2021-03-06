//各种工具函数
#pragma once
#include <map>
#include <string>
#define FRAGMENT_SIZE (4 * 1024 * 1024)
int FileToStr(std::string filepath, std::string &result);
std::string bool_to_string(bool _success);
std::string To_gbk(std::string utf8_str);
bool file_exists(const std::string &name);
bool check_password(const std::string &password);
int popen_cmd(std::string cmd, std::string &string, const int max_len);
int Parse_params(const std::string &params_str, std::map<std::string, std::string> &params);
