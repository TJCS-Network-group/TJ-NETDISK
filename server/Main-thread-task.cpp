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
Routers routers; //·�ɱ�
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
void epoll_mod_out(void *data, const int length, const int socketfd, const bool is_free)
{
    string client_http_request((const char *)data, length);

    if (is_free)
    {
        free(data);
        data = NULL;
    }
    //����һ��HttpRequest�������ԭ����
    HttpRequest new_request(client_http_request);
    // cout << new_request.Get_origin_headers() << endl;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char clientIP[INET_ADDRSTRLEN] = "";
    getpeername(socketfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    new_request.Set_IP(clientIP, clientAddr.sin_port);

    if (new_request.Get_body_len() > 0) //�������body
    {
        new_request.Parse_request_body();
    }
    HttpResponse new_response = routers.getResponse(new_request);
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
    // cout << "���Դ����߳�: " << socketfd << endl;
    ThreadArg *param = (ThreadArg *)malloc(sizeof(ThreadArg));
    param->data = (void *)data;
    param->length = length;
    param->socketfd = socketfd;
    param->isfree = isfree;
    pthread_attr_t a;                                         //�߳�����
    pthread_attr_init(&a);                                    //��ʼ���߳�����
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED); //�����߳�����
    if (pthread_create(&threads[socketfd], &a, Epoll_mod_out_thread, (void *)param) != 0)
    {
        cerr << "Error: pthread_create" << endl;
    }
}
int main()
{
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
    epollfd = epoll_create(MAX_EPOLL_SIZE);
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
            Myepoll_data *current_ptr = (Myepoll_data *)(events[i].data.ptr);
            int socketfd = current_ptr->sockfd;
            pthread_mutex_lock(&mutex[socketfd]);

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

                ev.events = EPOLLIN;
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
                cout << "EPOLLIN: " << socketfd << endl;
                // memset(buf, 0, BUFFER_SIZE);
                char *buf = (char *)malloc(BUFFER_SIZE);       //���մ�������http request����
                int len = recv(socketfd, buf, BUFFER_SIZE, 0); //��������
                if (len == 0)                                  // recv����len=0, �Է��Ͽ�
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
                    if (current_ptr->length == 0)
                    {
                        string client_http_request(buf, len);
                        //����һ��HttpRequest�������ԭ����
                        HttpRequest new_request(client_http_request);

                        if (new_request.Get_request_len() == len) //������
                        {
                            My_epoll_create_thread(buf, len, socketfd, true);
                            // cout << "�����̳߳ɹ�: " << socketfd << endl;
                        }
                        else
                        {
                            Myepoll_data *tep = (Myepoll_data *)malloc(sizeof(Myepoll_data));
                            tep->length = len;
                            tep->data = (char *)malloc(len);
                            memcpy(tep->data, buf, len);
                            tep->sockfd = socketfd;
                            tep->recv_capacity = new_request.Get_request_len();
                            ev.data.ptr = (void *)tep; //�Ӵ˲�����NULL
                            ev.events = EPOLLIN;
                            epoll_ctl(epollfd, EPOLL_CTL_MOD, socketfd, &ev); //��Ȼ��read
                        }
                    }
                    else // recv׷���µ�����
                    {
                        Myepoll_data *md = (Myepoll_data *)events[i].data.ptr; //ȡ�ϴε�
                        md->data = (char *)realloc((void *)md->data, md->length + len);
                        memcpy(md->data + md->length, buf, len);
                        md->length += len;                   //����length
                        if (md->recv_capacity == md->length) //�ϴε�header���������ˣ�������ھ��൱�ڶ�����
                        {
                            My_epoll_create_thread(md->data, md->length, socketfd, true);
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
                                My_epoll_create_thread(md->data, md->length, socketfd, true);
                                free(md); //���ٸ���
                            }
                            else
                            { //�ȴ��´ε��ȣ�������body/header����
                                ;
                            }
                        }
                        else //�ϴε�header�������ˣ���������һ�η���body��û����
                        {
                            //�ȴ��´ε��ȣ�������body����
                            ;
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
                    // break;//���Ǳ��ַ���ɣ�����Ҫ����ά����Ա��
                }
                if (md->length == md->send_length) //������
                {
                    md->clear();
                    md->sockfd = socketfd;
                    ev.data.ptr = (void *)md; //��sendת��recv�����Ը����ϴε�Myepoll_data *
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