extern long taskDelay(long);

typedef struct _OAM_FILETX_PKTHD_S_
{
    long    lFileLen;                          /* �ļ�����             */        
    long    lFileOff;                          /* �ļ�ƫ��             */
    long    lDataLen;                          /* ���ݳ���             */
    long    lDataFcs;                          /* ����У���           */

    char    pData[1458];

}OAM_FILETX_PKTHD_S;

typedef struct _OAM_FILETX_ACKHD_S_
{
    short   sAckCode;
    short   sErrCode;

    long    lFileLen;                         /* �ļ�����             */
    long    lFileOff;                         /* �ļ�ƫ��             */
    long    lDataLen;                         /* ���ݳ���             */
    long    lDataFcs;                         /* ����У���           */

}OAM_FILETX_ACKHD_S;



/*=================================================================================================================== */
/* ��ʼ������ */
/*================================================*/
/*name: funEponOamTransFileAllCallBack            */
/*para: lLpEnd  ָʾ�Ƿ��Ի������                */
/*para: lOpCode  ������                           */ 
/*para: pDatBuf  ����ָ��                         */
/*para: lDatLen  ���ݳ���                         */
/*retu: none                                      */
/*desc: �ļ����ͽ��յ�����OAM֡�ص�����           */
/*================================================*/
void funEponOamTransFileAllCallBack(unsigned short usMsgCode,  unsigned short usPonId, 
                                           unsigned short usOnuId,    unsigned short usDatLen, 
                                           unsigned char *pcDatBuf,   unsigned char* pcSessId)
{

    sys_console_printf("receive ONU pkt opcode = %d, ponid = %d, onuid  = %d datalen = %d, sessid = %d \r\n", 
		                usMsgCode, usPonId, usOnuId, usDatLen,  *(long*)pcSessId);
	{	
		int len = 0;
		sys_console_printf("\r\n");
		for(len = 0;len<usDatLen;len++)
			sys_console_printf("%02x ",pcDatBuf[len]);
		sys_console_printf("\r\n");
	}
    if(usMsgCode == 8)
    {
         OAM_FILETX_ACKHD_S *pHead = (OAM_FILETX_ACKHD_S*)pcDatBuf;
         sys_console_printf("Receive ONU Ack sAckCode = %d, sErrCode = %d, lFileLen = %l,  lFileOff = %l, lDataLen = %l\r\n",
                             pHead->sAckCode,pHead->sErrCode,pHead->lFileLen,pHead->lFileOff,pHead->lDataLen);
    }
    else if (usMsgCode == 7) 
    {
         OAM_FILETX_PKTHD_S *pData = (OAM_FILETX_PKTHD_S*)pcDatBuf;
         sys_console_printf("Receive ONU Data lFileLen = %d,  lFileOff = %d, lDataLen = %d\r\n",
                             pData->lFileLen,pData->lFileOff,pData->lDataLen);
    }
    else
    {
         sys_console_printf("Receive ONU error Packet opcode = %d\r\n",usMsgCode);
    }
   
    VOS_Free(pcDatBuf);
    VOS_Free(pcSessId);
}

/*================================================*/
/*name: funEponOamTransFileRWReqCallBack          */
/*para: pDatBuf  ����ָ��                         */
/*para: lDatLen  ���ݳ���                         */
/*retu: none                                      */
/*desc: �ļ�����д��������ص�����                */
/*================================================*/

void funEponOamTransFileRWReqCallBack(unsigned short usPonId,  unsigned short usOnuId,  unsigned short llid,
                                             unsigned short usDatLen, unsigned char *pcDatBuf,
                                             unsigned char *pcSessId)
{
    funEponOamTransFileAllCallBack(5,
                                   usPonId, usOnuId, usDatLen, pcDatBuf, pcSessId);
}

/*================================================*/
/*name: funEponOamTransFileTRDatCallBack          */
/*para: pDatBuf  ����ָ��                         */
/*para: lDatLen  ���ݳ���                         */
/*retu: none                                      */
/*desc: ���ݴ�������ص�����                      */
/*================================================*/
void funEponOamTransFileTRDatCallBack(unsigned short usPonId,  unsigned short usOnuId,  unsigned short llid,
                                             unsigned short usDatLen, unsigned char *pcDatBuf,
                                             unsigned char *pcSessId)
{
    funEponOamTransFileAllCallBack(7,
                                   usPonId, usOnuId, usDatLen, pcDatBuf, pcSessId);
}

/*================================================*/
/*name: funEponOamTransFileTRAckCallBack          */
/*para: pDatBuf  ����ָ��                         */
/*para: lDatLen  ���ݳ���                         */
/*retu: none                                      */
/*desc: ���ݴ���Ӧ��ص�����                      */
/*================================================*/
void funEponOamTransFileTRAckCallBack(unsigned short usPonId,  unsigned short usOnuId,  unsigned short llid,
                                      unsigned short usDatLen, unsigned char *pcDatBuf, 
                                      unsigned char *pcSessId)
{
    funEponOamTransFileAllCallBack(8,
                                   usPonId, usOnuId, usDatLen, pcDatBuf, pcSessId);
}

/*================================================*/
/*name: funEponOamTransFileInit                   */
/*retu: VOS_OK / VOS_ERROR                        */
/*desc: �ú������ڳ�ʼ���ļ����ͳ���              */
/*================================================*/
void funEponOamTransFileInit(void)
{
    CommOltMsgRvcCallbackInit(5, funEponOamTransFileRWReqCallBack);
    CommOltMsgRvcCallbackInit(7, funEponOamTransFileTRDatCallBack);
    CommOltMsgRvcCallbackInit(8, funEponOamTransFileTRAckCallBack);
}


/*================================================*/
/*name: funOamTransFileLocSendReq                 */
/*para: pCtrItem  ��Ϣ������                      */
/*retu: NONE                                      */
/*desc: ������������, ��Զ˷�������              */
/*================================================*/
void funOamSendReq(char type, char *filename , long sesid)
{
     char pktbuf[256];
     char sesbuf[8] = {0};
	 
     pktbuf[0] =  type;

     strcpy(pktbuf +1, filename);
     *(long*)sesbuf = sesid;
 
     FileRequestTransmit(6, 1, pktbuf, strlen(filename)+1, sesbuf);
}


/*=================================================*/
/*name: funOamTransFileLocSendErrAck               */
/*para: pCtrItem  ��Ϣ������                       */
/*para: pAckHead  Ӧ����ͷ                       */ 
/*retu: NONE                                       */
/*desc: ���ط��ִ�����Զ˷��ʹ���ACK, Ȼ����ֹ����*/
/*=================================================*/
void funOamSenAck(long sesid, long error , long code)
{
     char pktbuf[256];
     char sesbuf[8] = {0};

     OAM_FILETX_ACKHD_S   *pAckHead;

    *(long*)sesbuf = sesid;

    pAckHead = (OAM_FILETX_ACKHD_S*)pktbuf;

    pAckHead->lFileOff = 0;
    pAckHead->lDataLen = 90;
    pAckHead->lDataFcs = 0;
    pAckHead->sErrCode = error;
    pAckHead->lFileLen = 90;
    pAckHead->sAckCode = code;

    FileAckTransmit(6, 1,  pktbuf, sizeof(OAM_FILETX_ACKHD_S), sesbuf);
}

/*=================================================*/
/*name: funOamTransFileLocSendErrAck               */
/*para: pCtrItem  ��Ϣ������                       */
/*para: pAckHead  Ӧ����ͷ                       */ 
/*retu: NONE                                       */
/*desc: ���ط��ִ�����Զ˷��ʹ���ACK, Ȼ����ֹ����*/
/*=================================================*/
void funOamSenDat( long sesid)
{
     OAM_FILETX_PKTHD_S pData;
     char sesbuf[8] = {0};

    *(long*)sesbuf = sesid;


    pData.lFileOff = 0;
    pData.lDataLen = 90;
    pData.lDataFcs = 0;
    pData.lFileLen = 90;

    FileDataTransmit(6, 1,  (char*)&pData, 106, sesbuf);
}


void wfask(void)
{
   funOamSendReq(2, "suxiqing.log", 100);

   taskDelay(100);

   funOamSenDat(100);

   taskDelay(100);


   funOamSenAck(100, 0 , 0x303);

}

void wfdata(void)
{
/*   funOamSendReq(2, "test.log", 100);

   taskDelay(100);*/

   funOamSenDat(100);

   taskDelay(100);


   /*funOamSenAck(100, 0 , 0x303);*/

}


void rftest(void)
{
   funOamSendReq(1, "sunxiqing.log", 100);

   taskDelay(100);


   funOamSenAck(100, 0 , 0x301);


   taskDelay(100);

   funOamSenAck(100, 0 , 0x303);


}
