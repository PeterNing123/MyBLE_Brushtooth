#include "my_fds.h"
#include "my_ble.h"
#include "my_ble_uarts.h"

uint8_t cur_his_num = 0;
//��¼��ʷ���ݴ洢����������
his_cnt_t his_cnt =
{
	.num = 0,
};

devicetime_t devicetime = //��ʼ��
{
	.localtime = 620650965  //2019��1��11��
};

his_data_t his_data = 
{
	.end_sec = 0,
	.mode = 1,
	.dur_time = 0,
	.run_time = 120,
};
//�����豸������Ϣ�ļ�¼
fds_record_t const m_hiscnt_record =
{
    .file_id           = DEVICE_FILE,        //�ļ�id
    .key               = DEVICE_HIS_KEY,     //�ļ�key
    .data.p_data       = &his_cnt,
    //��¼�ĳ��ȱ�����4�ֽڣ��֣�Ϊ��λ
    .data.length_words = (sizeof(his_cnt) + 3) / sizeof(uint32_t),
};

//�����豸ʱ��ļ�¼
fds_record_t const m_devicetime_record =
{
    .file_id           = DEVICE_FILE,        //�ļ�id
    .key               = DEVICE_TIME_KEY,    //�ļ�key
    .data.p_data       = &devicetime,
    //��¼�ĳ��ȱ�����4�ֽڣ��֣�Ϊ��λ
    .data.length_words = (sizeof(devicetime) + 3) / sizeof(uint32_t),
};

//����FDS�첽������־�ṹ��
my_fds_info_t my_fds_info;

//FDS�¼�������
static void fds_evt_handler(fds_evt_t const * p_evt)
{
    //�ж��¼�����
    switch (p_evt->id)
    {
        case FDS_EVT_INIT://FDS��ʼ���¼�
					  //��ʼ���ɹ�
            if (p_evt->result == FDS_SUCCESS)
            {
                my_fds_info.busy = false;
            }
            break;
        //FDSд��¼�¼�
        case FDS_EVT_WRITE:
        {
            //д��¼�ɹ�
		    if (p_evt->result == FDS_SUCCESS)
            {
                my_fds_info.busy = false;
            }
        } break;
		//FDS���¼�¼�¼�
        case FDS_EVT_UPDATE:
        {
            //д��¼�ɹ�
		    if (p_evt->result == FDS_SUCCESS)
            {
                my_fds_info.busy = false;
            }
        } break;
		//FDS��Ƭ�����¼�
        case FDS_EVT_GC:
        {
            //��Ƭ����ɹ�
		    if (p_evt->result == FDS_SUCCESS)
            {
				  my_fds_info.busy = false;
            }
        } break;
        default:
            break;
    }
}

//�ȴ�FDS��ʼ�����
void wait_for_fds_ready(void)
{
    while (my_fds_info.busy)
    {
        (void) sd_app_evt_wait();
    }
}
void read_hiscnt_record(void)
{
	ret_code_t rc;
	
	fds_record_desc_t desc = {0};     //���岢��ʼ����¼�������ṹ�����
	fds_find_token_t  tok  = {0};     //���岢��ʼ����¼�������ƽṹ�����
   
	memset(&tok, 0x00, sizeof(fds_find_token_t)); //����tok����ͷ����		
	rc = fds_record_find(DEVICE_FILE, DEVICE_HIS_KEY, &desc, &tok);//��DEVICE_FILE�ļ��в��Ҽ�¼m_version_record
	//���ҵ���¼�󣬶�ȡ��¼����
	if (rc == FDS_SUCCESS)
	{
		fds_flash_record_t temp = {0};				
		rc = fds_record_open(&desc, &temp);//�򿪼�¼��ȡ��¼����
		APP_ERROR_CHECK(rc);
		static his_cnt_t temp_his_cnt;				
		memcpy(&temp_his_cnt, temp.p_data, sizeof(his_cnt_t));//������¼����
		cur_his_num = temp_his_cnt.num;
		//���ڴ�ӡ��¼����
		NRF_LOG_INFO("Current hisdata num is: %d  ", temp_his_cnt.num);		
		rc = fds_record_close(&desc);  //��ȡ�󣬹رռ�¼
		APP_ERROR_CHECK(rc);
	}
}
uint8_t send_his_data[16] = {0};
void read_hisdata_record(uint8_t num)
{
	ret_code_t rc;
	
	fds_record_desc_t desc = {0};     //���岢��ʼ����¼�������ṹ�����
	fds_find_token_t  tok  = {0};     //���岢��ʼ����¼�������ƽṹ�����
   
	memset(&tok, 0x00, sizeof(fds_find_token_t)); //����tok����ͷ����		
	rc = fds_record_find(DEVICE_FILE, HIS_DATA_START+num, &desc, &tok);//��DEVICE_FILE�ļ��в��Ҽ�¼m_version_record
	if (rc == FDS_SUCCESS)
	{
		fds_flash_record_t temp = {0};				
		rc = fds_record_open(&desc, &temp);//�򿪼�¼��ȡ��¼����
		APP_ERROR_CHECK(rc);
		static his_data_t temp_his_data;				
		memcpy(&temp_his_data, temp.p_data, sizeof(his_data_t));//������¼����
		his_data.end_sec = temp_his_data.end_sec;
		his_data.mode = temp_his_data.mode;
		his_data.dur_time = temp_his_data.dur_time;
		his_data.run_time = temp_his_data.run_time;
		//���ڴ�ӡ��¼����
		NRF_LOG_INFO("Current hisdata is:%d %d %d %d ", his_data.end_sec, his_data.mode, his_data.dur_time, his_data.run_time);		
		rc = fds_record_close(&desc);  //��ȡ�󣬹رռ�¼
		APP_ERROR_CHECK(rc);
		
		//ble����͸���ϱ���ʷ����	
		*((uint32_t *)&send_his_data[0]) = his_data.end_sec;
		send_his_data[4] = his_data.mode;
		*((uint16_t *)&send_his_data[5]) = his_data.dur_time;
		*((uint16_t *)&send_his_data[7]) = his_data.run_time;
		send_his_data[12] = 0x34;
		send_his_data[13] = 0x56;
		send_his_data[14] = 0x78;
		send_his_data[15] = 0x90;
		//uint16_t send_his_len = sizeof(send_his_data);
		//my_ble_uarts_data_send(send_his_data, &send_his_len);
	}
}

//������ʷ����
void update_hisdata_record(void)
{
	ret_code_t rc;
	
	fds_record_desc_t desc = {0};  //���岢��ʼ����¼�������ṹ�����
	fds_find_token_t  tok  = {0};  //���岢��ʼ����¼�������ƽṹ�����
	
	cur_his_num++;  //��ʷ���ݼ����ۼ�
	if(cur_his_num > 240)
		cur_his_num = 1;
		
	//������ʷ���ݼ�¼
	if((my_fds_info.hisdata_record_update == true) && (my_fds_info.busy == false))
	{
		//����tok����ͷ����
		memset(&tok, 0x00, sizeof(fds_find_token_t));
		my_fds_info.busy = true;
		//��DEVICE_FILE�ļ��в��Ҽ�¼m_version_record
		rc = fds_record_find(DEVICE_FILE, DEVICE_HIS_KEY, &desc, &tok);	
		if (rc == FDS_SUCCESS)
		{			 
			 //����������
			 his_cnt.num = cur_his_num;
			 rc = fds_record_update(&desc, &m_hiscnt_record);
			 APP_ERROR_CHECK(rc);
			 wait_for_fds_ready();
		}
		
		his_data.end_sec = rtc_sec;
		his_data.mode = my_motor.mode;
		his_data.dur_time = my_motor.dur_time;
		his_data.run_time = my_motor.run_time;
		
		//������ʷ���ݵļ�¼
		fds_record_t const m_hisdata_record =
		{
			.file_id           = DEVICE_FILE,                    //�ļ�id
			.key               = HIS_DATA_START+cur_his_num,     //�ļ�key
			.data.p_data       = &his_data,
			//��¼�ĳ��ȱ�����4�ֽڣ��֣�Ϊ��λ
			.data.length_words = (sizeof(his_data) + 3) / sizeof(uint32_t),
		};
		//����tok����ͷ����
		memset(&tok, 0x00, sizeof(fds_find_token_t));
		my_fds_info.busy = true;
		rc = fds_record_find(DEVICE_FILE, HIS_DATA_START+cur_his_num, &desc, &tok);	
		if (rc == FDS_SUCCESS)//����
		{	
			rc = fds_record_update(&desc, &m_hisdata_record);
			APP_ERROR_CHECK(rc);
			wait_for_fds_ready();
			NRF_LOG_INFO("Update hisdata!");
		}
		else //û���ҵ���д���¼
		{
			my_fds_info.busy = true;
			rc = fds_record_write(&desc, &m_hisdata_record);
			APP_ERROR_CHECK(rc);
			wait_for_fds_ready();
			NRF_LOG_INFO("Write hisdata!");
		}
		my_fds_info.hisdata_record_update = false;
	}
}

void read_devicetime_record(void)
{
	ret_code_t rc;
	fds_record_desc_t desc = {0};
	fds_find_token_t  tok  = {0};

	memset(&tok, 0x00, sizeof(fds_find_token_t));
	rc = fds_record_find(DEVICE_FILE, DEVICE_TIME_KEY, &desc, &tok);
	if (rc == FDS_SUCCESS)
	{
		fds_flash_record_t temp = {0};
		rc = fds_record_open(&desc, &temp);
		APP_ERROR_CHECK(rc);
		static devicetime_t temp_devtime;
		memcpy(&temp_devtime, temp.p_data, sizeof(devicetime_t));
		rtc_sec = temp_devtime.localtime;// + 0x386d4380;
		NRF_LOG_INFO("RTC: date=%d\r\n", rtc_sec);
		rc = fds_record_close(&desc);
		APP_ERROR_CHECK(rc);
	}
}

void update_devicetime_record(void)
{
	ret_code_t rc;
	
	fds_record_desc_t desc = {0};  //���岢��ʼ����¼�������ṹ�����
	fds_find_token_t  tok  = {0};  //���岢��ʼ����¼�������ƽṹ�����
		
	//���¼�¼m_version_record
	if(my_fds_info.busy == false)
	{
		//����tok����ͷ����
		memset(&tok, 0x00, sizeof(fds_find_token_t));
		my_fds_info.busy = true;
		//��DEVICE_FILE�ļ��в��Ҽ�¼m_version_record
		rc = fds_record_find(DEVICE_FILE, DEVICE_TIME_KEY, &desc, &tok);	
		if (rc == FDS_SUCCESS)
		{
			 //my_fds_info.devicetime_record_update = false;
			 //���¼�¼m_version_record
			 devicetime.localtime = rtc_sec;
			 rc = fds_record_update(&desc, &m_devicetime_record);
			 APP_ERROR_CHECK(rc);
			 wait_for_fds_ready();
			 NRF_LOG_INFO("RTC update: date=%d\r\n", rtc_sec);
		}
	}
}

void My_fds_Init()
{
	ret_code_t rc;
	
	fds_record_desc_t desc = {0};  //���岢��ʼ����¼�������ṹ�����
	fds_find_token_t  tok  = {0};  //���岢��ʼ����¼�������ƽṹ�����	
	(void)fds_register(fds_evt_handler); //ע��FDS�¼��ص���������FS�¼�
	
	my_fds_info.busy = true;
	rc = fds_init();       //��ʼ��FDS
	APP_ERROR_CHECK(rc);   //�ô�����ģ���麯������ֵ	
	wait_for_fds_ready();  //FDS��ʼ�����첽�ģ����Ҫ�ȴ�FDS��ʼ�����
	
	memset(&tok, 0x00, sizeof(fds_find_token_t)); //����tok����ͷ����
	
	rc = fds_record_find(DEVICE_FILE, DEVICE_HIS_KEY, &desc, &tok);	 //��DEVICE_FILE�ļ��в��Ҽ�¼m_version_record	
	if (rc != FDS_SUCCESS)    //û�в��ҵ�m_version_record��¼��д���¼
	{
		my_fds_info.busy = true;
		rc = fds_record_write(&desc, &m_hiscnt_record);
		APP_ERROR_CHECK(rc);
		wait_for_fds_ready();
	}
	//��ȡ�豸ʱ��
	memset(&tok, 0x00, sizeof(fds_find_token_t));//����tok����ͷ����	
	rc = fds_record_find(DEVICE_FILE, DEVICE_TIME_KEY, &desc, &tok);//��DEVICE_FILE�ļ��в��Ҽ�¼m_desp_record								
	//û�в��ҵ�m_desp_record��¼��д���¼
	if (rc != FDS_SUCCESS)
	{
		my_fds_info.busy = true;
		rc = fds_record_write(&desc, &m_devicetime_record);
		APP_ERROR_CHECK(rc);
		wait_for_fds_ready();
	}
}
