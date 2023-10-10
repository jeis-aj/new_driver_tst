#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/io.h>


// define .begin
#define	gpio_b_base_addr	(0x20A4000)
#define		gpio_b_dr			gpio_b_base_addr
#define		gpio_b_gdir			(gpio_b_base_addr+0x04)
#define 	led_blue_pin		(26)
#define 	WR_VALUE	_IOW('d','a', int32_t *)
#define 	RD_VALUE	_IOW('d','b', int32_t *)
#define 	ON		_IOW('d','b', int32_t *)
#define 	OFF		_IOW('d','c', int32_t *)


// global var
static void __iomem*		gpio_b_gdir_vm;
static void __iomem*		gpio_b_dr_vm  ;
static struct device * my_device ;
static struct class * my_class ;
static int major	;
static char str_cmd [20] ;

// fn prototype
static void setup (void);
static ssize_t read_fn (struct file *, char *, size_t, loff_t *);
static ssize_t write_fn (struct file *, const char *, size_t, loff_t *);
static int open_fn (struct inode *, struct file *);
static int release_fn (struct inode *, struct file *);
static long ioctl_fn(struct file *fl,unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
	owner: THIS_MODULE,
	read : read_fn,
	write : write_fn,
	open : open_fn,
	release : release_fn,
	unlocked_ioctl : ioctl_fn
};

#define DEVICE_NAME  "my_driver"
#define DEVICE_CLASS "my_driver_class" 


MODULE_LICENSE("GPL");
MODULE_AUTHOR("ARUN JYOTHISH K");
MODULE_DESCRIPTION(" device description my device driver .. ");
MODULE_VERSION("1.0");

// fn definition 

static long ioctl_fn(struct file *fl,unsigned int cmd, unsigned long arg){

	printk(KERN_INFO "\n cmd rec: %d\n", cmd);
	switch(cmd){
		case ON:
			iowrite32( 0b1 << led_blue_pin, gpio_b_dr_vm);		// turn on led 
			break;	
		case OFF:
			iowrite32( 0b0 << led_blue_pin, gpio_b_dr_vm);		// turn off led 
			break;	
	}
	return 0;
}

static int my_driver_init(void){
	printk(KERN_INFO "Module intit fn");
	major = register_chrdev(	0,DEVICE_NAME , &fops);
	if (major < 0 ){
	printk(KERN_INFO "Driver register failed\n");
	return major;
	}
	printk(KERN_INFO "My_driver Major NO: %d",major);
///		create class
	my_class = class_create( THIS_MODULE, DEVICE_CLASS );
	if ( IS_ERR( my_class ) ){
		unregister_chrdev ( major , DEVICE_NAME );
		printk(KERN_ALERT "My_driver CLASS CREATE FAILED");
		return PTR_ERR(my_class);
	}
	printk(KERN_INFO "My_driver class created ");

///		create device 
	my_device = device_create( my_class, NULL ,MKDEV(major,0), NULL, DEVICE_NAME );
	if ( IS_ERR( my_device ) ){
		class_destroy( my_class );
		unregister_chrdev ( major , DEVICE_NAME );
		printk(KERN_ALERT "My_driver DEVICE CREATE FAILED");
		return PTR_ERR(my_device);
	}
	printk(KERN_INFO "device node created ... !");
	
	gpio_b_gdir_vm	= ioremap(gpio_b_gdir, sizeof(u32));
	gpio_b_dr_vm	= ioremap(gpio_b_dr, sizeof(u32));
	setup ();
	/* iowrite32( 0b0 << led_blue_pin, gpio_b_dr_vm);		// turn off led */ 
	return 0;

}

static void  my_driver_exit(void){
	iowrite32( 0b1 << led_blue_pin, gpio_b_dr_vm);		// turn on led 
	/* gpio_b_gdir_vm	= ioremap(gpio_b_gdir, sizeof(u32)); */
	iounmap(gpio_b_dr_vm);
	iounmap(gpio_b_gdir_vm);
	printk(KERN_INFO "Module exit fn called\n");
	device_destroy(my_class, MKDEV(major,0));
	unregister_chrdev(major, DEVICE_NAME);
	class_destroy(my_class);
	class_unregister(my_class);
	printk(KERN_INFO "Module exit fn");
}

module_init(my_driver_init);
module_exit(my_driver_exit);

static ssize_t read_fn (struct file *fl, char * ch, size_t e, loff_t *oth){
	printk(KERN_INFO "read_fn called !");
	copy_to_user(ch , str_cmd , sizeof(str_cmd));
	iowrite32( 0b0 << led_blue_pin, gpio_b_dr_vm);		// turn off led 
	return 0;
}
static ssize_t write_fn (struct file *fl, const char *ch, size_t e, loff_t *oth){
	int sz = sizeof(ch);
	printk(KERN_INFO "write_fn called ! cmd len: %d",sz);
	printk(KERN_INFO "cmd: %s\n",ch);
	if ( !strncmp("ON",ch ,2 )){
		iowrite32( 0b1 << led_blue_pin, gpio_b_dr_vm);		// turn on led 
		strcpy( str_cmd,"LED ON");
	}
	if ( !strncmp("OFF",ch,3 ) ){
		static char str_cmd[] = "LED OFF";
		iowrite32( 0b0 << led_blue_pin, gpio_b_dr_vm);		// turn off led 
		strcpy( str_cmd,"LED ON");
	}
	return sz;
}

static int open_fn (struct inode *lk, struct file *kl){
	printk(KERN_INFO "open_fn called !");
	return 0;
}

static int release_fn(struct inode *lk, struct file *kl){
	printk(KERN_INFO "close_fn called !");
	return 0;
}

static void setup (void){
		 u32 read  = ioread32(gpio_b_gdir_vm);		//  
		 /* printk(KERN_INFO "gdir gp3: %x",mask_pin); */
		 printk(KERN_INFO "reg val: %x",read);
		 iowrite32( 0b1 << led_blue_pin , gpio_b_gdir_vm);		// sets led pin as output
		 read  = ioread32(gpio_b_gdir_vm);		//  
		 printk(KERN_INFO "reg val: %x",read);
}
