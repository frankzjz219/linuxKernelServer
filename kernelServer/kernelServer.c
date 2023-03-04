#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <net/sock.h>
#include <net/netlink.h>
/*************创建内核线程需要的库***************/
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()、kthread_run()

/*************互斥锁用的库********************/
#include <linux/mutex.h>


#define KERNEL_SERVER_CNT 1
#define KERNEL_SERVER_NAME "kernelServer"
#define KSERVER_UNIT 20
#define MAX_MSGSIZE 1024
#define MAX_CLIENT 10
// 处理的字数的统计
static int wordCnt = 0;
// 字符串处理回调函数指针
void (*strProcess)(char*);
/****************内核服务器部分定义**************/

struct sock *nl_sk = NULL;
struct task_struct* threadStruct;
/*********************客户端计数部分以及对应的互斥锁******************/
static int cnt = 0;
struct mutex cntMutex;

/************************************************************/
int sendMsg(char * mes, int pid)
{

    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    int len = NLMSG_SPACE(MAX_MSGSIZE);
    printk(KERN_INFO "pid:%d\n", pid);
    skb = alloc_skb(len, GFP_KERNEL);
    if (!skb) 
    {
        printk(KERN_ERR "send_msg:alloc_skb error\n");
        return -1;
    }
    nlh = nlmsg_put(skb, 0, 0, 0, MAX_MSGSIZE, 0);
    NETLINK_CB(skb).portid = 0;
    NETLINK_CB(skb).dst_group = 0;
    strcpy(NLMSG_DATA(nlh), mes);
    // printk(KERN_INFO "my_net_link:send message '%s'.\n",(char *)NLMSG_DATA(nlh));
    netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);
    return 0;
}
//更改所有英文字母为大写
void toUpperCase(char* strToProc)
{
    int i = 0;
    for(i = 0;i<strlen(strToProc);++i)
    {
        //每个字符延时200ms
        mdelay(200);
        if(strToProc[i]>='a'&&strToProc[i]<='z')
        {
            strToProc[i] -= 'a'-'A';
        }
    }
}
//更改所有字母为小写
void toLowerCase(char* strToProc)
{
    int i = 0;
    for(i = 0;i<strlen(strToProc);++i)
    {
        //每个字符延时200ms
        mdelay(200);
        if(strToProc[i]>='A'&&strToProc[i]<='Z')
        {
            strToProc[i] += 'a'-'A';
        }
    }
}
//不改变字符串
void keepSame(char* strToProc)
{
    int i = 0;
    for(i = 0;i<strlen(strToProc);++i)
    {
        //每个字符延时200ms
        mdelay(200);
    }
}

/***********线程工作函数**************/
int handleClient(void *skb)
{
    int pid;
    char* strToProc;
    char* strBuff;
    struct nlmsghdr *nlh = nlmsg_hdr(skb);
    /**************获取锁以独自访问计数变量**************/
    mutex_lock(&cntMutex);
    if(cnt>MAX_CLIENT)
    {
        return -1;
    }
    ++cnt;
    mutex_unlock(&cntMutex);
    /********************************************************/
    printk(KERN_NOTICE "Message %s received!\r\n", (char*)NLMSG_DATA(nlh));
    pid = nlh->nlmsg_pid;
    // 处理并且回显数据
    strToProc =(char*)NLMSG_DATA(nlh);
    // 计数总文字数
    wordCnt+=strlen(strToProc);
    // 字符串空间分配
    strBuff = kmalloc(strlen(strToProc)+5, GFP_KERNEL);
    if(!strBuff)
    {
        printk(KERN_ERR "Alloc String Buffer Failure!\n");
        return -1;
    }
    strcpy(strBuff, strToProc);
    // 处理字符串
    strProcess(strBuff);
    int ret = sendMsg(strBuff, pid);
    if(0>ret)
    {
        printk(KERN_ERR "Netlink Error!\n");
        return -1;
    }
    /************计数变量-1**************/
    mutex_lock(&cntMutex);
    --cnt;
    mutex_unlock(&cntMutex);
    /***********************************/
    return 0;
}

void recMsg(struct sk_buff *skb)
{
    /*************直接创建新的线程*************/
    threadStruct = kthread_create(handleClient, (void*)skb, "handleClient");
    wake_up_process(threadStruct);
}

struct netlink_kernel_cfg nl_kernel_cfg = {
        // .groups = 0,
        // .flags = 0,
        .input = recMsg,
        // .cb_mutex = NULL,
        // .bind = NULL,
        // .compare = NULL,
};



void netlinkInit(void)
{
    nl_sk = netlink_kernel_create(&init_net, KSERVER_UNIT, &nl_kernel_cfg);
    if(!nl_sk)
    {
        printk(KERN_ERR "my_net_link: create netlink socket error.\n");
    }
}

/**********************************************************/

typedef struct 
{
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class *class;		/* 类 		*/
	struct device *device;	/* 设备 	 */
	int major;				/* 主设备号	  */
	int minor;				/* 次设备号   */
} kernelServer;

static kernelServer kstruct = {0};
static struct proc_dir_entry *proc_file = NULL;
static struct proc_dir_entry *proc_dir = NULL;

// static int demo_show(struct seq_file *seq, void *v)
// {
//     seq_puts(seq, "Hello!\n");
//     return 0;        
// }
/****************用户通过proc写入数据时的会回调函数*****/
static ssize_t demo_set(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char kBuf[20];
    memset(kBuf, 0, sizeof(kBuf));
    copy_from_user(kBuf, buffer, count);
    // kBuf[count] = '\0';
    // printk(KERN_ALERT "Data got is %s. \r\n", kBuf);

    // 用户输入0切换为统一转换为大写
    // 输入1切换为统一转换为小写
    // 输入2切换为不改变内容
    switch (kBuf[0])
    {
    case '0':
        strProcess = toUpperCase;
        break;
    case '1':
        strProcess = toLowerCase;
        break;
    case '2':
        strProcess = keepSame;
        break;
    default:
        break;
    }
    return 0;
}

// static int demo_open(struct inode *inode, struct file *file)
// {
//     return simple_open(inode, file);
// }

/****************用户通过proc读取数据时的会回调函数*****/
ssize_t demo_read(struct file * f, char __user * buffer, size_t count, loff_t * offset)
{
    char greetings[100];
    sprintf(greetings, "Processed words: %d\r\nConnecting client: %d\r\n", wordCnt, cnt);
    copy_to_user(buffer, greetings, strlen(greetings)+1);
    return strlen(greetings);
}

/*************模块本身的文件操作函数***********/
struct file_operations serverOptions = 
{
    .owner = THIS_MODULE,
};

static int __init kernelServer_init(void)
{
    /***************创建/proc下的文件*********************/
    struct proc_ops procOps = 
    {
        // .proc_open = demo_open,
        // .owner = THIS_MODULE,
        .proc_write = demo_set,
        .proc_read = demo_read,

    };
    proc_dir = proc_mkdir("kernelServer", NULL);
    if(proc_dir)
    {
        proc_file = proc_create("server", 0666, proc_dir, &procOps);
        printk(KERN_ALERT "procFile created!\r\n");
    }
    /*************************************************/
    

    // 注册设备
    if(kstruct.major) // 已经具有设备号
    {
        kstruct.devid = MKDEV(kstruct.major, 0);
        register_chrdev_region(kstruct.devid, KERNEL_SERVER_CNT, KERNEL_SERVER_NAME);
    }
    // 没有设备号
    else
    {
        alloc_chrdev_region(&kstruct.devid, 0, KERNEL_SERVER_CNT, KERNEL_SERVER_NAME);
        kstruct.major = MAJOR(kstruct.devid);
        kstruct.minor = MINOR(kstruct.devid);
    }
    // 初始化cdev
    kstruct.cdev.owner = THIS_MODULE;
    cdev_init(&kstruct.cdev, &serverOptions);
    // 添加cdev
    cdev_add(&kstruct.cdev, kstruct.devid, KERNEL_SERVER_CNT);
    // 创建类
    kstruct.class = class_create(THIS_MODULE, KERNEL_SERVER_NAME);
    // 创建设备
    kstruct.device = device_create(kstruct.class, NULL, kstruct.devid, NULL, KERNEL_SERVER_NAME);
    printk(KERN_ALERT"Char device mounted!\r\n");
    /**************************************************************/
    mutex_init(&cntMutex);
    /************初始化互斥锁*****************/
    netlinkInit();
    /*********************创建netlink完成*************************/
    /****************给出缺省的字符串处理函数******************/
    strProcess = toUpperCase;
    return 0;
}

static void __exit kernelServer_exit(void)
{
    /*******************注销netlink***********/
    if(nl_sk)
    {
        sock_release(nl_sk->sk_socket);
    }


    /*************删除/proc下的文件*******************/
    // remove_proc_entry("driver/kernelServer", NULL);
    proc_remove(proc_file);
    /*******************************************/
    // 注销设备
    cdev_del(&kstruct.cdev);
    unregister_chrdev_region(kstruct.devid, KERNEL_SERVER_CNT);

    device_destroy(kstruct.class, kstruct.devid);
    class_destroy(kstruct.class);
    /*********************************************/
    printk(KERN_ALERT "Char device unmounted!\r\n");

}

module_init(kernelServer_init);
module_exit(kernelServer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("FrankZhang");