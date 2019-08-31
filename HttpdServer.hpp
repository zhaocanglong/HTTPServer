#ifndef __HTTPD_SERVER_HPP__
#define __HTTPD_SERVER_HPP__

#include <pthread.h>
#include "ProtocolUtil.hpp"
#include "ThreadPool.hpp"

//HTTP服务器整体流程
class HttpdServer{
    private:
        int listen_sock;
        int port;
        ThreadPool *tp;
    public:
        HttpdServer(int port_)
          : port(port_)
          ,listen_sock(-1)
          ,tp(NULL)
        {}

        void InitServer()
        {
            listen_sock = socket(AF_INET, SOCK_STREAM, 0);
            if(listen_sock < 0){
                LOG(ERROR, "create socket error!");
                exit(2);
            }

            int opt_ = 1;
            setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt_, sizeof(opt_)); //如果在套接字级别上设置选项的话，必须设置为SOL_SOCKET,SO_REUSEADDR，打开或关闭地址复用功能。当option_value不等于0时，打开，否则，关闭。它实际所做的工作是置sock->sk->sk_reuse为1或0。

            struct sockaddr_in local_;
            local_.sin_family = AF_INET;
            local_.sin_port = htons(port);
            local_.sin_addr.s_addr = INADDR_ANY;

            if(bind(listen_sock, (struct sockaddr*)&local_, sizeof(local_)) < 0){
                LOG(ERROR, "bind socket error!");
                exit(3);
            }

            if(listen(listen_sock, 5) < 0){
                LOG(ERROR, "listen socket error!");
                exit(4);
            }

            LOG(INFO, "initServer success!");
            tp = new ThreadPool();//服务端需要接受客户端连接需要创建线程池
            tp->initThreadPool();//对线程池进行初始化
        }

        void Start()
        {
            LOG(INFO, "Start Server begin!");
            for( ; ; ){
                struct sockaddr_in peer_;
                socklen_t len_ = sizeof(peer_);
                int sock_ = accept(listen_sock, (struct sockaddr*)&peer_, &len_);
                if(sock_ < 0){
                    LOG(WARNING, "accept error!");
                    continue;
                }
                Task t;
                t.SetTask(sock_, Entry::HandlerRequest);
                tp->PushTask(t);
            }
        }

        ~HttpdServer()
        {
            if(listen_sock != -1){
                close(listen_sock);
            }
            port = -1;
        }
};

#endif











