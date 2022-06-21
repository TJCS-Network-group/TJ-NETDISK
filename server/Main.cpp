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
#include <iostream>
#include <string>

#include "./include/HttpRequest.h"
#include "./include/HttpResponse.h"
#include "./include/HttpServer.h"
using namespace std;

char buf[20000000];  //���մ�������http request������ʱ����ǧ���ֽڴ�(���Դ洢16MB������)�����ܲ�����Ҫע��һ��

int main() {
    Routers routers;
    routers.Init_routers();  //ע���api�ӿ�

    int port = 6666;  // 6666�˿�
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

        while (true) {
            memset(buf, 0, sizeof(buf));
            // cout << "׼��recv" << endl;
            int len = recv(conn, buf, sizeof(buf), 0);
            // cout << "len: " << len << endl;

            if (len == 0)  //�Է�����0��˵���Ѿ��������
            {
                //���ǾͶϿ�Ȼ��listen��ȥ����һ�ң�֮��Ӧ�øĳ�select/epoll
                // cout << errno << endl << strerror(errno) << endl;
                cout << "disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
                break;
            }

            buf[len] = '\0';
            string client_http_request = buf;
            cout << client_http_request << endl;

            //����һ��HttpRequest�������ԭ����
            HttpRequest new_request(client_http_request);
            HttpResponse new_response = routers.getResponse(new_request);

            string send_content = new_response.getMessage();  //Ҫ����client�˵ı���
            cout << send_content << endl;
            send(conn, send_content.c_str(), send_content.length(), 0);
            cout << "send over" << endl;
        }
        close(conn);
    }
    close(listenfd);
    return 0;
}