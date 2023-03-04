#include <linux/module.h>
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xa7bd469c, "module_layout" },
	{ 0xbed13d6f, "class_destroy" },
	{ 0xf3095d09, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xcfb9462f, "cdev_del" },
	{ 0x7889a1f, "proc_remove" },
	{ 0x9a8fa03c, "sock_release" },
	{ 0x977f511b, "__mutex_init" },
	{ 0x5403ad78, "device_create" },
	{ 0xda792424, "__class_create" },
	{ 0x476579ae, "cdev_add" },
	{ 0x6d767a66, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0xbdefdf75, "proc_create" },
	{ 0xfad2e5ba, "proc_mkdir" },
	{ 0xdcdfd05e, "__netlink_kernel_create" },
	{ 0xd5cf6edc, "init_net" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0x37bca627, "netlink_unicast" },
	{ 0xe914e41e, "strcpy" },
	{ 0x144f813f, "__nlmsg_put" },
	{ 0x72bd8f79, "__alloc_skb" },
	{ 0xc5850110, "printk" },
	{ 0x56470118, "__warn_printk" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xb44ad4b3, "_copy_to_user" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x4cd60ca4, "wake_up_process" },
	{ 0x89321514, "kthread_create_on_node" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x754d539c, "strlen" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "EA63A5B3278AFC75F9CB200");
