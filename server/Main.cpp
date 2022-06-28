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

const int MAX_LISTEN_QUEUE = 100; // listen
const int PORT = 7777;            // server port
const int MAX_EPOLL_EVENT = 2048;
const int MAX_EPOLL_SIZE = 2048;
const int BUFFER_SIZE = 20000;
char buf[BUFFER_SIZE]; //���մ�������http request������ʱ��2���ֽڴ棬���ܲ�����Ҫע��һ��

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
    char *data;        //�յ������ݻ�Ҫ����������
    int length;        // dataĿǰ�ĳ���
    int send_length;   //��ǰsend�˶���
    int recv_capacity; // recv����������header�н�������
    int sockfd;        // socket���ļ�������
    void init()
    {
        sockfd = -1;
        length = 0;
        send_length = 0;
        recv_capacity = -1;
        data = NULL; //��ʼ��dataΪNULL���У�free(NULL)���·���
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
//������ ����api�õ�response ��socketfd��Ӧ��ģʽתΪepollout
int epoll_mod_out(const Routers &routers, const char *data, const int length, const int socketfd, const int epollfd)
{
    string client_http_request(data, length);
    //����һ��HttpRequest�������ԭ����
    HttpRequest new_request(client_http_request);
    if (new_request.Get_body_len() > 0) //�������body
    {
        new_request.Parse_request_body();
    }
    HttpResponse new_response = routers.getResponse(new_request);
    // cout << new_response.getHeaders() << endl;
    string send_content = new_response.getMessage();

    //��Ϊ��recv��send�п��ܲ�����Myepoll_data *����˾Ͳ��ٸ��ø�ָ��
    Myepoll_data *resp = (Myepoll_data *)malloc(sizeof(Myepoll_data));
    resp->data = (char *)malloc(send_content.length());
    memcpy(resp->data, send_content.c_str(), send_content.length());
    resp->send_length = 0;                //�Ѿ�send��len
    resp->length = send_content.length(); //�ܳ���
    resp->sockfd = socketfd;
    struct epoll_event ev;
    ev.data.ptr = (void *)resp;
    ev.events = EPOLLOUT | EPOLLET; // OUT
    return epoll_ctl(epollfd, EPOLL_CTL_MOD, socketfd, &ev);
}
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
    memset(events, sizeof(events), 0);

    ev.events = EPOLLIN;
    Myepoll_data ev_data;
    ev_data.sockfd = listenfd;
    // ev.data.fd = listenfd;//ʹ��ptr����ʹ��fd
    ev.data.ptr = (void *)&ev_data; //��

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1)
    {
        cerr << "Error: epoll_create" << endl;
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        //�ж�cookie��ʱ��disconnect֮��Ҫ��session��ɾ��
        // if (session.count(new_request.current_user_id) != 0)
        //    session.erase(new_request.current_user_id);

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
            // cout << "i: " << i << " " << events[i].data.ptr << endl;
            Myepoll_data *current_ptr = (Myepoll_data *)(events[i].data.ptr);
            int socketfd = current_ptr->sockfd;
            if (socketfd == listenfd) //���µ�����
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
                Myepoll_data *tep = (Myepoll_data *)malloc(sizeof(Myepoll_data));
                tep->init();
                tep->sockfd = conn; // socket�ļ�������
                ev.data.ptr = (void *)tep;
                // �����ļ���������ӵ�epoll�¼��������б��У�ʹ��ETģʽ
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &ev) == -1)
                {
                    cerr << "epoll_ctl: conn_sock" << endl;
                    continue;
                    // exit(EXIT_FAILURE);
                }
            }
            else if (events[i].events & EPOLLIN) //��������
            {
                memset(buf, 0, BUFFER_SIZE);
                int len = recv(socketfd, buf, BUFFER_SIZE, 0); //��������
                // cout << "len: " << len << endl;
                if (len == 0) // recv����len=0, �Է��Ͽ�
                {
                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    char clientIP[INET_ADDRSTRLEN] = "";
                    getpeername(socketfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
                    cout << "disconnect " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;

                    current_ptr->clear();
                    free(current_ptr);

                    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, socketfd, &events[i]) == -1) //�Ͽ�
                    {
                        cerr << "epoll_ctl: disconnect_sock" << endl;
                        continue;
                        // exit(EXIT_FAILURE);
                    }
                }
                if (len > 0)
                {
                    // cout << events[i].data.ptr << endl;
                    if (current_ptr->length == 0)
                    {
                        // cout << "enter first point" << endl;
                        string client_http_request(buf, len);
                        //����һ��HttpRequest�������ԭ����
                        HttpRequest new_request(client_http_request);
                        // cout << "Parser Header" << endl;
                        if (new_request.Get_request_len() == len) //������
                            epoll_mod_out(routers, buf, len, socketfd, epollfd);
                        else
                        {
                            Myepoll_data *tep = (Myepoll_data *)malloc(sizeof(Myepoll_data));
                            tep->length = len;
                            tep->data = (char *)malloc(len);
                            memcpy(tep->data, buf, len);
                            tep->sockfd = socketfd;
                            tep->recv_capacity = new_request.Get_request_len();
                            ev.data.ptr = (void *)tep; //�Ӵ˲�����NULL
                            ev.events = EPOLLIN | EPOLLET;
                            epoll_ctl(epollfd, EPOLL_CTL_MOD, socketfd, &ev); //��Ȼ��read
                        }
                    }
                    else // recv׷���µ�����
                    {
                        Myepoll_data *md = (Myepoll_data *)events[i].data.ptr; //ȡ�ϴε�
                        md = (Myepoll_data *)realloc((void *)md, md->length + len);
                        memcpy(md->data + md->length, buf, len);
                        md->length += len;                   //����length
                        if (md->recv_capacity == md->length) //�ϴε�header���������ˣ�������ھ��൱�ڶ�����
                        {
                            epoll_mod_out(routers, md->data, md->length, socketfd, epollfd);
                            md->clear();
                            free(md); //���ٸ���
                        }
                        else if (md->recv_capacity == -1) //�ϴε�header��û�����꣬����յ���Ҫ�ٽ���һ��
                        {
                            string client_http_request(md->data, md->length);
                            //����һ��HttpRequest�������ԭ����
                            HttpRequest new_request(client_http_request);
                            md->recv_capacity = new_request.Get_request_len();
                            if (md->recv_capacity == len) //������ζ�����
                            {
                                epoll_mod_out(routers, md->data, md->length, socketfd, epollfd);
                                md->clear();
                                free(md); //���ٸ���
                            }
                            else
                            {
                                ev.data.ptr = (void *)md; // realloc��������ָ��
                                ev.events = EPOLLIN | EPOLLET;
                                epoll_ctl(epollfd, EPOLL_CTL_MOD, socketfd, &ev); //��Ȼ��read
                            }
                        }
                        else //�ϴε�header�������ˣ���������һ�η���body��û����
                        {
                            //�ȴ��´ε��ȣ�������body����
                            ev.data.ptr = (void *)md; // realloc��������ָ��
                            ev.events = EPOLLIN | EPOLLET;
                            epoll_ctl(epollfd, EPOLL_CTL_MOD, socketfd, &ev); //��Ȼ��read
                        }
                    }
                }
                if (len < 0 && errno != EAGAIN)
                {
                    cerr << "errno: " << errno << endl;
                    cerr << strerror(errno) << endl;
                    // break;//���Ǳ��ַ���ɣ�����Ҫ����ά����Ա��
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                Myepoll_data *md = (Myepoll_data *)events[i].data.ptr;

                int len_sum = 0;
                // cout << "max_len: " << md->length << endl;
                while (len_sum < md->length)
                {
                    int len = send(md->sockfd, md->data + len_sum, md->length - len_sum, 0);
                    cout << "len: " << len << endl;
                    if (len > 0)
                    {
                        len_sum += len;
                        // cout << "len_sum: " << len_sum << endl;
                    }
                    else if (len < 0 && errno != EAGAIN)
                    {
                        cout << errno << ' ' << strerror(errno) << endl;
                    }
                    else
                    {
                        sleep(1); //���Ե�С��
                    }
                }
                md->clear();
                ev.data.ptr = (void *)md; //��sendת��recv�����Ը����ϴε�Myepoll_data *
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_MOD, md->sockfd, &ev);
            }
        }
    }

    close(listenfd);
    return 0;
}