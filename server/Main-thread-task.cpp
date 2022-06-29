#include "./include/HttpRequest.h"
#include "./include/HttpResponse.h"
#include "./include/HttpServer.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <exception>
#include <fcntl.h>
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

const int MAX_LISTEN_QUEUE = 100; // listen queue
const int PORT = 7778;            // server port
const int MAX_EPOLL_EVENT = 2048;
const int MAX_EPOLL_SIZE = 204800;
pthread_mutex_t mutex[MAX_EPOLL_SIZE] = {PTHREAD_MUTEX_INITIALIZER};
pthread_t threads[MAX_EPOLL_SIZE];
const int BUFFER_SIZE = 500000;
Routers routers; //路由表
int epollfd;
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
struct Myepoll_data
{
    char *data;        //收到的数据或要发出的数据
    int length;        // data目前的长度
    int send_length;   //此前send了多少
    int recv_capacity; // recv的容量，从header中解析出来
    int sockfd;        // socket的文件描述符
    void init()
    {
        sockfd = -1;
        length = 0;
        send_length = 0;
        recv_capacity = -1;
        data = NULL; //初始令data为NULL就行，free(NULL)无事发生
    }
    void clear()
    {
        free(data);
        init();
    }
    Myepoll_data()
    {
        init();
    }
};
//读完了 调用api得到response 将socketfd对应的模式转为epollout
void epoll_mod_out(void *data, const int length, const int socketfd, const bool is_free)
{
    string client_http_request((const char *)data, length);

    if (is_free)
    {
        free(data);
        data = NULL;
    }
    //创建一个HttpRequest对象解析原报文
    HttpRequest new_request(client_http_request);
    // cout << new_request.Get_origin_headers() << endl;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char clientIP[INET_ADDRSTRLEN] = "";
    getpeername(socketfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    new_request.Set_IP(clientIP, clientAddr.sin_port);

    if (new_request.Get_body_len() > 0) //有则解析body
    {
        new_request.Parse_request_body();
    }
    HttpResponse new_response = routers.getResponse(new_request);
    string send_content = new_response.getMessage();

    //因为从recv到send有可能不创建Myepoll_data *，因此就不再复用该指针
    Myepoll_data *resp = (Myepoll_data *)malloc(sizeof(Myepoll_data));
    resp->data = (char *)malloc(send_content.length());
    memcpy(resp->data, send_content.c_str(), send_content.length());
    resp->send_length = 0;                //已经send的len
    resp->length = send_content.length(); //总长度
    resp->sockfd = socketfd;
    struct epoll_event ev;
    ev.data.ptr = (void *)resp;
    ev.events = EPOLLOUT; // OUT
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, socketfd, &ev) < 0)
    {
        cerr << "modify error" << endl;
        // return
    }
}
struct ThreadArg
{
    void *data;
    int length;
    int socketfd;
    int isfree;
};

void *Epoll_mod_out_thread(void *param)
{
    ThreadArg *ptr = (ThreadArg *)param;
    pthread_mutex_lock(&mutex[ptr->socketfd]);
    epoll_mod_out(ptr->data, ptr->length, ptr->socketfd, ptr->isfree);
    threads[ptr->socketfd] = 0;
    pthread_mutex_unlock(&mutex[ptr->socketfd]);
    free(param);
    pthread_exit(NULL);
    // return NULL;
}
void My_epoll_create_thread(char *data, const int length, const int socketfd, const bool isfree)
{
    // cout << "尝试创建线程: " << socketfd << endl;
    ThreadArg *param = (ThreadArg *)malloc(sizeof(ThreadArg));
    param->data = (void *)data;
    param->length = length;
    param->socketfd = socketfd;
    param->isfree = isfree;
    pthread_attr_t a;                                         //线程属性
    pthread_attr_init(&a);                                    //初始化线程属性
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED); //设置线程属性
    if (pthread_create(&threads[socketfd], &a, Epoll_mod_out_thread, (void *)param) != 0)
    {
        cerr << "Error: pthread_create" << endl;
    }
}
int main()
{
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
    epollfd = epoll_create(MAX_EPOLL_SIZE);
    if (epollfd == -1)
    {
        cerr << "Error: epoll_create" << endl;
        exit(EXIT_FAILURE);
    }
    // 将监听的端口的socket对应的文件描述符添加到epoll事件列表中
    struct epoll_event ev, events[MAX_EPOLL_EVENT];
    memset(events, sizeof(events), 0);

    ev.events = EPOLLIN;
    Myepoll_data ev_data;
    ev_data.sockfd = listenfd;
    // ev.data.fd = listenfd;//使用ptr，不使用fd
    ev.data.ptr = (void *)&ev_data; //绑定

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1)
    {
        cerr << "Error: epoll_create" << endl;
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        //判断cookie超时，disconnect之后要从session中删掉
        // if (session.count(new_request.current_user_id) != 0)
        //    session.erase(new_request.current_user_id);

        // wait阻塞线程，等待事件
        int ndfs = epoll_wait(epollfd, events, MAX_EPOLL_EVENT, -1);
        if (ndfs == -1)
        {
            cerr << "Error: epoll_wait" << endl;
            continue;
            // exit(EXIT_FAILURE);
        }
        for (int i = 0; i < ndfs; ++i)
        {
            Myepoll_data *current_ptr = (Myepoll_data *)(events[i].data.ptr);
            int socketfd = current_ptr->sockfd;
            pthread_mutex_lock(&mutex[socketfd]);

            if (socketfd == listenfd) //有新的连接
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

                if (setnonblocking(conn) == -1)
                    continue; //设为非阻塞

                ev.events = EPOLLIN;
                Myepoll_data *tep = (Myepoll_data *)malloc(sizeof(Myepoll_data));
                tep->init();
                tep->sockfd = conn; // socket文件描述符
                ev.data.ptr = (void *)tep;
                // 将该文件描述符添加到epoll事件监听的列表中，使用ET模式
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &ev) == -1)
                {
                    cerr << "epoll_ctl: conn_sock" << endl;
                    continue;
                    // exit(EXIT_FAILURE);
                }
            }
            else if (events[i].events & EPOLLIN) //读新数据
            {
                cout << "EPOLLIN: " << socketfd << endl;
                // memset(buf, 0, BUFFER_SIZE);
                char *buf = (char *)malloc(BUFFER_SIZE);       //接收传过来的http request请求
                int len = recv(socketfd, buf, BUFFER_SIZE, 0); //接受数据
                if (len == 0)                                  // recv出来len=0, 对方断开
                {
                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    char clientIP[INET_ADDRSTRLEN] = "";
                    getpeername(socketfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
                    cout << "disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;

                    current_ptr->clear();
                    free(current_ptr);

                    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, socketfd, &events[i]) == -1) //断开
                    {
                        cerr << "epoll_ctl: disconnect_sock" << endl;
                        continue;
                        // exit(EXIT_FAILURE);
                    }
                }
                if (len > 0)
                {
                    if (current_ptr->length == 0)
                    {
                        string client_http_request(buf, len);
                        //创建一个HttpRequest对象解析原报文
                        HttpRequest new_request(client_http_request);

                        if (new_request.Get_request_len() == len) //读完了
                        {
                            My_epoll_create_thread(buf, len, socketfd, true);
                            // cout << "创建线程成功: " << socketfd << endl;
                        }
                        else
                        {
                            Myepoll_data *tep = (Myepoll_data *)malloc(sizeof(Myepoll_data));
                            tep->length = len;
                            tep->data = (char *)malloc(len);
                            memcpy(tep->data, buf, len);
                            tep->sockfd = socketfd;
                            tep->recv_capacity = new_request.Get_request_len();
                            ev.data.ptr = (void *)tep; //从此不再是NULL
                            ev.events = EPOLLIN;
                            epoll_ctl(epollfd, EPOLL_CTL_MOD, socketfd, &ev); //依然是read
                        }
                    }
                    else // recv追加新的内容
                    {
                        Myepoll_data *md = (Myepoll_data *)events[i].data.ptr; //取上次的
                        md->data = (char *)realloc((void *)md->data, md->length + len);
                        memcpy(md->data + md->length, buf, len);
                        md->length += len;                   //更新length
                        if (md->recv_capacity == md->length) //上次的header都解析完了，这里等于就相当于读完了
                        {
                            My_epoll_create_thread(md->data, md->length, socketfd, true);
                            free(md); //不再复用
                        }
                        else if (md->recv_capacity == -1) //上次的header都没解析完，这次收到了要再解析一次
                        {
                            string client_http_request(md->data, md->length);
                            //创建一个HttpRequest对象解析原报文
                            HttpRequest new_request(client_http_request);
                            md->recv_capacity = new_request.Get_request_len();
                            if (md->recv_capacity == len) //发现这次读完了
                            {
                                My_epoll_create_thread(md->data, md->length, socketfd, true);
                                free(md); //不再复用
                            }
                            else
                            { //等待下次调度，继续读body/header即可
                                ;
                            }
                        }
                        else //上次的header解析完了，但经过这一次发现body还没读完
                        {
                            //等待下次调度，继续读body即可
                            ;
                        }
                    }
                }
                if (len < 0 && errno != EAGAIN)
                {
                    cerr << "errno: " << errno << endl;
                    cerr << strerror(errno) << endl;
                    // break;//还是保持服务吧，但是要警告维护人员了
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                cout << "EPOLLOUT: " << socketfd << endl;
                Myepoll_data *md = (Myepoll_data *)events[i].data.ptr;
                if (md->length > md->send_length)
                {
                    int len = send(md->sockfd, md->data + md->send_length, md->length - md->send_length, 0);
                    if (len > 0)
                    {
                        md->send_length += len;
                    }
                    else if (len < 0 && errno != EAGAIN)
                    {
                        cout << errno << ' ' << strerror(errno) << endl;
                    }
                }
                else
                {
                    cout << errno << ' ' << strerror(errno) << endl;
                    // break;//还是保持服务吧，但是要警告维护人员了
                }
                if (md->length == md->send_length) //发完了
                {
                    md->clear();
                    md->sockfd = socketfd;
                    ev.data.ptr = (void *)md; //从send转到recv，可以复用上次的Myepoll_data *
                    ev.events = EPOLLIN;
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, md->sockfd, &ev);
                }
            }
            pthread_mutex_unlock(&mutex[socketfd]);
        }
    }

    close(listenfd);
    return 0;
}