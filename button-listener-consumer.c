#include <linux/init.h>
#include <linux/module.h>
#include "button-listener.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andriy Dmytruk");

static int __init button_listener_consumer_init(void)
{
    printk("[PRESS CONSUMER] Initializing the PRESS CONSUMER\n");
    printk("[PRESS CONSUMER] Consumed value from PRESS COUNTER: %d\n", get_count());
    return 0;
}

static void __exit button_listener_consumer_exit(void)
{
    printk("[PRESS CONSUMER] Exiting the PRESS CONSUMER\n");
}

module_init(button_listener_consumer_init);
module_exit(button_listener_consumer_exit);
