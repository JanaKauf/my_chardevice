/* my_chardevice.c
 *
 * Copyright 2018 JanaKauf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/uaccess.h>

#include <linux/semaphore.h>

//#include <linux/string.h>

#include "my_chardevice.h"

#define MY_DEVICE_NAME "my_chardevice"

MODULE_DESCRIPTION("my chardevice");
MODULE_AUTHOR("Janaina");
MODULE_LICENSE("GPL");

struct my_device_data {
    char data[100];
    int data_lenght;
    struct semaphore sem;
} my_device;

struct cdev * my_cdev;
int my_major;
int ret;
dev_t dev_num;

static ssize_t
my_write (struct file * file, const char __user * user_buffer, size_t size, loff_t * offset) {
    PRINT_DEBUG;
    sprintf(my_device.data, "%s(%zu letters)", user_buffer, size);
    my_device.data_lenght = strlen(my_device.data);
    return size;
}

static int
my_open(struct inode *inode, struct file *file) {
    PRINT_DEBUG;
    if (down_interruptible(&my_device.sem) != 0 ) {
        printk(KERN_ALERT "my_chardevice : could not lock device during open\n");
        return -1;
    }

    printk(KERN_INFO "my_chardevice: opened device\n");
    return 0;
}

static ssize_t
my_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    PRINT_DEBUG;
    ret = copy_to_user(user_buffer, my_device.data, my_device.data_lenght);
    if (ret < 0) {
        return -EFAULT;
    }

    printk(KERN_INFO "my_chardevice: Sent %d characters to the user\n", my_device.data_lenght);

    return (my_device.data_lenght = 0);
}

static int
my_close (struct inode * inode, struct file *file) {
   PRINT_DEBUG;
    up(&my_device.sem);
    return 0;
}

const struct file_operations chardevice_fops = {
   .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
    .release = my_close
};

static int
my_init(void) {
    PRINT_DEBUG;

    ret = alloc_chrdev_region(&dev_num, 0, 1, MY_DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "faild to allocate mojor number\n");
        return ret;
    }

    my_major = MAJOR(dev_num);

    printk(KERN_INFO "my_chardevice: major number is %d\n", my_major);

    printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\" for device file\n", MY_DEVICE_NAME, my_major);

    my_cdev = cdev_alloc();
    my_cdev->ops = &chardevice_fops;
    my_cdev->owner = THIS_MODULE;

    ret = cdev_add(my_cdev, dev_num, 1);
    if (ret < 0) {
        printk(KERN_ALERT "faild to add cdevto kernel\n");

    }

    sema_init(&my_device.sem, 1);
    return 0;
}

static void
my_exit(void) {
    PRINT_DEBUG;

    cdev_del(my_cdev);
    unregister_chrdev_region(dev_num, 1);

}

module_init(my_init);
module_exit(my_exit);
