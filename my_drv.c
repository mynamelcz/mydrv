#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

#define DEV_NAME "my_drv"
#define MOD_NAME "xyz"

static struct class *mydrv_class;
static struct class_device	*mydrv_class_dev;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;
volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

struct pin_dec{
    unsigned int pin;
    unsigned int key_val;
};
struct pin_dec pins_dec[4] = {
    {S3C2410_GPF0, 0x01},
    {S3C2410_GPF2, 0x02},
    {S3C2410_GPG3, 0x03},
    {S3C2410_GPG11, 0x04},
};
int key_val_g;
static volatile int ev_press = 0;  
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);  
static irqreturn_t key_irq_handle(int irq, void *dev_id)
{
    struct pin_dec *pins_t = (struct pin_dec *)dev_id;
    unsigned int pinval;

    pinval =  s3c2410_gpio_getpin(pins_t->pin);
    if(pinval)
    {
        key_val_g = 0x80 | pins_t->key_val;    
    }
    else
    {
        key_val_g =  pins_t->key_val;    
    }

    printk("irq id: %d\n",irq);
    ev_press = 1;  
    wake_up_interruptible(&button_waitq); 
    return IRQ_RETVAL(IRQ_HANDLED); 
}

static int my_drv_open(struct inode *inode, struct file *file)
{
    request_irq(IRQ_EINT0, key_irq_handle, IRQT_BOTHEDGE, "key0", &pins_dec[0]);/**/
    request_irq(IRQ_EINT2, key_irq_handle, IRQT_BOTHEDGE, "key1", &pins_dec[1]);/**/
    request_irq(IRQ_EINT3, key_irq_handle, IRQT_BOTHEDGE, "key2", &pins_dec[2]);/**/
    request_irq(IRQ_EINT11, key_irq_handle, IRQT_BOTHEDGE, "key3",&pins_dec[3]);/**/
	return 0;
}

ssize_t my_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{

    if(size != 1)
        return -EINVAL;
    
    wait_event_interruptible(button_waitq, ev_press);
    copy_to_user(buf, &key_val_g, 1);
    ev_press = 0; 
	return 1;
}
int my_drv_close(struct inode *inode, struct file *file)
{
   free_irq(IRQ_EINT0, &pins_dec[0]);
   free_irq(IRQ_EINT2, &pins_dec[1]);
   free_irq(IRQ_EINT3, &pins_dec[2]);
   free_irq(IRQ_EINT11, &pins_dec[3]);
   return 0;
}

static struct file_operations my_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   my_drv_open,     
	.read	=	my_drv_read,	   
    .release=   my_drv_close,
};


int major;
static int my_drv_init(void)
{
	major = register_chrdev(0, DEV_NAME, &my_drv_fops);

	mydrv_class = class_create(THIS_MODULE, DEV_NAME);

	mydrv_class_dev = class_device_create(mydrv_class, NULL, MKDEV(major, 0), NULL, MOD_NAME); /* /dev/buttons */

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void my_drv_exit(void)
{
	unregister_chrdev(major, DEV_NAME);
	class_device_unregister(mydrv_class_dev);
	class_destroy(mydrv_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}


module_init(my_drv_init);

module_exit(my_drv_exit);

MODULE_LICENSE("GPL");

