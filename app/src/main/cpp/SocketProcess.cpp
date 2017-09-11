//
// Created by sunxiaoyu on 2017/9/11.
//
#include <jni.h>
#include <string>
#include "SocketProcess.h"

int m_child;
int m_parent = -1;

const  char * user_id1;
const char *PATH = "/data/data/com.sxy.daemonprocessdemo/my.sock";

extern "C"
JNIEXPORT void JNICALL
Java_com_sxy_daemonprocessdemo_ProcessSocketService_createWatcher(JNIEnv *env, jobject instance, jint userId) {
    user_id1 = (const char *) userId;
    create_child1();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sxy_daemonprocessdemo_ProcessSocketService_connectToMonitor(JNIEnv *env, jobject instance) {

    // TODO
    int sockfd;
    struct sockaddr_un addr;

    while (1){
        LOGE("客户端父进程开始连接！");
        sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
        if (sockfd < 0){
            break;
        }
        memset(&addr, 0, sizeof(sockaddr_un));
        addr.sun_family = AF_LOCAL;
        strcpy(addr.sun_path, PATH);
        if (connect(sockfd, (const sockaddr *) &addr, sizeof(addr)) < 0){
            LOGE("连接失败，休眠");
            close(sockfd);
            sleep(1);

            continue;
        }
        //连接成功！
        m_parent = sockfd;
        LOGE("连接成功  父进程跳出循环");

        break;
    }
}


void create_child1(){

    pid_t pid = fork();

    if (pid == 0){
        LOGE("子进程开启！");
        child_do_work();
    }
}

void child_do_work(){
    //建立socket服务，读取消息
    if (child_create_channel()){
        child_listen_msg();
    }
}

int child_create_channel(){
    //创建socket对象
    int listenfd=socket(AF_LOCAL, SOCK_STREAM, 0);
    //取消之前的进程文件连接
    unlink(PATH);
    struct  sockaddr_un addr;
    //清空内存
    memset(&addr, 0, sizeof(sockaddr_un));
    addr.sun_family = AF_LOCAL;

    strcpy(addr.sun_path, PATH);
    int connfd = 0;

    LOGE("绑定端口号");
    if (bind(listenfd, (const sockaddr *) &addr, sizeof(addr)) < 0){
        LOGE("绑定失败！");
        return 0;
    }
    listen(listenfd, 5);

    while (1){
        LOGE("子进程循环等待连接 %d ", m_child);

        if ((connfd = accept(listenfd, NULL, NULL)) < 0){
            if (errno == EINTR){
                continue;
            }else{
                LOGE("读取错误！");
                return 0;
            }
        }

        m_child = connfd;
        LOGE("apk 父进程连接上了  %d ",m_child);
        break;
    }
    LOGE("返回成功");
    return 1;
}

void child_listen_msg(){
    fd_set rfds;
    while (1){
        //清空端口号
        FD_ZERO(&rfds);
        FD_SET(m_child, &rfds);
        //设置超时时间
        struct timeval timeout = {3,0};
        int r = select(m_child+1,&rfds,NULL,NULL,&timeout);
        LOGE("读取消息前：%d", r);
        if (r > 0){
            char pkg[256] = {0};
            //确保读到的内容是制定的端口号
            if (FD_ISSET(m_child, &rfds)){
                // 阻塞式函数  客户端写到内容  apk进程  没有进行任何写入    连接
                int result = read(m_child, pkg, sizeof(pkg));
                //读到内容的唯一方式 是客户端断开
                LOGE("重启父进程  %d ",result);
                LOGE("读到信息  %d    userid  %d ",result,user_id1);

                execlp("am", "am", "startservice", "--user",user_id1,
                       "com.dongnao.socketprocess/com.dongnao.socketprocess.ProcessSignalService", (char*)NULL);
            }

            break;
        }
    }
}

