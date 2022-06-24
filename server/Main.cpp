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
char buf[BUFFER_SIZE]; //���մ�������http request������ʱ��ʮ���ֽڴ棬���ܲ�����Ҫע��һ��

//��ͣ���� ֱ���Ͽ����ӻ����յ�����HTTP���� �˺�Ӧ�����ó�ʱ�˳�����
HttpRequest http_recv_request(int sockfd)
{ //����accept��socketfd
    memset(buf, 0, BUFFER_SIZE);
    int len = recv(sockfd, buf, BUFFER_SIZE, 0); //��һ�ν������ݣ�������header�ҵ�
    // buf[len] = '\0';
    cout << "len: " << len << endl;

    string client_http_request(buf, len);
    //����һ��HttpRequest���������һ�η�������ԭ���ģ���������header, �����ܲ���������body��
    HttpRequest new_request(client_http_request);

    while (new_request.Read_body_over() == false) //�յ�Content-Lengthû�ﵽbodyֵ
    {
        memset(buf, 0, BUFFER_SIZE);
        int len = recv(sockfd, buf, BUFFER_SIZE, 0); //��������
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
        //��buf����copy��body��
        string tep_str(buf, len);
        new_request.Concat_body(tep_str);
    }
    if (len != 0)
    {
        //����һ��HttpRequest�������ԭ���ģ�����볬ʱ��try����
        new_request.Parse_request_body(); //����body
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

                // if (setnonblocking(conn) == -1)
                //     continue; //��Ϊ������

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
                    close(sockfd); //�Ͽ�
                    sockfd = -1;
                    //���ǵ�����cookie��ֻ���ڱ��λػ���disconnect֮��Ҫ��session��ɾ��
                    if (session.count(new_request.current_user_id) != 0)
                        session.erase(new_request.current_user_id);
                    cout << "disconnect" << endl;
                    // cout << "disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
                    continue;
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

                    string send_content = new_response.getMessage(); //Ҫ����client�˵ı��ģ���һ��Ҫ����string��
                    send(sockfd, send_content.c_str(), send_content.length(), 0);
                    /*
                    Myepoll_data *tep = (Myepoll_data *)malloc(sizeof(Myepoll_data));
                    tep->length = send_content.length();
                    tep->data = (char *)malloc(send_content.length() + 1);
                    memcpy(tep->data, send_content.c_str(), send_content.length() + 1);
                    tep->data[tep->length] = '\0';
                    cout << tep->data << endl;
                    cout << "��current fd: " << sockfd << endl;
                    ev.data.ptr = tep;
                    cout << "��ʱָ���ֵ: " << ev.data.ptr << endl;
                    ev.data.fd = sockfd;
                    ev.events = EPOLLOUT | EPOLLET; // OUT
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev);
                    */
                }
            }
            else if (events[i].events & EPOLLOUT) //�����ݴ����ͣ�дsocket
            {
                cout << "ò��û��out���¼�" << endl;
                /*
                cout << "׼��ȡ����" << endl;
                Myepoll_data *md = (Myepoll_data *)events[i].data.ptr; //ȡ����
                cout << "дʱָ���ֵ: " << md << endl;
                cout << "ת�ɹ�" << endl;
                cout << "дcurrent fd: " << sockfd << endl;
                cout << md->length << endl;
                send(sockfd, md->data, md->length, 0); //��������
                free(md->data);
                free(md);

                ev.data.fd = sockfd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev); //�޸ı�ʶ�����ȴ���һ��ѭ��ʱ��������
                */
            }
        }
    }

    close(listenfd);
    return 0;
}