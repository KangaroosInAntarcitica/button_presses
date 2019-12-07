#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>  
#include <linux/kobject.h>
#include <linux/kthread.h> 
#include <linux/delay.h> 

#include <asm/irq.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include "gpio_lkm.h"
#include "button-listener.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andriy Dmytruk");
MODULE_DESCRIPTION("");
MODULE_VERSION("0.1");

static unsigned int gpio_led = 16;
module_param(gpio_led, uint, 0660);
MODULE_PARM_DESC(gpio_led, "GPIO LED number (default=16)");

static unsigned int gpio_button = 19;
module_param(gpio_button, uint, 0660);
MODULE_PARM_DESC(gpio_button, "GPIO BUTTON number (default=19)");

static char group_name[10] = "buttonXXX";
static int press_count = 0;

static ssize_t count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   return sprintf(buf, "%d\n", press_count);
}

static ssize_t count_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
    int p_count;
    sscanf(buf, "%d\n", &p_count);
    press_count = p_count;
    return p_count;
}

extern int get_count(void) {
    return press_count;
}
EXPORT_SYMBOL(get_count);

static struct kobj_attribute press_count_attr = __ATTR(press_count, 0660, count_show, count_store);

static struct attribute *attrs[] = {&press_count_attr.attr, NULL};
static struct attribute_group attr_group = {.name = group_name, .attrs = attrs};

static struct kobject *kobj;

uint64_t last_press = 0;
static uint64_t debounce_time = (uint64_t) (600 * 1e6); // nanoseconds

static irqreturn_t press_handler(int irq, void *dev_id) {
   uint64_t current_press = ktime_to_ns(ktime_get());
   if (current_press - last_press > debounce_time) { 
      if (press_count == 10) return IRQ_RETVAL(1);

      last_press = current_press;
      ++press_count;
      printk("[PRESS COUNTER] Button pressed\n");

      if (press_count == 10) {
	 printk("[PRESS COUNTER] Turned on the LED\n");
	 gpio_set_value(gpio_led, 1); 
	 gpio_unavailable = true;
      }
   }
   return IRQ_RETVAL(1);
}

int int_number = -1;

static int __init press_counter_init(void){
   int result = 0;

   printk(KERN_INFO "[PRESS COUNTER] Initializing the PRESS COUNTER\n");
   sprintf(group_name, "button%d", gpio_button);

   gpio_lkm_free_gpio(gpio_led);
   gpio_lkm_free_gpio(gpio_button);

   // kernel object points to /sys/kernel
   kobj = kobject_create_and_add("press_counter", kernel_kobj->parent); 
   if(!kobj){
      printk(KERN_ALERT "[PRESS COUNTER] Failed to create kobject\n");
      return -ENOMEM;
   }

   // create group with the name of our led
   result = sysfs_create_group(kobj, &attr_group);
   if(result) {
      printk(KERN_ALERT "[PRESS COUNTER] Failed to create sysfs group\n");
      kobject_put(kobj);
      return result;
   }

   if (gpio_request(gpio_led, "sysfs")) {
	printk(KERN_ALERT "[PRESS COUNTER] Failed to request GPIO for LED\n");
   }
   if (gpio_request(gpio_button, "sysfs")) {
	printk(KERN_ALERT "[PRESS COUNTER] Failed to request GPIO for BUTTON\n");
   }

   if (gpio_direction_output(gpio_led, 0)) {
	printk(KERN_ALERT "[PRESS COUNTER] Failed to set GPIO mode for LED\n");
   }
   if (gpio_direction_input(gpio_button)) {
	printk(KERN_ALERT "[PRESS COUNTER] Failed to set GPIO mode for BUTTON\n");
   }

   gpio_set_debounce(gpio_button, 500000);

   gpio_export(gpio_led, false);
   gpio_export(gpio_button, false);

   int_number = gpio_to_irq(gpio_button);
   if (int_number < 0) {
	printk(KERN_ALERT "[PRESS COUNTER] Failed to get interrupt number for BUTTON\n");
   } else {
      int request_result = request_irq(int_number, press_handler, IRQF_TRIGGER_RISING, "button_listener_int", "button_listener"); 
      if (request_result) {
         printk(KERN_ALERT "[PRESS COUNTER] Failed to request interrupt");
      }
   }

   printk(KERN_INFO "[PRESS COUNTER] turned off the LED\n");

   return result;
}

static void __exit press_counter_exit(void){
   kobject_put(kobj);

   if (int_number > 0) {
      free_irq(int_number, "button_listener");
   }

   gpio_set_value(gpio_led, 0);
   gpio_unexport(gpio_led);
   gpio_free(gpio_led);

   gpio_direction_output(gpio_button, 0);
   gpio_unexport(gpio_button);
   gpio_free(gpio_button);

   printk(KERN_INFO "[PRESS COUNTER] Goodbye from the PRESS COUNTER\n");
}

module_init(press_counter_init);
module_exit(press_counter_exit);
