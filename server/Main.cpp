#include "./include/HttpRequest.h"
#include "./include/HttpResponse.h"
#include "./include/HttpServer.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <exception>
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
struct Myepoll_data
{
    char *data;
    // bool is_malloc;//����Ҫ����ʼ��dataΪNULL���У�free(NULL)���·���
    int length;
    int sockfd;
};
int main()
{
    Routers routers;        //·�ɱ�
    routers.Init_routers(); //ע��api�ӿ�

    cout << "This is server" << endl;

    // ����socket
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
        // closesocket��һ�㲻�������رն�����TIME_WAIT�Ĺ��̣�����������ø�socket
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

    // ����epollʵ��
    int epollfd = epoll_create(MAX_EPOLL_SIZE);
    if (epollfd == -1)
    {
        cerr << "Error: epoll_create" << endl;
        exit(EXIT_FAILURE);
    }
    // �������Ķ˿ڵ�socket��Ӧ���ļ���������ӵ�epoll�¼��б���
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
        // wait�����̣߳��ȴ��¼�
        int ndfs = epoll_wait(epollfd, events, MAX_EPOLL_EVENT, -1);
        if (ndfs == -1)
        {
            cerr << "Error: epoll_wait" << endl;
            continue;
            // exit(EXIT_FAILURE);
        }
        for (int i = 0; i < ndfs; ++i)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd) //���µ�����
            {
                // accept�����µ�����
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
                    continue; //��Ϊ������

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn;
                // �����ļ���������ӵ�epoll�¼��������б��У�ʹ��ETģʽ
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    continue;
                    // exit(EXIT_FAILURE);
                }
            }
            else if (events[i].events & EPOLLIN) //��������
            {
                //����һ��HttpRequest�������ԭ���ģ�����볬ʱ��try����
                HttpRequest new_request = http_recv_request(sockfd);

                if (new_request.disconnect == true) // recv����len=0, �Է��Ͽ�
                {
                    //���ǵ�����cookie��ֻ���ڱ��λػ���disconnect֮��Ҫ��session��ɾ��
                    // if (session.count(new_request.current_user_id) != 0)
                    //    session.erase(new_request.current_user_id);

                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    char clientIP[INET_ADDRSTRLEN] = "";
                    getpeername(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
                    cout << "disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;

                    // close(sockfd); //�Ͽ�
                    // sockfd = -1;
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, &events[i]);
                }
                else
                {
                    //���µ�����response�洢����
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

                    string send_content = new_response.getMessage();

                    // send(sockfd, send_content.c_str(), send_content.length(), 0);
                    int len_sum = 0;
                    const int TIME_OUT = 5; // 5s
                    time_t past_time;
                    time(&past_time);
                    /*while (len_sum < send_content.length())
                    {
                        time_t cur_time;
                        time(&cur_time);
                        if (cur_time - past_time >= TIME_OUT)
                        {
                            break;
                        }

                        int len = send(sockfd, send_content.c_str() + len_sum, send_content.length() - len_sum, 0);
                        if (len > 0)
                            len_sum += len;
                        else if (errno != EAGAIN)
                        {
                            cout << errno << ' ' << strerror(errno) << endl;
                            break;
                        }
                    }*/
                    cout << "EPOLLIN" << endl;

                    Myepoll_data *tep = (Myepoll_data *)malloc(sizeof(Myepoll_data));
                    tep->length = send_content.length();
                    tep->data = (char *)malloc(send_content.length() + 1);
                    memcpy(tep->data, send_content.c_str(), send_content.length());
                    tep->data[tep->length] = '\0';
                    tep->sockfd = sockfd;

                    ev.data.ptr = (void *)tep;

                    // ev.data.fd = sockfd;
                    ev.events = EPOLLOUT | EPOLLET; // OUT
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev);
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                cout << "EPOLLOUT" << endl;
                Myepoll_data *md = (Myepoll_data *)events[i].data.ptr;

                int len_sum = 0;
                // cout << "len: " << md->length << endl;
                while (len_sum < md->length)
                {
                    int len = send(md->sockfd, md->data + len_sum, md->length - len_sum, 0);
                    if (len > 0)
                    {
                        len_sum += len;
                        // cout << "len_sum: " << len_sum << endl;
                    }
                    else if (errno != EAGAIN)
                    {
                        cout << errno << ' ' << strerror(errno) << endl;
                    }
                }
                free(md->data);
                free(md);

                ev.data.fd = md->sockfd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_MOD, md->sockfd, &ev);
            }
        }
    }

    close(listenfd);
    return 0;
}