#ifndef __MY_FDS__
#define __MY_FDS__
//����FDSͷ�ļ�
#include "fds.h"
#include "common.h"

#define MAX_HIS_CNT                    240
//�����ļ�ID�͸��ļ������ļ�¼��KEY
#define DEVICE_FILE                    (0x1000)//�ļ�ID
#define DEVICE_HIS_KEY                 (0x1001)//��¼��ʷ���ݵĸ���
#define DEVICE_TIME_KEY                (0x1002)//��¼�豸ʱ�䣬�ü�¼��ŵ��ļ�ID=0X1000
#define HIS_DATA_START                 (0x2000)//��ʷ���ݴ洢��ʼλ
//#define DEVICE_DESP_KEY                   (0x1002)//��¼KEY���ü�¼��ŵ��ļ�ID=0X1000

//��¼��ʷ���ݴ洢����������
typedef struct
{
	uint8_t num;
}__attribute__((aligned(4)))his_cnt_t;

//��¼�豸ʱ��
typedef struct
{
	uint32_t     localtime;//����ʱ�䣬
}__attribute__((aligned(4)))devicetime_t;

void wait_for_fds_ready(void);
void read_hiscnt_record(void);
void update_hisdata_record(void);
void read_hisdata_record(uint8_t num);

void read_devicetime_record(void);
void update_devicetime_record(void);

void My_fds_Init(void);

#endif
