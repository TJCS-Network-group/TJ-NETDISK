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

//不停的收 直接断开连接
string http_recv_request(int sockfd) {
    const int BUFFER_SIZE = 4096;
    char buffer_tep[BUFFER_SIZE + 1];
    string result;
    while (1) {
        memset(buffer_tep, 0, BUFFER_SIZE);
        int len = recv(sockfd, buffer_tep, BUFFER_SIZE, 0);  //接受数据
        if (len == -1) {
            cerr << "Error: recv\n";
            cerr << errno << endl << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        } else if (len == 0)  // disconnect或收到Content-Length达到了某个值
        {
            break;
        }
        buffer_tep[len] = '\0';  //加尾零
        result += buffer_tep;    //把buffer_tep数据copy到result中
    }
    return result;
}

char buf[20000000];  //接收传过来的http request请求，暂时用两千万字节存(可以存储16MB的数据)，可能不够大，要注意一下

int main() {
    Routers routers;
    routers.Init_routers();  //注册的api接口

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

    // closesocket（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket
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
            // cout << "准备recv" << endl;
            int len = recv(conn, buf, sizeof(buf), 0);
            cout << "len: " << len << endl;

            if (len <= 0)  //对方返回0，说明已经传输完毕
            {
                //我们就断开然后listen，去连下一家，之后应该改成select/epoll
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
            //创建一个HttpRequest对象解析原报文
            // HttpRequest new_request(client_http_request);
            // HttpResponse new_response = routers.getResponse(new_request);
            // string send_content = new_response.getMessage();  //要发给client端的报文
            // cout << send_content << endl;
            string send_json =
                "{\
\"data\": {}, \
\"message\": \"请求格式错误\", \
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