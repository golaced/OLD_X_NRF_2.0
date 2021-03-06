
#include "rc.h"
#include "led_fc.h"
#include "ms5611.h"
#include "height_ctrl.h"
#include "hml5833l.h"
#include "alt_kf.h"
#include "circle.h"

#define RX_DR			6		//????
#define TX_DS			5
#define MAX_RT		4

vs16 QH,ZY,XZ;
u8 is_lock=1;
u8 EN_FIX_GPSF=0;
u8 EN_FIX_LOCKWF=0;
u8 EN_CONTROL_IMUF=0;
u8 EN_FIX_INSF=0;
u8 EN_FIX_HIGHF=0;
u8 tx_lock=1;
u8 EN_FIX_GPS=0;
u8 EN_FIX_LOCKW=0;
u8 EN_CONTROL_IMU=0;
u8 EN_FIX_INS=0;
u8 EN_FIX_HIGH=0;
u8 EN_TX_GX=1;
u8 EN_TX_AX=1;
u8 EN_TX_HM=1;
u8 EN_TX_YRP=1;
u8 EN_TX_GPS=1;
u8 EN_TX_HIGH=1;
u8 up_load_set=0;
u8 up_load_pid=0;
u8 key_rc[6]={1,1,1,1,1,1};
u16 Yaw_sb_rc=0;

u8 cnt_rst=0,delta_pitch=0,delta_roll=0,delta_yew=0;



//中值滤波
float GetMedianNum(float * bArray, u16 iFilterLen)
{  
    int i,j;// 循环变量  
    float bTemp;  
      
    // 用冒泡法对数组进行排序  
    for (j = 0; j < iFilterLen - 1; j ++)  
    {  
        for (i = 0; i < iFilterLen - j - 1; i ++)  
        {  
            if (bArray[i] > bArray[i + 1])  
            {  
                // 互换  
                bTemp = bArray[i];  
                bArray[i] = bArray[i + 1];  
                bArray[i + 1] = bTemp;  
            }  
        }  
    }  
      
    // 计算中值  
    if ((iFilterLen & 1) > 0)  
    {  
        // 数组有奇数个元素，返回中间一个元素  
        bTemp = bArray[(iFilterLen + 1) / 2];  
    }  
    else  
    {  
        // 数组有偶数个元素，返回中间两个元素平均值  
        bTemp = (bArray[iFilterLen / 2] + bArray[iFilterLen / 2 + 1]) / 2;  
    }  
  
    return bTemp;  
}  
u16 data_rate;
#define MID_RC_KEY 15
uint8_t NRF24L01_RXDATA_REG[32];//nrf24l01接收到的数据
 u8 key_rc_reg[7][MID_RC_KEY];
#define MID_RC_GET 4
float RC_GET[4][MID_RC_GET];
float control_scale=1;
float ypr_sb[3];
u8 Not_sent;
void NRF_DataAnl(void)
{ 
int16_t RC_GET_TEMP[4];
float RC_GETR[4][MID_RC_GET];	
u8 temp_key[7];
u8 temp;
	u8 i=0,j,sum = 0;
	for( i=0;i<31;i++)
		sum += NRF24L01_RXDATA[i];
	if(!(sum==NRF24L01_RXDATA[31]))	
	{i=0;
		return;
		}	//??sum}
	if(!(NRF24L01_RXDATA[0]==0x8A))		
		{
			i=0;
		return;
			}	//??sum}
	
	if(NRF24L01_RXDATA[1]==0x8A)								//?????,=0x8a,?????
	{ Feed_Rc_Dog(2);//通信看门狗喂狗
		data_rate++;
		RC_GET_TEMP[0]= (vs16)(NRF24L01_RXDATA[3]<<8)|NRF24L01_RXDATA[4];//Rc_Get.THROTTLE
		Rc_Get.THROTTLE= (vs16)(NRF24L01_RXDATA[3]<<8)|NRF24L01_RXDATA[4];//Rc_Get.THROTTLE
		Rc_Get.YAW			= (vs16)(NRF24L01_RXDATA[5]<<8)|NRF24L01_RXDATA[6];
		Rc_Get.PITCH		= (vs16)(NRF24L01_RXDATA[7]<<8)|NRF24L01_RXDATA[8];
		Rc_Get.ROLL 		= (vs16)(NRF24L01_RXDATA[9]<<8)|NRF24L01_RXDATA[10];
		RC_GET_TEMP[1]= (vs16)((NRF24L01_RXDATA[5]<<8)|NRF24L01_RXDATA[6])/3+1000;//RC_Data.YAW	
		RC_GET_TEMP[2]= (vs16)((NRF24L01_RXDATA[7]<<8)|NRF24L01_RXDATA[8])/3+1000;//RC_Data.PITCH	
		RC_GET_TEMP[3]= (vs16)((NRF24L01_RXDATA[9]<<8)|NRF24L01_RXDATA[10])/3+1000;//RC_Data.ROLL
		RX_CH[AUX1r]=Rc_Get.AUX1			= (vs16)(NRF24L01_RXDATA[11]<<8)|NRF24L01_RXDATA[12];
		RX_CH[AUX4r]=Rc_Get.AUX4			= (vs16)(NRF24L01_RXDATA[13]<<8)|NRF24L01_RXDATA[14];
		RX_CH[AUX3r]=Rc_Get.AUX3			= (vs16)(NRF24L01_RXDATA[15]<<8)|NRF24L01_RXDATA[16];
		RX_CH[AUX2r]=Rc_Get.AUX2			= (vs16)(NRF24L01_RXDATA[17]<<8)|NRF24L01_RXDATA[18];
		Rc_Get.AUX5			= (vs16)(NRF24L01_RXDATA[19]<<8)|NRF24L01_RXDATA[20];

		ctrl_angle_offset.x =(float)(Rc_Get.AUX1-500)/1000.*MAX_FIX_ANGLE*2;
		ctrl_angle_offset.y =(float)(Rc_Get.AUX2-500)/1000.*MAX_FIX_ANGLE*2;

		if(fabs(ctrl_angle_offset.x )<0.2)  {ctrl_angle_offset.x =0;}
	 	if(fabs(ctrl_angle_offset.y )<0.2)  {ctrl_angle_offset.y =0;}
		
	  RX_CH[THRr]=	Rc_Get.THROTTLE-RX_CH_FIX[THRr]	;
	  RX_CH[ROLr]=  Rc_Get.ROLL-RX_CH_FIX[ROLr]	;
	  RX_CH[PITr]=  Rc_Get.PITCH-RX_CH_FIX[PITr]	;
		KEY_SEL[0]=(NRF24L01_RXDATA[21])&0x01;
		KEY_SEL[1]=(NRF24L01_RXDATA[21]>>1)&0x01;
		KEY_SEL[2]=(NRF24L01_RXDATA[21]>>2)&0x01;
		KEY_SEL[3]=(NRF24L01_RXDATA[21]>>3)&0x01; 
	  KEY[0]=(NRF24L01_RXDATA[22])&0x01;
		KEY[1]=(NRF24L01_RXDATA[22]>>1)&0x01;
		KEY[2]=(NRF24L01_RXDATA[22]>>2)&0x01;
		KEY[3]=(NRF24L01_RXDATA[22]>>3)&0x01;
		KEY[4]=(NRF24L01_RXDATA[22]>>4)&0x01;
		KEY[5]=(NRF24L01_RXDATA[22]>>5)&0x01;
		KEY[6]=(NRF24L01_RXDATA[22]>>6)&0x01;
		KEY[7]=(NRF24L01_RXDATA[22]>>7)&0x01;
		
		ypr_sb[0]=(int)(((NRF24L01_RXDATA[23]<<8)|NRF24L01_RXDATA[24]))/100.;
		ypr_sb[1]=(int)(((NRF24L01_RXDATA[25]<<8)|NRF24L01_RXDATA[26]))/100.;
		ypr_sb[2]=(int)(((NRF24L01_RXDATA[27]<<8)|NRF24L01_RXDATA[28]))/100.;		
		for(j=0;j<3;j++)
		  if(ypr_sb[j]>360)
				ypr_sb[j]-=655.35;
		if(mode.yaw_imu_control)	
		RX_CH[YAWr]=  Rc_Get.YAW-RX_CH_FIX[YAWr]	;
		else{	
		if(fabs( ypr_sb[2])>32&&fabs(ypr_sb[1])<15)	
		RX_CH[YAWr]=  limit_mine(ypr_sb[2]*10,500)	+1500;
		else
		RX_CH[YAWr]=1500;	
	}
		if(mode.dj_lock){
		if(fabs( ypr_sb[1]-17)>40&&fabs(ypr_sb[2])<15)	
		{dj_angle_set+= (ypr_sb[1]-17)*0.008;
		dj_angle_set=LIMIT(dj_angle_set,-12,12);//SCALE_DJ*30,SCALE_DJ*30);
		}
	}
		else
		dj_angle_set=0;	
//-------------------------------------------------------------------------------------------------------------------------		
	 }
	else if(NRF24L01_RXDATA[1]==0x8B)								//?????,=0x8a,?????
	{
			tx_lock=(NRF24L01_RXDATA[3]);
			EN_FIX_GPS=(NRF24L01_RXDATA[4]);
			EN_FIX_LOCKW=(NRF24L01_RXDATA[5]);
			EN_CONTROL_IMU=(NRF24L01_RXDATA[6]);
			EN_FIX_INS=(NRF24L01_RXDATA[7]);
			EN_FIX_HIGH=(NRF24L01_RXDATA[8]);
			EN_TX_GX=(NRF24L01_RXDATA[9]);
			EN_TX_AX=(NRF24L01_RXDATA[10]);
			EN_TX_HM=(NRF24L01_RXDATA[11]);
			EN_TX_YRP=(NRF24L01_RXDATA[12]);
			EN_TX_GPS=(NRF24L01_RXDATA[13]);
			EN_TX_HIGH=(NRF24L01_RXDATA[14]);
			(up_load_set)=(NRF24L01_RXDATA[15]);
			(up_load_pid)=(NRF24L01_RXDATA[16]);
		
	EN_FIX_GPSF=EN_FIX_GPS;
	EN_FIX_LOCKWF=EN_FIX_GPS;
	EN_CONTROL_IMUF=EN_FIX_GPS;
	EN_FIX_INSF=EN_FIX_GPS;
	EN_FIX_HIGHF=EN_FIX_GPS;	
	}
	else 		if(NRF24L01_RXDATA[1]==0x8C)	//??PID1
		{
		  if(mode.en_pid_sb_set){
			SPID.OP = (float)((vs16)(NRF24L01_RXDATA[4]<<8)|NRF24L01_RXDATA[5]);
			SPID.OI = (float)((vs16)(NRF24L01_RXDATA[6]<<8)|NRF24L01_RXDATA[7]);
			SPID.OD = (float)((vs16)(NRF24L01_RXDATA[8]<<8)|NRF24L01_RXDATA[9]);
		  SPID.IP = (float)((vs16)(NRF24L01_RXDATA[10]<<8)|NRF24L01_RXDATA[11]);
			SPID.II = (float)((vs16)(NRF24L01_RXDATA[12]<<8)|NRF24L01_RXDATA[13]);
			SPID.ID = (float)((vs16)(NRF24L01_RXDATA[14]<<8)|NRF24L01_RXDATA[15]);
		  SPID.YP = (float)((vs16)(NRF24L01_RXDATA[16]<<8)|NRF24L01_RXDATA[17]);
			SPID.YI = (float)((vs16)(NRF24L01_RXDATA[18]<<8)|NRF24L01_RXDATA[19]);
			SPID.YD = (float)((vs16)(NRF24L01_RXDATA[20]<<8)|NRF24L01_RXDATA[21]);
	/*	ctrl_1.PID[PIDPITCH].kp =ctrl_1.PID[PIDROLL].kp  = 0.001*SPID.IP;
		ctrl_1.PID[PIDPITCH].ki =ctrl_1.PID[PIDROLL].ki  = 0.001*SPID.II;
		ctrl_1.PID[PIDPITCH].kd =ctrl_1.PID[PIDROLL].kd  = 0.001*SPID.ID;
		ctrl_1.PID[PIDYAW].kp 	= 0.001*SPID.YP;
		//ctrl_1.PID[PIDYAW].ki 	= 0.001*;
		//ctrl_1.PID[PIDYAW].kd 	= 0.001*;
		ctrl_2.PID[PIDPITCH].kp =ctrl_2.PID[PIDROLL].kp  = 0.001*SPID.OP;
		ctrl_2.PID[PIDPITCH].ki =ctrl_2.PID[PIDROLL].ki  = 0.001*SPID.OI;
		ctrl_2.PID[PIDPITCH].kd =ctrl_2.PID[PIDROLL].kd  = 0.001*SPID.OD;
		ctrl_2.PID[PIDYAW].kp 	= 0.001*SPID.YD;
		ctrl_2.PID[PIDYAW].ki 	= 0.001*SPID.YI;
		//ctrl_2.PID[PIDYAW].kd 	= 0.001*SPID.YD;
*/
//nav    01
		
    //height  11
		 if(KEY[0]==1&&KEY[1]==1){
			 
//			 if(mode.height_safe){
			//if(mode.test3||mode.test2)
			//{
//		  ultra_pid_head.kp =  		0.001*(float)SPID.OP;
//		  ultra_pid_head.ki =  		0.001*(float)SPID.OI;
//			ultra_pid_head.kd = 		0.001*(float)SPID.OD;
//      exp_height_head_scan=			  SPID.YP;
//			exp_height_head_shoot=			  SPID.YI;
//			AVOID[0]=AVOID[1]=      0.001*(float)SPID.IP;
//			YUN_PER_OFF=SPID.YD;
			 //k_m100_yaw=  		0.001*(float)SPID.OP;
			 
//			}else{
//			 ctrl_2.PID[PIDYAW].kp=  		0.001*(float)SPID.IP;
//			 ctrl_2.PID[PIDYAW].ki=  		0.001*(float)SPID.II;
//			 ctrl_2.PID[PIDYAW].kd=  		0.001*(float)SPID.ID;
		  ultra_pid.kp =  		0.001*(float)SPID.OP;
			ultra_pid.ki =  		0.001*(float)SPID.OI;
			ultra_pid.kd = 			0.001*(float)SPID.OD;
//			k_m100_gps[0]=0.001*(float)SPID.OP;//p R
//			k_m100_gps[1]=0.001*(float)SPID.OI;//T
//			k_m100_gps[2]=0.001*(float)SPID.OD;//y
//			  
//			k_m100_track[1] = 0.001*(float)SPID.IP;
//			k_m100_shoot[1] = 0.001*(float)SPID.II;
//			pid.nav.in.d = 0.001*(float)SPID.ID;
//			 
//			pid.nav.out.p= 0.01*(float)SPID.OP;
//			pid.nav.out.i= 0.01*(float)SPID.OI;
//			pid.nav.out.d= 0.001*(float)SPID.OD;
//        Yaw_set_dji= 0.1*(float)SPID.YP;
//				k_m100_laser_avoid= 0.001*(float)SPID.YP;//位置死区mm
//				circle.control_k=-0.001*(float)SPID.YI;//位置死区mm
//				AVOID[0]=AVOID[1]=0.001*(float)SPID.YD;
//			pid.nav.in.dead=  (float)SPID.YI/1000.;//速度死区mm/s
//			pid.nav.in.dead2= (float)SPID.YD/1000.;
			 }
	 else
	 {  
		track.control_yaw=   0.001*(float)SPID.OP;//左右增益
		circle.control_k=  	0.001*(float)SPID.OI;//上下增益
//		track.forward_end_dj_pwm=0.001*(float)SPID.OD;//航向增益
//		   //YUN_PER_OFF=SPID.YI;//预偏值
//		   Yaw_set_dji=SPID.YD;//设定航线 0~360
//	 gps_pid.kp=0.001*(float)SPID.OP;
//	 gps_pid.ki=0.001*(float)SPID.OI;
//	 gps_pid.kd=0.001*(float)SPID.OD;
//		  ultra_pid_head.kp =  		0.001*(float)SPID.OP;
//			ultra_pid_head.ki =  		0.001*(float)SPID.OI;
//			ultra_pid_head.kd = 		0.001*(float)SPID.OD;	
	 //	pid.circle.in.p = 0.001*(float)SPID.IP;
	//	pid.circle.in.i = 0.001*(float)SPID.II;
	//	pid.circle.in.d = 0.001*(float)SPID.ID;
	//	pid.circle.out.p= 0.001*(float)SPID.OP;
	  
   // track.forward= (float)SPID.YP;
	 // track.forward_end_dj_pwm= 0.01*(float)SPID.YI; 
	 //	track.control_yaw=0.001*(float)SPID.YD; 
	}
	 
//	if(KEY[0]==1&&KEY[1]==0){
//			pid.nav.in.p = 0.001*(float)SPID.IP;
//			pid.nav.in.i = 0.001*(float)SPID.II;
//			pid.nav.in.d = 0.001*(float)SPID.ID;
//			pid.nav.out.p= 0.01*(float)SPID.OP;
//			pid.nav.out.i= 0.01*(float)SPID.OI;
//			pid.nav.out.d= 0.001*(float)SPID.OD;
//			//if(SPID.OI!=0)
//			//pid.nav.out.i=    (float)SPID.OI/100.;//dlf_nav
//			//else
//			//pid.nav.out.i=8;	
//			//pid.avoid.out.p=  (float)SPID.OD/100.;
//			
//			pid.nav.out.dead= (float)SPID.YP/1000.;//位置死区mm
//			pid.nav.in.dead=  (float)SPID.YI/1000.;//速度死区mm/s
//			pid.nav.in.dead2= (float)SPID.YD/1000.;
//			

//		}
//			 {
//			ultra_pid_safe.kp =  		 0.001*(float)SPID.OP;
//			ultra_pid_safe.ki =  		 0.001*(float)SPID.OI;
//			ultra_pid_safe.kd = 		 0.001*(float)SPID.OD;
//			wz_speed_pid_safe.kp =   0.01*(float)SPID.IP; 
//			wz_speed_pid_safe.ki =   0.01*(float)SPID.II;
//			wz_speed_pid_safe.kd =   0.01*(float)SPID.ID;
//			 }
//		}
//		else{
//			pid.nav.in.p = 0.01*(float)SPID.IP;
//			pid.nav.in.i = 0.01*(float)SPID.II;
//			pid.nav.in.d = 0.01*(float)SPID.ID;
//			pid.nav.out.p= 0.01*(float)SPID.OP;
//			//if(SPID.OI!=0)
//			//pid.nav.out.i=    (float)SPID.OI/100.;//dlf_nav
//			//else
//			//pid.nav.out.i=8;	
//			//pid.avoid.out.p=  (float)SPID.OD/100.;
//			
//			pid.nav.out.dead= (float)SPID.YP/100.;//位置死区mm
//			pid.nav.in.dead=  (float)SPID.YI/100.;//速度死区mm/s
//			pid.nav.in.dead2= (float)SPID.YD/100.;
//			

//		}
			}
		//	EE_SAVE_PID();
		}
	else 		if(NRF24L01_RXDATA[1]==0x8D)	//??PID2
		{
			HPID.OP = (float)((vs16)(NRF24L01_RXDATA[4]<<8)|NRF24L01_RXDATA[5]);
			HPID.OI = (float)((vs16)(NRF24L01_RXDATA[6]<<8)|NRF24L01_RXDATA[7]);
			HPID.OD = (float)((vs16)(NRF24L01_RXDATA[8]<<8)|NRF24L01_RXDATA[9]);
			
		//	EE_SAVE_PID();
		}


	

}

u8 RC_CONNECT;
int cnt_timer2_r=0;
u8 cnt_led_rx=0;
void Nrf_Check_Event(void)
{ 
	u8 rx_len =0;
	static u16 cnt_loss_rc=0;
	u8 sta;

	sta= NRF_Read_Reg(NRF_READ_REG + NRFRegSTATUS);		//??2401?????
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
	if(sta & (1<<RX_DR))	//??ing 
	{ 
		cnt_loss_rc=0;
			
				
				
				rx_len= NRF_Read_Reg(R_RX_PL_WID);
				
				if(rx_len<33)	//??????33?????,???????
				{ RC_CONNECT=1;
					NRF_Read_Buf(RD_RX_PLOAD,NRF24L01_RXDATA,RX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer
					Rc_Get.update=1;
					Rc_Get.lose_cnt=0;
					if(1){
					for(int i=0;i<32;i++)
					NRF24L01_RXDATA_REG[i]=NRF24L01_RXDATA[i];
					}
					//LEDRGB_STATE();
					NRF_DataAnl();	//??2401??????
				
				}
				else 
				{ 
					NRF_Write_Reg(FLUSH_RX,0xff);//?????
				}
				if(cnt_led_rx<2)
				cnt_led_rx++;
				else 
				{
				cnt_led_rx=0;
				}
	}
	else//---------losing_nrf
	 {	
		 if(cnt_loss_rc++>200 )//0.5ms
		 {	force_Thr_low=1;
			 NRF_Write_Reg(FLUSH_RX,0xff);//?????
//		   CH[PIT]=CH[ROL]=CH[YAW]=0;
//			 if(height_ctrl_mode==0)
//			 CH[THR]=-500; 
//			 else
//			 CH[THR]=0;
			 
				
	  	RX_CH[PITr]=RX_CH[ROLr]=RX_CH[YAWr]=1500;
//			if(height_ctrl_mode==0)
//			 RX_CH[THR]=1500; 
//			 else
			 RX_CH[THRr]=1000;
			
		 }
  }
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
	if(sta & (1<<TX_DS))	//????,?????
	{
	
	}
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
	
	if(sta & (1<<MAX_RT))//??,????
	{
		if(sta & 0x01)	//TX FIFO FULL
		{
			NRF_Write_Reg(FLUSH_TX,0xff);
		}
	}
	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
	NRF_Write_Reg(NRF_WRITE_REG + NRFRegSTATUS, sta);
}

#include "pwm_in.h"

void NRF_Send_ARMED(void)//????
{
	uint8_t i,sum;
	u32 _temp;
	u8 cnt=0;

	NRF24L01_TXDATA[cnt++]=0x88;
	NRF24L01_TXDATA[cnt++]=0x01;
	NRF24L01_TXDATA[cnt++]=0x1C;
	_temp=8400;//BAT_FLY;
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);//添加 gps 姿态 状态通信
	_temp = (int)(31.1234*10000000);
  NRF24L01_TXDATA[cnt++]=BYTE3(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE2(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);
	_temp = 0;
  NRF24L01_TXDATA[cnt++]=BYTE3(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE2(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);
	_temp = (int)(fly_controller.imu.roll*100);//(int)(Pitch*100);
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);
	_temp = (int)(fly_controller.imu.pitch*100);//(int)(Roll*100);// (int)((Target.Yaw+180)*100);//
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);
  _temp = (int)(fly_controller.imu.yaw*100);
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);
	_temp= fly_controller.now.thr;//(plane.get.altitude.sonar);//hight
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);
	_temp=data_rate;//(plane.get.altitude.sonar);//hight
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);
	_temp= height_ctrl_mode;//(plane.get.altitude.sonar);//hight
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);

	_temp= fly_controller.now.alt_fushion;//ultra_speed;}
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);
	_temp=0;//baroAlt;//(plane.get.altitude.sonar);//hight
	NRF24L01_TXDATA[cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[cnt++]=BYTE0(_temp);
	//gps mode
	NRF24L01_TXDATA[cnt++]=(0);
	//gps no
	NRF24L01_TXDATA[cnt++]=(1);
	//fly mode
	NRF24L01_TXDATA[cnt++]=(2);
//	NRF24L01_TXDATA[cnt++]=(sys.flow<<3)|(sys.circle<<2)|(sys.avoid<<1)|(sys.gps);
//	NRF24L01_TXDATA[cnt++]=EN_FIX_GPSF;
//	NRF24L01_TXDATA[cnt++]=EN_FIX_LOCKWF;
//	NRF24L01_TXDATA[cnt++]=EN_CONTROL_IMUF;
//	NRF24L01_TXDATA[cnt++]=EN_FIX_INSF;
//	NRF24L01_TXDATA[cnt++]=EN_FIX_HIGHF;
//	//if(ARMED)	NRF24L01_TXDATA[3]=0xA1;
//	//else 			NRF24L01_TXDATA[3]=0xA0;
	sum = 0;
	for(i=0;i<31;i++)
		sum += NRF24L01_TXDATA[i];
	
	NRF24L01_TXDATA[31]=sum;
	
	NRF_TxPacket(NRF24L01_TXDATA,32);
}

void NRF_Send_RC_PID1(void)//????
{	vs16 _temp;	u8 i;	u8 sum = 0;

	NRF24L01_TXDATA[0] = 0x88;	//?????8BSET??
	NRF24L01_TXDATA[1] = 0x02;	//?????8BSET??
	NRF24L01_TXDATA[2] = 0x1C;
	NRF24L01_TXDATA[3] = 0xAD;
	
	_temp =SPID.OP;
	NRF24L01_TXDATA[4]=BYTE1(_temp);
	NRF24L01_TXDATA[5]=BYTE0(_temp);
	_temp =SPID.OI;
	NRF24L01_TXDATA[6]=BYTE1(_temp);
	NRF24L01_TXDATA[7]=BYTE0(_temp);
	_temp = SPID.OD;
	NRF24L01_TXDATA[8]=BYTE1(_temp);
	NRF24L01_TXDATA[9]=BYTE0(_temp);
	_temp =SPID.IP;
	NRF24L01_TXDATA[10]=BYTE1(_temp);
	NRF24L01_TXDATA[11]=BYTE0(_temp);
	_temp =SPID.II;
	NRF24L01_TXDATA[12]=BYTE1(_temp);
	NRF24L01_TXDATA[13]=BYTE0(_temp);
	_temp = SPID.ID;
	NRF24L01_TXDATA[14]=BYTE1(_temp);
	NRF24L01_TXDATA[15]=BYTE0(_temp);
	_temp = SPID.YP;//.YP * 1;
	NRF24L01_TXDATA[16]=BYTE1(_temp);
	NRF24L01_TXDATA[17]=BYTE0(_temp);
	_temp = SPID.YI;//SPID.YI * 1;
	NRF24L01_TXDATA[18]=BYTE1(_temp);
	NRF24L01_TXDATA[19]=BYTE0(_temp);
  _temp = SPID.YD;//.YD * 1;
	NRF24L01_TXDATA[20]=BYTE1(_temp);
	NRF24L01_TXDATA[21]=BYTE0(_temp);
	
	

sum = 0;
	for(i=0;i<31;i++)
		sum += NRF24L01_TXDATA[i];
	
	NRF24L01_TXDATA[31]=sum;
	
	NRF_TxPacket(NRF24L01_TXDATA,32);
}

void NRF_Send_RC_PID2(void)//????
{	vs16 _temp;	u8 _cnt=0;u8 i;u8 sum = 0;

	NRF24L01_TXDATA[_cnt++] = 0x88;	//?????8BSET??
	NRF24L01_TXDATA[_cnt++] = 0x03;	//?????8BSET??
	NRF24L01_TXDATA[_cnt++] = 0x1C;
	NRF24L01_TXDATA[_cnt++]=	0xAD;
	_temp = 	  HPID.OP;
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
		_temp = 	HPID.OI;
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
		_temp = 	HPID.OD;
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
		_temp = 	0*100;
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
		_temp = 	0*100;
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
	
	
	
sum = 0;
	for(i=0;i<31;i++)
		sum += NRF24L01_TXDATA[i];
	
	NRF24L01_TXDATA[31]=sum;
	
	NRF_TxPacket(NRF24L01_TXDATA,32);
}

void NRF_Send_RC_Sensor(void)//????
{	vs16 _temp;	u8 _cnt=0;u8 i;u8 sum = 0;

	NRF24L01_TXDATA[_cnt++] = 0x88;	//?????8BSET??
	NRF24L01_TXDATA[_cnt++] = 0x04;	//?????8BSET??
	NRF24L01_TXDATA[_cnt++] = 0x1C;
	NRF24L01_TXDATA[_cnt++]=	0xAD;

	_temp =  0;	
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
	_temp =  0;	
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
	_temp =  0;	
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
	_temp =  0;	
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
	
	
	sum = 0;
	for(i=0;i<31;i++)
		sum += NRF24L01_TXDATA[i];
	
	NRF24L01_TXDATA[31]=sum;
	
	NRF_TxPacket(NRF24L01_TXDATA,32);
}


void NRF_Send_RC_GPS(void)
{u8 i;	u8 sum = 0;
	u8 _cnt=0;
	vs16 _temp;
	vs32 _temp32;
	NRF24L01_TXDATA[_cnt++]=0x88;
	NRF24L01_TXDATA[_cnt++]=0x05;
	NRF24L01_TXDATA[_cnt++]=0x1C;//功能字
	NRF24L01_TXDATA[_cnt++]=0xAD;
	_temp32 =  imu_nav.gps.J;
	NRF24L01_TXDATA[_cnt++]=BYTE3(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE2(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp32);
	_temp32 =  imu_nav.gps.W;
	NRF24L01_TXDATA[_cnt++]=BYTE3(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE2(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp32);
	_temp =imu_nav.gps.gps_mode;
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
	_temp =imu_nav.gps.star_num;
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
	_temp32 =  imu_nav.gps.X_O;
	NRF24L01_TXDATA[_cnt++]=BYTE3(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE2(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp32);
	_temp32 =  imu_nav.gps.Y_O;
	NRF24L01_TXDATA[_cnt++]=BYTE3(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE2(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp32);
	_temp32 =  imu_nav.gps.X_UKF;
	NRF24L01_TXDATA[_cnt++]=BYTE3(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE2(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp32);
	_temp32 =  imu_nav.gps.Y_UKF;
	NRF24L01_TXDATA[_cnt++]=BYTE3(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE2(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp32);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp32);


sum = 0;
	for(i=0;i<31;i++)
		sum += NRF24L01_TXDATA[i];
	
	NRF24L01_TXDATA[31]=sum;
	
	NRF_TxPacket(NRF24L01_TXDATA,32);;
}


void NRF_Send_RC_DEBUG1(void)
{u8 i;	u8 sum = 0;
	u8 _cnt=0;
	vs16 _temp;
	vs32 _temp32;
	NRF24L01_TXDATA[_cnt++]=0x88;
	NRF24L01_TXDATA[_cnt++]=0x08;
	NRF24L01_TXDATA[_cnt++]=0x1C;
	for(i=0;i<14;i++){
	_temp =  BLE_DEBUG[i];	
	NRF24L01_TXDATA[_cnt++]=BYTE1(_temp);
	NRF24L01_TXDATA[_cnt++]=BYTE0(_temp);
  }
sum = 0;
	for(i=0;i<31;i++)
		sum += NRF24L01_TXDATA[i];
	
	NRF24L01_TXDATA[31]=sum;
	
	NRF_TxPacket(NRF24L01_TXDATA,32);
}

void RC_Send_Task(void)
{
static u16 cnt[4]={0,0,0,0};
static u8 state;

switch(state)
{
	case 0:NRF_Send_ARMED();state=1;
	  break;
	case 1:
		NRF_Send_RC_GPS();state=2;
	 break;
	case 2: NRF_Send_RC_DEBUG1();
		state=3;
	 break;
	case 3:
		if(cnt[0]==0){cnt[0]=1;
		NRF_Send_RC_PID1();}
		else
		{cnt[0]=0;
		NRF_Send_RC_PID2();}
		state=0;
 break;
}
}




void Send_RC_PWM(void)
{u8 i;	u8 sum = 0;
	u8 data_to_send[50];
	static u8 test;
	u8 _cnt=0;
	vs16 _temp;
  data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=0xAF;
	data_to_send[_cnt++]=0x88;//功能字
	data_to_send[_cnt++]=0;//数据量
  
	_temp=RX_CH[PITr];
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	_temp=m100.Pit;//Rc_Get_PWM.ROLL;
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	_temp=m100.Pit;//(vs16)Rc_Get.PITCH ;//Rc_Get_PWM.THROTTLE;
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	_temp=m100.Pit;//4;//Rc_Get_PWM.YAW;
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	
	_temp=m100.Pit;//5;//Rc_Get_PWM.AUX1;
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	_temp=m100.Pit;//6;//Rc_Get_PWM.RST;
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	_temp=m100.Pit;//7;//Rc_Get_PWM.HEIGHT_MODE;
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	_temp=m100.Pit;//8;//Rc_Get_PWM.POS_MODE;
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	
	
	data_to_send[3] = _cnt-4;

	for( i=0;i<_cnt;i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;
	
	Send_Data_GOL_LINK(data_to_send, _cnt);
}

void Send_RC(void)
{u8 i;	u8 sum = 0;
	u8 data_to_send[50]={0};
	u8 _cnt=0;
	vs16 _temp;
	Not_sent=0;
  data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=0xAF;
	data_to_send[_cnt++]=0x66;//功能字
	data_to_send[_cnt++]=0;//数据量
	for(i=0;i<32;i++)
	data_to_send[_cnt++]=NRF24L01_RXDATA_REG[i];
	
	data_to_send[3] = _cnt-4;

	for( i=0;i<_cnt;i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;
	
	Send_Data_GOL_LINK(data_to_send, _cnt);
	Not_sent=1;
}


