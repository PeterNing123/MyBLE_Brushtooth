#ifndef __MY_COMMON_H__
#define __MY_COMMON_H__

//���õ�C��ͷ�ļ�
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
//Log��Ҫ���õ�ͷ�ļ�
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//SAADC���õ�ͷ�ļ�
#include "nrf_drv_saadc.h"
//APP��ʱ����Ҫ���õ�ͷ�ļ�
#include "app_timer.h"
//APP������Ҫ���õ�ͷ�ļ�
#include "app_button.h"
#include "nrf_delay.h"
//���Ź�
#include "nrfx_wdt.h"
#include "nrf_drv_clock.h"


#define DEVICE_NAME              "TCSY"                             // �豸�����ַ��� 
#define MANUFACTURER_NAME        "GDE"    /**< Manufacturer. Will be passed to Device Information Service. */
#define MODEL_NAME               "H1601"
//�̼��汾
#define FW_VER_1            1 
#define FW_VER_2            8   //'4':SDK6.1; '5':SDK10.0; '6':SDK12.3,'8':SDK15.2
#define FW_VER_3            02  //<=99 //vtest_changed_20181129
//Ӳ���汾
#define HW_VER_1            3 
#define HW_VER_2            0
#define HW_VER_3            00  



extern volatile uint32_t working_sec;
extern volatile uint32_t rtc_sec;

/******************����״̬*******************/
enum    //0-�ػ���1-������2-�ػ�Logo, 3-����Logo, 4-�ػ����� 5-��������6-���Ѵ���7-˯��״̬
{
    WORK_FLAG_POWER_OFF = 0,
    WORK_FLAG_WORKING,                  //�����ϵ繤��ģʽ
    WORK_FLAG_WORKING_LOW_BATT,         //�͵�������ģʽ
	WORK_FLAG_AUTO_OFF,                 //�͵����Զ��ػ�
	WORK_FLAG_CHARING,                  //���ģʽ
	WORK_FLAG_TSET,                     //����ģʽ
}; 
extern uint8_t work_flag;

/******************��س��*******************/
enum
{
	NO_CHARGE = 0,
	CHARGING,
	CHARGE_FULL,
};
extern uint8_t charge_flag;

typedef struct
{
	bool batt_low_f;   //�����ͱ�־λ
	uint8_t batt_off_s;//û���־λ
	bool batt_3v3_on_f;//��س�絽3v3�����־λ
	
	uint16_t batt_mv;  //��ص���
	uint16_t batt_mv_last;//��һ�ε�ص�ѹ
	uint8_t  batt_percent;//�����ٷֱ�
} my_charge_t;
extern my_charge_t my_charge;

/******************�����ж�*******************/
typedef struct
{
	/**@brief ����ģʽ�л���
	*0���ػ�
	*1������ģʽ1��2������ģʽ2��3������ģʽ3��4������ģʽ4��5������ģʽ5*/
	uint8_t key_mode;
	bool iskeypre_f;//�����Ƿ��ڰ��±�־λ
	uint16_t key_Pre_cnt;//��������ʱ�����
	bool key_mode_change_f;//ģʽ�л���־λ
} my_key_t;
extern my_key_t my_key;

typedef struct
{
	bool                   is_test_mode;//�Ƿ�����˲���ģʽ
	bool                   is_renmender;//�Ƿ���������
	bool                   is_end_rendender;
	
	uint8_t                mode;    //��﹤��ģʽ
	uint16_t               freq;    //���Ƶ��
	uint8_t				   duty;    //���ռ�ձ�
	uint8_t                strength;//���ǿ��
	uint16_t               run_time;//����ˢ��ʱ��
	uint16_t               dur_time;//ʵ��ˢ��ʱ��
	uint8_t                start_delay;//������ʱ
} my_motor_t;
extern my_motor_t my_motor;
extern uint16_t motor_data[7][4];
/******************BLE����*******************/
typedef struct
{
	bool is_connected; //�Ƿ�����������app
	bool is_ble_con;   //��������
	bool is_send_device_info;//�ϱ��豸��Ϣ
	bool is_send_his_data;   //�ϱ���ʷ����
	
	uint8_t device_info_s;
	uint8_t his_data_s;
	uint8_t mac_addr[6];
} my_ble_t;
extern my_ble_t my_ble;

//FDS�첽������־�ṹ��
typedef struct
{
	bool hisdata_record_update;    //��ʷ���ݼ�¼���±�־
	bool devicetime_record_update; //desp_record��¼���±�־
	bool read;       //����¼��־
	bool gc;         //��Ƭ�ռ���־
	bool busy;       //FDSæ��־
}my_fds_info_t;
extern my_fds_info_t my_fds_info;//����FDS�첽������־�ṹ��

//��¼��ʷ����
typedef struct
{
	uint32_t end_sec;    //ˢ�����ʱ��
	uint8_t  mode;       //ˢ��ģʽ
	uint16_t dur_time;   //ˢ��ʵ����ʱ
	uint16_t run_time;   //ˢ��������ʱ
}__attribute__((aligned(4)))his_data_t;
extern his_data_t his_data;

/***********************LED����************************/
#define LED_NUM    5
//����LEDʹ�õĹܽ�
#define LED1 		3      //����ģʽ1ָʾLED
#define LED2        7
#define LED3 	    19
#define LED4 	    30
#define LED5 	    29    
#define LED_BLE     9     //����ָʾLED
#define LED_WORK    13
#define LED_POWER   28 	  //��ԴָʾLED


#define LED1_ON  nrf_gpio_pin_clear(LED1)   //����͵�ƽ������
#define LED1_OFF nrf_gpio_pin_set(LED1)     //����ߵ�ƽ������
#define LED2_ON  nrf_gpio_pin_clear(LED2)   //����͵�ƽ������
#define LED2_OFF nrf_gpio_pin_set(LED2)     //����ߵ�ƽ������
#define LED3_ON  nrf_gpio_pin_clear(LED3)   //����͵�ƽ������
#define LED3_OFF nrf_gpio_pin_set(LED3)     //����ߵ�ƽ������
#define LED4_ON  nrf_gpio_pin_clear(LED4)   //����͵�ƽ������
#define LED4_OFF nrf_gpio_pin_set(LED4)     //����ߵ�ƽ������
#define LED5_ON  nrf_gpio_pin_clear(LED5)   //����͵�ƽ������
#define LED5_OFF nrf_gpio_pin_set(LED5)     //����ߵ�ƽ������
#define LED_WORK_ON  nrf_gpio_pin_clear(LED_WORK)   //����͵�ƽ������
#define LED_WORK_OFF nrf_gpio_pin_set(LED_WORK)     //����ߵ�ƽ������
#define LED_BLE_ON  nrf_gpio_pin_clear(LED_BLE)   //����͵�ƽ������
#define LED_BLE_OFF nrf_gpio_pin_set(LED_BLE)     //����ߵ�ƽ������
#define LED_POWER_ON  nrf_gpio_pin_clear(LED_POWER)   //����͵�ƽ������
#define LED_POWER_OFF nrf_gpio_pin_set(LED_POWER)     //����ߵ�ƽ������

#define LED_MODE_OFF LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;LED5_OFF;
#define LED_MODE_1   LED1_ON;LED2_OFF;LED3_OFF;LED4_OFF;LED5_OFF;
#define LED_MODE_2   LED1_OFF;LED2_ON;LED3_OFF;LED4_OFF;LED5_OFF;
#define LED_MODE_3   LED1_OFF;LED2_OFF;LED3_ON;LED4_OFF;LED5_OFF;
#define LED_MODE_4   LED1_OFF;LED2_OFF;LED3_OFF;LED4_ON;LED5_OFF;
#define LED_MODE_5   LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;LED5_ON;
#define LED_MODE_ALL LED1_ON;LED2_ON;LED3_ON;LED4_ON;LED5_ON;

#define LED_MODE_ON(mode)   switch(mode)		\
							{					\
								case 1:			\
									LED_MODE_1;	\
									break;		\
								case 2:			\
									LED_MODE_2;	\
									break;		\
								case 3:			\
									LED_MODE_3;	\
									break;		\
								case 4:			\
									LED_MODE_4;	\
									break;		\
								case 5:			\
									LED_MODE_5;	\
									break;		\
							}					\

/***********************BSP����************************/
/* 
* CHARGER
*/
#define CHG_IN_PIN_NO           23
#define CHG_IN_SET              nrf_gpio_cfg_input(CHG_IN_PIN_NO, NRF_GPIO_PIN_NOPULL);
#define CHG_IN_RD               nrf_gpio_pin_read(CHG_IN_PIN_NO)
#define STB_IN_PIN_NO           24
#define STB_IN_SET              nrf_gpio_cfg_input(STB_IN_PIN_NO, NRF_GPIO_PIN_NOPULL);
#define STB_IN_RD               nrf_gpio_pin_read(STB_IN_PIN_NO)

#define BATT_FULL_POWER 4170   //����������־λ 
#define BATT_LOW_POWER  3500   //3.5V���͵�������
#define BATT_AUTO_OFF   3000   //����3V�Զ��ػ�

/* 
* SAADC
*/
#define BATTERY_ADC_CHN     0
#define BATTERY_ADC_AIN     NRF_SAADC_INPUT_AIN0
#define BATTERY_ADC_RESULT_IN_MV(adc_value) (((uint32_t)(adc_value) * 600 * 1155) / (1024 * 150)) + 10

/***********************��������************************/
#define MY_BUTTON_1      6 //���尴��1����
//#define BUTTON_2       16//���尴��2���ţ�δ��
#define BUTTON_DETECTION_DELAY    APP_TIMER_TICKS(50) //������ʱ��⣬50ms


#endif
