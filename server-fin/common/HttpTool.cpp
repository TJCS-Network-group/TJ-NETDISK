#include <fstream>
#include <iostream>
#include <iterator>
#include <locale.h>
#include <map>
#include <regex>
#include <string>
#include <vector>
using namespace std;

//�����ļ�ת��string
int FileToStr(string filepath, string &result)
{
    fstream f(filepath, ios::in | ios::binary);
    if (f.good())
    {
        f.unsetf(ios::skipws); // �ر�inputFile�ĺ��Կո��־,�����ļ��еı����ո�
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

//ת��form-data��ʽ
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

// boolתstring
string bool_to_string(bool _success)
{
    if (_success)
        return "true";
    else
        return "false";
}

/**
 * DESCRIPTION: ʵ����utf8���뵽gbk�����ת��
 *
 * Input: gbkStr,ת������ַ���;  srcStr,��ת�����ַ���; maxGbkStrlen, gbkStr����󳤶�
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

    //�����Ƚ�utf8����ת��Ϊunicode����
    if (NULL == setlocale(LC_ALL, "zh_CN.utf8")) //����ת��Ϊunicodeǰ����,��ǰΪutf8����
    {
        printf("Bad Parameter\n");
        return -1;
    }

    int unicodeLen = mbstowcs(NULL, srcStr, 0); //����ת����ĳ���
    if (unicodeLen <= 0)
    {
        printf("Can not Transfer!!!\n");
        return -1;
    }
    wchar_t *unicodeStr = (wchar_t *)calloc(sizeof(wchar_t), unicodeLen + 1);
    mbstowcs(unicodeStr, srcStr, strlen(srcStr)); //��utf8ת��Ϊunicode

    //��unicode����ת��Ϊgbk����
    if (NULL == setlocale(LC_ALL, "zh_CN.gbk")) //����unicodeת�������,��ǰΪgbk
    {
        printf("Bad Parameter\n");
        return -1;
    }
    int gbkLen = wcstombs(NULL, unicodeStr, 0); //����ת����ĳ���
    if (gbkLen <= 0)
    {
        printf("Can not Transfer!!!\n");
        return -1;
    }
    else if (gbkLen >= maxGbkStrlen) //�жϿռ��Ƿ��㹻
    {
        printf("Dst Str memory not enough\n");
        return -1;
    }
    wcstombs(gbkStr, unicodeStr, gbkLen);
    gbkStr[gbkLen] = 0; //��ӽ�����

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
    else //����Ҫת
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