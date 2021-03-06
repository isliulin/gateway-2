/*
 * =============================================================================
 *
 *       Filename:  smart_home_pro.c
 *
 *    Description:  智能家居相关协议
 *
 *        Version:  v1.0
 *        Created:  2018-05-04 14:10:09
 *       Revision:  none
 *
 *         Author:  xubin
 *        Company:  Taichuan
 *
 * =============================================================================
 */
/* ---------------------------------------------------------------------------*
 *                      include head files
 *----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "zigbee.h"
#include "gateway.h"
#include "sql_handle.h"
#include "smart_home_pro.h"

/* ---------------------------------------------------------------------------*
 *                  extern variables declare
 *----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*
 *                  internal functions declare
 *----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*
 *                        macro define
 *----------------------------------------------------------------------------*/
#define VAILDTIME 	(200)				//200ms
typedef struct _PacketsID {
	uint16_t addr;  //短地址
	uint16_t current_channel; // 当前控制通道
	uint8_t cmd; // 当前控制通道
	uint64_t dwTick;		//时间
}PacketsID;

/* ---------------------------------------------------------------------------*
 *                      variables define
 *----------------------------------------------------------------------------*/
//因为要共用原来RF868智能家居的APP，而ZIGBEE是没有单元码，只有唯一地址，为实现兼容，
// (uint32_t)单元码 = (高 uint16_t)唯一地址 + (低 uint16_t) 通道数量 + 当前通道

// sendBuf_TypeDef zb_sendQueue[SEND_QUEUE_NUM];
// sendBuf_TypeDef zbSend;
// extern ipAddr_TypeDef phoneIP;

static PacketsID packets_id[10];
static int packet_pos;
/* ---------------------------------------------------------------------------*/
/**
 * @brief smarthomeSendDataPacket 单控命令封包
 *
 * @param d_addr 目的地址
 * @param cmd 命令
 * @param device_type 设备类型
 * @param ch_num 通道号
 * @param ch 通道
 * @param param 参数
 * @param param_len 参数长度
 */
/* ---------------------------------------------------------------------------*/
static void smarthomeSendDataPacket(
		uint16_t d_addr,
		uint8_t cmd,
		uint8_t device_type,
	   	uint8_t ch_num,
		uint8_t ch,
	   	uint8_t* param,
	   	uint8_t param_len)
{
	uint8_t buf[128] = {0};
	SMART_HOME_PRO *packet;
	packet = (SMART_HOME_PRO *)buf;
	packet->addr = 0;  //协调器地址为0
	packet->device_type = device_type;	//协调器的设备类型为0
	packet->channel_num = ch_num;	//通道数据
	packet->current_channel = ch;	//当前通道
	packet->cmd = cmd;
	
	if (param)
		memcpy(packet->param, param, param_len);
	zigbeeSendData(d_addr,buf,param_len + 6);
}

/*********************************************************************************************************
** Descriptions:       群控命令封包
** input parameters:   id 被控设备的ID   param 控制的参数
** output parameters:   buf 封好的数据包   len 数据包的长度
** Returned value:      无
*********************************************************************************************************/
void smarthomeMultiPacket(uint8_t *buf, uint8_t *len,uint8_t devNum)
{
#if 0
	MULTI_CTRL_TypeDef *multiPacket;
	multiPacket = (MULTI_CTRL_TypeDef *)&buf[3];
	
	memset(multiPacket,0,sizeof(multiPacket)*MULTI_PACKET_DEV_MAX);
	
	memset(buf, 0xff, 3);	//协议类型，群控协议固定为3个0XFF
	
	for (uint32_t i=0; i<devNum; i++)
	{
		
		EEPROM_GetDeviceInfo(multi[i].id, &sDev);
		multiPacket[i].addr = (uint16_t)(sDev.devUnit>>16);		//设备的ZIGBEE地址
		multiPacket[i].channel = (uint8_t)(sDev.devUnit&0xff);	//当前通道
		if (multi[i].param == 0)
		{
			multiPacket[i].cmd = Device_Off;
		}
		else
		{
			multiPacket[i].cmd = Device_On;
			multiPacket[i].param = multi[i].param;
		}
	}
	
	*len = 3+devNum*6;
#endif	
}



//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&以下是接收处理&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

/*********************************************************************************************************
** Descriptions:      收到想要的数据后，终止发送，并且回复给APP
** input parameters:   addr 命令的源地址   ch 设备通道  cmd功能指令  id为收到的设备的ID号
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void smarthomeReceiveNeeded(uint16_t addr, uint8_t ch, uint8_t cmd, uint8_t id)
{
#if 0
	SMART_HOME_PRO *packet;
	int32_t param[3];

	
	packet = (SMART_HOME_PRO *)zbSend.cmdData;
	
	//正在发送中的数据是否为单控指令，只有单控指令才会等待回复
	if (zbSend.dstAddr == 0xffff) //正在发送的指令为群控，不需回复
	{
		return;
	}
	
	//判断收到的指令是不是正在等待回复的指令
	if( (addr == packet->addr) && (ch == packet->current_channel) && (cmd == packet->cmd+1) )
	{
		if (zbSend.dAddr.port) //端口号为零表示不需要发送到网络
		{
			param[0] = 0; //控制成功
			param[1] = id;//设备的ID号
			CtrlDeviceRep(param,zbSend.TAG,&zbSend.dAddr,&zbSend.tAddr);
		}
		
		memset( &zbSend,0,sizeof(zbSend) );	//终止发送此命令
	}
	else
	{	//发送到最后一次控制的手机端，不过手机有可能收不到
		param[0] = 0; //控制成功
		param[1] = id;//设备的ID号
		CtrlDeviceRep(param,zbSend.TAG,&phoneIP,&zbSend.tAddr);
	}
#endif
}

/*********************************************************************************************************
** Descriptions:       在ZIGBEE列表中删除设备
** input parameters:  devUnit设备的单元码
** output parameters:   无
** Returned value:      1成功
*********************************************************************************************************/
uint32_t smarthomeDelDev(uint32_t devUnit)
{
#if 0
	uint16_t addr;
	uint8_t ch;
	zbDev_TypeDef addrBuf[MAX_REGIST_DEVICE];
	
	EEPROM_GetZbAddr((uint8_t *)addrBuf);	//读出所有设备的地址及通道信息
	
	addr = (uint16_t)(devUnit>>16); //从单元码中提取地址
	ch = (uint8_t)(devUnit&0xff);	//提取出当前通道
	
	for (int i=0; i<MAX_REGIST_DEVICE; i++)
	{
		if( (addr == addrBuf[i].addr) && (ch == addrBuf[i].channel) )
		{
			addrBuf[i].addr = 0;
			addrBuf[i].channel = 0;
			addrBuf[i].id = 0;
		}
	}
	EEPROM_WriteZbAddr((uint8_t *)addrBuf);
#endif
	return 1;
}

/*********************************************************************************************************
** Descriptions:       添加新设备
** input parameters:  
** output parameters:   无
** Returned value:      1成功
*********************************************************************************************************/
static char* smarthomeAddNewDev(SMART_HOME_PRO *cmdBuf,char *id)
{
	DPRINT("[%s]id:%s,type:%d,addr:%x\n", 
			__FUNCTION__,id,cmdBuf->device_type,cmdBuf->addr);
	sqlInsertDevice(id,
			cmdBuf->device_type,
			cmdBuf->addr,
			cmdBuf->channel_num);
	return id;
}

static int smarthomeCmdFilter(SMART_HOME_PRO *data)
{
	int i;
	uint64_t dwTick;
	if (data == NULL) {
		return 0;
	}
    //判断包是否重发
    dwTick = GetMs();
    for(i=0;i<10;i++) {
        if (data->addr == packets_id[i].addr
                 && data->current_channel == packets_id[i].current_channel
				 && data->cmd == packets_id[i].cmd
				 && (getDiffSysTick(dwTick,packets_id[i].dwTick) < VAILDTIME)){
            // saveLog("Packet ID %d is already receive,diff:%d!\n",
                    // packets_id[i].id,getDiffSysTick(dwTick,packets_id[i].dwTick));
            return 0;
        }
    }

	//保存ID
    packets_id[packet_pos].addr = data->addr;
	packets_id[packet_pos].cmd = data->cmd;
	packets_id[packet_pos].current_channel = data->current_channel;
	packets_id[packet_pos++].dwTick = dwTick;
	packet_pos %= 10;
    return 1;
}

static void smarthomeGetId(SMART_HOME_PRO *cmdBuf,char *id)
{
	if (!id)
		return;
	sqlGetDeviceId(cmdBuf->addr,id);
}
/*********************************************************************************************************
** Descriptions:       协议解析
** input parameters:   id 为设备的序列号， param 命令参数，dAddr  tAddr TAG 为网络控制时用，
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
static void smarthomeRecieve(uint8_t *buf, uint8_t len)
{
	char id[32] = {0};
	SMART_HOME_PRO *packet = (SMART_HOME_PRO *)buf;
    if (smarthomeCmdFilter(packet) == 0)
        return;
	switch (packet->cmd)
	{
		case NetIn_Report:
			{
				sprintf(id,"%02X%02X%02X%02X%02X%02X%02X%02X",
						packet->param[0],
						packet->param[1],
						packet->param[2],
						packet->param[3],
						packet->param[4],
						packet->param[5],
						packet->param[6],
						packet->param[7]);
				smarthomeSendDataPacket(
						packet->addr,
						NetIn_Report_Res,
						packet->device_type,
						packet->channel_num,
						packet->current_channel,
						NULL,0);
				smarthomeSendDataPacket(
						packet->addr,
						Demand_Sw_Status,
						packet->device_type,
						packet->channel_num,
						packet->current_channel,
						NULL,0);
				int ret = gwRegisterSubDevice(id,packet->device_type,packet->addr,packet->channel_num);
				if (ret == 0) 
					smarthomeAddNewDev(packet,id);
				// 根据阿里APP设定，完成入网后禁止入网
				zigbeeNetIn(0);
			}
			break;
		case NetOut_Report_Res:	//有设备要退网
			DPRINT("out\n");
			break;	
			
		case Demand_Sw_Status_Res:	//开关状态返回
			DPRINT("status\n");
			// for (i=0; i<idCnt; i++)
			// {
				// if(packet->param[i*2] > 100)
					// packet->param[i*2] = 100;
				// dev.status = packet->param[i*2];
				// EEPROM_WriteDeviceInfo(id[i],&dev);	//更新EEPROM中的状态
				// EEPROM_GetDeviceInfo(id[i+1], &dev);	//读出下一个待处理设备信息
			// }
			break;
		
		case Device_On_Res:		//设备开
			{
				smarthomeGetId(packet,id);
				DPRINT("on:%s\n",id);
				gwReportPowerOn(id,packet->param);
			} break;
			
		case Device_Off_Res:	//设备关
			{
				smarthomeGetId(packet,id);
				DPRINT("off:%s\n",id);
				gwReportPowerOff(id);
			} break;
		
		case Demand_Device_Alarm_Type_Res:		//查询单元的警报状态返回
			{
				smarthomeGetId(packet,id);
				DPRINT("alarm_status:%s,%d\n",id,packet->param[0]);
				gwReportAlarmStatus(id,packet->param);
			} break;
			
		case Device_Scene:		//情景控制
			// SceneStart(packet->param[0],1);
			break;
			
		case Device_Ele_Quantity:		//计量插座每隔30分钟上报用电量
			{
				smarthomeGetId(packet,id);
				DPRINT("ele quantity:%s\n",id);
				gwReportEleQuantity(id,packet->param);
				smarthomeSendDataPacket(
						packet->addr,
						Device_Ele_Quantity_Res,
						packet->device_type,
						packet->channel_num,
						packet->current_channel,
						NULL,0);
			} break;
			
		case Device_Ele_Power:		//计量插座每隔30分钟上报用功率
			{
				smarthomeGetId(packet,id);
				DPRINT("ele power:%s\n",id);
				gwReportElePower(id,packet->param);
				smarthomeSendDataPacket(
						packet->addr,
						Device_Ele_Power_Res,
						packet->device_type,
						packet->channel_num,
						packet->current_channel,
						NULL,0);
			} break;
			
	default:
		break;
	}
}

static void *smarthomeThread(void *arg)
{

	while (1) {
		// smarthomeNetInReport();
		// smarthomOutHandle();
		// smarthomeCheckTemprature();
		usleep(10000);
	}
	pthread_exit(NULL);
}

void smarthomeInit(void)                                        
{                                                               
	sqlInit();
	zigbeeInit();
    zigbeeSetDataRecvFunc(smarthomeRecieve);                    
                                                                
    pthread_t task;                                             
    pthread_attr_t attr;                                        
                                                                
    pthread_attr_init(&attr);                                   
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, smarthomeThread, NULL);        
}                                                               

void smarthomeAllDeviceCmdGetSwichStatus(DeviceStr *dev,uint16_t channel)
{
	smarthomeSendDataPacket(
			dev->addr,
			Demand_Sw_Status,
			dev->type_para->device_type,
			dev->channel,
			channel, NULL, 0);
}

void smarthomeLightCmdCtrOpen(DeviceStr *dev,uint16_t channel)
{
	uint8_t param[2] = {0xff,0};
	DPRINT("[%s]type:%d\n",__FUNCTION__, dev->type_para->device_type);
	smarthomeSendDataPacket(
			dev->addr,
			Device_On,
			dev->type_para->device_type,
			dev->channel,
			channel,param,sizeof(param));
}

void smarthomeLightCmdCtrClose(DeviceStr *dev,uint16_t channel)
{
	DPRINT("[%s]type:%d\n",__FUNCTION__, dev->type_para->device_type);
	smarthomeSendDataPacket(
			dev->addr,
			Device_Off,
			dev->type_para->device_type,
			dev->channel,
			channel, NULL, 0);
}

void smarthomeFreshAirCmdCtrOpen(DeviceStr *dev,uint8_t value)
{
	uint8_t param[2] = {0};
	param[0] = value;
	DPRINT("[%s]type:%d,value:%d\n",
			__FUNCTION__, 
			dev->type_para->device_type,
			param[0]);
	smarthomeSendDataPacket(
			dev->addr,
			Device_On,
			dev->type_para->device_type,
			dev->channel,
			0,param,sizeof(param));
}

void smarthomeFreshAirCmdCtrClose(DeviceStr *dev)
{
	DPRINT("[%s]type:%d\n",__FUNCTION__, dev->type_para->device_type);
	smarthomeSendDataPacket(
			dev->addr,
			Device_Off,
			dev->type_para->device_type,
			dev->channel,
			0,NULL,0);
}

void smarthomeAlarmWhistleCmdCtrOpen(DeviceStr *dev)
{
	uint8_t param[2] = {0};
	param[0] = 6; // 默认为６分钟
	DPRINT("[%s]type:%d,value:%d\n",
			__FUNCTION__, 
			dev->type_para->device_type,
			param[0]);
	smarthomeSendDataPacket(
			dev->addr,
			Device_On,
			dev->type_para->device_type,
			dev->channel,
			0,param,sizeof(param));
}

void smarthomeAlarmWhistleCmdCtrClose(DeviceStr *dev)
{
	DPRINT("[%s]type:%d\n",__FUNCTION__, dev->type_para->device_type);
	smarthomeSendDataPacket(
			dev->addr,
			Device_Off,
			dev->type_para->device_type,
			dev->channel,
			0,NULL,0);
}

void smarthomeCurtainCmdCtrOpen(DeviceStr *dev,uint16_t value)
{
	uint8_t param[2] = {0xff,0};
	param[0] = value;
	DPRINT("[%s]type:%d,value:%d\n",__FUNCTION__, dev->type_para->device_type,value);
	smarthomeSendDataPacket(
			dev->addr,
			Device_On,
			dev->type_para->device_type,
			dev->channel,
			1,param,sizeof(param));
}

void smarthomeCurtainCmdCtrClose(DeviceStr *dev)
{
	DPRINT("[%s]type:%d\n",__FUNCTION__, dev->type_para->device_type);
	smarthomeSendDataPacket(
			dev->addr,
			Device_Off,
			dev->type_para->device_type,
			dev->channel,
			1, NULL, 0);
}

/* ---------------------------------------------------------------------------*/
/**
 * @brief smarthomeAirCondtionCmdCtrOpen 
 * APP端对应风速为 0（自动） 2（低档） 3（中档） 4（高档） 
 * 		  模式为 0（自动） 1（制冷） 2（制热） 3（通风） 4（除湿）
 * 协议对应为
 * 第1个Byte为空调温度，范围16-32℃
 * 第2个byte:
 * 高4位:0制冷  1制热  2自动
 *        3除湿  4送风
 *        低4位: 0自动    1风高速 
 *        2风中速  3风低速
 *
 * @param dev
 * @param temp
 * @param mode
 * @param speed
 */
/* ---------------------------------------------------------------------------*/
void smarthomeAirCondtionCmdCtrOpen(DeviceStr *dev,
		uint8_t temp,
		uint8_t mode,
		uint8_t speed)
{
	char speed_change[] = {0,0,3,2,1}; // 速度转换
	char mode_change[] = {2,0,1,4,3}; // 模式转换
	uint8_t param[2] = {0};
	DPRINT("[%s]type:%d,temp:%d:,mode:%d,speed:%d\n",
			__FUNCTION__, 
			dev->type_para->device_type,
			temp,
			mode_change[mode],
			speed_change[speed]);
	param[0] = temp;
	param[1] = mode_change[mode]<< 4 | speed_change[speed];
	smarthomeSendDataPacket(
			dev->addr,
			Device_On,
			dev->type_para->device_type,
			dev->channel,
			1,param,sizeof(param));
}

