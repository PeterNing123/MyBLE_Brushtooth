#include "my_key.h"
#include "my_led.h"
#include "my_motor.h"

my_key_t my_key = 
{
	.key_mode = 0,
	.iskeypre_f = false,
	.key_Pre_cnt = 0,
	.key_mode_change_f = false,
};

extern volatile uint32_t systick;//�ϵ繤��ʱ��
uint8_t charger_flag = NO_CHARGE;     // 0-no charger;  1-charging;  2-charge full
//APP�����¼��ص�����
void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
	switch (pin_no)//�жϼ�ֵ
    {
        case MY_BUTTON_1://��BUTTON_1����
			if(systick < 6)//���ϵ磬��������
				return;
			if(work_flag == WORK_FLAG_CHARING)//��粻������
			{
				return;
			}
			if(my_motor.is_renmender)//�������ڼ����ΰ���
				return;
            if(button_action == APP_BUTTON_PUSH)//���������¼�
			{
				my_key.iskeypre_f = true;
				if(work_flag == WORK_FLAG_TSET)//�����ڲ���ģʽ���ٰ��˳�
				{
					work_flag = WORK_FLAG_POWER_OFF;
					return;
				}
				
				if(my_ble.is_ble_con)
				{
					my_ble.is_ble_con = false;
					my_key.key_mode--;
				}
				
				if(working_sec <= 5)//5s���ڰ����л�ģʽ
				{
					my_key.key_mode_change_f = true;
					working_sec = 0; //���㿪ʼ���㹤��ʱ��
					my_key.key_mode++;
					if(my_key.key_mode > LED_NUM)
						my_key.key_mode = 1;
					my_motor.mode = my_key.key_mode;
					//m_motor_mode = key_mode;
				}
				else
				{
					working_sec = 0; //���㿪ʼ���㹤��ʱ��
					my_key.key_mode--;
					work_flag = WORK_FLAG_POWER_OFF;						
					return;
				}			
				work_flag = WORK_FLAG_WORKING;
			}
			
			if(button_action == APP_BUTTON_RELEASE)//�����ͷ��¼�
			{
				my_key.iskeypre_f = false;
				my_key.key_Pre_cnt = 0;
			}
			break;
			
		case CHG_IN_PIN_NO://������������ؿ�ʼ���
           if(button_action == APP_BUTTON_PUSH)//���������¼�
			{
				if(work_flag == WORK_FLAG_WORKING)
				{//��⵽�����ͣ����
					my_key.key_mode--;					
					Led_all_Off();
					m_motor_stop();
				}
				work_flag = WORK_FLAG_CHARING;
				charge_flag = CHARGING;
				Led_softblink_on();
				NRF_LOG_INFO("Charging.\r\n"); 
			}
			
			if(button_action == APP_BUTTON_RELEASE)//��������»����
			{
				//if(STB_IN_RD)
				{
					charge_flag = NO_CHARGE;
					work_flag = WORK_FLAG_POWER_OFF;
					Led_softblink_off();
					NRF_LOG_INFO("Charge is not\r\n"); 
				}
			}
           break;
			
		/*case STB_IN_PIN_NO://��ص�������
           if(button_action == APP_BUTTON_PUSH)//���������¼�
			{
				work_flag = WORK_FLAG_CHARING;
				//LED_POWER_ON;
				LED_MODE_OFF;
				
				NRF_LOG_INFO("Charge is over.\r\n"); 
			}
			
			if(button_action == APP_BUTTON_RELEASE)//�����ͷ��¼�
			{
				work_flag = WORK_FLAG_POWER_OFF;
				LED_POWER_OFF;
				NRF_LOG_INFO("Charge is not\r\n");
			}*/
         //  break;
        default:
            return; 
    }
}
void buttons_init(void)
{
    ret_code_t err_code;
	
    //app����������붨��Ϊstatic���ͣ���Ϊ��������ģ����Ҫ����ָ��������ָ��
    static app_button_cfg_t buttons[] =
    {
        {MY_BUTTON_1, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
		{CHG_IN_PIN_NO, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},//���ָʾ��
		{STB_IN_PIN_NO, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},//����ָʾ��
    };
    //��ʼ��APP����
    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);		
	//ʹ��APP����
	err_code = app_button_enable();
	APP_ERROR_CHECK(err_code);
}

