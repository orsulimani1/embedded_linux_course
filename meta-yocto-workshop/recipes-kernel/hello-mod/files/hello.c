#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ors");
MODULE_DESCRIPTION("A simple Hello World kernel module");

static int __init hello_init(void)
{
    pr_info("Hello Kernel: Module loaded!\n");
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("Hello Kernel: Module unloaded!\n");
}

module_init(hello_init);
module_exit(hello_exit);
