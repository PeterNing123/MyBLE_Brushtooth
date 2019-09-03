/**--------------File Info-----------------------------------------------------------------------------
** File          name:my_ble_uarts.c
** Last modified Date:          
** Last       Version:		   
** Descriptions      :����͸�������ļ�			
**---------------------------------------------------------------------------------------------------*/
#include "sdk_common.h"
//���������˴���͸���������ģ��ġ����뿪�ء�������Ҫ��sdk_config.h������BLE_MY_UARTS
#if NRF_MODULE_ENABLED(BLE_MY_UARTS)
#include "my_ble_uarts.h"

#include <stdlib.h>
#include <string.h>
#include "app_error.h"
#include "ble_gatts.h"
#include "ble_srv_common.h"
#include "common.h"
#include "my_motor.h"
#include "my_fds.h"

#define BLE_UARTS_MAX_RX_CHAR_LEN        BLE_UARTS_MAX_DATA_LEN //RX������󳤶ȣ��ֽ�����
#define BLE_UARTS_MAX_TX_CHAR_LEN        BLE_UARTS_MAX_DATA_LEN //TX������󳤶ȣ��ֽ����� 

uint32_t sec;

uint8_t receive_resp[4] = {0x04,0x01,0x01,0x01};//���ջظ���bit4:0����1�ɹ�
uint16_t resp_len = 4;
uint8_t send_data[BLE_UARTS_MAX_TX_CHAR_LEN] = {0};//����͸��Ҫ���͵�����
uint16_t send_len = BLE_UARTS_MAX_TX_CHAR_LEN;

extern uint32_t my_ble_uarts_resp_send(uint8_t  * p_data, uint16_t  * p_length);
extern uint32_t my_ble_uarts_data_send(uint8_t  * p_data, uint16_t  * p_length);

//SoftDevice�ύ��"write"�¼�������
static void on_write(ble_uarts_t * p_uarts, ble_evt_t const * p_ble_evt)
{
    //����һ������͸���¼��ṹ�����������ִ�лص�ʱ���ݲ���
    ble_uarts_evt_t                 evt;
    //����write�¼��ṹ��ָ�벢ָ��GATT�¼��е�wirite
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
   
    memset(&evt, 0, sizeof(ble_uarts_evt_t));              //����evt  
    evt.p_uarts       = p_uarts;                           //ָ�򴮿�͸������ʵ��  
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;//�������Ӿ��

    //дRX����ֵ
    if ((p_evt_write->handle == p_uarts->rx_handles.value_handle) && (p_uarts->data_handler != NULL))
    {      
		evt.type                  = BLE_UARTS_EVT_RX_DATA; //�����¼�����		
        evt.params.rx_data.p_data = p_evt_write->data;     //�������ݳ���		  
        evt.params.rx_data.length = p_evt_write->len;	   //�������ݳ���     
        p_uarts->data_handler(&evt);                       //ִ�лص�
    }
    else
    {
        //���¼��ʹ���͸�������޹أ�����
    }
}

//SoftDevice�ύ��"BLE_GATTS_EVT_HVN_TX_COMPLETE"�¼�������
static void on_hvx_tx_complete(ble_uarts_t * p_uarts, ble_evt_t const * p_ble_evt)
{
    //����һ������͸���¼��ṹ�����evt������ִ�лص�ʱ���ݲ���
	ble_uarts_evt_t              evt;
    
    memset(&evt, 0, sizeof(ble_uarts_evt_t));                //����evt	  
    evt.type        = BLE_UARTS_EVT_TX_RDY;                  //�����¼�����	  
    evt.p_uarts     = p_uarts;                               //ָ�򴮿�͸������ʵ��
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;  //�������Ӿ��
   
    p_uarts->data_handler(&evt); //ִ�лص�
}

//
//����͸������BLE�¼������ߵ��¼��ص�����
void ble_uarts_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{   
	if ((p_context == NULL) || (p_ble_evt == NULL))//�������Ƿ���Ч
    {
        return;
    }
    //����һ������͸���ṹ��ָ�벢ָ�򴮿�͸���ṹ��
    ble_uarts_t * p_uarts = (ble_uarts_t *)p_context;
    //�ж��¼�����
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GATTS_EVT_WRITE://д�¼�
			//����д�¼�
            on_write(p_uarts, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE://TX�����¼�
			//����TX�����¼�
            on_hvx_tx_complete(p_uarts, p_ble_evt);
            break;

        default:
            break;
    }
}

//��ʼ������͸������
uint32_t ble_uarts_init(ble_uarts_t * p_uarts, ble_uarts_init_t const * p_uarts_init)
{
    ret_code_t            err_code;  
    ble_uuid_t            ble_uuid;       //����16λUUID�ṹ�����	  
   // ble_uuid128_t         nus_base_uuid = UARTS_BASE_UUID;//����128λUUID�ṹ�����������ʼ��Ϊ����͸������UUID����
	  
    ble_add_char_params_t add_char_params;//�������������ṹ�����
   
    VERIFY_PARAM_NOT_NULL(p_uarts);       //���ָ���Ƿ�ΪNULL
    VERIFY_PARAM_NOT_NULL(p_uarts_init);

    p_uarts->data_handler = p_uarts_init->data_handler;//��������͸�������ʼ���ṹ���е��¼����

    //���Զ���UUID������ӵ�SoftDevice
    //err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &p_uarts->uuid_type);
    //VERIFY_SUCCESS(err_code);
    //UUID���ͺ���ֵ��ֵ��ble_uuid����
   // ble_uuid.type = p_uarts->uuid_type;
    //ble_uuid.uuid = BLE_UUID_UARTS_SERVICE;
	BLE_UUID_BLE_ASSIGN(ble_uuid, NUS_SEVICE_UUID);  // Add a UUID.
    //��Ӵ���͸������
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_uarts->service_handle);
    VERIFY_SUCCESS(err_code);
    /*---------------------���´������RX����--------------------*/
    memset(&add_char_params, 0, sizeof(add_char_params));						//���ò���֮ǰ������add_char_params		
    add_char_params.uuid                     = NUS_RX_CHARACTERISTIC;//RX������UUID		
    add_char_params.uuid_type                = p_uarts->uuid_type;				//RX������UUID����		
    add_char_params.max_len                  = BLE_UARTS_MAX_RX_CHAR_LEN;		//����RX����ֵ����󳤶�	
    add_char_params.init_len                 = sizeof(uint8_t);					//����RX����ֵ�ĳ�ʼ����	
    add_char_params.is_var_len               = true;							//����RX������ֵ����Ϊ�ɱ䳤��		
    add_char_params.char_props.write         = 1;								//����RX����������֧��д	
	add_char_params.char_props.notify = 1;										//����TX���������ʣ�֧��֪ͨ
    //add_char_params.char_props.write_wo_resp = 1;								//����RX����������֧������Ӧд 
    add_char_params.read_access  = SEC_OPEN;									//���ö�/дRX����ֵ�İ�ȫ�����ް�ȫ��
    add_char_params.write_access = SEC_OPEN;
	add_char_params.cccd_write_access = SEC_OPEN;
    //Ϊ����͸���������RX����
    err_code = characteristic_add(p_uarts->service_handle, &add_char_params, &p_uarts->rx_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	/*---------------------���RX����-END------------------------*/
		
    /*---------------------���´������TX����--------------------*/
		
    memset(&add_char_params, 0, sizeof(add_char_params));						//���ò���֮ǰ������add_char_params		
    add_char_params.uuid              = NUS_TX_CHARACTERISTIC;		//TX������UUID	
    add_char_params.uuid_type         = p_uarts->uuid_type;						//TX������UUID����	
    add_char_params.max_len           = BLE_UARTS_MAX_TX_CHAR_LEN;				//����TX����ֵ����󳤶�	
    add_char_params.init_len          = sizeof(uint8_t);						//����TX����ֵ�ĳ�ʼ����	
    add_char_params.is_var_len        = true;									//����TX������ֵ����Ϊ�ɱ䳤��
    add_char_params.char_props.notify = 1;										//����TX���������ʣ�֧��֪ͨ
    //���ö�/дRX����ֵ�İ�ȫ�����ް�ȫ��
    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;
    //Ϊ����͸���������TX����
    return characteristic_add(p_uarts->service_handle, &add_char_params, &p_uarts->tx_handles);
		/*---------------------���TX����-END------------------------*/
}
uint32_t ble_uarts_resp_send(ble_uarts_t * p_uarts,
                             uint8_t     * p_data,
                             uint16_t    * p_length,
                             uint16_t    conn_handle)
{
    ble_gatts_hvx_params_t     hvx_params;			//��֤p_uartsû��ָ��NULL
    
    VERIFY_PARAM_NOT_NULL(p_uarts);

    //������Ӿ����Ч����ʾû�к������������ӣ�����NRF_ERROR_NOT_FOUND
	  if (conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (*p_length > BLE_UARTS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    memset(&hvx_params, 0, sizeof(hvx_params));					//����֮ǰ������hvx_params    
    hvx_params.handle = p_uarts->rx_handles.value_handle;		//RX����ֵ���	
    hvx_params.p_data = p_data;									//���͵�����		
    hvx_params.p_len  = p_length;								//���͵����ݳ���		
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;				//����Ϊ֪ͨ
    
    return sd_ble_gatts_hvx(conn_handle, &hvx_params);			//����TX����ֵ֪ͨ
}

uint32_t ble_uarts_data_send(ble_uarts_t * p_uarts,
                             uint8_t     * p_data,
                             uint16_t    * p_length,
                             uint16_t    conn_handle)
{
    ble_gatts_hvx_params_t     hvx_params;			//��֤p_uartsû��ָ��NULL
    
    VERIFY_PARAM_NOT_NULL(p_uarts);

    //������Ӿ����Ч����ʾû�к������������ӣ�����NRF_ERROR_NOT_FOUND
	  if (conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (*p_length > BLE_UARTS_MAX_DATA_LEN)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    memset(&hvx_params, 0, sizeof(hvx_params));					//����֮ǰ������hvx_params    
    hvx_params.handle = p_uarts->tx_handles.value_handle;		//TX����ֵ���	
    hvx_params.p_data = p_data;									//���͵�����		
    hvx_params.p_len  = p_length;								//���͵����ݳ���		
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;				//����Ϊ֪ͨ
    
    return sd_ble_gatts_hvx(conn_handle, &hvx_params);			//����TX����ֵ֪ͨ
}

bool is_ble_motor_con = 0;
//����͸���¼��ص�����������͸�������ʼ��ʱע��
void uarts_data_handler(ble_uarts_evt_t * p_evt)
{	
	//�ж��¼�����:���յ��������¼�
    if (p_evt->type == BLE_UARTS_EVT_RX_DATA)
    {
		uint8_t const *p_data_buff = p_evt->params.rx_data.p_data;
		//��ȡ�豸��Ϣ
		if((p_data_buff[0] == 0x03)  &&  (p_data_buff[1] == 0x01))
		{
			switch(p_data_buff[2])
			{
				case 0x01:
					//resp
					receive_resp[1] = p_data_buff[1];
					receive_resp[2] = p_data_buff[2];
					my_ble_uarts_resp_send(receive_resp, &resp_len);
				
					NRF_LOG_INFO("Send device info!");
					my_ble.device_info_s = 0;
					my_ble.is_send_device_info = true;
					break;
			}
		}
		//����ʱ��
		else if((p_data_buff[0] == 0x07)  &&  (p_data_buff[1] == 0x02))
		{
			switch(p_data_buff[2])
			{
				case 0x01:
					//resp
					receive_resp[1] = p_data_buff[1];
					receive_resp[2] = p_data_buff[2];
					my_ble_uarts_resp_send(receive_resp, &resp_len);
					//save
//					my_fds_info.devicetime_record_update = true;
					rtc_sec = p_data_buff[3] + ((uint32_t)p_data_buff[4]<<8) + ((uint32_t)p_data_buff[5]<<16) + 
									  (   (uint32_t)p_data_buff[6]<<24)  -28800;
//					//sec += 0x386d4380;    //since 1970
//					//rtc_tm = *localtime((time_t*)&sec);
//					//rtc_sec = mktime(&rtc_tm);
//					update_devicetime_record();
					//update_devicetime_record();
					my_fds_info.devicetime_record_update = true;
					NRF_LOG_INFO("RTC: date=%d\r\n", rtc_sec);
					
					send_data[0] = 0x07;
					send_data[1] = 0x02;
					send_data[2] = 0x01;
					send_data[3] = p_data_buff[3];
					send_data[4] = p_data_buff[4];
					send_data[5] = p_data_buff[5];
					send_data[6] = p_data_buff[6];
					send_len = sizeof(send_data);
					my_ble_uarts_data_send(send_data, &send_len);
					//update_devicetime_record();
					break;
			}
		}
		else if((p_data_buff[0] == 0x09)  &&  (p_data_buff[1] == 0x03))
		{
			//if(work_flag == WORK_FLAG_CHARING)//��粻������
			//{
				//return;
			//}
			if(p_data_buff[2])
			{
				//resp
				receive_resp[1] = p_data_buff[1];
				receive_resp[2] = p_data_buff[2];
				my_ble_uarts_resp_send(receive_resp, &resp_len);
				
				my_motor.mode = my_key.key_mode = p_data_buff[3] + 1; //����ˢ��ģʽ
				motor_data[my_motor.mode][2] = p_data_buff[4]; //����ǿ��
				motor_data[my_motor.mode][3] = ((uint16_t)p_data_buff[6]<<8) | p_data_buff[5]; //����ˢ��ʱ��				
				
				NRF_LOG_INFO("Setting motor run!");
				LED_MODE_ON(my_key.key_mode);
				if(work_flag == WORK_FLAG_WORKING)//����Ѿ������������������
					m_motor_start(my_motor.mode,0,0,2,120);
				else
					my_ble.is_ble_con = true;      //�ȴ�������
			}
		}
		//��ȡ����
		else if((p_data_buff[0] == 0x04)  &&  (p_data_buff[1] == 0x04))
		{
			switch(p_data_buff[2])
			{
				case 0x01:
					//resp
					receive_resp[1] = p_data_buff[1];
					receive_resp[2] = p_data_buff[2];
					my_ble_uarts_resp_send(receive_resp, &resp_len);
					NRF_LOG_INFO("Read motor param!");
					//nus_data_type
					send_data[0] = 0x09;
					send_data[1] = 0x04;
					send_data[2] = 0x01;
					send_data[3] = p_data_buff[3];
					send_data[4] = motor_data[send_data[3]+1][2];
					send_data[5] = motor_data[send_data[3]+1][3] & 0xFF;
					send_data[6] = (motor_data[send_data[3]+1][3])>>8;		
					send_len = sizeof(send_data);
					my_ble_uarts_data_send(send_data, &send_len);
					break;
			}
		}
		//��ȡ��ʷ����
		else if((p_data_buff[0] == 0x03)  &&  (p_data_buff[1] == 0x06))
		{
			switch(p_data_buff[2])
			{
				case 0x01:
					//resp
					receive_resp[1] = p_data_buff[1];
					receive_resp[2] = p_data_buff[2];
					my_ble_uarts_resp_send(receive_resp, &resp_len);
					NRF_LOG_INFO("Read history data!");
					my_ble.is_send_his_data = true;
					my_ble.his_data_s = 0;
					break;
			}
		}
		//�����ʷ����
		else if((p_data_buff[0] == 0x03)  &&  (p_data_buff[1] == 0x07))
		{
			switch(p_data_buff[2])
			{
				case 0x01:
					//resp
					receive_resp[1] = p_data_buff[1];
					receive_resp[2] = p_data_buff[2];
					my_ble_uarts_resp_send(receive_resp, &resp_len);
					
					NRF_LOG_INFO("Clear history data!");
//					//todo
//					//record_flag = 1;
//					//record_clear();
					break;
			}
		}
		//���ɽ�����
		else if((p_data_buff[0] == 0x04)  &&  (p_data_buff[1] == 0x09))
		{
			switch(p_data_buff[2])
			{
				case 0x01:
				//resp
				receive_resp[1] = p_data_buff[1];
				receive_resp[2] = p_data_buff[2];
				my_ble_uarts_resp_send(receive_resp, &resp_len);
				
				my_motor.start_delay = p_data_buff[3];
				NRF_LOG_INFO("Set start delay!");
				//nus_data_type
				send_data[0] = 0x04;
				send_data[1] = 0x09;
				send_data[2] = 0x01;
				send_data[3] = my_motor.start_delay;
				send_len = sizeof(send_data);
				my_ble_uarts_data_send(send_data, &send_len);
				break;
			}
		}
    }
		
		//�ж��¼�����:���;����¼������¼��ں����������õ�����ǰ�����ڸ��¼��з�תָʾ��D4��״̬��ָʾ���¼��Ĳ���
    if (p_evt->type == BLE_UARTS_EVT_TX_RDY)
    {
			//nrf_gpio_pin_toggle(LED_4);
	}
}

void send_device_info(uint8_t type)
{
	uint16_t i;
    uint16_t len, len_all;
    char buff[NRF_SDH_BLE_GATT_MAX_MTU_SIZE-OPCODE_LENGTH-HANDLE_LENGTH];
    char *p;
    ble_gap_addr_t addr;
	
	memset(send_data, 0, sizeof(send_data));
	p = (char *)send_data;
	len_all = 0;
	//
	strcpy(buff, "\x01\x01");
	strcat(buff, MANUFACTURER_NAME);
	len = strlen(buff)+1;
	*p = len;
	strcat(p, buff);
	p += len;
	len_all += len;
	//
	strcpy(buff, "\x01\x02");
	strcat(buff, MODEL_NAME);
	len = strlen(buff)+1;
	*p = len;
	strcat(p, buff);
	p += len;
	len_all += len;
	//
	strcpy(buff, "\x01\x03");
	sd_ble_gap_addr_get(&addr);
	for(i=0; i<6; i++)
	{
		len = (addr.addr[5-i]>>4);
		len = len>=10?(len+0x37):(len+0x30);
		buff[2+2*i] = len;
		len = addr.addr[5-i]%0x10;
		len = len>=10?(len+0x37):(len+0x30);
		buff[2+2*i+1] = len;
	}
	len = strlen(buff)+1;
	*p = len;
	strcat(p, buff);
	p += len;
	len_all += len;
	//
	strcpy(buff, "\x01\x04");
	buff[2] = HW_VER_1 + 0x30;
	buff[3] = '.';
	buff[4] = HW_VER_2 + 0x30;
	buff[5] = '.';
	buff[6] = HW_VER_3/10 + 0x30;
	buff[7] = HW_VER_3%10 + 0x30;
	buff[8] = 0;
	len = strlen(buff)+1;
	*p = len;
	strcat(p, buff);
	p += len;
	len_all += len;
	//
	strcpy(buff, "\x01\x05");
	buff[2] = FW_VER_1 + 0x30;
	buff[3] = '.';
	buff[4] = FW_VER_2 + 0x30;
	buff[5] = '.';
	buff[6] = FW_VER_3/10 + 0x30;
	buff[7] = FW_VER_3%10 + 0x30;
	buff[8] = 0;
	len = strlen(buff)+1;
	*p = len;
	strcat(p, buff);
	p += len;
	len_all += len;
	//
	strcpy(buff, "\x01\x06");
	*((uint32_t *)&buff[2]) = rtc_sec;
	len = 6+1;
	*p = len;
	memcpy(p+1, buff, 6);
	p += len;
	len_all += len;
	//
	strcpy(buff, "\x01\x07");
	if(charge_flag == 1)
		buff[2] = 0xF0;
	else if(charge_flag == 2)
		buff[2] = 0xF1;
	else
		buff[2] = my_charge.batt_percent;
	len = 3+1;
	*p = len;
	memcpy(p+1, buff, 3);
	p += len;
	len_all += len;
	
	my_ble_uarts_data_send(send_data, &len_all);
	
/*	switch(type)
	{
		case 1://������
			send_data[2] = 0x01;
			buff = MANUFACTURER_NAME;
			send_data[0] = 3 + sizeof(buff);
			for(i = 0;i < sizeof(buff);i++)
				send_data[i+0x03] = buff[i];
			send_len = sizeof(send_data);
			my_ble_uarts_data_send(send_data, &send_len);
			break;
		
		case 2://��Ʒ�ͺ�
			send_data[2] = 0x01;
			buff = MODEL_NAME;
			send_data[0] = 3 + sizeof(buff);
			for(i = 0;i < sizeof(buff);i++)
				send_data[i+0x03] = buff[i];
			send_len = sizeof(send_data);
			my_ble_uarts_data_send(send_data, &send_len);
			break;
		
		case 3://MAC��ַ
			send_data[0] = 9;
			send_data[2] = 0x03;
			memcpy(send_data+3,my_ble.mac_addr,6);		
			send_len = sizeof(send_data);
			my_ble_uarts_data_send(send_data, &send_len);
			break;
		
		case 4://Ӳ���汾
			send_data[0] = 10;
			send_data[2] = 0x04;
			send_data[3] = HW_VER_1 + 0x30;
			send_data[4] = '.';
			send_data[5] = HW_VER_2 + 0x30;
			send_data[6] = '.';
			send_data[7] = HW_VER_3/10 + 0x30;
			send_data[8] = HW_VER_3%10 + 0x30;
			send_data[9] = 0;
			send_len = sizeof(send_data);
			my_ble_uarts_data_send(send_data, &send_len);
			break;
		
		case 5://�̼��汾
			send_data[0] = 10;
			send_data[2] = 0x05;
			send_data[3] = FW_VER_1 + 0x30;
			send_data[4] = '.';
			send_data[5] = FW_VER_2 + 0x30;
			send_data[6] = '.';
			send_data[7] = FW_VER_3/10 + 0x30;
			send_data[8] = FW_VER_3%10 + 0x30;
			send_data[9] = 0;
			send_len = sizeof(send_data);
			my_ble_uarts_data_send(send_data, &send_len);
			break;
		
		case 6://�豸ʱ��
			send_data[0] = 0x07;
			send_data[1] = 0x01;
			send_data[2] = 0x06;
			*((uint32_t *)&send_data[3]) = rtc_sec;
			send_len = sizeof(send_data);
			my_ble_uarts_data_send(send_data, &send_len);
			break;
		
		case 7://�豸����
			send_data[0] = 0x04;
			send_data[1] = 0x01;
			send_data[2] = 0x07;
			if(charge_flag == CHARGING)
				send_data[3] = 0xF0;
			else if(charge_flag == CHARGE_FULL)
				send_data[3] = 0xF1;
			else
				send_data[3] = my_charge.batt_percent;
			send_len = sizeof(send_data);
			my_ble_uarts_data_send(send_data, &send_len);
			break;
			
		default:
			break;
	}*/
}
#endif // NRF_MODULE_ENABLED(BLE_DIS)



















