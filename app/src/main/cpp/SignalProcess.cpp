//
// Created by sunxiaoyu on 2017/9/7.
//
#include <jni.h>
#include <string>
#include "SignalProcess.h"

int user_id;

extern "C"
JNIEXPORT void JNICALL
Java_com_sxy_daemonprocessdemo_ProcessSignalService_createWatcher(JNIEnv *env, jobject instance, jint userId) {

    //父进程id
    user_id = userId;
    LOGE("父进程id: %d", user_id);
    create_child();

}

void create_child(){
    pid_t pid = fork();

    if (pid == 0){
        LOGE("子进程开启");
        child_start_monitor();
    }
}


void child_start_monitor(){
    pthread_t  tid;
    pthread_create(&tid, NULL, thread_rt, NULL);
}



void *thread_rt(void *data){
    LOGE("子进程开启循环判断");
    pid_t  pid;
    while ((pid = getppid()) != 1){
        sleep(2);
        LOGE("父进程活着，循环判断");
    }
    LOGE("父进程挂了，现在重新启动");

    execlp("am", "am", "startservice", "--user",user_id,
           "com.dongnao.socketprocess/com.dongnao.socketprocess.ProcessSocketService", (char*)NULL);

    return NULL;
}
