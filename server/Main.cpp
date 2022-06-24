#include "./include/HttpRequest.h"
#include "./include/HttpResponse.h"
#include "./include/HttpServer.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <net/if.h>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
const int BUFFER_SIZE = 100000;
char buf[BUFFER_SIZE]; //接收传过来的http request请求，暂时用十万字节存，可能不够大，要注意一下

//不停的收 直到断开连接或者收到完整HTTP报文 此后应该设置超时退出条件
HttpRequest http_recv_request(int sockfd)
{ //传入accept的socketfd
    memset(buf, 0, BUFFER_SIZE);
    int len = recv(sockfd, buf, BUFFER_SIZE, 0); //第一次接受数据，把所有header找到
    // buf[len] = '\0';
    cout << "len: " << len << endl;

    string client_http_request(buf, len);
    //创建一个HttpRequest对象解析第一次发过来的原报文（包含完整header, 但可能不包含完整body）
    HttpRequest new_request(client_http_request);

    while (new_request.Read_body_over() == false) //收到Content-Length没达到body值
    {
        memset(buf, 0, BUFFER_SIZE);
        int len = recv(sockfd, buf, BUFFER_SIZE, 0); //接受数据
        cout << "len: " << len << endl;
        if (len == -1)
        {
            cerr << "Error: recv\n";
            cerr << errno << endl
                 << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }
        else if (len == 0) // disconnect
        {
            break;
        }
        //把buf数据copy到body中
        string tep_str(buf, len);
        new_request.Concat_body(tep_str);
    }
    if (len != 0)
    {
        //创建一个HttpRequest对象解析原报文，需加入超时和try机制
        new_request.Parse_request_body(); //解析body
    }
    return new_request;
}
const int MAX_LISTEN_QUEUE = 100; // listen
const int PORT = 7777;            // server port
const int MAX_EPOLL_EVENT = 2048;
const int MAX_EPOLL_SIZE = 2048;
#include <fcntl.h>
int setnonblocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(sock,GETFL)");
        return -1;
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        perror("fcntl(sock,SETFL,opts)");
        return -1;
    }
    return 0;
}
/*
struct Myepoll_data
{
    char *data;
    int length;
};
*/
int main()
{
    Routers routers;        //路由表
    routers.Init_routers(); //注册api接口

    cout << "This is server" << endl;

    // 创建socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        cerr << "Error: socket" << endl;
        exit(EXIT_FAILURE);
    }

    {
        // bind
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;         // TCP
        addr.sin_port = htons(PORT);       // PORT
        addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
        // closesocket（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket
        int opt = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
        if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            cerr << "Error: bind" << endl;
            exit(EXIT_FAILURE);
        }
    }

    {
        // listen
        if (listen(listenfd, MAX_LISTEN_QUEUE) == -1)
        {
            cerr << "Error: listen" << endl;
            exit(EXIT_FAILURE);
        }
    }

    // 创建epoll实例
    int epollfd = epoll_create(MAX_EPOLL_SIZE);
    if (epollfd == -1)
    {
        cerr << "Error: epoll_create" << endl;
        exit(EXIT_FAILURE);
    }
    // 将监听的端口的socket对应的文件描述符添加到epoll事件列表中
    struct epoll_event ev, events[MAX_EPOLL_EVENT];
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1)
    {
        cerr << "Error: epoll_create" << endl;
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        // wait阻塞线程，等待事件
        int ndfs = epoll_wait(epollfd, events, MAX_EPOLL_EVENT, -1);
        cout << "ndfs: " << ndfs << endl;
        if (ndfs == -1)
        {
            cerr << "Error: epoll_wait" << endl;
            continue;
            // exit(EXIT_FAILURE);
        }
        for (int i = 0; i < ndfs; ++i)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd) //有新的连接
            {
                // accept建立新的连接
                char clientIP[INET_ADDRSTRLEN] = "";
                struct sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                int conn = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                if (conn < 0)
                {
                    cerr << "Error: accept" << endl;
                    continue;
                }
                inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
                cout << "connect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;

                // if (setnonblocking(conn) == -1)
                //     continue; //设为非阻塞

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn;
                // 将该文件描述符添加到epoll事件监听的列表中，使用ET模式
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    continue;
                    // exit(EXIT_FAILURE);
                }
            }
            else if (events[i].events & EPOLLIN) //读新数据
            {
                //创建一个HttpRequest对象解析原报文，需加入超时和try机制
                HttpRequest new_request = http_recv_request(sockfd);

                if (new_request.disconnect == true) // recv出来len=0, 对方断开
                {
                    close(sockfd); //断开
                    sockfd = -1;
                    //我们的所有cookie都只限于本次回话，disconnect之后要从session中删掉
                    if (session.count(new_request.current_user_id) != 0)
                        session.erase(new_request.current_user_id);
                    cout << "disconnect" << endl;
                    // cout << "disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
                    continue;
                }
                else
                {
                    //将新的完整response存储下来
                    fstream myf("./request_file/response.txt", ios::out | ios::binary);
                    if (myf.good())
                    {
                        myf << new_request.Get_origin_headers() << "\r\n"
                            << new_request.Get_body();
                    }
                    else
                    {
                        cout << "Can't open file!" << endl;
                    }
                    myf.close();
                    HttpResponse new_response = routers.getResponse(new_request);

                    string send_content = new_response.getMessage(); //要发给client端的报文，不一定要返回string了
                    send(sockfd, send_content.c_str(), send_content.length(), 0);
                    /*
                    Myepoll_data *tep = (Myepoll_data *)malloc(sizeof(Myepoll_data));
                    tep->length = send_content.length();
                    tep->data = (char *)malloc(send_content.length() + 1);
                    memcpy(tep->data, send_content.c_str(), send_content.length() + 1);
                    tep->data[tep->length] = '\0';
                    cout << tep->data << endl;
                    cout << "读current fd: " << sockfd << endl;
                    ev.data.ptr = tep;
                    cout << "读时指针的值: " << ev.data.ptr << endl;
                    ev.data.fd = sockfd;
                    ev.events = EPOLLOUT | EPOLLET; // OUT
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev);
                    */
                }
            }
            else if (events[i].events & EPOLLOUT) //有数据待发送，写socket
            {
                cout << "貌似没有out的事件" << endl;
                /*
                cout << "准备取数据" << endl;
                Myepoll_data *md = (Myepoll_data *)events[i].data.ptr; //取数据
                cout << "写时指针的值: " << md << endl;
                cout << "转成功" << endl;
                cout << "写current fd: " << sockfd << endl;
                cout << md->length << endl;
                send(sockfd, md->data, md->length, 0); //发送数据
                free(md->data);
                free(md);

                ev.data.fd = sockfd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev); //修改标识符，等待下一个循环时接收数据
                */
            }
        }
    }

    close(listenfd);
    return 0;
}