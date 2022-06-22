#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "./include/HttpRequest.h"
#include "./include/HttpResponse.h"
#include "./include/HttpServer.h"
using namespace std;

//��ͣ���� ֱ�ӶϿ�����
string http_recv_request(int sockfd) {
    const int BUFFER_SIZE = 4096;
    char buffer_tep[BUFFER_SIZE + 1];
    string result;
    while (1) {
        memset(buffer_tep, 0, BUFFER_SIZE);
        int len = recv(sockfd, buffer_tep, BUFFER_SIZE, 0);  //��������
        if (len == -1) {
            cerr << "Error: recv\n";
            cerr << errno << endl << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        } else if (len == 0)  // disconnect���յ�Content-Length�ﵽ��ĳ��ֵ
        {
            break;
        }
        buffer_tep[len] = '\0';  //��β��
        result += buffer_tep;    //��buffer_tep����copy��result��
    }
    return result;
}

char buf[20000000];  //���մ�������http request������ʱ����ǧ���ֽڴ�(���Դ洢16MB������)�����ܲ�����Ҫע��һ��

int main() {
    Routers routers;
    routers.Init_routers();  //ע���api�ӿ�

    int port = 7777;  // 7777
    cout << "This is server" << endl;

    // socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        cerr << "Error: socket" << endl;
        exit(EXIT_FAILURE);
    }

    // bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    // closesocket��һ�㲻�������رն�����TIME_WAIT�Ĺ��̣�����������ø�socket
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        cerr << "Error: bind" << endl;
        exit(EXIT_FAILURE);
    }

    // listen
    if (listen(listenfd, 1) == -1) {
        cerr << "Error: listen" << endl;
        exit(EXIT_FAILURE);
    }

    // accept
    int conn;
    char clientIP[INET_ADDRSTRLEN] = "";
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    while (true) {
        cout << "listening..." << endl;
        conn = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (conn < 0) {
            cerr << "Error: accept" << endl;
            continue;
        }

        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        cout << "connect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;

        int recv_cnt = 0;
        while (true) {
            memset(buf, 0, sizeof(buf));
            // cout << "׼��recv" << endl;
            int len = recv(conn, buf, sizeof(buf), 0);
            cout << "len: " << len << endl;

            if (len <= 0)  //�Է�����0��˵���Ѿ��������
            {
                //���ǾͶϿ�Ȼ��listen��ȥ����һ�ң�֮��Ӧ�øĳ�select/epoll
                cout << errno << endl << strerror(errno) << endl;
                cout << "disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
                break;
            }

            buf[len] = '\0';
            string client_http_request = buf;
            // cout << client_http_request << endl;
            fstream myf("./request_file/" + to_string(recv_cnt) + "_response.txt", ios::out | ios::binary);
            if (myf.good()) {
                myf << client_http_request;
            } else {
                cout << "Can't open file!" << endl;
            }
            ++recv_cnt;
            //����һ��HttpRequest�������ԭ����
            // HttpRequest new_request(client_http_request);
            // HttpResponse new_response = routers.getResponse(new_request);
            // string send_content = new_response.getMessage();  //Ҫ����client�˵ı���
            // cout << send_content << endl;
            string send_json =
                "{\
\"data\": {}, \
\"message\": \"�����ʽ����\", \
\"statusCode\": 400, \
\"success\": false}\
";

            string send_str =
                "HTTP/1.1 200 OK\r\n\
Content-Type: application/json;charset=GBK\r\n\
Access-Control-Allow-Origin: *\r\n\
Content-Length: \
" + to_string(send_json.length()) +
                "\r\n\n";
            string send_content = send_str + send_json;
            send(conn, send_content.c_str(), send_content.length(), 0);
            cout << "send over" << endl;
        }
        close(conn);
    }
    close(listenfd);
    return 0;
}