#include "../include/Json.h"

#include <stack>
using namespace std;
std::map<std::string, JSON> JSON_to_map(JSON json) {
    string raw_json = json.raw_json;
    map<string, JSON> res;
    string key, value;
    stack<char> find_value_stack;
    bool find_key = 0, find_value = 0;
    for (size_t i = 1; i < raw_json.size(); ++i) {
        if (find_value == true) {
            if (find_value_stack.size() == 0 && raw_json[i] == '}') {
                res[key] = JSON(value);
                // cout << "key: " << key << endl << "value: " << value << endl;
                break;
            } else if (find_value_stack.size() == 0 && raw_json[i] == ',') {
                res[key] = JSON(value);
                // cout << "key: " << key << endl << "value: " << value << endl;
                value.clear();
                find_value = false;
            } else {
                if (raw_json[i] == '[' || raw_json[i] == '{') {
                    find_value_stack.push(raw_json[i]);
                } else if (raw_json[i] == '}' || raw_json[i] == ']') {
                    find_value_stack.pop();
                }
                value.push_back(raw_json[i]);
            }
        }
        if (raw_json[i] == '\"' && find_value == false) {
            if (find_key == true) {
                // key准备好了,准备找value
                find_key = false;
                ++i;  //跳过冒号
                find_value = true;
            } else {  //准备找key
                find_key = true;
                key.clear();
            }
        } else if (find_key == true) {
            key.push_back(raw_json[i]);
        }
    }
    return res;
}
std::vector<JSON> JSON_to_vector(JSON json) {
    vector<JSON> res;
    string raw_json = json.raw_json;
    stack<char> find_item_stack;
    string target_json;
    for (size_t i = 1; i < raw_json.size(); ++i) {
        if (raw_json[i] == ',' && find_item_stack.size() == 0) {
            ++i;
            res.push_back(JSON(target_json));
            target_json.clear();
        }
        if (raw_json[i] == ']' && find_item_stack.size() == 0) {
            res.push_back(JSON(target_json));
            target_json.clear();
        }
        target_json.push_back(raw_json[i]);
        if (raw_json[i] == '[' || raw_json[i] == '{') {
            find_item_stack.push(raw_json[i]);
        } else if (raw_json[i] == '}' || raw_json[i] == ']') {
            find_item_stack.pop();
        }
    }
    return res;
}

std::map<std::string, JSON> JSON::as_map() { return JSON_to_map(*this); }
std::vector<JSON> JSON::as_vector() { return JSON_to_vector(*this); }
std::ostream &operator<<(std::ostream &os, JSON &a) {
    os << a.raw_json << endl;
    return os;
}
JSON JSON::operator[](const std::string &key) { return this->as_map()[key]; }
JSON JSON::operator[](int index) { return this->as_vector()[index]; }

//参考用例
void test() {
    string send_json =
        "{\n\
\"data\": [{\"md5\":1},{\"md5\":3}], \n\
\"message\": \"请求格式错误\", \n\
\"statusCode\": 400, \n\
\"success\": false\n}\
";
    cout << send_json << endl;
    JSON tep(send_json);
    std::map<string, JSON> tep_map = JSON_to_map(tep);
    cout << "message: " << tep_map["message"].as_string() << endl;
    cout << "size: " << JSON_to_vector(tep_map["data"]).size() << endl;
    cout << "md5(1): " << JSON_to_map(JSON_to_vector(tep_map["data"])[1])["md5"].as_int() << endl;
    cout << "md5(0): " << tep_map["data"][0]["md5"].as_int() << endl;
    cout << "md5(1): " << tep_map["data"][1]["md5"].as_int() << endl;
}