#include <fstream>
#include <iostream>
#include <iterator>
#include <locale.h>
#include <map>
#include <regex>
#include <string>
#include <vector>
using namespace std;

//本地文件转成string
int FileToStr(string filepath, string &result)
{
    fstream f(filepath, ios::in | ios::binary);
    if (f.good())
    {
        f.unsetf(ios::skipws); // 关闭inputFile的忽略空格标志,可以文件中的保留空格
        istream_iterator<char> iter(f);
        string s(iter, istream_iterator<char>());
        result = s;
        return 0;
    }
    else
    {
        cout << "Can't open file!Srcfile path error!" << endl;
        return -1;
    }
}

//转成form-data格式
string MapToString(map<string, string> data)
{
    string res = "";
    map<string, string>::iterator iter;
    for (iter = data.begin(); iter != data.end(); ++iter)
    {
        res += iter->first + "=" + iter->second + "&";
    }
    res.erase(res.end() - 1);
    return res;
}

// bool转string
string bool_to_string(bool _success)
{
    if (_success)
        return "true";
    else
        return "false";
}

/**
 * DESCRIPTION: 实现由utf8编码到gbk编码的转换
 *
 * Input: gbkStr,转换后的字符串;  srcStr,待转换的字符串; maxGbkStrlen, gbkStr的最大长度
 * Output: gbkStr
 * Returns: -1,fail;>0,success
 *
 */
int utf82gbk(char *gbkStr, const char *srcStr, int maxGbkStrlen)
{
    if (NULL == srcStr)
    {
        printf("Bad Parameter\n");
        return -1;
    }

    //首先先将utf8编码转换为unicode编码
    if (NULL == setlocale(LC_ALL, "zh_CN.utf8")) //设置转换为unicode前的码,当前为utf8编码
    {
        printf("Bad Parameter\n");
        return -1;
    }

    int unicodeLen = mbstowcs(NULL, srcStr, 0); //计算转换后的长度
    if (unicodeLen <= 0)
    {
        printf("Can not Transfer!!!\n");
        return -1;
    }
    wchar_t *unicodeStr = (wchar_t *)calloc(sizeof(wchar_t), unicodeLen + 1);
    mbstowcs(unicodeStr, srcStr, strlen(srcStr)); //将utf8转换为unicode

    //将unicode编码转换为gbk编码
    if (NULL == setlocale(LC_ALL, "zh_CN.gbk")) //设置unicode转换后的码,当前为gbk
    {
        printf("Bad Parameter\n");
        return -1;
    }
    int gbkLen = wcstombs(NULL, unicodeStr, 0); //计算转换后的长度
    if (gbkLen <= 0)
    {
        printf("Can not Transfer!!!\n");
        return -1;
    }
    else if (gbkLen >= maxGbkStrlen) //判断空间是否足够
    {
        printf("Dst Str memory not enough\n");
        return -1;
    }
    wcstombs(gbkStr, unicodeStr, gbkLen);
    gbkStr[gbkLen] = 0; //添加结束符

    free(unicodeStr);
    return gbkLen;
}

string To_gbk(string utf8_str)
{
    if (utf8_str == "")
    {
        return "";
    }
    int maxGbkLen = sizeof(char) * utf8_str.length() * 2;
    char *buf = (char *)malloc(maxGbkLen);
    int gbkLen = utf82gbk(buf, utf8_str.c_str(), maxGbkLen);
    if (gbkLen != -1)
    {
        string gbk_str(buf, gbkLen);
        free(buf);
        return gbk_str;
    }
    else //不需要转
    {
        free(buf);
        return utf8_str;
    }
}
int popen_cmd(string cmd, string &result, const int max_result_len)
{
    if (cmd == "")
    {
        result = "";
        return -1;
    }
    FILE *fp = popen(cmd.c_str(), "r");
    int ans;
    if (fp != NULL)
    {
        char buf[max_result_len];
        result = fgets(buf, max_result_len, fp);
        pclose(fp);
        ans = result.size();
    }
    else
    {
        cout << "Popen Error,cmd: " << cmd << endl;
        cout << errno << strerror(errno) << endl;
        ans = -1;
    }
    return ans;
}
int encoding_url(const string &url, string &result)
{
    if (url == "")
    {
        result = "";
        return 0;
    }
    string cmd = "echo -n \"" + url + "\" |tr -d \'\\n\' |od -An -tx1|tr \' \' %";
    if (popen_cmd(cmd, result, 4 * url.size()) == -1)
    {
        return result.size();
    }
    else
        return -1;
}
int decoding_url(const string &url, string &result)
{
    if (url == "")
    {
        result = "";
        return 0;
    }

    string cmd = "printf %b $(echo -n \"" + url + "\" | sed \'s/\\\\/\\\\\\\\/g;s/\\(%\\)\\([0-9a-fA-F][0-9a-fA-F]\\)/\\\\x\\2/g\')";

    int size = popen_cmd(cmd, result, 4 * url.size());

    result = To_gbk(result);
    if (size != -1)
    {
        return result.size();
    }
    else
        return -1;
}

int Parse_params(const string &params_str, map<string, string> &params)
{
    string key, value, newkey, newvalue;
    bool find_key = true;
    for (size_t i = 0; i < params_str.size(); ++i)
    {
        if (params_str[i] == '&')
        {
            find_key = true;
            if (decoding_url(key, newkey) != -1 && decoding_url(value, newvalue) != -1)
                params.insert({newkey, newvalue});
            else
                return -1;
            key.clear();
            value.clear();
        }
        else if (params_str[i] == '=')
        {
            find_key = false;
        }
        else
        {
            if (find_key)
                key.push_back(params_str[i]);
            else
            {
                value.push_back(params_str[i]);
            }
        }
    }
    if (decoding_url(key, newkey) != -1 && decoding_url(value, newvalue) != -1)
        params.insert({newkey, newvalue});
    else
        return -1;
    return 0;
}
bool check_password(const string &password)
{
    if (password.length() < 12)
    {
        return false;
    }
    bool small = false, big = false, num = false, other = false;
    int class_number = 0;
    for (int i = 0; i != password.length(); i++)
    {
        if (password[i] >= 'a' && password[i] <= 'z')
        {
            if (!small)
            {
                small = true;
                class_number += 1;
                if (class_number == 3)
                    break;
            }
        }
        else if (password[i] >= 'A' && password[i] <= 'Z')
        {
            if (!big)
            {
                big = true;
                class_number += 1;
                if (class_number == 3)
                    break;
            }
        }
        else if (password[i] >= '0' && password[i] <= '9')
        {
            if (!num)
            {
                num = true;
                class_number += 1;
                if (class_number == 3)
                    break;
            }
        }
        else
        {
            if (!other)
            {
                other = true;
                class_number += 1;
                if (class_number == 3)
                    break;
            }
        }
    }
    return class_number == 3;
}