#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>

#define KSERVER_UNIT 20
#define MAX_PAYLOAD 1024
#define SRC_PORT 40001


// gcc clientSocket.c -o client

int main()
{
    int state;
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;
    int sock_fd, retval;
    int state_smg = 0;

    // char* buf[10];
    int clientCnt = 0;
    FILE* fp = fopen("/root/serverLock", "r+");
    if(!fp)
    {
        // 不存在就创建
        fp = fopen("/root/serverLock", "w");
        fclose(fp);
    }
    int temp = fread(&clientCnt, 4, 1, fp);
    // 文件没有内容
    if(temp<1)
    {
        clientCnt = 0;
    }
    else
    {
        if(clientCnt>=5)
        {
            printf("Client Maximum reached!\n");
            return -1;
        }
        clientCnt+=1;
        fseek(fp, 0, 0);
        fwrite(&clientCnt, 4, 1, fp);
        fclose(fp);
    }

    /**创建socket***/
    sock_fd = socket(AF_NETLINK, SOCK_RAW, KSERVER_UNIT);
    if(0>sock_fd)
    {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }

    /*****配置网络*****/
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = SRC_PORT; //设置源端端口号
    src_addr.nl_groups = 0;

    /*********Bind********/
    retval = bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
    if (retval < 0) {
            printf("bind failed: %s", strerror(errno));
            close(sock_fd);
            return -1;
    }
    /*********给消息分配内存********/
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh) {
            printf("malloc nlmsghdr error!\n");
            close(sock_fd);
            return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = SRC_PORT; //设置源端口
    nlh->nlmsg_flags = 0;
    strcpy(NLMSG_DATA(nlh), "Hello, kernel!"); //设置消息体
    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

    /***********创建消息体**********/
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    /*************发送数据*************/
    printf("send_smg\n");
    state_smg = sendmsg(sock_fd,&msg,0);
    if (state_smg == -1) {
            printf("get error sendmsg = %s\n",strerror(errno));
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

    /********************接收数据****************/
    printf("waiting received!\n");
    state = recvmsg(sock_fd, &msg, 0);
    if (state < 0) {
            printf("state<1");
    }
    printf("Received message: %s\n", (char*)NLMSG_DATA(nlh));

    close(sock_fd);

    /***************写回文件***************/
    clientCnt--;
    fp = fopen("/root/serverLock", "w");
    fwrite(&clientCnt, 4, 1, fp);
    fclose(fp);
    return 0;
}