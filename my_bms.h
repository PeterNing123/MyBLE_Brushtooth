#ifndef __MY_BMS_H__
#define __MY_BMS_H__

//SAADC���õ�ͷ�ļ�
#include "nrf_drv_saadc.h"
#include "common.h"


//��ʼ��SAADC������ʹ�õ�SAADCͨ���Ĳ���
void My_saadc_Init(void);
void saadc_battery_read(void);

#endif 
