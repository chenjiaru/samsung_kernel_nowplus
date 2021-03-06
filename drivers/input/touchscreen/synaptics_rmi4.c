/*
 * Synaptics RMI4 touchscreen driver
 *
 * Copyright (C) 2010
 * Author: Joerie de Gram <j.de.gram@gmail.com>
 * Modified for i8320 replaced ts by <millay2630@yahoo.com>
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/regulator/consumer.h>
#define DRIVER_NAME "synaptics_rmi4_i2c"
#define GPIO_IRQ	142

#define ABS_X_MAX	480
#define ABS_Y_MAX	800

//#define TP_USE_WORKQUEUE
extern int nowplus_enable_touch_pins(int enable);//r3d4 added in nowplus_board_config, enable pins here

static void synaptics_rmi4_early_suspend(struct early_suspend *handler);
static void synaptics_rmi4_late_resume(struct early_suspend *handler);

static struct workqueue_struct *synaptics_wq;

struct synaptics_rmi4_data {
	struct input_dev *input;
	struct i2c_client *client;
	struct hrtimer timer;
#ifdef TP_USE_WORKQUEUE
	struct work_struct work;
#endif	
	unsigned int irq;
	unsigned char f01_control_base;
	unsigned char f01_data_base;
	unsigned char f11_data_base;

	struct regulator	*regulator;
	struct early_suspend	early_suspend;
};

#ifdef TP_USE_WORKQUEUE
static enum hrtimer_restart synaptics_rmi4_timer_func(struct hrtimer *timer)
{
	printk("%s() called\n",__FUNCTION__);

	struct synaptics_rmi4_data *ts = container_of(timer, struct synaptics_rmi4_data, timer);

	queue_work(synaptics_wq, &ts->work);

	hrtimer_start(&ts->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}
static void synaptics_rmi4_work(struct work_struct *work)
{	

	struct synaptics_rmi4_data *ts = container_of(work, struct synaptics_rmi4_data, work);

	/*register status*/
	int i;
	s32 finger_pressure,finger_contact;
	int x,y;
	unsigned char val[27];
	i2c_smbus_read_i2c_block_data(ts->client,0,27,val);
	finger_pressure	=	val[26];	
	finger_contact	=	i2c_smbus_read_byte_data(ts->client,12);
	
		
	/*	for(i=0;i<27;i++)
		printk("%02x ",val[i]);
	printk("\n");
	
	/* report to input subsystem */
	 
	//x = val[2] << 8 | val[3];//x
	//y = val[4] << 8 | val[5];//y
	/*if(finger_pressure < 5)
		{
		if(finger_pressure < 4)goto goback;
		finger_contact = 0;finger_pressure = 3;
		}*/
	input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR,finger_pressure);//finger contact finger_contact
	input_report_abs(ts->input, ABS_MT_WIDTH_MAJOR,finger_contact);//pressure finger_pressure
	x = val[22] << 4 | (val[24] & 0x0f);//x
	y = val[23] << 4 | (val[24] & 0xf0) >> 4;//y

	//input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR,finger_contact*5);//finger contact
	//input_report_abs(ts->input, ABS_MT_WIDTH_MAJOR,finger_pressure);//pressure
	if (x > 479)
		x = 479;
	if (y > 799)
		y = 799;
	input_report_abs(ts->input, ABS_MT_POSITION_X,x - 1);//x
	input_report_abs(ts->input, ABS_MT_POSITION_Y,y - 1);//y

	input_mt_sync(ts->input);

	input_sync(ts->input);


//debug*lcq*****************
	//printk("X= %d Y= %d finger_contact= %d finger_pressure= %d\n",x,y,finger_contact, finger_pressure );
	
	//printk("[TSP-LCQ]: X= %d Y= %d CONTACT= %d SIZE= %d R13= %d=%02x\n",(val[2] << 8 | val[3]), (val[4] << 8 | val[5]), val[1],val[12], val[13],val[13] );
//end-debug-lcq


goback:
	if(ts->irq)
	
	{
		/* Clear interrupt status register */
		//printk("[TSP-LCQ] Clear interrupt status register. ts->irq= %d\n",ts->irq);	
		i2c_smbus_read_word_data(ts->client, ts->f01_data_base + 1);
		enable_irq(ts->irq);
	}

}
static void synaptics_rmi4_irq_setup(struct synaptics_rmi4_data *ts)
{
	printk("[TSP]%s() called\n",__FUNCTION__);
	i2c_smbus_write_byte_data(ts->client,ts->f01_control_base + 1, 0x01);
}

static irqreturn_t synaptics_rmi4_irq_handler(int irq, void *dev_id)
{
	//printk("%s() called\n",__FUNCTION__);
	struct synaptics_rmi4_data *ts = dev_id;

	disable_irq_nosync(ts->irq);
	//udelay(10);
	queue_work(synaptics_wq, &ts->work);
	//udelay(1);
	return IRQ_HANDLED;
}
#else
static irqreturn_t synaptics_rmi4_irq_handler(int irq, void *dev_id)
{	
	struct synaptics_rmi4_data *ts = dev_id;

	/*register status*/
	int i;
	s32 finger_pressure,finger_contact;
	int x,y;
	unsigned char val[27];
	i2c_smbus_read_i2c_block_data(ts->client,0,27,val);
	finger_pressure =	val[26];	
	finger_contact	=	i2c_smbus_read_byte_data(ts->client,12);
	
/*	for(i=0;i<27;i++)
		printk("%02x ",val[i]);
	printk("\n");
	
	/* report to input subsystem */
	 
	//x = val[2] << 8 | val[3];//x
	//y = val[4] << 8 | val[5];//y
	/*if(finger_pressure < 5)
		{
		if(finger_pressure < 4)goto goback;
		finger_contact = 0;finger_pressure = 3;
		}*/
	input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR,finger_pressure);//finger contact finger_contact
	input_report_abs(ts->input, ABS_MT_WIDTH_MAJOR,finger_contact);//pressure finger_pressure
	x = val[22] << 4 | (val[24] & 0x0f);//x
	y = val[23] << 4 | (val[24] & 0xf0) >> 4;//y

	//input_report_abs(ts->input, ABS_MT_TOUCH_MAJOR,finger_contact*5);//finger contact
	//input_report_abs(ts->input, ABS_MT_WIDTH_MAJOR,finger_pressure);//pressure
	if (x > 479)
		x = 479;
	if (y > 799)
		y = 799;
	input_report_abs(ts->input, ABS_MT_POSITION_X,x - 1);//x
	input_report_abs(ts->input, ABS_MT_POSITION_Y,y - 1);//y

	input_mt_sync(ts->input);

	input_sync(ts->input);

//debug*lcq*****************
	//printk("X= %d Y= %d finger_contact= %d finger_pressure= %d\n",x,y,finger_contact, finger_pressure );
	
	//printk("[TSP-LCQ]: X= %d Y= %d CONTACT= %d SIZE= %d R13= %d=%02x\n",(val[2] << 8 | val[3]), (val[4] << 8 | val[5]), val[1],val[12], val[13],val[13] );
//end-debug-lcq


goback:
	if(ts->irq)
	
	{
		/* Clear interrupt status register */
		//printk("[TSP-LCQ] Clear interrupt status register. ts->irq= %d\n",ts->irq);	
		i2c_smbus_read_word_data(ts->client, ts->f01_data_base + 1);
	}
	return IRQ_HANDLED;
}

#endif

static int __devinit synaptics_rmi4_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk("[TSP]%s() called\n",__FUNCTION__);
	struct synaptics_rmi4_data *ts;
	struct input_dev *input_dev;
	int err;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_WORD_DATA)) {
		return -EIO;
	}

	ts = kzalloc(sizeof(struct synaptics_rmi4_data), GFP_KERNEL);
	input_dev = input_allocate_device();

	if (!ts || !input_dev) {
		printk("synaptics-rmi4: failed to allocate memory\n");
		err = -ENOMEM;
		goto err_free_mem;
	}
	
	ts->regulator = regulator_get(&client->dev, "vaux2");
	if (IS_ERR(ts->regulator)) {
		dev_err(&client->dev, "%s:get regulator failed\n",
							__func__);
		err = -ENOMEM;//PTR_ERR(ts->regulator);
		goto err_regulator;
	}
	regulator_enable(ts->regulator);

	
	nowplus_enable_touch_pins(1);//r3d4 added!
	i2c_set_clientdata(client, ts);

	ts->irq = GPIO_IRQ;
	ts->f01_control_base = 0x23;
	ts->f01_data_base = 0x13;
	ts->f11_data_base = 0x15; /* FIXME */

	ts->client = client;
	ts->input = input_dev;

#ifdef TP_USE_WORKQUEUE
	INIT_WORK(&ts->work, synaptics_rmi4_work);
#endif
//	input_dev->name = "Synaptics RMI4 touchscreen";
//	input_dev->phys = "synaptics-rmi4/input0";

	input_dev->name = "synaptics_rmi4_i2c";
	input_dev->phys = "Synaptics_Clearpad";

	input_dev->id.bustype = BUS_I2C;

	//input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	//input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_dev->evbit[0] = BIT_MASK(EV_ABS);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ABS_X_MAX, 0, 0);	
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, ABS_Y_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 15 , 0, 0);



	err = input_register_device(input_dev);
	if (err) {
		printk("synaptics-rmi4: failed to register input device\n");
		goto err_free_mem;
	}

#ifdef TP_USE_WORKQUEUE

	if(ts->irq) {
		err = request_irq(gpio_to_irq(ts->irq), synaptics_rmi4_irq_handler, IRQF_TRIGGER_FALLING, DRIVER_NAME, ts);
	}

	if(!err && ts->irq) {
		synaptics_rmi4_irq_setup(ts);
	} else {
		printk("synaptics-rmi4: GPIO IRQ missing, falling back to polled mode\n");
		ts->irq = 0;

		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = synaptics_rmi4_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}
#else
	if(ts->irq)
		err = request_threaded_irq(gpio_to_irq(ts->irq), NULL, synaptics_rmi4_irq_handler,IRQF_TRIGGER_FALLING, DRIVER_NAME, ts);		
	else {
		printk("[TSP] no irq!\n");
		goto err_regulator;	
	}
		
	if(err) {
		printk("[TSP] irq request error!\n");
		goto err_regulator;	
	}
#endif
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = synaptics_rmi4_early_suspend;
	ts->early_suspend.resume = synaptics_rmi4_late_resume;
	register_early_suspend(&ts->early_suspend);
	return 0;

err_free_mem:
	regulator_disable(ts->regulator);
	regulator_put(ts->regulator);
	
err_regulator:
	input_free_device(input_dev);
	kfree(ts);
	return err;
}

static int __devexit synaptics_rmi4_remove(struct i2c_client *client)
{
	printk("[TSP]%s() called\n",__FUNCTION__);
	struct synaptics_rmi4_data *ts = i2c_get_clientdata(client);
	input_unregister_device(ts->input);

	if(ts->irq) {
		free_irq(ts->irq, ts);
	}
#ifdef TP_USE_WORKQUEUE
	else {
		hrtimer_cancel(&ts->timer);
	}
#endif
	regulator_disable(ts->regulator);
	regulator_put(ts->regulator);
	kfree(ts);

	return 0;
}

static int synaptics_rmi4_suspend(struct i2c_client *client, pm_message_t msg)
{
	/*touch sleep mode*/
	int ret;
	struct synaptics_rmi4_data *ts = i2c_get_clientdata(client);
	if(ts->irq)
		disable_irq(ts->irq);
#ifdef TP_USE_WORKQUEUE
	else
		hrtimer_cancel(&ts->timer);
#endif
	#if 0
	ret = cancel_work_sync(&ts->work);
	if (ret && ts->irq)
		enable_irq(ts->irq);
	#endif
	//i2c_smbus_read_word_data(ts->client,20);
	nowplus_enable_touch_pins(0);
	//regulator_disable(ts->regulator);
	printk("[TSP] touchscreen suspend!\n");
	return 0;
}

static int synaptics_rmi4_resume(struct i2c_client *client)
{
	/*touch sleep mode*/

	struct synaptics_rmi4_data *ts = i2c_get_clientdata(client);
	//regulator_enable(ts->regulator);
	nowplus_enable_touch_pins(1);
	if (ts->irq)
		enable_irq(ts->irq);
#ifdef TP_USE_WORKQUEUE
	if (!ts->irq)
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
#endif
	printk("[TSP] touchscreen resume!\n");
	return 0;
}

static void synaptics_rmi4_early_suspend(struct early_suspend *h)
{
	printk("[TSP]%s() called\n",__FUNCTION__);
	struct synaptics_rmi4_data *ts;
	ts = container_of(h, struct synaptics_rmi4_data, early_suspend);
	synaptics_rmi4_suspend(ts->client, PMSG_SUSPEND);
}

static void synaptics_rmi4_late_resume(struct early_suspend *h)
{
	printk("[TSP]%s() called\n",__FUNCTION__);
	struct synaptics_rmi4_data *ts;
	ts = container_of(h, struct synaptics_rmi4_data, early_suspend);
	synaptics_rmi4_resume(ts->client);
}

static struct i2c_device_id synaptics_rmi4_idtable[] = {
	{ DRIVER_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, synaptics_rmi4_idtable);

static struct i2c_driver synaptics_rmi4_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= DRIVER_NAME,
	},	
	
	.probe		= synaptics_rmi4_probe,
	.remove		= __devexit_p(synaptics_rmi4_remove),
	.suspend	= synaptics_rmi4_suspend,
	.resume		= synaptics_rmi4_resume,	
	.id_table	= synaptics_rmi4_idtable,
};

static int __init synaptics_rmi4_init(void)
{
	synaptics_wq = create_singlethread_workqueue("synaptics_wq");
	if (!synaptics_wq) {
		return -ENOMEM;
	}

	return i2c_add_driver(&synaptics_rmi4_driver);
}

static void __exit synaptics_rmi4_exit(void)
{
	i2c_del_driver(&synaptics_rmi4_driver);
}

module_init(synaptics_rmi4_init);
module_exit(synaptics_rmi4_exit);

MODULE_AUTHOR("Joerie de Gram <j.de.gram@gmail.com");
MODULE_DESCRIPTION("Synaptics RMI4 touchscreen driver");
MODULE_LICENSE("GPL");

