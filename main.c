#include "common.h"
#include "my_led.h"
#include "my_key.h"
#include "my_bms.h"
#include "my_fds.h"
#include "my_ble.h"
#include "my_ble_uarts.h"
#include "my_motor.h"

uint8_t work_flag = WORK_FLAG_POWER_OFF;//����״̬��־λ��Ĭ�Ϲػ�
//RTC
APP_TIMER_DEF(m_second_timer_id);
#define SECOND_INTERVAL         APP_TIMER_TICKS(1000)//���ʱӦ�ö�ʱ����� 1��
//���Ź�ͨ��
nrfx_wdt_channel_id m_wdt_channel_id;
//WDT�ж��пɻ��ѵ����ʱ����2��32.768KHzʱ�����ڣ�֮��ϵͳ��λ
void wdt_event_handler(void)
{}
//���Ź���ʼ������ʼ����ɺ���������Ź������Ź�һ����������޷�ֹͣ
void My_wdt_Init(void)
{
    uint32_t err_code = NRF_SUCCESS;
    
    //����WDT���ýṹ�岢ʹ��
    nrfx_wdt_config_t config = NRFX_WDT_DEAFULT_CONFIG;
	  //��ʼ��WDT
    err_code = nrfx_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
	  //����ι��ͨ����Ҳ����ʹ���ĸ�
    err_code = nrfx_wdt_channel_alloc(&m_wdt_channel_id);
    APP_ERROR_CHECK(err_code);
	  //����WDT
    nrfx_wdt_enable();       
}
//ʱ�����
volatile uint32_t working_sec = 0;//�ϵ繤��ʱ��
volatile uint32_t systick = 0;//�ϵ繤��ʱ��
volatile uint32_t rtc_sec = 0;//ʱ��

//���ʱ Ӧ�ö�ʱ���¼��ص�����
static void second_updata_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
	 //����tm��ʽʱ�������ʱ����±�־
    //CAL_updata();
	rtc_sec++;
	systick++;
	
	if((systick % 5 == 0) && (my_key.iskeypre_f == false))//ÿ10s����һ�ε�ص�ѹ
		saadc_battery_read();  //������ص�ѹ
	
	if(my_key.iskeypre_f)
	{
		my_key.key_Pre_cnt++;
		if(my_key.key_Pre_cnt == 2)//����2s�ػ�
		{			
			my_key.key_mode--;//�����´ο�������ģʽ����
			//if(work_flag == WORK_FLAG_WORKING)
				//My_device_sleep();
			work_flag = WORK_FLAG_POWER_OFF;
		}
		if(my_key.key_Pre_cnt >= 8)//����8s�������ģʽ
		{
			if(my_charge.batt_low_f == false)//�͵������������ģʽ
				work_flag = WORK_FLAG_TSET;
		}
	}
	
	if(work_flag == WORK_FLAG_WORKING)
	{
		working_sec++;
	}
}

//��ʼ��APP��ʱ��ģ��
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();//��ʼ��APP��ʱ��ģ��
    APP_ERROR_CHECK(err_code);             //��鷵��ֵ

    //���봴���û���ʱ����Ĵ��룬�����û���ʱ���� 
	err_code = app_timer_create(&m_second_timer_id, APP_TIMER_MODE_REPEATED, second_updata_handler);
    APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(m_second_timer_id, SECOND_INTERVAL, NULL);//����RTC��ʱ��
    APP_ERROR_CHECK(err_code);
	
	motor_timer_creat();
}
//��ʼ����Դ����ģ��
static void power_management_init(void)
{
    ret_code_t err_code;
	  //��ʼ����Դ����
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

static void log_init(void)
{
    //��ʼ��log����ģ��
	ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    //����log����նˣ�����sdk_config.h�е�������������ն�ΪUART����RTT��
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

//����״̬�����������û�й������־��������˯��ֱ����һ���¼���������ϵͳ
static void idle_state_handle(void)
{
    //��������log
	if (NRF_LOG_PROCESS() == false)
    {  
		nrf_pwr_mgmt_run();//���е�Դ�����ú�����Ҫ�ŵ���ѭ������ִ��
    }
}


void Device_Work_Loop(void);
//������
int main(void)
{
	NRF_UICR->NFCPINS = 0;
	uint32_t                err_code;
	err_code = ble_dfu_buttonless_async_svci_init();
    APP_ERROR_CHECK(err_code);
	
	log_init();             //��ʼ��log����ģ��	
	timers_init();          //��ʼ��APP��ʱ��
	power_management_init();//��ʼ����Դ����
	
	My_ble_Init();
	My_led_Init();
	buttons_init();
	My_saadc_Init();
	My_wdt_Init();
	My_fds_Init();
	
    NRF_LOG_INFO("Smart brushtooth started.");  
	read_devicetime_record();
	read_hiscnt_record();
	advertising_start();    //�����㲥
	//saadc_battery_read();  //������ص�ѹ
	//m_motor_config(1);

	while(true)
	{
		Device_Work_Loop();
		//��������LOG�����е�Դ����
		idle_state_handle();
	}
}

void Device_Work_Loop(void)
{
	//ι��
	nrfx_wdt_channel_feed(m_wdt_channel_id);
	
	//�����¼�
	if(my_ble.is_connected)
	{
		if(work_flag == WORK_FLAG_WORKING)//��������������ָʾ�Ʋ���
			LED_BLE_OFF;
		else
			LED_BLE_ON;
		
		if(my_fds_info.devicetime_record_update)//���±����¼�
		{
			my_fds_info.devicetime_record_update = false;
			update_devicetime_record();
		}
		
		if(my_ble.is_send_device_info)//�����豸��Ϣ
		{
//			my_ble.device_info_s++;
//			if(my_ble.device_info_s > 7)
//			{
//				my_ble.device_info_s = 0;
				my_ble.is_send_device_info = false;
				//return;
			//}
			send_device_info(my_ble.device_info_s);
		}
		
		if(my_ble.is_send_his_data)//������ʷ����
		{
			my_ble.his_data_s++;
			if(my_ble.his_data_s > 240)
			{
				my_ble.is_send_his_data = false;
				return;
			}
			read_hisdata_record(my_ble.his_data_s);
			
		}
	}
	
	//����ģʽ�ж�
	switch(work_flag)
	{
		case WORK_FLAG_AUTO_OFF:
		{
			break;
		}
		
		case WORK_FLAG_CHARING:
		{
			if(charge_flag == CHARGE_FULL)
			{
				Led_softblink_off();
				LED_POWER_ON;
				NRF_LOG_INFO("Charge is not\r\n");
			}
			break;
		}
		
		case WORK_FLAG_POWER_OFF:
		{	
			m_motor_test_off();
			if(my_motor.is_end_rendender)
			{
				my_motor.is_end_rendender = false;
				m_motor_remender();	
			}
			if(my_fds_info.devicetime_record_update)
			{
				my_fds_info.devicetime_record_update = false;
				update_devicetime_record();
			}
			if(my_ble.is_ble_con == false)
				Led_all_Off();
			if((my_motor.mode == 4) && is_mode4_final)
			{
				mode4_timer_stop(); 
				is_mode4_final = false;
			}
			m_motor_stop();
			
			if(my_fds_info.hisdata_record_update)
				update_hisdata_record();
			break;
		}
		
		case WORK_FLAG_WORKING:
		{
			//��⵽û����
			if(my_charge.batt_off_s == 1)
			{
				my_charge.batt_off_s = 2;
				Led_all_Off();
				m_motor_stop();
				Led_quick_flash30();
				work_flag = WORK_FLAG_AUTO_OFF;
				return;
			}
			
			//��⵽������
			if(my_charge.batt_low_f)
			{
				LED_WORK_OFF;
				Led_flash_on();
			}
			else
			{
				Led_flash_off();
				LED_WORK_ON;
			}
			
			if(my_key.key_mode_change_f)
			{
				my_key.key_mode_change_f = false;
				LED_MODE_ON(my_key.key_mode);
				//��⵽�����ͣ����������
				if(my_charge.batt_low_f)
					m_motor_remender();	
				//������ʱ
				if(my_motor.start_delay)
					nrf_delay_ms(my_motor.start_delay);
				
				m_motor_start(my_motor.mode, 0, 0, 2, 0);
			}			
			break;
		}
		
		case WORK_FLAG_TSET:
		{
			//��⵽������
			if(my_charge.batt_low_f)
			{
				work_flag = WORK_FLAG_POWER_OFF;
			}
			else
			{
				m_motor_test_on();
			}
			break;
		}
	}
}

