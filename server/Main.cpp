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
#include <iterator>
#include <string>

#include "./include/HttpRequest.h"
#include "./include/HttpResponse.h"
#include "./include/HttpServer.h"
using namespace std;
const int BUFFER_SIZE = 100000;
char buf[BUFFER_SIZE];  //���մ�������http request������ʱ��ʮ���ֽڴ棬���ܲ�����Ҫע��һ��

//��ͣ���� ֱ�ӶϿ�����
HttpRequest http_recv_request(int sockfd) {  //����accept��socketfd
    memset(buf, 0, BUFFER_SIZE);
    int len = recv(sockfd, buf, BUFFER_SIZE, 0);  //��һ�ν������ݣ�������header�ҵ�
    // buf[len] = '\0';
    cout << "len: " << len << endl;

    string client_http_request(buf, len);
    //����һ��HttpRequest���������һ�η�������ԭ���ģ���������header, �����ܲ���������body��
    HttpRequest new_request(client_http_request);

    while (new_request.Read_body_over() == false)  //�յ�Content-Lengthû�ﵽbodyֵ
    {
        memset(buf, 0, BUFFER_SIZE);
        int len = recv(sockfd, buf, BUFFER_SIZE, 0);  //��������
        cout << "len: " << len << endl;
        if (len == -1) {
            cerr << "Error: recv\n";
            cerr << errno << endl << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        } else if (len == 0)  // disconnect
        {
            break;
        }
        //��buf����copy��body��
        string tep_str(buf, len);
        new_request.Concat_body(tep_str);
    }
    return new_request;
}

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
        HttpRequest new_request;
        while (true) {
            //����һ��HttpRequest�������ԭ����
            new_request = http_recv_request(conn);
            if (new_request.disconnect == true) {
                cout << "disconnect" << endl;
                break;
            }
            // header
            fstream myf_headers("./request_file/" + to_string(recv_cnt) + "_response_headers.txt", ios::out | ios::binary);
            if (myf_headers.good()) {
                myf_headers << new_request.Get_origin_headers();
            } else {
                cout << "Can't open file!" << endl;
            }
            // body
            fstream myf_body("./request_file/" + to_string(recv_cnt) + "_response_body.txt", ios::out | ios::binary);
            if (myf_body.good()) {
                myf_body << new_request.Get_body();
            } else {
                cout << "Can't open file!" << endl;
            }
            //����response
            fstream myf("./request_file/" + to_string(recv_cnt) + "_response.txt", ios::out | ios::binary);
            if (myf.good()) {
                myf << new_request.Get_origin_headers() << "\r\n" << new_request.Get_body();
            } else {
                cout << "Can't open file!" << endl;
            }
            ++recv_cnt;

            new_request.Parse_request_body();  //����body

            HttpResponse new_response = routers.getResponse(new_request);

            string send_content = new_response.getMessage();  //Ҫ����client�˵ı���

            cout << "Send response:" << endl;
            cout << send_content << endl;

            send(conn, send_content.c_str(), send_content.length(), 0);
        }

        close(conn);
        //���ǵ�����cookie��ֻ���ڱ��λػ���disconnect֮��Ҫ��session��ɾ��
        if (session.count(new_request.current_user_id) != 0) session.erase(new_request.current_user_id);
        cout << "disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
    }
    close(listenfd);
    return 0;
}