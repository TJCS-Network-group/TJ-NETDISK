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
using namespace std;
struct SocketArg {
    int port;
    int ip;
};
int main() {
    SocketArg arg;
    arg.port = 6666;
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
    addr.sin_port = htons(arg.port);
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

        char buf[255];
        while (true) {
            memset(buf, 0, sizeof(buf));
            int len = recv(conn, buf, sizeof(buf), 0);
            buf[len] = '\0';
            cout << buf << endl;

            string send_json =
                "{\
\"data\": {}, \
\"message\": \"请求格式错误\", \
\"statusCode\": 400, \
\"success\": false}\
";
            string send_str =
                "HTTP/1.1 200 OK\r\n\
Content-Type: application/json\r\n\
Connection: keep-alive\r\n\
Content-Length: \
" + to_string(send_json.length()) +
                "\r\n\n";

            string send_content = send_str + send_json;
            cout << send_content << endl;
            send(conn, send_content.c_str(), send_content.length(), 0);

            if (strcmp(buf, "exit") == 0 || len == 0)  //对方传输了exit，或者对方连接断开了，我们就断开然后listen，去连下一家
            {
                cout << errno << endl << strerror(errno) << endl;
                cout << "disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
                break;
            }
        }
        close(conn);
    }
    close(listenfd);
    return 0;
}