
#ifndef    ONUOAMUPD_H
#define    ONUOAMUPD_H


/*  �ļ���FLASH�е�ͷ�ṹ
char *onu_type : onu ���ͣ��ַ���,16�ֽ�
int  offset : ONU�����ļ���FLASH����ƫ��
int  file_len: ONU�����ļ���FLASH���ڳ���
int  compress_flag : �ļ�ѹ����־��0����ʾ��ѹ����0x000000ff ����ʾѹ��
int *reserve:  ������8�ֽ�
int *version: �汾�ţ��ַ�����92�ֽ�
*/


#define MSGTYPE_UPDATE_ONE		0
#define MSGTYPE_UPDATE_ALL		1
#define MSGTYPE_UPDATE_OVER		2
#define MSGTYPE_UPDATE_FAIL		3
#define MSGTYPE_UPDATE_SAVE_OK	4
#define MSGTYPE_UPDATE_TIMER		5
#define MSGTYPE_UPDATEGPON_ONE  6
#define MSGTYPE_UPDATEGPON_OVER  7
#define MSGTYPE_UPDATEGPONWAITING_ACTIVE  8

#define IMAGE_TYPE_ALL   0
#define IMAGE_TYPE_NONE  0xa5  
#define IMAGE_TYPE_APP  3     /* ONUӦ�ó���*/
#define IMAGE_TYPE_VOIP 1    /* ONU���VOIP����*/
#define IMAGE_TYPE_FPGA 2   /* ONU��FPGA����*/

#define IMAGE_UPDATE_MODE_INVALID	0
#define IMAGE_UPDATE_MODE_GW		1
#define IMAGE_UPDATE_MODE_CTC	2
#define IMAGE_UPDATE_MODE_UNKNOWN	3


#define OAM_UPD_MSG_CODE_CDP_RESULT_DISP	1
#define OAM_UPD_MSG_CODE_CDP_STATUS_REP	2

typedef struct {
	ULONG onuDevIdx;
	UCHAR mode;
	UCHAR act;
	USHORT file_type;
	USHORT status;
	USHORT result;
	UCHAR data[128];
} __attribute__((packed)) onu_oam_upd_msg_t;


extern ULONG  onuOamUpdQueId;

extern LONG sendOnuOamUpdMsg( int update_mode, ULONG act, ULONG para, ULONG fileType );
extern STATUS onuOamUpdateInit( void );

extern char *onuOamUpdFileType2Str(int nTypeBit);
extern USHORT *getOnuOamUpdWaitingOnuList(ULONG slot, ULONG pon);

extern CHAR *onu_oam_upg_file_type_2_str( ULONG fileType);
extern LONG onuOamUpdPrintf( ULONG slot, ULONG port, ULONG onu, CHAR *str );


#endif
