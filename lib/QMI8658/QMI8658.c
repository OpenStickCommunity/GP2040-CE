/*****************************************************************************
* | File      	:   QMI8658.c
* | Author      :
* | Function    :   Hardware Interface of QMI8658 Sensor
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2022-06-07
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of theex Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
******************************************************************************/
//#include "stdafx.h"
#include "QMI8658.h"

#define QMI8658_printf (void)

#define QMI8658_UINT_MG_DPS

enum
{
	AXIS_X = 0,
	AXIS_Y = 1,
	AXIS_Z = 2,

	AXIS_TOTAL
};

typedef struct
{
	short sign[AXIS_TOTAL];
	unsigned short map[AXIS_TOTAL];
} qst_imu_layout;

static unsigned short acc_lsb_div = 0;
static unsigned short gyro_lsb_div = 0;
static unsigned short ae_q_lsb_div = (1 << 14);
static unsigned short ae_v_lsb_div = (1 << 10);
static unsigned int imu_timestamp = 0;
static struct QMI8658Config QMI8658_config;
static unsigned char QMI8658_device_addr = QMI8658_DEVICE_ADDR_L;

int16_t (*QMI8658_I2C_Write)(uint8_t address, uint8_t *data, uint16_t len, bool isBlock);
int16_t (*QMI8658_I2C_ReadRegister)(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);

unsigned char QMI8658_write_reg(unsigned char reg, unsigned char value)
{
	unsigned char ret = 0;
	unsigned int retry = 0;

	uint8_t data[2] = {reg, value};
	while ((!ret) && (retry++ < 5))
	{
		ret = QMI8658_I2C_Write(QMI8658_device_addr, data, 2, false);
	}
	return ret;
}

unsigned char QMI8658_write_regs(unsigned char reg, unsigned char *value, unsigned char len)
{
	int i, ret;

	for (i = 0; i < len; i++)
	{
		ret = QMI8658_write_reg(reg + i, value[i]);
	}

	return ret;
}

unsigned char QMI8658_read_reg(unsigned char reg, unsigned char *buf, unsigned short len)
{
	return QMI8658_I2C_ReadRegister(QMI8658_device_addr, reg, buf, len);
}

#if 0
static qst_imu_layout imu_map;

void QMI8658_set_layout(short layout)
{
	if(layout == 0)
	{
		imu_map.sign[AXIS_X] = 1;
		imu_map.sign[AXIS_Y] = 1;
		imu_map.sign[AXIS_Z] = 1;
		imu_map.map[AXIS_X] = AXIS_X;
		imu_map.map[AXIS_Y] = AXIS_Y;
		imu_map.map[AXIS_Z] = AXIS_Z;
	}
	else if(layout == 1)
	{
		imu_map.sign[AXIS_X] = -1;
		imu_map.sign[AXIS_Y] = 1;
		imu_map.sign[AXIS_Z] = 1;
		imu_map.map[AXIS_X] = AXIS_Y;
		imu_map.map[AXIS_Y] = AXIS_X;
		imu_map.map[AXIS_Z] = AXIS_Z;
	}
	else if(layout == 2)
	{
		imu_map.sign[AXIS_X] = -1;
		imu_map.sign[AXIS_Y] = -1;
		imu_map.sign[AXIS_Z] = 1;
		imu_map.map[AXIS_X] = AXIS_X;
		imu_map.map[AXIS_Y] = AXIS_Y;
		imu_map.map[AXIS_Z] = AXIS_Z;
	}
	else if(layout == 3)
	{
		imu_map.sign[AXIS_X] = 1;
		imu_map.sign[AXIS_Y] = -1;
		imu_map.sign[AXIS_Z] = 1;
		imu_map.map[AXIS_X] = AXIS_Y;
		imu_map.map[AXIS_Y] = AXIS_X;
		imu_map.map[AXIS_Z] = AXIS_Z;
	}
	else if(layout == 4)
	{
		imu_map.sign[AXIS_X] = -1;
		imu_map.sign[AXIS_Y] = 1;
		imu_map.sign[AXIS_Z] = -1;
		imu_map.map[AXIS_X] = AXIS_X;
		imu_map.map[AXIS_Y] = AXIS_Y;
		imu_map.map[AXIS_Z] = AXIS_Z;
	}
	else if(layout == 5)
	{
		imu_map.sign[AXIS_X] = 1;
		imu_map.sign[AXIS_Y] = 1;
		imu_map.sign[AXIS_Z] = -1;
		imu_map.map[AXIS_X] = AXIS_Y;
		imu_map.map[AXIS_Y] = AXIS_X;
		imu_map.map[AXIS_Z] = AXIS_Z;
	}
	else if(layout == 6)
	{
		imu_map.sign[AXIS_X] = 1;
		imu_map.sign[AXIS_Y] = -1;
		imu_map.sign[AXIS_Z] = -1;
		imu_map.map[AXIS_X] = AXIS_X;
		imu_map.map[AXIS_Y] = AXIS_Y;
		imu_map.map[AXIS_Z] = AXIS_Z;
	}
	else if(layout == 7)
	{
		imu_map.sign[AXIS_X] = -1;
		imu_map.sign[AXIS_Y] = -1;
		imu_map.sign[AXIS_Z] = -1;
		imu_map.map[AXIS_X] = AXIS_Y;
		imu_map.map[AXIS_Y] = AXIS_X;
		imu_map.map[AXIS_Z] = AXIS_Z;
	}
	else
	{
		imu_map.sign[AXIS_X] = 1;
		imu_map.sign[AXIS_Y] = 1;
		imu_map.sign[AXIS_Z] = 1;
		imu_map.map[AXIS_X] = AXIS_X;
		imu_map.map[AXIS_Y] = AXIS_Y;
		imu_map.map[AXIS_Z] = AXIS_Z;
	}
}
#endif

void QMI8658_config_acc(enum QMI8658_AccRange range, enum QMI8658_AccOdr odr, enum QMI8658_LpfConfig lpfEnable, enum QMI8658_StConfig stEnable)
{
	unsigned char ctl_dada;

	switch (range)
	{
	case QMI8658AccRange_2g:
		acc_lsb_div = (1 << 14);
		break;
	case QMI8658AccRange_4g:
		acc_lsb_div = (1 << 13);
		break;
	case QMI8658AccRange_8g:
		acc_lsb_div = (1 << 12);
		break;
	case QMI8658AccRange_16g:
		acc_lsb_div = (1 << 11);
		break;
	default:
		range = QMI8658AccRange_8g;
		acc_lsb_div = (1 << 12);
	}
	if (stEnable == QMI8658St_Enable)
		ctl_dada = (unsigned char)range | (unsigned char)odr | 0x80;
	else
		ctl_dada = (unsigned char)range | (unsigned char)odr;

	QMI8658_write_reg(QMI8658Register_Ctrl2, ctl_dada);
	// set LPF & HPF
	QMI8658_read_reg(QMI8658Register_Ctrl5, &ctl_dada, 1);
	ctl_dada &= 0xf0;
	if (lpfEnable == QMI8658Lpf_Enable)
	{
		ctl_dada |= A_LSP_MODE_3;
		ctl_dada |= 0x01;
	}
	else
	{
		ctl_dada &= ~0x01;
	}
	ctl_dada = 0x00;
	QMI8658_write_reg(QMI8658Register_Ctrl5, ctl_dada);
	// set LPF & HPF
}

void QMI8658_config_gyro(enum QMI8658_GyrRange range, enum QMI8658_GyrOdr odr, enum QMI8658_LpfConfig lpfEnable, enum QMI8658_StConfig stEnable)
{
	// Set the CTRL3 register to configure dynamic range and ODR
	unsigned char ctl_dada;

	// Store the scale factor for use when processing raw data
	switch (range)
	{
	case QMI8658GyrRange_32dps:
		gyro_lsb_div = 1024;
		break;
	case QMI8658GyrRange_64dps:
		gyro_lsb_div = 512;
		break;
	case QMI8658GyrRange_128dps:
		gyro_lsb_div = 256;
		break;
	case QMI8658GyrRange_256dps:
		gyro_lsb_div = 128;
		break;
	case QMI8658GyrRange_512dps:
		gyro_lsb_div = 64;
		break;
	case QMI8658GyrRange_1024dps:
		gyro_lsb_div = 32;
		break;
	case QMI8658GyrRange_2048dps:
		gyro_lsb_div = 16;
		break;
	case QMI8658GyrRange_4096dps:
		gyro_lsb_div = 8;
		break;
	default:
		range = QMI8658GyrRange_512dps;
		gyro_lsb_div = 64;
		break;
	}

	if (stEnable == QMI8658St_Enable)
		ctl_dada = (unsigned char)range | (unsigned char)odr | 0x80;
	else
		ctl_dada = (unsigned char)range | (unsigned char)odr;
	QMI8658_write_reg(QMI8658Register_Ctrl3, ctl_dada);

	// Conversion from degrees/s to rad/s if necessary
	// set LPF & HPF
	QMI8658_read_reg(QMI8658Register_Ctrl5, &ctl_dada, 1);
	ctl_dada &= 0x0f;
	if (lpfEnable == QMI8658Lpf_Enable)
	{
		ctl_dada |= G_LSP_MODE_3;
		ctl_dada |= 0x10;
	}
	else
	{
		ctl_dada &= ~0x10;
	}
	ctl_dada = 0x00;
	QMI8658_write_reg(QMI8658Register_Ctrl5, ctl_dada);
	// set LPF & HPF
}

void QMI8658_config_mag(enum QMI8658_MagDev device, enum QMI8658_MagOdr odr)
{
	QMI8658_write_reg(QMI8658Register_Ctrl4, device | odr);
}

void QMI8658_config_ae(enum QMI8658_AeOdr odr)
{
	// QMI8658_config_acc(QMI8658AccRange_8g, AccOdr_1000Hz, Lpf_Enable, St_Enable);
	// QMI8658_config_gyro(QMI8658GyrRange_2048dps, GyrOdr_1000Hz, Lpf_Enable, St_Enable);
	QMI8658_config_acc(QMI8658_config.accRange, QMI8658_config.accOdr, QMI8658Lpf_Enable, QMI8658St_Disable);
	QMI8658_config_gyro(QMI8658_config.gyrRange, QMI8658_config.gyrOdr, QMI8658Lpf_Enable, QMI8658St_Disable);
	QMI8658_config_mag(QMI8658_config.magDev, QMI8658_config.magOdr);
	QMI8658_write_reg(QMI8658Register_Ctrl6, odr);
}

unsigned char QMI8658_readStatus0(void)
{
	unsigned char status[2];

	QMI8658_read_reg(QMI8658Register_Status0, status, sizeof(status));
	// printf("status[0x%x	0x%x]\n",status[0],status[1]);

	return status[0];
}
/*!
 * \brief Blocking read of data status register 1 (::QMI8658Register_Status1).
 * \returns Status byte \see STATUS1 for flag definitions.
 */
unsigned char QMI8658_readStatus1(void)
{
	unsigned char status;

	QMI8658_read_reg(QMI8658Register_Status1, &status, sizeof(status));

	return status;
}

float QMI8658_readTemp(void)
{
	unsigned char buf[2];
	short temp = 0;
	float temp_f = 0;

	QMI8658_read_reg(QMI8658Register_Tempearture_L, buf, 2);
	temp = ((short)buf[1] << 8) | buf[0];
	temp_f = (float)temp / 256.0f;

	return temp_f;
}

void QMI8658_read_acc_xyz(float acc_xyz[3])
{
	unsigned char buf_reg[6];
	short raw_acc_xyz[3];

	QMI8658_read_reg(QMI8658Register_Ax_L, buf_reg, 6); // 0x19, 25
	raw_acc_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
	raw_acc_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
	raw_acc_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

	acc_xyz[0] = (raw_acc_xyz[0] * ONE_G) / acc_lsb_div;
	acc_xyz[1] = (raw_acc_xyz[1] * ONE_G) / acc_lsb_div;
	acc_xyz[2] = (raw_acc_xyz[2] * ONE_G) / acc_lsb_div;

	// QMI8658_printf("fis210x acc:	%f	%f	%f\n", acc_xyz[0], acc_xyz[1], acc_xyz[2]);
}

void QMI8658_read_gyro_xyz(float gyro_xyz[3])
{
	unsigned char buf_reg[6];
	short raw_gyro_xyz[3];

	QMI8658_read_reg(QMI8658Register_Gx_L, buf_reg, 6); // 0x1f, 31
	raw_gyro_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
	raw_gyro_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
	raw_gyro_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

	gyro_xyz[0] = (raw_gyro_xyz[0] * 1.0f) / gyro_lsb_div;
	gyro_xyz[1] = (raw_gyro_xyz[1] * 1.0f) / gyro_lsb_div;
	gyro_xyz[2] = (raw_gyro_xyz[2] * 1.0f) / gyro_lsb_div;

	// QMI8658_printf("fis210x gyro:	%f	%f	%f\n", gyro_xyz[0], gyro_xyz[1], gyro_xyz[2]);
}

void QMI8658_read_xyz(float acc[3], float gyro[3], unsigned int *tim_count)
{
	unsigned char buf_reg[12];
	short raw_acc_xyz[3];
	short raw_gyro_xyz[3];
	//	float acc_t[3];
	//	float gyro_t[3];

	if (tim_count)
	{
		unsigned char buf[3];
		unsigned int timestamp;
		QMI8658_read_reg(QMI8658Register_Timestamp_L, buf, 3); // 0x18	24
		timestamp = (unsigned int)(((unsigned int)buf[2] << 16) | ((unsigned int)buf[1] << 8) | buf[0]);
		if (timestamp > imu_timestamp)
			imu_timestamp = timestamp;
		else
			imu_timestamp = (timestamp + 0x1000000 - imu_timestamp);

		*tim_count = imu_timestamp;
	}

	QMI8658_read_reg(QMI8658Register_Ax_L, buf_reg, 12); // 0x19, 25
	raw_acc_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
	raw_acc_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
	raw_acc_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

	raw_gyro_xyz[0] = (short)((unsigned short)(buf_reg[7] << 8) | (buf_reg[6]));
	raw_gyro_xyz[1] = (short)((unsigned short)(buf_reg[9] << 8) | (buf_reg[8]));
	raw_gyro_xyz[2] = (short)((unsigned short)(buf_reg[11] << 8) | (buf_reg[10]));

#if defined(QMI8658_UINT_MG_DPS)
	// mg
	acc[AXIS_X] = (float)(raw_acc_xyz[AXIS_X] * 1000.0f) / acc_lsb_div;
	acc[AXIS_Y] = (float)(raw_acc_xyz[AXIS_Y] * 1000.0f) / acc_lsb_div;
	acc[AXIS_Z] = (float)(raw_acc_xyz[AXIS_Z] * 1000.0f) / acc_lsb_div;
#else
	// m/s2
	acc[AXIS_X] = (float)(raw_acc_xyz[AXIS_X] * ONE_G) / acc_lsb_div;
	acc[AXIS_Y] = (float)(raw_acc_xyz[AXIS_Y] * ONE_G) / acc_lsb_div;
	acc[AXIS_Z] = (float)(raw_acc_xyz[AXIS_Z] * ONE_G) / acc_lsb_div;
#endif
	//	acc[AXIS_X] = imu_map.sign[AXIS_X]*acc_t[imu_map.map[AXIS_X]];
	//	acc[AXIS_Y] = imu_map.sign[AXIS_Y]*acc_t[imu_map.map[AXIS_Y]];
	//	acc[AXIS_Z] = imu_map.sign[AXIS_Z]*acc_t[imu_map.map[AXIS_Z]];

#if defined(QMI8658_UINT_MG_DPS)
	// dps
	gyro[0] = (float)(raw_gyro_xyz[0] * 1.0f) / gyro_lsb_div;
	gyro[1] = (float)(raw_gyro_xyz[1] * 1.0f) / gyro_lsb_div;
	gyro[2] = (float)(raw_gyro_xyz[2] * 1.0f) / gyro_lsb_div;
#else
	// rad/s
	gyro[AXIS_X] = (float)(raw_gyro_xyz[AXIS_X] * 0.01745f) / gyro_lsb_div; // *pi/180
	gyro[AXIS_Y] = (float)(raw_gyro_xyz[AXIS_Y] * 0.01745f) / gyro_lsb_div;
	gyro[AXIS_Z] = (float)(raw_gyro_xyz[AXIS_Z] * 0.01745f) / gyro_lsb_div;
#endif
	//	gyro[AXIS_X] = imu_map.sign[AXIS_X]*gyro_t[imu_map.map[AXIS_X]];
	//	gyro[AXIS_Y] = imu_map.sign[AXIS_Y]*gyro_t[imu_map.map[AXIS_Y]];
	//	gyro[AXIS_Z] = imu_map.sign[AXIS_Z]*gyro_t[imu_map.map[AXIS_Z]];
}

void QMI8658_read_xyz_raw(short raw_acc_xyz[3], short raw_gyro_xyz[3], unsigned int *tim_count)
{
	unsigned char buf_reg[12];

	if (tim_count)
	{
		unsigned char buf[3];
		unsigned int timestamp;
		QMI8658_read_reg(QMI8658Register_Timestamp_L, buf, 3); // 0x18	24
		timestamp = (unsigned int)(((unsigned int)buf[2] << 16) | ((unsigned int)buf[1] << 8) | buf[0]);
		if (timestamp > imu_timestamp)
			imu_timestamp = timestamp;
		else
			imu_timestamp = (timestamp + 0x1000000 - imu_timestamp);

		*tim_count = imu_timestamp;
	}
	QMI8658_read_reg(QMI8658Register_Ax_L, buf_reg, 12); // 0x19, 25

	raw_acc_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
	raw_acc_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
	raw_acc_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));

	raw_gyro_xyz[0] = (short)((unsigned short)(buf_reg[7] << 8) | (buf_reg[6]));
	raw_gyro_xyz[1] = (short)((unsigned short)(buf_reg[9] << 8) | (buf_reg[8]));
	raw_gyro_xyz[2] = (short)((unsigned short)(buf_reg[11] << 8) | (buf_reg[10]));
}

void QMI8658_read_ae(float quat[4], float velocity[3])
{
	unsigned char buf_reg[14];
	short raw_q_xyz[4];
	short raw_v_xyz[3];

	QMI8658_read_reg(QMI8658Register_Q1_L, buf_reg, 14);
	raw_q_xyz[0] = (short)((unsigned short)(buf_reg[1] << 8) | (buf_reg[0]));
	raw_q_xyz[1] = (short)((unsigned short)(buf_reg[3] << 8) | (buf_reg[2]));
	raw_q_xyz[2] = (short)((unsigned short)(buf_reg[5] << 8) | (buf_reg[4]));
	raw_q_xyz[3] = (short)((unsigned short)(buf_reg[7] << 8) | (buf_reg[6]));

	raw_v_xyz[1] = (short)((unsigned short)(buf_reg[9] << 8) | (buf_reg[8]));
	raw_v_xyz[2] = (short)((unsigned short)(buf_reg[11] << 8) | (buf_reg[10]));
	raw_v_xyz[2] = (short)((unsigned short)(buf_reg[13] << 8) | (buf_reg[12]));

	quat[0] = (float)(raw_q_xyz[0] * 1.0f) / ae_q_lsb_div;
	quat[1] = (float)(raw_q_xyz[1] * 1.0f) / ae_q_lsb_div;
	quat[2] = (float)(raw_q_xyz[2] * 1.0f) / ae_q_lsb_div;
	quat[3] = (float)(raw_q_xyz[3] * 1.0f) / ae_q_lsb_div;

	velocity[0] = (float)(raw_v_xyz[0] * 1.0f) / ae_v_lsb_div;
	velocity[1] = (float)(raw_v_xyz[1] * 1.0f) / ae_v_lsb_div;
	velocity[2] = (float)(raw_v_xyz[2] * 1.0f) / ae_v_lsb_div;
}

void QMI8658_enableWakeOnMotion(void)
{
	unsigned char womCmd[3];
	enum QMI8658_Interrupt interrupt = QMI8658_Int1;
	enum QMI8658_InterruptState initialState = QMI8658State_low;
	enum QMI8658_WakeOnMotionThreshold threshold = QMI8658WomThreshold_low;
	unsigned char blankingTime = 0x00;
	const unsigned char blankingTimeMask = 0x3F;

	QMI8658_enableSensors(QMI8658_CTRL7_DISABLE_ALL);
	QMI8658_config_acc(QMI8658AccRange_2g, QMI8658AccOdr_LowPower_21Hz, QMI8658Lpf_Disable, QMI8658St_Disable);

	womCmd[0] = QMI8658Register_Cal1_L; // WoM Threshold: absolute value in mg (with 1mg/LSB resolution)
	womCmd[1] = threshold;
	womCmd[2] = (unsigned char)interrupt | (unsigned char)initialState | (blankingTime & blankingTimeMask);
	QMI8658_write_reg(QMI8658Register_Cal1_L, womCmd[1]);
	QMI8658_write_reg(QMI8658Register_Cal1_H, womCmd[2]);

	// QMI8658_doCtrl9Command(Ctrl9_ConfigureWakeOnMotion);
	QMI8658_enableSensors(QMI8658_CTRL7_ACC_ENABLE);
	// while(1)
	//{
	//	QMI8658_read_reg(QMI8658Register_Status1,&womCmd[0],1);
	//	if(womCmd[0]&0x01)
	//		break;
	// }
}

void QMI8658_disableWakeOnMotion(void)
{
	QMI8658_enableSensors(QMI8658_CTRL7_DISABLE_ALL);
	QMI8658_write_reg(QMI8658Register_Cal1_L, 0);
	// QMI8658_doCtrl9Command(Ctrl9_ConfigureWakeOnMotion);
}

void QMI8658_enableSensors(unsigned char enableFlags)
{
	if (enableFlags & QMI8658_CONFIG_AE_ENABLE)
	{
		enableFlags |= QMI8658_CTRL7_ACC_ENABLE | QMI8658_CTRL7_GYR_ENABLE;
	}

	QMI8658_write_reg(QMI8658Register_Ctrl7, enableFlags & QMI8658_CTRL7_ENABLE_MASK);
}

void QMI8658_Config_apply(struct QMI8658Config const *config)
{
	unsigned char fisSensors = config->inputSelection;

	if (fisSensors & QMI8658_CONFIG_AE_ENABLE)
	{
		QMI8658_config_ae(config->aeOdr);
	}
	else
	{
		if (config->inputSelection & QMI8658_CONFIG_ACC_ENABLE)
		{
			QMI8658_config_acc(config->accRange, config->accOdr, QMI8658Lpf_Enable, QMI8658St_Disable);
		}
		if (config->inputSelection & QMI8658_CONFIG_GYR_ENABLE)
		{
			QMI8658_config_gyro(config->gyrRange, config->gyrOdr, QMI8658Lpf_Enable, QMI8658St_Disable);
		}
	}

	if (config->inputSelection & QMI8658_CONFIG_MAG_ENABLE)
	{
		QMI8658_config_mag(config->magDev, config->magOdr);
	}
	QMI8658_enableSensors(fisSensors);
}

unsigned char QMI8658_init(void)
{
	unsigned char QMI8658_chip_id = 0x00;
	unsigned char QMI8658_revision_id = 0x00;
	unsigned char QMI8658_device[2] = {QMI8658_DEVICE_ADDR_L, QMI8658_DEVICE_ADDR_H};
	unsigned char iCount = 0;
	int retry = 0;

	while (iCount < 2)
	{
		QMI8658_device_addr = QMI8658_device[iCount];
		retry = 0;

		while ((QMI8658_chip_id != 0x05) && (retry++ < 5))
		{

			QMI8658_read_reg(QMI8658Register_WhoAmI, &QMI8658_chip_id, 1);
			QMI8658_printf("QMI8658Register_WhoAmI = 0x%x\n", QMI8658_chip_id);
		}
		if (QMI8658_chip_id == 0x05)
		{
			break;
		}
		iCount++;
	}
	QMI8658_read_reg(QMI8658Register_Revision, &QMI8658_revision_id, 1);
	if (QMI8658_chip_id == 0x05)
	{
		QMI8658_printf("QMI8658_init addr=0x%x  \r\nQMI8658Register_WhoAmI=0x%x 0x%x\n", QMI8658_device_addr, QMI8658_chip_id, QMI8658_revision_id);
		QMI8658_write_reg(QMI8658Register_Ctrl1, 0x60);
		QMI8658_config.inputSelection = QMI8658_CONFIG_ACCGYR_ENABLE;
		QMI8658_config.accRange = QMI8658AccRange_8g;
		QMI8658_config.accOdr = QMI8658AccOdr_1000Hz;
		QMI8658_config.gyrRange = QMI8658GyrRange_2048dps;
		QMI8658_config.gyrOdr = QMI8658GyrOdr_1000Hz;
		QMI8658_config.magOdr = QMI8658MagOdr_125Hz;
		QMI8658_config.magDev = MagDev_AKM09918;
		QMI8658_config.aeOdr = QMI8658AeOdr_128Hz;

		QMI8658_Config_apply(&QMI8658_config);
		if (1)
		{
			unsigned char read_data = 0x00;
			QMI8658_read_reg(QMI8658Register_Ctrl1, &read_data, 1);
			QMI8658_printf("QMI8658Register_Ctrl1=0x%x \n", read_data);
			QMI8658_read_reg(QMI8658Register_Ctrl2, &read_data, 1);
			QMI8658_printf("QMI8658Register_Ctrl2=0x%x \n", read_data);
			QMI8658_read_reg(QMI8658Register_Ctrl3, &read_data, 1);
			QMI8658_printf("QMI8658Register_Ctrl3=0x%x \n", read_data);
			QMI8658_read_reg(QMI8658Register_Ctrl4, &read_data, 1);
			QMI8658_printf("QMI8658Register_Ctrl4=0x%x \n", read_data);
			QMI8658_read_reg(QMI8658Register_Ctrl5, &read_data, 1);
			QMI8658_printf("QMI8658Register_Ctrl5=0x%x \n", read_data);
			QMI8658_read_reg(QMI8658Register_Ctrl6, &read_data, 1);
			QMI8658_printf("QMI8658Register_Ctrl6=0x%x \n", read_data);
			QMI8658_read_reg(QMI8658Register_Ctrl7, &read_data, 1);
			QMI8658_printf("QMI8658Register_Ctrl7=0x%x \n", read_data);
		}
		//		QMI8658_set_layout(2);
		return 1;
	}
	else
	{
		QMI8658_printf("QMI8658_init fail\n");
		QMI8658_chip_id = 0;
		return 0;
	}
	// return QMI8658_chip_id;
}
