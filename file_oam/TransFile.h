/* ================================================ */
/* FileName: TransFile.h                            */
/* Author  : suxiqing                               */
/* Date    : 2006-10-10                             */
/* Description: ���ļ���ΪEPON���, ʵ��ͨ��OAMͨ�� */
/*              ��ONU���ͻ�����ļ�                 */
/* ================================================ */

/*=================================================================================================================== */
/*=================================================================================================================== */
/*
#define OAM_FILETX_MODULE_ID         0x8750*/       /* ģ��ID           */
                                                 /* ��Ҫ��Ʒͳһ����ģ��ID */

#define OAM_FILETX_WAITTIME          900          /* ���͵ȴ�900��    */ 

#define OAM_FILETX_RETRY_COU         6           /* �ط�����3         */
#define OAM_FILETX_RETRY_INT         6        /* �ط����3��     994967290 */
#define OAM_FILETX_RETRY_NO          0           /* ���ط�           */

#define OAM_FILETX_RATETIME          (OAM_FILETX_RETRY_INT * OAM_FILETX_RETRY_COU + 2)
/*=================================================================================================================== */

/* ���ͺ������÷�ʽ    */
#define OAM_FILETX_CALL_NORMAL       1           /* ���÷�������ʹ��ƽ������ ���ú���������    */
#define OAM_FILETX_CALL_BLOCK        2           /* ���÷�������ʹ���������� ���ú�ȴ����귵��*/

/* ��д��������        */

#define OAM_FILETX_REQ_READ          1           /* ���������������� ���� ��                 */
#define OAM_FILETX_REQ_WRITE         2           /* ���������������� ���� д                 */

/*=================================================================================================================== */

                                                 /* ִ�������㷢��Ķ�/д������Ļص�����,����ִ֪ͨ�н�� */
                                                 /* lReqType ��������, pFileName �ļ���, lErrCode �������*/
                                                 /* lFileLen �ļ����ȣ�lSucLen ��ǰ����*/
                                                 /* �� lFileLen = 0(ʧ�ܵĴ���) �� lFileLen > 0 �� lSucLen = lFileLen ʱ, ��ʾ�ļ�������� ���һ�λص�*/
typedef void (*POAMTRANSFILECALLBACK)(long  lReqType,  unsigned short usPonID, unsigned short usOnuID,
                                      char *pFileName, void *pArgVoid, long lFileLen, long lSucLen, long lErrCode); 
                                                 /* ִ�д�ONU��ȡ�ļ�����ʱ, ����ȷ�������ļ���, ִ�еĻص����� */
												 /* pFileType �ļ�����, pFileName �ļ���, pFileBuf �ļ��Ľ��ջ��� lFileLen �ļ�����*/
												 /* pFileBuf �����������ͷ�, �ͷź��� free() */ 
typedef void (*POAMRECVFILECALLBACK)(unsigned short usPonID, unsigned short usOnuID, 
                                     char *pFileName, char *pFileBuf, long lFileLen);   
                                                 
/*=================================================================================================================== */
/*=================================================================================================================== */
/*API ����   */
/*================================================*/
/*name: funOamTransFileLocReqApi                  */
/*para: lReqType  ָʾ�Ƿ�Ϊ��������1 read/2 write*/ 
/*para: lOnuID    ONU ��                          */ 
/*para: lFileType ��Ҫ���͵��ļ�����              */
/*para: lCallType �����ִ�з�ʽ 1 normal 2block  */
/*                1 �����˳� 2 ���� ����ִ�����˳�*/
/*para: pReqCallback ����ִ�н���ص�����         */
/*para: pFileCallBack �����ļ��ص�����            */
/*retu: ERROR CODE (0 no error >0 have error)     */
/*desc: ���øú�����ָ��ONU����ָ�������ļ�       */
/*================================================*/
   
long funOamTransFileLocReqApi(long  lReqType,   unsigned short usPonID, unsigned short usOnuID, 
                              char *pcFileType, char *pcFileName, long lCallType, void *pArgVoid,
                              POAMTRANSFILECALLBACK pReqCallback, POAMRECVFILECALLBACK pFileCallBack);
 
/*================================================*/
/*name: funOamTransFileGetErrStrApi               */
/*para: lErrorNo  �����                          */ 
/*retu: error inf string                          */
/*desc: ���øú�����ȡ�����ļ��Ĵ�����Ϣ��        */
/*================================================*/
char *funOamTransFileGetErrStrApi(long lErrorNo);
 

/*================================================*/
/*name: funEponOamTransFileInit                   */
/*retu: VOS_OK / VOS_ERROR                        */
/*desc: �ú������ڳ�ʼ���ļ����ͳ���              */
/*================================================*/
long funOamTransFileInit(void);
/*=====================================================================================================================*/
/*=====================================================================================================================*/

