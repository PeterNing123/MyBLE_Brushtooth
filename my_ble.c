#include "my_ble.h"


NRF_BLE_GATT_DEF(m_gatt);               //��������Ϊm_gatt��GATTģ��ʵ��
NRF_BLE_QWR_DEF(m_qwr);                 //����һ������Ϊm_qwr���Ŷ�д��ʵ��
BLE_ADVERTISING_DEF(m_advertising);     //��������Ϊm_advertising�Ĺ㲥ģ��ʵ��
BLE_UARTS_DEF(m_uarts);                 //��������Ϊm_uarts�Ĵ���͸������ʵ��

static ble_uuid_t m_adv_uuids[] =       //����UUID���飬����ֻ������һ���û��Զ���UUID������ble���ڷ���             
{
	{BLE_UUID_USR1_SERVICE,                 BLE_UUID_TYPE_BLE},//����͸������
};

//���崮��͸������UUID�б�
/*static ble_uuid_t nus_adv_uuids[]          =                                          
{
    {BLE_UUID_UARTS_SERVICE, 				UARTS_SERVICE_UUID_TYPE}
};*/

//�ñ������ڱ������Ӿ������ʼֵ����Ϊ������
uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; 

/*************************************���غ���*************************************/
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context);//BLE�¼�������


uint32_t my_ble_uarts_data_send(uint8_t  * p_data, uint16_t  * p_length)
{
	return ble_uarts_data_send(&m_uarts, p_data, p_length, m_conn_handle);
}

uint32_t my_ble_uarts_resp_send(uint8_t  * p_data, uint16_t  * p_length)
{
	return ble_uarts_resp_send(&m_uarts, p_data, p_length, m_conn_handle);
}

my_ble_t my_ble = {
	.is_connected = false,
	.is_ble_con = false,
	.is_send_device_info = false,
	.is_send_his_data = false,
	.device_info_s = 0,
	.his_data_s = 0,
};

//BLE�¼�������
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;
    //�ж�BLE�¼����ͣ������¼�����ִ����Ӧ����
    switch (p_ble_evt->header.evt_id)
    {
        //�Ͽ������¼�
		case BLE_GAP_EVT_DISCONNECTED:           
			NRF_LOG_INFO("Disconnected.");//��ӡ��ʾ��Ϣ
			my_ble.is_connected = false;
			LED_BLE_OFF;
            break;
				
        //�����¼�
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.");	
			my_ble.is_connected = true;
            LED_BLE_ON;//����ָʾ��״̬Ϊ����״̬����ָʾ��D2����
				    
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;//�������Ӿ��
			//�����Ӿ��������Ŷ�д��ʵ����������Ŷ�д��ʵ���͸����ӹ�����������
			//���ж�����ӵ�ʱ��ͨ��������ͬ���Ŷ�д��ʵ�����ܷ��㵥�������������
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;
				
        //PHY�����¼�
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
			//��ӦPHY���¹��
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;
				
        //GATT�ͻ��˳�ʱ�¼�
        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Client Timeout.");
		//�Ͽ���ǰ����
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
				
        //GATT��������ʱ�¼�
        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG_DEBUG("GATT Server Timeout.");
				    //�Ͽ���ǰ����
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}



/******************************************DFU��ش���******************************************/
//�ػ�׼����������ڹرչ����У�����1��ļ�����ô˺�����ֱ����������true������������trueʱ����ʾӦ�ó�����׼���ø�λΪDFUģʽ
static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
{
    switch (event)
    {
        case NRF_PWR_MGMT_EVT_PREPARE_DFU:
            NRF_LOG_INFO("Power management wants to reset to DFU mode.");
            // YOUR_JOB: Get ready to reset into DFU mode
            //
            // If you aren't finished with any ongoing tasks, return "false" to
            // signal to the system that reset is impossible at this stage.
            //
            // Here is an example using a variable to delay resetting the device.
            //
            // if (!m_ready_for_reset)
            // {
            //      return false;
            // }
            // else
            //{
            //
            //    // Device ready to enter
            //    uint32_t err_code;
            //    err_code = sd_softdevice_disable();
            //    APP_ERROR_CHECK(err_code);
            //    err_code = app_timer_stop_all();
            //    APP_ERROR_CHECK(err_code);
            //}
            break;

        default:
            // YOUR_JOB: Implement any of the other events available from the power management module:
            //      -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
            //      -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
            //      -NRF_PWR_MGMT_EVT_PREPARE_RESET
            return true;
    }

    NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
    return true;
}

//ע�����ȼ�Ϊ0��Ӧ�ó���رմ������
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);

//SoftDevice״̬������
static void buttonless_dfu_sdh_state_observer(nrf_sdh_state_evt_t state, void * p_context)
{
    if (state == NRF_SDH_EVT_STATE_DISABLED)
    {
	    //����Softdevice�ڸ�λ֮ǰ�Ѿ����ã���֮bootloader����ʱӦ����CRC
        nrf_power_gpregret2_set(BOOTLOADER_DFU_SKIP_CRC);

        //����system off.
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }
}

//ע��SoftDevice״̬�����ߣ�����SoftDevice״̬�ı���߼����ı�ʱ����SoftDevice�¼�
NRF_SDH_STATE_OBSERVER(m_buttonless_dfu_state_obs, 0) =
{
    .handler = buttonless_dfu_sdh_state_observer,
};

//��ȡ�㲥ģʽ������ͳ�ʱʱ��
static void advertising_config_get(ble_adv_modes_config_t * p_config)
{
    memset(p_config, 0, sizeof(ble_adv_modes_config_t));

    p_config->ble_adv_fast_enabled  = true;
    p_config->ble_adv_fast_interval = APP_ADV_INTERVAL;
    p_config->ble_adv_fast_timeout  = APP_ADV_DURATION;
}
//�Ͽ���ǰ���ӣ��豸׼������bootloader֮ǰ����Ҫ�ȶϿ�����
static void disconnect(uint16_t conn_handle, void * p_context)
{
    UNUSED_PARAMETER(p_context);
    //�Ͽ���ǰ����
    ret_code_t err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Failed to disconnect connection. Connection handle: %d Error: %d", conn_handle, err_code);
    }
    else
    {
        NRF_LOG_DEBUG("Disconnected connection handle %d", conn_handle);
    }
}

//DFU�¼��������������Ҫ��DFU�¼���ִ�в�������������Ӧ���¼�������봦�����
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        //���¼�ָʾ�豸����׼������bootloader
	    case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
        {
            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
            //��ֹ�豸�ڶϿ�����ʱ�㲥
            ble_adv_modes_config_t config;
            advertising_config_get(&config);			  
            config.ble_adv_on_disconnect_disabled = true;//���ӶϿ����豸���Զ����й㲥					  
            ble_advertising_modes_config_set(&m_advertising, &config);//�޸Ĺ㲥����
			
		    //�Ͽ���ǰ�Ѿ����ӵ������������豸�����豸�̼����³ɹ�������ֹ������Ҫ������ʱ���շ������ָʾ
            uint32_t conn_count = ble_conn_state_for_each_connected(disconnect, NULL);
            NRF_LOG_INFO("Disconnected %d links.", conn_count);
            break;
        }
        //���¼�ָʾ�������غ��豸������bootloader
        case BLE_DFU_EVT_BOOTLOADER_ENTER:
			//���Ӧ�ó�����������Ҫ���浽Flash��ͨ��app_shutdown_handler����flase���ӳٸ�λ���Ӷ���֤������ȷд�뵽Flash
		    LED1_ON;
			LED2_ON;
		    LED3_ON;
			LED4_ON;
			LED5_ON;
            NRF_LOG_INFO("Device will enter bootloader mode.");
            break;
        //���¼�ָʾ����bootloaderʧ��
        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
		    //����bootloaderʧ�ܣ�Ӧ�ó�����Ҫ��ȡ������ʩ����������
            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            break;
        //���¼�ָʾ������Ӧʧ��
        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            NRF_LOG_ERROR("Request to send a response to client failed.");
            //������Ӧʧ�ܣ�Ӧ�ó�����Ҫ��ȡ������ʩ����������
            APP_ERROR_CHECK(false);
            break;

        default:
            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}



const unsigned char Num2CharTable[] = "0123456789ABCDEF";
//ʮ��������ת��Ϊ�ַ���
void HexArrayToString(unsigned char *hexarray,int length,unsigned char *string)
{
    int i = length-1;
	while(i >= 0)
	{
	    *(string++) = Num2CharTable[((hexarray[i] >> 4) & 0x0f)];
		*(string++) = Num2CharTable[(hexarray[i] & 0x0f)];
		i--;
	}
	*string = 0x0;
}


/******************************************��ʼ����ش���******************************************/
//��ʼ��BLEЭ��ջ
static void ble_stack_init(void)
{
    ret_code_t err_code;
    //����ʹ��SoftDevice���ú����л����sdk_config.h�ļ��е�Ƶʱ�ӵ����������õ�Ƶʱ��
	//�ڲ�RCʱ�ӣ�У׼���16������У׼���2������20ppm
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);
    
    //���屣��Ӧ�ó���RAM��ʼ��ַ�ı���
    uint32_t ram_start = 0;
	  //ʹ��sdk_config.h�ļ���Ĭ�ϲ�������Э��ջ����ȡӦ�ó���RAM��ʼ��ַ�����浽����ram_start
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    //ʹ��BLEЭ��ջ
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    //ע��BLE�¼������߻ص�����
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


//GAP������ʼ�����ú���������Ҫ��GAP�����������豸���ƣ������������ѡ���Ӳ���
static void gap_params_init(void)
{
    ret_code_t              err_code;	  
    ble_gap_conn_params_t   gap_conn_params;//�������Ӳ����ṹ�����
    ble_gap_conn_sec_mode_t sec_mode;
	ble_gap_addr_t          my_addr;        //��ַ�ṹ�����,���������ȡ���豸��ַ������
	
	char name[BLE_GAP_DEVNAME_MAX_LEN];      //�����豸��
	err_code = sd_ble_gap_addr_get(&my_addr);//��ȡ�豸��ַ
	memset(name,0,sizeof(name));             //���� 
	strcpy(name,DEVICE_NAME);
	unsigned char str[20];
	HexArrayToString(my_addr.addr,6,str);
	strcat(name,(char *)str);
	
	for(uint8_t i=0;i<6;i++)
	{
		my_ble.mac_addr[i] = my_addr.addr[5-i];
	}
	if(err_code == NRF_SUCCESS) //��ȡ���̵�ַ�ɹ���log�����ַ
	{
		NRF_LOG_INFO("Address Type:%02X\r\n", my_addr.addr_type);      //��ӡ��ַ����
		NRF_LOG_INFO("Address:%02X:%02X:%02X:%02X:%02X:%02X\r\n", //��ӡ��ַ
										my_ble.mac_addr[0],my_ble.mac_addr[1],my_ble.mac_addr[2],
		                                my_ble.mac_addr[3],my_ble.mac_addr[4],my_ble.mac_addr[5]);
	}	
	
	my_addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC; //BLE_GAP_ADDR_TYPE_PUBLIC BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
	err_code =  sd_ble_gap_addr_set(&my_addr);
	APP_ERROR_CHECK(err_code);
	
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&sec_mode);//����GAP�İ�ȫģʽ���޼���
    //����GAP�豸����
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)name,
                                          strlen(name));
    //��麯�����صĴ������
	APP_ERROR_CHECK(err_code);
																				
    //���������384
	err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_REMOTE_CONTROL);
	APP_ERROR_CHECK(err_code); 
																					
    //������ѡ���Ӳ���������ǰ������gap_conn_params
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;//��С���Ӽ��
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;//��С���Ӽ��
    gap_conn_params.slave_latency     = SLAVE_LATENCY;    //�ӻ��ӳ�
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT; //�ල��ʱ
    //����Э��ջAPI sd_ble_gap_ppcp_set����GAP����
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
										 
	//err_code = sd_ble_gap_tx_power_set(TX_POWER_LEVEL);
   // APP_ERROR_CHECK(err_code);
																					
}
static uint16_t   m_ble_uarts_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3;  
//GATT�¼����������ú����д���MTU�����¼�
void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    //�����MTU�����¼�
	  if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        //���ô���͸���������Ч���ݳ��ȣ�MTU-opcode-handle��
			  m_ble_uarts_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_uarts_max_data_len, m_ble_uarts_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}

//��ʼ��GATT����ģ��
static void gatt_init(void)
{
	//��ʼ��GATT����ģ��
	ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
	//��麯�����صĴ������
	APP_ERROR_CHECK(err_code);
	//����ATT MTU�Ĵ�С���������õ�ֵΪ247
	err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
	APP_ERROR_CHECK(err_code);
}

//�㲥�¼�������
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    //ret_code_t err_code;
    //�жϹ㲥�¼�����
    switch (ble_adv_evt)
    {
        //���ٹ㲥�����¼������ٹ㲥�������������¼�
		case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");		
            //LED1_ON;    //���ù㲥ָʾ��Ϊ���ڹ㲥��D1ָʾ����˸��
            break;
        //�㲥IDLE�¼����㲥��ʱ���������¼�
        case BLE_ADV_EVT_IDLE:	  
            break;

        default:
            break;
    }
}

//�㲥��ʼ��
static void advertising_init(void)
{
    ret_code_t             err_code;
	
    ble_advertising_init_t init;       //����㲥��ʼ�����ýṹ�����   
    memset(&init, 0, sizeof(init));    //����֮ǰ������  
    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;//�豸�������ͣ�ȫ��	  
    //init.advdata.include_appearance      = false;                 //�Ƿ������ۣ�����
	//init.advdata.include_ble_device_addr = true;				 //�Ƿ�����豸��ַ������
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;//Flag:һ��ɷ���ģʽ����֧��BR/EDR 
	
	init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);//�㲥�����а����ķ���UUID
	init.advdata.uuids_complete.p_uuids  = m_adv_uuids; //ָ�����UUID�������
	
	//ble����UUID�ŵ�ɨ����Ӧ����
	//init.srdata.uuids_complete.uuid_cnt = sizeof(nus_adv_uuids) / sizeof(nus_adv_uuids[0]);
   // init.srdata.uuids_complete.p_uuids  = nus_adv_uuids;
	
    init.config.ble_adv_fast_enabled  = true;//���ù㲥ģʽΪ���ٹ㲥
	//���ù㲥����͹㲥����ʱ��
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;//0������ʱ
    
    init.evt_handler = on_adv_evt;						   //�㲥�¼��ص�����
    
    err_code = ble_advertising_init(&m_advertising, &init);//��ʼ���㲥
    APP_ERROR_CHECK(err_code);
    //�����������ñ�־����ǰSoftDevice�汾�У�S132 V6.1�汾����ֻ��д1��
    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


//�Ŷ�д���¼������������ڴ����Ŷ�д��ģ��Ĵ���
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    //���������
	  APP_ERROR_HANDLER(nrf_error);
}

//�����ʼ����������ʼ���Ŷ�д��ģ��ͳ�ʼ��Ӧ�ó���ʹ�õķ���,DFU
static void services_init(void)
{
    ret_code_t         err_code;

	ble_uarts_init_t     uarts_init;          	   //���崮��͸����ʼ���ṹ��
	ble_dfu_buttonless_init_t dfus_init = {0};     //����DFU�����ʼ���ṹ��
    nrf_ble_qwr_init_t qwr_init = {0};			   //�����Ŷ�д���ʼ���ṹ�����
    qwr_init.error_handler = nrf_qwr_error_handler;//�Ŷ�д���¼�������    
	
    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);//��ʼ���Ŷ�д��ģ��
    APP_ERROR_CHECK(err_code);

/*******************������ӷ���Ĵ���*******************/
	//��ʼ��DFU����
	dfus_init.evt_handler = ble_dfu_evt_handler;
    err_code = ble_dfu_buttonless_init(&dfus_init);
    APP_ERROR_CHECK(err_code);
		
/*------------------���´����ʼ������͸������-------------*/
	//���㴮��͸�������ʼ���ṹ��
	memset(&uarts_init, 0, sizeof(uarts_init));	
    uarts_init.data_handler = uarts_data_handler;      //���ô���͸���¼��ص�����
    err_code = ble_uarts_init(&m_uarts, &uarts_init);  //��ʼ������͸������
    APP_ERROR_CHECK(err_code);
/*------------------��ʼ������͸������-END-----------------*/
}

//���Ӳ���Э��ģ���¼�������
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;
    //�ж��¼����ͣ������¼�����ִ�ж���
	//���Ӳ���Э��ʧ��
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
		//LED3_OFF;
    }
	//���Ӳ���Э�̳ɹ�
	if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
    {
        //���ܴ���;
		//LED3_ON;//Э�̳ɹ���LED3��
		//Э�̳ɹ��󣬶�ȡ��Χ�豸��ѡ���Ӳ���
		//�������������ȡ����Χ�豸��ѡ���Ӳ����ı���
		ble_gap_conn_params_t   my_gap_conn_params;
		//����Э��ջAPI sd_ble_gap_ppcp_set����GAP����
        err_code = sd_ble_gap_ppcp_get(&my_gap_conn_params);
		//��ȡ�ɹ��󣬴�ӡ����ȡ����Χ�豸��ѡ���Ӳ���
		if(err_code == NRF_SUCCESS)
		{
			 NRF_LOG_INFO("min_conn_interval = %d",my_gap_conn_params.min_conn_interval);
			 NRF_LOG_INFO("max_conn_interval = %d",my_gap_conn_params.max_conn_interval);
			 NRF_LOG_INFO("slave_latency = %d",my_gap_conn_params.slave_latency);
			 NRF_LOG_INFO("conn_sup_timeout = %d",my_gap_conn_params.conn_sup_timeout);
		}
    }
}

//���Ӳ���Э��ģ��������¼�������nrf_error�����˴�����룬ͨ��nrf_error���Է���������Ϣ
static void conn_params_error_handler(uint32_t nrf_error)
{
    //���������
	  APP_ERROR_HANDLER(nrf_error);
}

//���Ӳ���Э��ģ���ʼ��
static void conn_params_init(void)
{
    ret_code_t             err_code;
	  
    ble_conn_params_init_t cp_init;      //�������Ӳ���Э��ģ���ʼ���ṹ��
    memset(&cp_init, 0, sizeof(cp_init));//����֮ǰ������
    
    cp_init.p_conn_params                  = NULL;//����ΪNULL����������ȡ���Ӳ���
	//���ӻ�����֪ͨ���״η������Ӳ�����������֮���ʱ������Ϊ5��
	cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
	//ÿ�ε���sd_ble_gap_conn_param_update()�����������Ӳ������������֮��ļ��ʱ������Ϊ��30��
	cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
	//�������Ӳ���Э��ǰ�������Ӳ���Э�̵�����������Ϊ��3��
	cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
	//���Ӳ������´������¼���ʼ��ʱ
	cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
	//���Ӳ�������ʧ�ܲ��Ͽ�����
	cp_init.disconnect_on_fail             = false;
	//ע�����Ӳ��������¼����
	cp_init.evt_handler                    = on_conn_params_evt;
	//ע�����Ӳ������´����¼����
	cp_init.error_handler                  = conn_params_error_handler;
	//���ÿ⺯���������Ӳ������³�ʼ���ṹ��Ϊ�����������ʼ�����Ӳ���Э��ģ��
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


//�����㲥���ú������õ�ģʽ����͹㲥��ʼ�������õĹ㲥ģʽһ��
void advertising_start(void)
{
	//ʹ�ù㲥��ʼ�������õĹ㲥ģʽ�����㲥
	ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
	//��麯�����صĴ������
   APP_ERROR_CHECK(err_code);
}

void My_ble_Init(void)
{	
	ble_stack_init();   //��ʼ��Э��ջ	
	gap_params_init();  //����GAP����	
	gatt_init();        //��ʼ��GATT	
	advertising_init(); //��ʼ���㲥	
	services_init();    //��ʼ������	
	conn_params_init(); //���Ӳ���Э�̳�ʼ��
}

