/*****************************************************************************
 *Filename:
 *---------
 *    GC02M1_OFILM_MIPI_RAW_Sensor.c
 *
 *Project:
 *--------
 *    ALPS
 *
 *Description:
 *------------
 *    Source code of Sensor driver
 *
 *------------------------------------------------------------------------------
 *Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "gc02m1_ofilm_mipi_raw_Sensor.h"

#define PFX "GC02M1_OFILM_MIPI_RAW_camera_sensor"
#define LOG_1 LOG_INF("GC02M1_OFILM,MIPI 1LANE\n")
#define LOG_INF(format, args...)    pr_err(PFX "[%s] " format, __func__, ##args)

#define USER_OTP 0
#define MULTI_WRITE    1
#define VENDOR_ID 0x07

static DEFINE_SPINLOCK(imgsensor_drv_lock);

static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = GC02M1_OFILM_SENSOR_ID,
	.checksum_value = 0x5622f670,

	.pre = {
		.pclk = 84000000,
		.linelength = 2192,
		.framelength = 1276,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1600,
		.grabwindow_height = 1200,

		.mipi_data_lp2hs_settle_dc = 85,

		.mipi_pixel_rate = 67200000,
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 84000000,
		.linelength = 2192,
		.framelength = 1276,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1600,
		.grabwindow_height = 1200,

		.mipi_data_lp2hs_settle_dc = 85,

		.mipi_pixel_rate = 67200000,
		.max_framerate = 300,
	},
	.cap1 = {
		.pclk = 84000000,
		.linelength = 2192,
		.framelength = 1276,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1600,
		.grabwindow_height = 1200,

		.mipi_data_lp2hs_settle_dc = 85,

		.mipi_pixel_rate = 67200000,
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 84000000,
		.linelength = 2192,
		.framelength = 1276,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1600,
		.grabwindow_height = 1200,

		.mipi_data_lp2hs_settle_dc = 85,

		.mipi_pixel_rate = 67200000,
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 84000000,
		.linelength = 2192,
		.framelength = 1276,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1600,
		.grabwindow_height = 1200,

		.mipi_data_lp2hs_settle_dc = 85,

		.mipi_pixel_rate = 67200000,
		.max_framerate = 300,
	},
	.slim_video = {
		.pclk = 84000000,
		.linelength = 2192,
		.framelength = 1276,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1600,
		.grabwindow_height = 1200,

		.mipi_data_lp2hs_settle_dc = 85,

		.mipi_pixel_rate = 67200000,
		.max_framerate = 300,
	},
	.custom1 = {
		.pclk = 84000000,
		.linelength = 2192,
		.framelength = 1276,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1600,
		.grabwindow_height = 1200,

		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 67200000,

		.max_framerate = 300,
	},
	.custom2 = {
		.pclk = 84000000,
		.linelength = 2192,
		.framelength = 1276,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1600,
		.grabwindow_height = 1200,

		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 67200000,

		.max_framerate = 300,
	},
	.margin = 16,
	.min_shutter = 4,
	.min_gain = 64,
	.max_gain = 768,
	.min_gain_iso = 50,
	.gain_step = 1,
	.gain_type = 4,
	.max_frame_length = 0x3fff,
	.ae_shut_delay_frame = 0,
	.ae_sensor_gain_delay_frame = 0,
	.ae_ispGain_delay_frame = 2,
	.frame_time_delay_frame = 1,
	.ihdr_support = 0,
	.ihdr_le_firstline = 0,
	.sensor_mode_num = 7,

	.cap_delay_frame = 2,
	.pre_delay_frame = 2,
	.video_delay_frame = 2,
	.hs_video_delay_frame = 2,
	.slim_video_delay_frame = 2,
	.custom1_delay_frame = 2,
	.custom2_delay_frame = 2,

	.isp_driving_current = ISP_DRIVING_6MA,
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,
	.mipi_sensor_type = MIPI_OPHY_NCSI2,
	.mipi_settle_delay_mode = MIPI_SETTLEDELAY_AUTO,
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_R,
	.mclk = 24,
	.mipi_lane_num = SENSOR_MIPI_1_LANE,
	.i2c_addr_table = {0x6e, 0xff},
	.i2c_speed = 400,
};

static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,
	.sensor_mode = IMGSENSOR_MODE_INIT,
	.shutter = 0x3ED,
	.gain = 0x40,
	.dummy_pixel = 0,
	.dummy_line = 0,
	.current_fps = 300,
	.autoflicker_en = KAL_FALSE,
	.test_pattern = KAL_FALSE,
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,
	.ihdr_en = 0,
	.i2c_write_id = 0x6e,
};

static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[7] = {
	{
		1600, 1200, 0, 0, 1600, 1200, 1600, 1200, 0000, 0000, 1600,
		1200, 0, 0, 1600, 1200
	},
	{
		1600, 1200, 0, 0, 1600, 1200, 1600, 1200, 0000, 0000, 1600,
		1200, 0, 0, 1600, 1200
	},
	{
		1600, 1200, 0, 0, 1600, 1200, 1600, 1200, 0000, 0000, 1600,
		1200, 0, 0, 1600, 1200
	},
	{
		1600, 1200, 0, 0, 1600, 1200, 1600, 1200, 0000, 0000, 1600,
		1200, 0, 0, 1600, 1200
	},
	{
		1600, 1200, 0, 0, 1600, 1200, 1600, 1200, 0000, 0000, 1600,
		1200, 0, 0, 1600, 1200
	},
	{
		1600, 1200, 0, 0, 1600, 1200, 1600, 1200, 0000, 0000, 1600,
		1200, 0, 0, 1600, 1200
	},
	{
		1600, 1200, 0, 0, 1600, 1200, 1600, 1200, 0000, 0000, 1600,
		1200, 0, 0, 1600, 1200
	}
};

#if MULTI_WRITE
#define I2C_BUFFER_LEN    510
#else
#define I2C_BUFFER_LEN    2
#endif

static kal_uint16
gc02m1_ofilm_mipi_raw_table_write_cmos_sensor(kal_uint16 *para, kal_uint32 len)
{
	char puSendCmd[I2C_BUFFER_LEN];
	kal_uint32 tosend, IDX;
	kal_uint16 addr = 0, addr_last = 0, data;

	tosend = 0;
	IDX = 0;
	while (len > IDX) {
		addr = para[IDX];

		puSendCmd[tosend++] = (char)(addr & 0xff);
		data = para[IDX + 1];
		puSendCmd[tosend++] = (char)(data & 0xff);
		IDX += 2;
		addr_last = addr;

#if MULTI_WRITE
		if ((I2C_BUFFER_LEN - tosend) < 2 || IDX == len
				|| addr != addr_last) {
			LOG_INF("burst write\n");
			iBurstWriteReg_multi(puSendCmd, tosend,
					imgsensor.i2c_write_id, 2,
					imgsensor_info.i2c_speed);
			tosend = 0;
		}
#else
		iWriteRegI2CTiming(puSendCmd, 2, imgsensor.i2c_write_id,
						   imgsensor_info.i2c_speed);
		tosend = 0;
#endif
	}
	return 0;
}

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[1] = { (char)(addr & 0xff) };

	iReadRegI2C(pu_send_cmd, 1, (u8 *) &get_byte, 1,
				imgsensor.i2c_write_id);

	return get_byte;

}

static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[2] = { (char)(addr & 0xff), (char)(para & 0xff) };

	iWriteRegI2C(pu_send_cmd, 2, imgsensor.i2c_write_id);
}

static void set_dummy(void)
{
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x41, (imgsensor.frame_length >> 8) & 0x3f);
	write_cmos_sensor(0x42, imgsensor.frame_length & 0xff);
}

static kal_uint32 return_sensor_id(void)
{
	int tmp_id = 0x0;
	tmp_id = ((read_cmos_sensor(0xf0) << 8) | read_cmos_sensor(0xf1));
	return tmp_id;
}

static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{

	kal_uint32 frame_length = imgsensor.frame_length;

	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;

	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length = (frame_length > imgsensor.min_frame_length)
				? frame_length : imgsensor.min_frame_length;
	imgsensor.dummy_line =
		imgsensor.frame_length - imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line =
			imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}

/*************************************************************************
*FUNCTION
*   set_shutter
*
*DESCRIPTION
*   This function set e-shutter of sensor to change exposure time.
*
*PARAMETERS
*   iShutter : exposured lines
*
*RETURNS
*   None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter =
		(shutter < imgsensor_info.min_shutter) ?
		 imgsensor_info.min_shutter : shutter;
	shutter =
		(shutter >
		 (imgsensor_info.max_frame_length -
		  imgsensor_info.margin)) ? (imgsensor_info.max_frame_length -
			imgsensor_info.margin) : shutter;

	realtime_fps =
		imgsensor.pclk / imgsensor.line_length * 10 /
		imgsensor.frame_length;

	if (imgsensor.autoflicker_en) {
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else
			set_max_framerate(realtime_fps, 0);
	} else
		set_max_framerate(realtime_fps, 0);

	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x03, (shutter >> 8) & 0x3f);
	write_cmos_sensor(0x04, shutter & 0xff);
	LOG_INF("shutter = %d, framelength = %d\n", shutter,
			imgsensor.frame_length);
}

static void
set_shutter_frame_length(kal_uint16 shutter, kal_uint16 frame_length, kal_bool auto_extend_en)
{
	unsigned long flags;

	kal_int32 dummy_line = 0;

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	spin_lock(&imgsensor_drv_lock);

	if (frame_length > 1)
		dummy_line = frame_length - imgsensor.frame_length;
	imgsensor.frame_length = imgsensor.frame_length + dummy_line;

	if (shutter > imgsensor.frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);

	shutter =
		(shutter < imgsensor_info.min_shutter) ?
		 imgsensor_info.min_shutter : shutter;
	shutter =
		(shutter >
		 (imgsensor_info.max_frame_length -
		  imgsensor_info.margin)) ? (imgsensor_info.max_frame_length -
			imgsensor_info.margin) : shutter;
	set_dummy();

	if (shutter == (imgsensor.frame_length - 1))
		shutter += 1;

	if (shutter > 16383)
		shutter = 16383;
	if (shutter < 1)
		shutter = 1;

	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x03, (shutter >> 8) & 0x3F);
	write_cmos_sensor(0x04, shutter & 0xFF);

	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,
			imgsensor.frame_length);
}

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = gain << 4;

	if (reg_gain < GC02M1_OFILM_MIPI_RAW_SENSOR_GAIN_BASE)
		reg_gain = GC02M1_OFILM_MIPI_RAW_SENSOR_GAIN_BASE;
	else if (reg_gain > GC02M1_OFILM_MIPI_RAW_SENSOR_GAIN_MAX)
		reg_gain = GC02M1_OFILM_MIPI_RAW_SENSOR_GAIN_MAX;

	return (kal_uint16) reg_gain;
}

static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;
	kal_uint32 temp_gain;
	kal_int16 gain_index;
	kal_uint16 GC02M1_OFILM_MIPI_RAW_AGC_Param[GC02M1_OFILM_MIPI_RAW_SENSOR_GAIN_MAX_VALID_INDEX][2] = {
		{1024, 0},
		{1536, 1},
		{2035, 2},
		{2519, 3},
		{3165, 4},
		{3626, 5},
		{4147, 6},
		{4593, 7},
		{5095, 8},
		{5697, 9},
		{6270, 10},
		{6714, 11},
		{7210, 12},
		{7686, 13},
		{8214, 14},
		{10337, 15},
	};

	reg_gain = gain2reg(gain);

	for (gain_index = GC02M1_OFILM_MIPI_RAW_SENSOR_GAIN_MAX_VALID_INDEX - 1;
			gain_index >= 0; gain_index--)
		if (reg_gain >= GC02M1_OFILM_MIPI_RAW_AGC_Param[gain_index][0])
			break;

	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xb6, GC02M1_OFILM_MIPI_RAW_AGC_Param[gain_index][1]);
	temp_gain =
		reg_gain * GC02M1_OFILM_MIPI_RAW_SENSOR_DGAIN_BASE /
		GC02M1_OFILM_MIPI_RAW_AGC_Param[gain_index][0];
	write_cmos_sensor(0xb1, (temp_gain >> 8) & 0x1f);
	write_cmos_sensor(0xb2, temp_gain & 0xff);
	LOG_INF
	("GC02M1_OFILM_MIPI_RAW[gain_index][1] = 0x%x, temp_gain = 0x%x, reg_gain = %d\n",
	 GC02M1_OFILM_MIPI_RAW_AGC_Param[gain_index][1], temp_gain, reg_gain);

	return reg_gain;
}

/*
static void
ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
{
	LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n", le, se, gain);

}

static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);

}
*/
/*************************************************************************
*FUNCTION
*   night_mode
*
*DESCRIPTION
*   This function night mode of sensor.
*
*PARAMETERS
*   bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
*RETURNS
*   None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static void night_mode(kal_bool enable)
{

}

kal_uint16 addr_data_pair_init_gc02m1_ofilm_mipi_raw[] = {

	0xfc, 0x01,
	0xf4, 0x41,
	0xf5, 0xc0,
	0xf6, 0x44,
	0xf8, 0x38,
	0xf9, 0x82,
	0xfa, 0x00,
	0xfd, 0x80,
	0xfc, 0x81,
	0xfe, 0x03,
	0x01, 0x0b,
	0xf7, 0x01,
	0xfc, 0x80,
	0xfc, 0x80,
	0xfc, 0x80,
	0xfc, 0x8e,

	0xfe, 0x00,
	0x87, 0x09,
	0xee, 0x72,
	0xfe, 0x01,
	0x8c, 0x90,
	0xfe, 0x00,
	0x90, 0x00,
	0x03, 0x04,
	0x04, 0x7d,
	0x41, 0x04,
	0x42, 0xf4,
	0x05, 0x04,
	0x06, 0x48,
	0x07, 0x00,
	0x08, 0x18,
	0x9d, 0x18,
	0x09, 0x00,
	0x0a, 0x02,
	0x0d, 0x04,
	0x0e, 0xbc,
	0x17, GC02M1_OFILM_MIPI_RAW_MIRROR,
	0x19, 0x04,
	0x24, 0x00,
	0x56, 0x20,
	0x5b, 0x00,
	0x5e, 0x01,

	0x21, 0x3c,
	0x44, 0x20,
	0xcc, 0x01,

	0x1a, 0x04,
	0x1f, 0x11,
	0x27, 0x30,
	0x2b, 0x00,
	0x33, 0x00,
	0x53, 0x90,
	0xe6, 0x50,

	0x39, 0x07,
	0x43, 0x04,
	0x46, 0x2a,
	0x7c, 0xa0,
	0xd0, 0xbe,
	0xd1, 0x60,
	0xd2, 0x40,
	0xd3, 0xf3,
	0xde, 0x1d,

	0xcd, 0x05,
	0xce, 0x6f,

	0xfc, 0x88,
	0xfe, 0x10,
	0xfe, 0x00,
	0xfc, 0x8e,
	0xfe, 0x00,
	0xfe, 0x00,
	0xfe, 0x00,
	0xfe, 0x00,
	0xfc, 0x88,
	0xfe, 0x10,
	0xfe, 0x00,
	0xfc, 0x8e,
	0xfe, 0x04,
	0xe0, 0x01,
	0xfe, 0x00,

	0xfe, 0x01,
	0x53, 0x44,
	0x87, 0x53,
	0x89, 0x03,

	0xfe, 0x00,
	0xb0, 0x74,
	0xb1, 0x04,
	0xb2, 0x00,
	0xb6, 0x00,
	0xfe, 0x04,
	0xd8, 0x00,
	0xc0, 0x40,
	0xc0, 0x00,
	0xc0, 0x00,
	0xc0, 0x00,
	0xc0, 0x60,
	0xc0, 0x00,
	0xc0, 0xc0,
	0xc0, 0x2a,
	0xc0, 0x80,
	0xc0, 0x00,
	0xc0, 0x00,
	0xc0, 0x40,
	0xc0, 0xa0,
	0xc0, 0x00,
	0xc0, 0x90,
	0xc0, 0x19,
	0xc0, 0xc0,
	0xc0, 0x00,
	0xc0, 0xD0,
	0xc0, 0x2F,
	0xc0, 0xe0,
	0xc0, 0x00,
	0xc0, 0x90,
	0xc0, 0x39,
	0xc0, 0x00,
	0xc0, 0x01,
	0xc0, 0x20,
	0xc0, 0x04,
	0xc0, 0x20,
	0xc0, 0x01,
	0xc0, 0xe0,
	0xc0, 0x0f,
	0xc0, 0x40,
	0xc0, 0x01,
	0xc0, 0xe0,
	0xc0, 0x1a,
	0xc0, 0x60,
	0xc0, 0x01,
	0xc0, 0x20,
	0xc0, 0x25,
	0xc0, 0x80,
	0xc0, 0x01,
	0xc0, 0xa0,
	0xc0, 0x2c,
	0xc0, 0xa0,
	0xc0, 0x01,
	0xc0, 0xe0,
	0xc0, 0x32,
	0xc0, 0xc0,
	0xc0, 0x01,
	0xc0, 0x20,
	0xc0, 0x38,
	0xc0, 0xe0,
	0xc0, 0x01,
	0xc0, 0x60,
	0xc0, 0x3c,
	0xc0, 0x00,
	0xc0, 0x02,
	0xc0, 0xa0,
	0xc0, 0x40,
	0xc0, 0x80,
	0xc0, 0x02,
	0xc0, 0x18,
	0xc0, 0x5c,
	0xfe, 0x00,
	0x9f, 0x10,

	0xfe, 0x00,
	0x26, 0x20,
	0xfe, 0x01,
	0x40, 0x22,
	0x46, 0x7f,
	0x49, 0x0f,
	0x4a, 0xf0,
	0xfe, 0x04,
	0x14, 0x80,
	0x15, 0x80,
	0x16, 0x80,
	0x17, 0x80,

	0xfe, 0x01,
	0x41, 0x20,
	0x4c, 0x00,
	0x4d, 0x0c,
	0x44, 0x08,
	0x48, 0x03,

	0xfe, 0x01,
	0x90, 0x01,
	0x91, 0x00,
	0x92, 0x06,
	0x93, 0x00,
	0x94, 0x06,
	0x95, 0x04,
	0x96, 0xb0,
	0x97, 0x06,
	0x98, 0x40,

	0xfe, 0x03,
	0x01, 0x23,
	0x03, 0xce,
	0x04, 0x48,
	0x15, 0x00,
	0x21, 0x10,
	0x22, 0x05,
	0x23, 0x20,
	0x25, 0x20,
	0x26, 0x08,
	0x29, 0x06,
	0x2a, 0x0a,
	0x2b, 0x08,

	0xfe, 0x01,
	0x8c, 0x10,
	0xfe, 0x00,
	0x3e, 0x00,
};

kal_uint16 addr_data_pair_preview_gc02m1_ofilm_mipi_raw[] = {
	0xfe, 0x00,
	0x3e, 0x90,
};

kal_uint16 addr_data_pair_capture_gc02m1_ofilm_mipi_raw[] = {
	0xfe, 0x00,
	0x3e, 0x90,
};

kal_uint16 addr_data_pair_normal_video_gc02m1_ofilm_mipi_raw[] = {
	0xfe, 0x00,
	0x3e, 0x90,
};

kal_uint16 addr_data_pair_hs_video_gc02m1_ofilm_mipi_raw[] = {
	0xfe, 0x00,
	0x3e, 0x90,
};

kal_uint16 addr_data_pair_slim_video_gc02m1_ofilm_mipi_raw[] = {
	0xfe, 0x00,
	0x3e, 0x90,
};

static void sensor_init(void)
{
	LOG_INF("E\n");

	gc02m1_ofilm_mipi_raw_table_write_cmos_sensor(addr_data_pair_init_gc02m1_ofilm_mipi_raw,
		   sizeof(addr_data_pair_init_gc02m1_ofilm_mipi_raw) /
		   sizeof(kal_uint16));
}

static void preview_setting(void)
{
	LOG_INF("E\n");
	gc02m1_ofilm_mipi_raw_table_write_cmos_sensor(addr_data_pair_preview_gc02m1_ofilm_mipi_raw,
		   sizeof(addr_data_pair_preview_gc02m1_ofilm_mipi_raw) /
		   sizeof(kal_uint16));
}

static void capture_setting(void)
{
	LOG_INF("E\n");
	gc02m1_ofilm_mipi_raw_table_write_cmos_sensor(addr_data_pair_capture_gc02m1_ofilm_mipi_raw,
		  sizeof(addr_data_pair_capture_gc02m1_ofilm_mipi_raw) /
		   sizeof(kal_uint16));
}

static void normal_video_setting(void)
{
	LOG_INF("E\n");
	gc02m1_ofilm_mipi_raw_table_write_cmos_sensor(addr_data_pair_normal_video_gc02m1_ofilm_mipi_raw,
		   sizeof
		   (addr_data_pair_normal_video_gc02m1_ofilm_mipi_raw) /
		   sizeof(kal_uint16));
}

static void hs_video_setting(void)
{
	LOG_INF("E\n");
	gc02m1_ofilm_mipi_raw_table_write_cmos_sensor(addr_data_pair_hs_video_gc02m1_ofilm_mipi_raw,
			   sizeof(addr_data_pair_hs_video_gc02m1_ofilm_mipi_raw) /
			   sizeof(kal_uint16));
}

static void slim_video_setting(void)
{
	LOG_INF("E\n");
	gc02m1_ofilm_mipi_raw_table_write_cmos_sensor(addr_data_pair_slim_video_gc02m1_ofilm_mipi_raw,
			   sizeof(addr_data_pair_slim_video_gc02m1_ofilm_mipi_raw)
			   / sizeof(kal_uint16));
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);

	if (enable) {
		write_cmos_sensor(0xfe, 0x01);
		write_cmos_sensor(0x8c, 0x11);
		write_cmos_sensor(0xfe, 0x00);
	} else {
		write_cmos_sensor(0xfe, 0x01);
		write_cmos_sensor(0x8c, 0x10);
		write_cmos_sensor(0xfe, 0x00);
	}
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static char otp_data[11] = { 0 };

unsigned int
gc02m1_ofilm_mipi_raw_read_otp_info(struct i2c_client *client,
		 unsigned int addr, unsigned char *data, unsigned int size)
{
	int ii = 0;
	LOG_INF("swl addr %d,size %d", addr, size);
	for (ii = 0; ii < 11; ii++) {
		data[ii] = otp_data[addr + ii];
		LOG_INF("swl otp_info %x,data %x", data[ii],
				otp_data[addr + ii]);
	}
	return size;
}

#if USER_OTP
static void gc02m1_ofilm_mipi_raw_read_otp(void)
{
	int i = 0;
	int otp_addr[] = { 16, 18, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
	kal_uint8 otp_info[11];
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfc, 0x01);
	write_cmos_sensor(0xf4, 0x41);
	write_cmos_sensor(0xf5, 0xc0);
	write_cmos_sensor(0xf6, 0x44);
	write_cmos_sensor(0xf8, 0x38);
	write_cmos_sensor(0xf9, 0x82);
	write_cmos_sensor(0xfa, 0x00);
	write_cmos_sensor(0xfd, 0x80);
	write_cmos_sensor(0xfc, 0x81);
	write_cmos_sensor(0xf7, 0x01);
	write_cmos_sensor(0xfc, 0x80);
	write_cmos_sensor(0xfc, 0x80);
	write_cmos_sensor(0xfc, 0x80);
	write_cmos_sensor(0xfc, 0x8e);
	write_cmos_sensor(0xf3, 0x30);
	write_cmos_sensor(0xfe, 0x02);
	for (i = 0; i < 11; i++) {
		write_cmos_sensor(0x17, otp_addr[i] * 8);
		write_cmos_sensor(0xf3, 0x34);
		otp_info[i] = read_cmos_sensor(0x19);
		LOG_INF("otp_info2[%d] = %x\n", i, otp_info[i]);
	}
	if (otp_info[1] == 68) {
		for (i = 0; i < 11; i++) {
			otp_data[i] = otp_info[i];
			LOG_INF("otp_info[%d] = %x\n", i, otp_data[i]);
		}
	}
	write_cmos_sensor(0xf7, 0x00);
	write_cmos_sensor(0xfe, 0x00);
}
#endif

#if USER_OTP
static int get_gc02m1_ofilm_mipi_raw_vendor_id(void)
{
	int module_id;
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfc, 0x01);
	write_cmos_sensor(0xf4, 0x41);
	write_cmos_sensor(0xf5, 0xc0);
	write_cmos_sensor(0xf6, 0x44);
	write_cmos_sensor(0xf8, 0x38);
	write_cmos_sensor(0xf9, 0x82);
	write_cmos_sensor(0xfa, 0x00);
	write_cmos_sensor(0xfd, 0x80);
	write_cmos_sensor(0xfc, 0x81);
	write_cmos_sensor(0xf7, 0x01);
	write_cmos_sensor(0xfc, 0x80);
	write_cmos_sensor(0xfc, 0x80);
	write_cmos_sensor(0xfc, 0x80);
	write_cmos_sensor(0xfc, 0x8e);
	write_cmos_sensor(0xf3, 0x30);
	write_cmos_sensor(0xfe, 0x02);
	write_cmos_sensor(0x17, 0x80);
	write_cmos_sensor(0xf3, 0x34);
	module_id = read_cmos_sensor(0x19);
	return module_id;
}
#endif

static kal_uint16 get_vendor_id(void)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = {(char)(0x01 >> 8), (char)(0x01 & 0xFF) };

	iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 1, 0xA4);
	return get_byte;

}

/*************************************************************************
*FUNCTION
*   get_imgsensor_id
*
*DESCRIPTION
*   This function get the sensor ID
*
*PARAMETERS
*   *sensorID : return the sensor ID
*
*RETURNS
*   None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint16 vendor_id = 0;

	vendor_id = get_vendor_id();
        if (vendor_id != VENDOR_ID){
            pr_err("get_vednor_id read is %x!", vendor_id);
	    *sensor_id = 0xFFFFFFFF;
	    return ERROR_SENSOR_CONNECT_FAIL;
        }

	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = return_sensor_id() + 1;
			if (*sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("sensor id: 0x%x, imgsensor.i2c_write_id: 0x%x\n", *sensor_id, imgsensor.i2c_write_id);
				return ERROR_NONE; 
			}
			LOG_INF("Read sensor id true, write id: 0x%x, id: 0x%x\n", imgsensor.i2c_write_id, *sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		retry = 2;
	}
	if (*sensor_id != imgsensor_info.sensor_id) {
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	return ERROR_NONE;
}

/*************************************************************************
*FUNCTION
*   open
*
*DESCRIPTION
*   This function initialize the registers of CMOS sensor
*
*PARAMETERS
*   None
*
*RETURNS
*   None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 sensor_id = 0;

	LOG_1;

	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = return_sensor_id() + 1;
			if (sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id, sensor_id);
				break;
			}
			LOG_INF("Read sensor id fail, write id: 0x%x, id: 0x%x\n", imgsensor.i2c_write_id, sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;

	sensor_init();

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en = KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_en = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

/*************************************************************************
*FUNCTION
*   close
*
*DESCRIPTION
*
*
*PARAMETERS
*   None
*
*RETURNS
*   None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");

	return ERROR_NONE;
}

/*************************************************************************
*FUNCTION
*preview
*
*DESCRIPTION
*   This function start the sensor preview.
*
*PARAMETERS
*   *image_window : address pointer of pixel numbers in one period of HSYNC
* *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
*RETURNS
*   None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32
preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *
		image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;

	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	preview_setting();

	return ERROR_NONE;
}

/*************************************************************************
*FUNCTION
*   capture
*
*DESCRIPTION
*   This function setup the CMOS sensor in capture MY_OUTPUT mode
*
*PARAMETERS
*
*RETURNS
*   None
*
*GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32
capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *
		image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {
		imgsensor.pclk = imgsensor_info.cap1.pclk;
		imgsensor.line_length = imgsensor_info.cap1.linelength;
		imgsensor.frame_length = imgsensor_info.cap1.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap1.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	} else {
		if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
			LOG_INF
			("fps %d is not support use cap setting: %d fps!\n",
			 imgsensor.current_fps,
			 imgsensor_info.cap.max_framerate / 10);
		imgsensor.pclk = imgsensor_info.cap.pclk;
		imgsensor.line_length = imgsensor_info.cap.linelength;
		imgsensor.frame_length = imgsensor_info.cap.framelength;
		imgsensor.min_frame_length = imgsensor_info.cap.framelength;
		imgsensor.autoflicker_en = KAL_FALSE;
	}
	spin_unlock(&imgsensor_drv_lock);
	capture_setting();
	return ERROR_NONE;
}

static kal_uint32
normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *
	 image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;

	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting();
	return ERROR_NONE;
}

static kal_uint32
hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *
		 image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;

	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	return ERROR_NONE;
}

static kal_uint32
slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *
		   image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	return ERROR_NONE;
}

static kal_uint32
custom1(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *
		image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM1;
	if (imgsensor.current_fps != imgsensor_info.custom1.max_framerate)
		LOG_INF
		("fps %d is not support use cap setting: %d fps!\n",
		 imgsensor.current_fps,
		 imgsensor_info.cap.max_framerate / 10);
	imgsensor.pclk = imgsensor_info.custom1.pclk;
	imgsensor.line_length = imgsensor_info.custom1.linelength;
	imgsensor.frame_length = imgsensor_info.custom1.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

static kal_uint32
custom2(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *
		image_window, MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM2;
	imgsensor.pclk = imgsensor_info.custom2.pclk;
	imgsensor.line_length = imgsensor_info.custom2.linelength;
	imgsensor.frame_length = imgsensor_info.custom2.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom2.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

static kal_uint32
get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INF("E\n");
	sensor_resolution->SensorFullWidth =
		imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight =
		imgsensor_info.cap.grabwindow_height;

	sensor_resolution->SensorPreviewWidth =
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight =
		imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth =
		imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight =
		imgsensor_info.normal_video.grabwindow_height;

	sensor_resolution->SensorHighSpeedVideoWidth =
		imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight =
		imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth =
		imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight =
		imgsensor_info.slim_video.grabwindow_height;
	sensor_resolution->SensorCustom1Width =
		imgsensor_info.custom1.grabwindow_width;
	sensor_resolution->SensorCustom1Height =
		imgsensor_info.custom1.grabwindow_height;
	sensor_resolution->SensorCustom2Width =
		imgsensor_info.custom2.grabwindow_width;
	sensor_resolution->SensorCustom2Height =
		imgsensor_info.custom2.grabwindow_height;
	return ERROR_NONE;
}

static kal_uint32
get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
		 MSDK_SENSOR_INFO_STRUCT *sensor_info,
		 MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4;
	sensor_info->SensorResetActiveHigh = FALSE;
	sensor_info->SensorResetDelayCount = 5;

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat =
		imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame =
		imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame =
		imgsensor_info.slim_video_delay_frame;
	sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;
	sensor_info->Custom2DelayFrame = imgsensor_info.custom2_delay_frame;
	sensor_info->FrameTimeDelayFrame =
		imgsensor_info.frame_time_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0;
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;
	sensor_info->AESensorGainDelayFrame =
		imgsensor_info.ae_sensor_gain_delay_frame;
	sensor_info->AEISPGainDelayFrame =
		imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;

	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3;
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2;
	sensor_info->SensorPixelClockCount = 3;
	sensor_info->SensorDataLatchCount = 2;

	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;
	sensor_info->SensorHightSampling = 0;
	sensor_info->SensorPacketECCOrder = 1;

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.cap.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		sensor_info->SensorGrabStartX =
			imgsensor_info.normal_video.startx;
		sensor_info->SensorGrabStartY =
			imgsensor_info.normal_video.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		sensor_info->SensorGrabStartX =
			imgsensor_info.slim_video.startx;
		sensor_info->SensorGrabStartY =
			imgsensor_info.slim_video.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		sensor_info->SensorGrabStartX = imgsensor_info.custom1.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom1.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_CUSTOM2:
		sensor_info->SensorGrabStartX = imgsensor_info.custom2.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom2.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.custom2.mipi_data_lp2hs_settle_dc;
		break;
	default:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
			imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
		break;
	}
	return ERROR_NONE;
}

static kal_uint32
control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
		MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
		MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		preview(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		capture(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		normal_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		hs_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		slim_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		custom1(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CUSTOM2:
		custom2(image_window, sensor_config_data);
		break;
	default:
		LOG_INF("Error ScenarioId setting");
		preview(image_window, sensor_config_data);
		return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}

static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INF("framerate = %d\n", framerate);

	/***********
	 *if (framerate == 0)
	 *	return ERROR_NONE;
	 *spin_lock(&imgsensor_drv_lock);
	 *if ((framerate == 300) &&(imgsensor.autoflicker_en == KAL_TRUE))
	 *	imgsensor.current_fps = 296;
	 *else if ((framerate == 150) &&(imgsensor.autoflicker_en == KAL_TRUE))
	 *	imgsensor.current_fps = 146;
	 *else
	 *	imgsensor.current_fps = framerate;
	 *spin_unlock(&imgsensor_drv_lock);
	 *set_max_framerate(imgsensor.current_fps, 1);
	 ********/
	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INF("enable = %d, framerate = %d\n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable)
		imgsensor.autoflicker_en = KAL_TRUE;
	else
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static kal_uint32
set_max_framerate_by_scenario(enum MSDK_SCENARIO_ID_ENUM
	  scenario_id, MUINT32 framerate)
{
	kal_uint32 frame_length;

	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		frame_length =
			imgsensor_info.pre.pclk / framerate * 10 /
			imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length >
			 imgsensor_info.pre.framelength) ? (frame_length -
				imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		if (framerate == 0)
			return ERROR_NONE;
		frame_length =
			imgsensor_info.normal_video.pclk / framerate * 10 /
			imgsensor_info.normal_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.normal_video.framelength)
			 ? (frame_length -
			 imgsensor_info.normal_video.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.normal_video.framelength +
			imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		if (imgsensor.current_fps == imgsensor_info.cap1.max_framerate) {
			frame_length =
				imgsensor_info.cap1.pclk / framerate * 10 /
				imgsensor_info.cap1.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line = (frame_length >
			 imgsensor_info.cap1.framelength) ? (frame_length -
				 imgsensor_info.cap1.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.cap1.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
		} else {
			if (imgsensor.current_fps !=
					imgsensor_info.cap.max_framerate)
				LOG_INF
				("fps %d not support use cap's  %d fps!\n",
				 framerate,
				 imgsensor_info.cap.max_framerate / 10);
			frame_length =
				imgsensor_info.cap.pclk / framerate * 10 /
				imgsensor_info.cap.linelength;
			spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
				(frame_length >
				 imgsensor_info.cap.framelength) ?
				 (frame_length - imgsensor_info.cap.framelength)
				 : 0;
			imgsensor.frame_length =
				imgsensor_info.cap.framelength +
				imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);
		}
		set_dummy();
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		frame_length =
			imgsensor_info.hs_video.pclk / framerate * 10 /
			imgsensor_info.hs_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line = (frame_length >
			 imgsensor_info.hs_video.framelength) ? (frame_length -
			 imgsensor_info.hs_video.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.hs_video.framelength +
			 imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		frame_length =
			imgsensor_info.slim_video.pclk / framerate * 10 /
			imgsensor_info.slim_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line = (frame_length >
			 imgsensor_info.slim_video.framelength) ?
			 (frame_length -
			 imgsensor_info.slim_video.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.slim_video.framelength +
			imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		if (imgsensor.current_fps !=
				imgsensor_info.custom1.max_framerate)
			LOG_INF
			("fps %d is not support use cap's %d fps!\n",
			 framerate, imgsensor_info.custom1.max_framerate / 10);
		frame_length =
			imgsensor_info.custom1.pclk / framerate * 10 /
			imgsensor_info.custom1.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line = (frame_length >
			 imgsensor_info.custom1.framelength) ? (frame_length -
			 imgsensor_info.custom1.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.custom1.framelength +
			imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CUSTOM2:
		frame_length =
			imgsensor_info.custom2.pclk / framerate * 10 /
			imgsensor_info.custom2.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length >
			 imgsensor_info.custom2.framelength) ? (frame_length -
					 imgsensor_info.custom2.framelength)
			: 0;
		imgsensor.frame_length =
			imgsensor_info.custom2.framelength +
			 imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	default:
		frame_length =
			imgsensor_info.pre.pclk / framerate * 10 /
			imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length >
			 imgsensor_info.pre.framelength) ? (frame_length -
			 imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		LOG_INF("error scenario_id = %d, we use preview scenario\n",
				scenario_id);
		break;
	}
	return ERROR_NONE;
}

static kal_uint32 get_default_framerate_by_scenario(enum
		MSDK_SCENARIO_ID_ENUM
		scenario_id,
		MUINT32 *framerate)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		*framerate = imgsensor_info.pre.max_framerate;
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		*framerate = imgsensor_info.normal_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		*framerate = imgsensor_info.cap.max_framerate;
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		*framerate = imgsensor_info.hs_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		*framerate = imgsensor_info.slim_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		*framerate = imgsensor_info.custom1.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CUSTOM2:
		*framerate = imgsensor_info.custom2.max_framerate;
		break;
	default:
		break;
	}

	return ERROR_NONE;
}

static kal_uint32 streaming_control(kal_bool enable)
{
        LOG_INF("NEW_LOG");
	LOG_INF("streaming_enable(0=Sw Standby,1=streaming): %d\n", enable);
	if (enable) {
		write_cmos_sensor(0xFE, 0X00);
		write_cmos_sensor(0x3E, 0X90);
		write_cmos_sensor(0xFE, 0X00);
	} else {
		write_cmos_sensor(0xFE, 0x00);
		write_cmos_sensor(0x3E, 0x00);
		write_cmos_sensor(0xFE, 0x00);
	}
	mdelay(2);
	return ERROR_NONE;
}

static kal_uint32
feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
				UINT8 *feature_para, UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
	UINT16 *feature_data_16 = (UINT16 *) feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
	UINT32 *feature_data_32 = (UINT32 *) feature_para;
	unsigned long long *feature_data = (unsigned long long *)feature_para;

	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data =
		(MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

	LOG_INF("feature_id = %d, len=%d\n", feature_id, *feature_para_len);
	switch (feature_id) {

	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= imgsensor_info.cap.pclk;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= imgsensor_info.normal_video.pclk;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= imgsensor_info.hs_video.pclk;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= imgsensor_info.slim_video.pclk;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= imgsensor_info.pre.pclk;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PIXEL_RATE:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.cap.pclk /
			(imgsensor_info.cap.linelength - 80))*
			imgsensor_info.cap.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.normal_video.pclk /
			(imgsensor_info.normal_video.linelength - 80))*
			imgsensor_info.normal_video.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.hs_video.pclk /
			(imgsensor_info.hs_video.linelength - 80))*
			imgsensor_info.hs_video.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.slim_video.pclk /
			(imgsensor_info.slim_video.linelength - 80))*
			imgsensor_info.slim_video.grabwindow_width;

			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.pre.pclk /
			(imgsensor_info.pre.linelength - 80))*
			imgsensor_info.pre.grabwindow_width;

			break;
		}
		break;
	case SENSOR_FEATURE_GET_PERIOD_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= (imgsensor_info.cap.framelength << 16)
				  + imgsensor_info.cap.linelength;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= (imgsensor_info.normal_video.framelength
				 << 16) +
				 imgsensor_info.normal_video.linelength;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= (imgsensor_info.hs_video.framelength << 16)
				  + imgsensor_info.hs_video.linelength;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= (imgsensor_info.slim_video.framelength << 16)
				  + imgsensor_info.slim_video.linelength;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1))
				= (imgsensor_info.pre.framelength << 16)
				  + imgsensor_info.pre.linelength;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PERIOD:
		*feature_return_para_16++ = imgsensor.line_length;
		*feature_return_para_16 = imgsensor.frame_length;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
		*feature_return_para_32 = imgsensor.pclk;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE: {
		kal_uint32 rate;

		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			rate = imgsensor_info.cap.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			rate =
				imgsensor_info.normal_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			rate = imgsensor_info.hs_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			rate =
				imgsensor_info.slim_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			rate = imgsensor_info.pre.mipi_pixel_rate;
			break;
		}
		*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = rate;
	}
	break;
	case SENSOR_FEATURE_SET_ESHUTTER:
		set_shutter(*feature_data);
		break;
	case SENSOR_FEATURE_SET_NIGHTMODE:
		night_mode((BOOL) * feature_data);
		break;
	case SENSOR_FEATURE_SET_GAIN:
		set_gain((UINT16) *feature_data);
		break;
	case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		break;
	case SENSOR_FEATURE_SET_REGISTER:
		write_cmos_sensor(sensor_reg_data->RegAddr,
						  sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_REGISTER:
		sensor_reg_data->RegData =
			read_cmos_sensor(sensor_reg_data->RegAddr);
		LOG_INF("adb_i2c_read 0x%x = 0x%x\n", sensor_reg_data->RegAddr,
				sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:

		*feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_VIDEO_MODE:
		set_video_mode(*feature_data);
		break;
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		get_imgsensor_id(feature_return_para_32);
		break;
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
		set_auto_flicker_mode((BOOL) * feature_data_16,
			  *(feature_data_16 + 1));
		break;
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		set_max_framerate_by_scenario((enum MSDK_SCENARIO_ID_ENUM)
			  *feature_data, *(feature_data + 1));
		break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		get_default_framerate_by_scenario((enum MSDK_SCENARIO_ID_ENUM)
			  *(feature_data), (MUINT32 *) (uintptr_t)
			  (*(feature_data + 1)));
		break;
	case SENSOR_FEATURE_SET_TEST_PATTERN:
		set_test_pattern_mode((BOOL) * feature_data);
		break;
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
		*feature_return_para_32 = imgsensor_info.checksum_value;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_FRAMERATE:
		LOG_INF("current fps: %d\n", (UINT32)*feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.current_fps = *feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME:
		set_shutter_frame_length((UINT16) *feature_data,
					 (UINT16) *(feature_data + 1),
					 (BOOL) (*(feature_data + 2)));
		break;
	case SENSOR_FEATURE_GET_FRAME_CTRL_INFO_BY_SCENARIO:
		/*
		 * 1, if driver support new sw frame sync
		 * set_shutter_frame_length() support third para auto_extend_en
		 */
		*(feature_data + 1) = 0;
		/* margin info by scenario */
		*(feature_data + 2) = imgsensor_info.margin;
		break;
	case SENSOR_FEATURE_SET_HDR:
		LOG_INF("ihdr enable: %d\n", (BOOL) * feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.ihdr_en = (BOOL) * feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_GET_CROP_INFO:
		LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId: %d\n",
				(UINT32) * feature_data_32);
		wininfo =
			(struct SENSOR_WINSIZE_INFO_STRUCT
			 *)(uintptr_t) (*(feature_data + 1));
		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)wininfo,
				   (void *)&imgsensor_winsize_info[1],
				   sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)wininfo,
				   (void *)&imgsensor_winsize_info[2],
				   sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy((void *)wininfo,
				   (void *)&imgsensor_winsize_info[3],
				   sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			memcpy((void *)wininfo,
				   (void *)&imgsensor_winsize_info[4],
				   sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			memcpy((void *)wininfo,
				   (void *)&imgsensor_winsize_info[5],
				   sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM2:
			memcpy((void *)wininfo,
				   (void *)&imgsensor_winsize_info[6],
				   sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)wininfo,
				   (void *)&imgsensor_winsize_info[0],
				   sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		}
		break;
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
		/*LOG_INF("SENSOR_SET_SENSOR_IHDR LE = %d, SE = %d, Gain = %d\n",
			(UINT16) *feature_data, (UINT16) *(feature_data + 1),
				(UINT16) *(feature_data + 2));
		ihdr_write_shutter_gain((UINT16) *feature_data,
			(UINT16) *(feature_data + 1),
				(UINT16) *(feature_data + 2));*/
		break;
	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_SUSPEND\n");
		streaming_control(KAL_FALSE);
		break;
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_RESUME, shutter:%llu\n",
				*feature_data);
		if (*feature_data != 0)
			set_shutter(*feature_data);
		streaming_control(KAL_TRUE);
		break;
	case SENSOR_FEATURE_GET_GAIN_RANGE_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_gain;
		*(feature_data + 2) = imgsensor_info.max_gain;
		break;
	case SENSOR_FEATURE_GET_BASE_GAIN_ISO_AND_STEP:
		*(feature_data + 0) = imgsensor_info.min_gain_iso;
		*(feature_data + 1) = imgsensor_info.gain_step;
		*(feature_data + 2) = imgsensor_info.gain_type;
		break;
	case SENSOR_FEATURE_GET_MIN_SHUTTER_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_shutter;
		break;
	case SENSOR_FEATURE_GET_BINNING_TYPE:
		switch (*(feature_data + 1)) {
		default:
			*feature_return_para_32 = 2; /*BINNING_AVERAGED*/
			break;
		}
		LOG_INF("SENSOR_FEATURE_GET_BINNING_TYPE AE_binning_type:%d,\n",
			*feature_return_para_32);
		*feature_para_len = 4;
		break;

	default:
		break;
	}
	return ERROR_NONE;
}

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 GC02M1_OFILM_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{

	if (pfFunc != NULL)
		*pfFunc = &sensor_func;
	return ERROR_NONE;
}
