#include "my_bms.h"
#include <stdlib.h>
#include <math.h>

//SAADC�¼��ص���������Ϊ�Ƕ���ģʽ�����Բ���Ҫ�¼������ﶨ����һ���յ��¼��ص�����
void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
}

//��ʼ��SAADC������ʹ�õ�SAADCͨ���Ĳ���
void My_saadc_Init(void)
{
	ret_code_t err_code;	
	//��ʼ��SAADC��ע���¼��ص�������
	err_code = nrf_drv_saadc_init(NULL, saadc_callback);
	APP_ERROR_CHECK(err_code);
	
	//��ʼ��SAADCͨ��0
	//����ADCͨ�����ýṹ�壬��ʹ�õ��˲������ú��ʼ����
	//NRF_SAADC_INPUT_AIN0��ʹ�õ�ģ������ͨ��
	nrf_saadc_channel_config_t channel_config =
	NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(BATTERY_ADC_AIN);
	channel_config.gain = NRF_SAADC_GAIN1;
	err_code = nrfx_saadc_channel_init(BATTERY_ADC_CHN, &channel_config);
	APP_ERROR_CHECK(err_code);
}

static uint8_t saadc_battery_level_in_percent_li_ion(const uint16_t mvolts)
{
    uint8_t battery_level;
    
    if (mvolts >= BATT_FULL_POWER)
    {
        battery_level = 100;
    }
    else if (mvolts > BATT_AUTO_OFF)
    {
        battery_level = ((mvolts - BATT_AUTO_OFF)*100) / (BATT_FULL_POWER - BATT_AUTO_OFF); //0~100%
    }
    else
    {
        battery_level = 0;
    }

    return battery_level;
}

my_charge_t my_charge = {
	.batt_low_f = false,   //�����ͱ�־λ
	.batt_off_s = 0,       //û���־λ
	.batt_3v3_on_f = false,//��س�絽3v3�����־λ
	.batt_mv = 0,
	.batt_mv_last = 0,
	.batt_percent = 0,
};
uint8_t charge_flag = NO_CHARGE;
void saadc_battery_read()
{
	uint8_t i;
	nrf_saadc_value_t batt_result, adc_value[10];//����10����ƽ��ֵ
	int32_t adc_value_sum = 0;
	uint16_t bat_max, bat_min;
	bat_max = 0;
	bat_min = 4500;

	for(i = 0; i < 10; i++)
	{
	//����һ��AD����
		nrfx_saadc_sample_convert(BATTERY_ADC_CHN, &adc_value[i]);
		if(bat_max < adc_value[i])
			bat_max = adc_value[i];
		if(bat_min > adc_value[i])
			bat_min = adc_value[i];
		adc_value_sum += adc_value[i];
    }
	adc_value_sum = adc_value_sum - bat_max - bat_min;//ȥ�����ֵ����Сֵ
	batt_result = adc_value_sum / 8;
    NRF_LOG_INFO("Sample value = %d\r\n", batt_result);
	
	my_charge.batt_mv = BATTERY_ADC_RESULT_IN_MV(batt_result);
	
	if((my_charge.batt_mv_last == 0) || (abs(my_charge.batt_mv - my_charge.batt_mv_last) < 50))
		my_charge.batt_mv_last = my_charge.batt_mv;
	else
		my_charge.batt_mv = my_charge.batt_mv_last;
	NRF_LOG_INFO("Voltage = %d\r\n", my_charge.batt_mv);
    my_charge.batt_percent = saadc_battery_level_in_percent_li_ion(my_charge.batt_mv);//��������ٷֱ�
	
	if(work_flag == WORK_FLAG_CHARING)//�Ƿ����
	{
		if(my_charge.batt_mv > BATT_FULL_POWER)
		{
			charge_flag = CHARGE_FULL;
			return;			
		}
	}
	
	if(my_charge.batt_mv < BATT_AUTO_OFF)//�͵����Զ��ػ�
	{
		my_charge.batt_off_s = 1;
		//work_flag = WORK_FLAG_AUTO_OFF;
	}
	else if(my_charge.batt_mv >= 3300)//�������
	{
		if(my_charge.batt_off_s)
		{
			my_charge.batt_off_s = 0;
			work_flag = WORK_FLAG_POWER_OFF;
		}
	}
	if((my_charge.batt_mv < (BATT_LOW_POWER - 50)) && (my_charge.batt_mv > BATT_AUTO_OFF))//������
	{
		my_charge.batt_low_f = true;
//		if(work_flag == WORK_FLAG_WORKING)//����ڹ���״̬
//		{
//			Led_flash_on();
//		}
//		if(work_flag == WORK_FLAG_TSET)//�˳�����ģʽ
//		{
//			work_flag = WORK_FLAG_POWER_OFF;
//		}
	}
	else if(my_charge.batt_mv > (BATT_LOW_POWER + 50))
		my_charge.batt_low_f = false;
	

}
