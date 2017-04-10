;;;����ڵĹ��ܺ��п��ܾ������̨�������ݵĶ˿�




//  for later running in practise only need rename 'port_send_begin_no_monitor' as 'port_send_begin'
;;;�ѵ���������ǵ��˵��Ժ�ʵ�������������������
void   port_send_begin_no_monitor(BYTE the_port_no)
{
  struct WR_REG_16554 *m_wr_554;         ;;;�ṹ��ָ�� 

  m_wr_554 = (struct WR_REG_16554 *)Register_554[the_port_no];
  {
      m_wr_554->MCR = 0x0a;         ;;;λ3����λʱ,�ⲿ����ͨ���жϱ�ʹ��,����Щ�ǿ������������ƽ��
  }
  if(the_port_no==10)  m_wr_554->MCR=0x0b;         ;;;Ϊ�˻����ȷ�����뼫��
  port_send_pt[the_port_no] = 1; 
  m_wr_554->THR = port_send[the_port_no][0];        //-���������ּĴ���,���Ӧ�þ������շ�����

}










//-һ����ʼ����һ֡��Ϣ,�Ͳ������ж���
void port_send_begin()
{
    BYTE the_port_no;
    WORD the_ram_ax;
    WORD the_ram_bx;
    
    disable();            ;;;���ж�
    port_send_begin_no_monitor(port_no);       ;;;�ڷ��Ϳ�ʼ�����,port_no��ʾ�����ĸ��˿�

////// for monitor port msg:  
    if((port_mirror[port_no] & 0x20)!=0)
    {
        the_port_no=port_mirror[port_no] & 0x0f;         ;;;ȡ���ĸ��ڴ��ڷ���״̬�������,������ʱͬʱ���е�
        if(the_port_no<0x0c)             ;;;С��12������16554���д��ڱ䲢�ڻ򲢿ڱ䴮�ڵ�����
        {
            the_ram_bx=port_send_len[port_no];  //-��ʾһ��������Ϣ�����ͼ����ֽڵ�����
            for(the_ram_ax=0;the_ram_ax<the_ram_bx;the_ram_ax++)
            {
                port_send[the_port_no][the_ram_ax]=port_send[port_no][the_ram_ax];   //-�ѵ�Ϊ�������ٶ�Ҫ�����˫������,�Ǻ�,���һ���һ���ռ���
            }
            port_send_len[the_port_no]=port_send_len[port_no]; //-���Ǽ�¼��Ҫ���ͼ����ֽ�
            port_send_begin_no_monitor(the_port_no);
        }
        else
        {
        	if(the_port_no==0x0e)        ;;;14������Ϊ����,���ǹ̶���CPU�˿�
        	{
        		the_ram_bx=port_send_len[port_no];
        		the_ram_ax=0;
        		while(the_ram_bx>0)
        		{
        			set_port_rece_send_buf[set_port_rs_buf_sav_pt]=port_send[port_no][the_ram_ax];
        			the_ram_ax++;
        			the_ram_bx--;
        			set_port_rs_buf_sav_pt++;   ;;;�ҲµĲ����Ļ�����Ƿ��͵��ж�����֮һ,�����ϼ�����Ѿ�׼����������,�¼���⵽��Щ��־�Զ�����
        			if(set_port_rs_buf_sav_pt==set_port_rs_buf_tak_pt) set_port_rs_buf_tak_pt++;
        		}
        	}
        }
    }
    enable();
}














/*===========================================================================*/
void init_port_not_clr_ptr()
{
    far  BYTE *p554_base_addr;
         BYTE  ram_axl;
         BYTE  ram_axh;


    if(port_no>11) return;     ;;;��ֻ�ܳ�ʼ��12����,��ֻ��ǰ12��������ô��

    p554_base_addr=(far  BYTE *)Register_554[port_no];         ;;;ȷ����ǰ12���˿��е��ĸ��˿�

//     initial   FCR
    p554_base_addr[P554_FCR]=0x41;        // receive trigger at 4 bytes 
                                          // enable FIFO
//     initial   LCR
    ram_axh=(byte0(port_info[port_no].bits)-5) & 0x03;
    if(byte0(port_info[port_no].stopbit)==1)
    {
        //ram_axl=ram_axl & 0xfb;
    }
    else
    {
        ram_axh=ram_axh+0x04;
    }

    if(byte0(port_info[port_no].semiduplex_parity)==PARITY_ODD)
    {
        ram_axh=ram_axh+0x08; 
    }
    else
    {
        if(byte0(port_info[port_no].semiduplex_parity)==PARITY_EVEN)
        {
            ram_axh=ram_axh+0x18; 
        }
    }
    
    p554_base_addr[P554_LCR]=ram_axh | 0x80;
    
//     initial   BAUD
    p554_base_addr[P554_DLL]=byte0(BaudRate[port_info[port_no].bauds]);
    p554_base_addr[P554_DLM]=byte1(BaudRate[port_info[port_no].bauds]);
    
//     reset    LCR
    p554_base_addr[P554_LCR]=ram_axh;
    ram_axh=p554_base_addr[P554_RBR];
//     initial  IER
    p554_base_addr[P554_IER]=0x03;   // 
//     initial  MCR
//    if(port_no==0x0b)
//        p554_base_addr[P554_MCR]=0x08;//0x0a;   // disable trans
//    else
//        p554_base_addr[P554_MCR]=0x0a;//0x08;   // disable trans
    if(port_no==10)  p554_base_addr[P554_MCR]=0x0b;
    else             p554_base_addr[P554_MCR]=0x08;

    if(Cn_SIO_CAN_Reset[port_no]<0xffff)   Cn_SIO_CAN_Reset[port_no]++;
}











/************************************************/
/* init_port   function  Must DI before call it */
/************************************************/
/*===========================================================================*/
void init_port()
{
    init_port_not_clr_ptr();
    
    port_recv_pt[port_no] = 0;
    port_recv_dl[port_no] = 0;
    port_send_pt[port_no] = 0;
    port_send_len[port_no]= 0;
}















;;;����γ�����Ժ������֪��C���Ժͻ������û���κ�����,��������Ϊ�����ϰ������,���յĽ���͹��̶���һ����,�����ڷ����ʱ��̫,��������αָ��ָ��
;;;�������ɵ����ģ�鲻���������

/*===========================================================================*/
void Initial_CAN(BYTE the_port_no)         ;;;�����β�
{
    far BYTE *can_base_addr;
        BYTE ram_iaxl;                  ;;;��������ڲ���������

    if(the_port_no>1) return;          ;;;����1�Ͳ���Ҫ��ʼ��
    
    can_base_addr=(far BYTE *)CAN_BaseAddr+(unsigned long)the_port_no*CAN_AddrDistance;
    *(far BYTE *)(can_base_addr+CAN_MODE     ) =0x01;     ;;;������λģʽ
    *(far BYTE *)(can_base_addr+CAN_CLKDIVIDE) =0x88;     ;;;ѡ����ǿCANģʽPELI

    *(far BYTE *)(can_base_addr+CAN_INTEN    ) =0x0d;   // open overrun_int, error_warn_int,
                                                        //      TX_int     , RX_int         

    ram_iaxl=port_info[PORT_NO_CAN_0+the_port_no].bauds;
    *(far BYTE *)(can_base_addr+CAN_BUST0    ) =byte0(CAN_BUST_XTAL16M[ram_iaxl]);         ;;;���Byte0û�����⺬�����һ���������� ,Ϊ����ģ�黯�����
    *(far BYTE *)(can_base_addr+CAN_BUST1    ) =byte1(CAN_BUST_XTAL16M[ram_iaxl]);         ;;;�����˲�����
    *(far BYTE *)(can_base_addr+CAN_OUTCONT  ) =0xda;       ;;;;;;������ƾ��������ʽ

    *(far BYTE *)(can_base_addr+CAN_ACCCODE0 ) =0x00;       ;;;����Ԫ��ַ
    *(far BYTE *)(can_base_addr+CAN_ACCMASK0 ) =0x00;

    *(far BYTE *)(can_base_addr+CAN_ACCCODE1 ) =0x00;
    *(far BYTE *)(can_base_addr+CAN_ACCMASK1 ) =0xff;

    *(far BYTE *)(can_base_addr+CAN_ACCCODE2 ) =0xff;
    *(far BYTE *)(can_base_addr+CAN_ACCMASK2 ) =0xff;    // to receive all, set it with 0xff ;���������CPU��ͬ

    *(far BYTE *)(can_base_addr+CAN_ACCCODE3 ) =0x00;
    *(far BYTE *)(can_base_addr+CAN_ACCMASK3 ) =0xff;

    *(far BYTE *)(can_base_addr+CAN_MODE     ) =0x00;     ;;;����ط������˹���ģʽ

    ram_iaxl =*(far BYTE *)(can_base_addr+CAN_STATUS);         ;;;����ָ��������ֵ
    ram_iaxl =*(far BYTE *)(can_base_addr+CAN_INTSTA);         ;;;������״̬
    
    if(Cn_SIO_CAN_Reset[the_port_no+PORT_NO_CAN_0]<0xffff)   Cn_SIO_CAN_Reset[the_port_no+PORT_NO_CAN_0]++;
}























/*===========================================================================*/
void Judge_P554_CAN_Reset(void)
{
//         BYTE the_port_no;
         BYTE the_ram_axl;
         WORD the_ram_bx;
    far  BYTE *p554_base_addr;
    
    for(port_no=0;port_no<12;port_no++)
    {
        p554_base_addr=(far BYTE *)Register_554[port_no];        ;;;��12�Ķ�ά����,�Ǳ�ʾ3��554���Դ�12��è��
        the_ram_axl=p554_base_addr[P554_LSR];
        the_ram_bx=0x0001;
        the_ram_bx=the_ram_bx<<port_no;
        if((the_ram_axl & 0x0c)!=0)
        {
            SIO_CAN_Need_Reset=SIO_CAN_Need_Reset | the_ram_bx;		//-���е�����˵����Ӧ�Ľӿ���Ҫ��ʼ����
//            if(port_no==7) (*((far BYTE *)(SOFT_ERR_FLAG+0x0000)))=the_ram_axl;
        }
        
        if((SIO_CAN_Need_Reset & the_ram_bx)!=0)
        {
            disable();
            //port_no=the_port_no;
            init_port_not_clr_ptr();
            enable();
            the_ram_bx=~the_ram_bx;
            SIO_CAN_Need_Reset=SIO_CAN_Need_Reset & the_ram_bx;
        }
        
    }


    p554_base_addr=(far BYTE *)CAN0_BaseAddr;
    if(((SIO_CAN_Need_Reset & 0x1000)!=0) || ((p554_base_addr[CAN_MODE] & 0x01)!=0) || ((p554_base_addr[CAN_STATUS] & 0x80)!=0))
    {                                          ;;;��ֹ����
        disable();
        Initial_CAN(0);
        enable();
        SIO_CAN_Need_Reset=SIO_CAN_Need_Reset & 0xefff;
    }
    
    p554_base_addr=(far BYTE *)CAN1_BaseAddr;
    if(((SIO_CAN_Need_Reset & 0x2000)!=0) || ((p554_base_addr[CAN_MODE] & 0x01)!=0) || ((p554_base_addr[CAN_STATUS] & 0x80)!=0))
    {
        disable();
        Initial_CAN(1);
        enable();
        SIO_CAN_Need_Reset=SIO_CAN_Need_Reset & 0xdfff;
    }
    
}

















/*===========================================================================*/
void init_all_port(void)
{
//ȱӲ����λ
  for(port_no=0;port_no<12;port_no++)
  {
     init_port();
     port_mon[port_no] = 0;
  }
}

    