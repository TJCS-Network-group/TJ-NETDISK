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
string FileToStr(string filepath)
{
    fstream f(filepath, ios::in | ios::binary);
    if (f.good())
    {
        f.unsetf(ios::skipws); // �ر�inputFile�ĺ��Կո��־,�����ļ��еı����ո�
        istream_iterator<char> iter(f);
        string s(iter, istream_iterator<char>());
        return s;
    }
    else
    {
        cout << "Can't open file!Srcfile path error!" << endl;
        exit(EXIT_FAILURE);
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
    int maxGbkLen = sizeof(char) * utf8_str.length() * 2;
    char *buf = (char *)malloc(maxGbkLen);
    int gbkLen = utf82gbk(buf, utf8_str.c_str(), maxGbkLen);
    string gbk_str(buf, gbkLen);
    free(buf);
    if (gbkLen != -1)
        return gbk_str;
    else //����Ҫת
        return utf8_str;
}
int popen_cmd(string cmd, string &result)
{
    FILE *fp = popen(cmd.c_str(), "r");
    int ans;
    if (fp != NULL)
    {
        char buf[33];
        result = fgets(buf, 33, fp);
        pclose(fp);
        ans = 0;
    }
    else
    {
        ans = 1;
    }
    return ans;
}