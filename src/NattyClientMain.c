/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2016
 * 
 *
 
****       *****
  ***        *
  ***        *                         *               *
  * **       *                         *               *
  * **       *                         *               *
  *  **      *                        **              **
  *  **      *                       ***             ***
  *   **     *       ******       ***********     ***********    *****    *****
  *   **     *     **     **          **              **           **      **
  *    **    *    **       **         **              **           **      *
  *    **    *    **       **         **              **            *      *
  *     **   *    **       **         **              **            **     *
  *     **   *            ***         **              **             *    *
  *      **  *       ***** **         **              **             **   *
  *      **  *     ***     **         **              **             **   *
  *       ** *    **       **         **              **              *  *
  *       ** *   **        **         **              **              ** *
  *        ***   **        **         **              **               * *
  *        ***   **        **         **     *        **     *         **
  *         **   **        **  *      **     *        **     *         **
  *         **    **     ****  *       **   *          **   *          *
*****        *     ******   ***         ****            ****           *
                                                                       *
                                                                      *
                                                                  *****
                                                                  ****


 *
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "NattyProtoClient.h"
#include "NattyProtocol.h"



void ntyUserRecvCb(int len) {
	int i = 0;
	U8 *buffer = ntyGetRecvBuffer();

	for (i = 0;i < len;i ++) {
		//ntydbg(" devid:%lld, len:%d, %s ", devid, len, buffer);
		ntydbg( "receive data:buffer[%d]=%c\n",i,buffer[i] );
	}
	ntydbg("\n");
}

void ntySendSuccess(int arg) {
	ntydbg("Success\n");
}

void ntySendFailed(int arg) {
	if (arg == STATUS_TIMEOUT)
		ntydbg("STATUS_TIMEOUT\n");
	else if (arg == STATUS_NOEXIST)
		ntydbg("STATUS_NOEXIST\n");
}

void ntyDisconnect(int arg) {
	ntydbg("ntyDisconnect\n");
}

void ntyReconnected(int arg) {
	ntydbg("ntyReconnected\n");
}

void ntyBindResult(int arg) {
	ntydbg(" ntyBindResult --> arg: %d\n", arg);
}

void ntyUnBindResult(int arg) {
	ntydbg(" ntyUnBindResult --> arg: %d\n", arg);
}

void ntyPacketRecv(DEVID fromId, DEVID gId, int arg) { //voice data recv success, arg: length of voice data
	ntydbg(" ntyUnBindResult --> arg: %d\n", arg);

	U8 *pBuffer = ntyGetRecvBigBuffer();	//
}

void ntyPacketSuccess(int arg) {
	ntydbg(" ntyUnBindResult --> arg: %d\n", arg);
}

void ntyLoginAckResult(U8 *json, int length) {
	ntydbg(" ntyLoginAckResult:%s\n", json);
}

void ntyHeartBeatAckResult(int status) {
	ntydbg(" ntyLoginAckResult:%d\n", status);
}

void ntyLogoutAckResult(int status) {
	ntydbg(" ntyLogoutAckResult:%d\n", status);
}

void ntyTimeAckResult(U8 *json, int length) {
	
}

void ntyICCIDAckResult(U8 *json, int length) {
	
}

void ntyCommonReqResult(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyCommonReqResult : %lld\n", fromId);
	ntydbg(" ntyCommonReqResult : %s\n", json);
}

void ntyCommonAckResult(U8 *json, int length) {
	
}

void ntyVoiceDataAckResult(int status) {
	//voice data send success
	ntydbg(" ntyVoiceDataAckResult: %d\n", status);
}

void ntyOfflineMsgAckResult(U8 *json, int length) {
	ntydbg(" ntyOfflineMsgAckResult:%s\n", json);
}

void ntyLocationPushResult(U8 *json, int length) {
	ntydbg(" ntyLocationPushResult:%s\n", json);
}

void ntyWeatherPushResult(U8 *json, int length) {
	ntydbg(" ntyWeatherPushResult:%s\n", json);
}

void ntyDataRoute(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyDataRoute:%s\n", json);
}

void ntyDataResult(U8 *json, int length) {
	ntydbg(" ntyDataResult:%d\n", length);
}

void ntyUserDataAck(U8 *json, int length) {
	ntydbg(" ntyUserDataAck: %s , %d\n",json, length);
}


void ntyVoiceBroadCastResult(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyVoiceBroadCastResult:%s\n", json);
}

void ntyLocationBroadCastResult(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyLocationBroadCastResult:%s\n", json);
}

void ntyCommonBoradCastResult(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyCommonBoradCastResult:%s\n", json);
}


void ntyLocationPush(U8 *arg, int length) {
	ntydbg(" ntyPush --> length: %s\n", arg);
}

void ntyBindConfirmResult(DEVID fromId, U8 *json, int length) {
	ntydbg(" ntyBindConfirmResult:%s\n", json);
}



//DEVID g_devid = 0x352315052834187;
//DEVID g_devid = 0x355637050066828;
//DEVID g_devid = 0x355637052238805;
//DEVID g_devid = 0x355637053172771;
DEVID g_devid = 0x8661040260208520;

int main() {
	//DEVID AppId = 13579;
	DEVID AppId = 700010001;
	DEVID aid = 0;
	
	int n = 0, length, i, result = 0;
	int ch;
	U8 tempBuf[CLIENT_BUFFER_SIZE] = {0};
	const char *url =  "GET http://apilocate.amap.com/position?accesstype=1&imei=352315052834187&macs=30:FC:68:B9:E6:E6,-48,TP-LINK_E6E6_JUGUI|50:BD:5F:00:5F:62,-79,TP-LINK_5F62|28:10:7B:F6:E1:BA,-80,joshen?D|C0:61:18:90:1D:64,-86,ttt222????|D4:EE:07:3E:C8:8A,-87,HiWiFi_3EC88A|&output=xml&key=81040f256992a218a8a20ffb7f13ba9f HTTP/1.1";
	
	//ntydbg(" Press DevId <1 or 2>: ");   	
	//n = scanf("%lld", &AppId);
	
	ntySetProxyCallback(ntyUserRecvCb);
	ntySetSendFailedCallback(ntySendFailed);
	ntySetSendSuccessCallback(ntySendSuccess);
	ntySetProxyDisconnect(ntyDisconnect);
	ntySetProxyReconnect(ntyReconnected);
	ntySetBindResult(ntyBindResult);
	ntySetUnBindResult(ntyUnBindResult);
	ntySetPacketRecv(ntyPacketRecv);
	ntySetPacketSuccess(ntyPacketSuccess);
	ntySetLoginAckResult(ntyLoginAckResult);
	ntySetLocationPushResult(ntyLocationPush);
	ntySetDataResult(ntyDataResult);
	ntySetUserDataAckResult(ntyUserDataAck);
	
	//ntySetDevId(did);
	//ntySetDevId(AppId);
	ntySetDevId(g_devid);
	
	
	ntyStartupClient(&result);
	sleep(10);
	
	//ntyBindClient(0xEDFF12342345613);
	//ntyUnBindClient(0xEDFF12342345613);
#if 0
	int count = 0;
	DEVID *list = ntyGetFriendsList(&count);
	for (i = 0;i < count;i ++) {
		ntydbg(" %d --> %lld\n", i+1, *(list+i));
	}
	aid = *list;
	ntyReleaseFriendsList(&list);
#endif
	//while(1);
#if 1
	while(1) {
		ntydbg("Proxy Please send msg:\n");
#if 0
		char *ptr = fgets(tempBuf, CLIENT_BUFFER_SIZE, stdin);
#else
		/*
		//{"results":[{"location":{"id":"WX4FBXXFKE4F","name":"����??","country":"CN","path": "����??,����??,?D1��","timezone":"Asia/Shanghai","timezone_offset":"+08:00"},"daily":[{"date":"2017-02-15","text_day":"?��??","code_day":"4","text_night":"��?","code_night":"9","high":"8","low":"-1","precip":"","wind_direction":"??","wind_direction_degree":"180","wind_speed":"10","wind_scale":"2"},{"date":"2017-02-16","text_day":"?��??","code_day":"4","text_night":"?��??","code_night":"4","high":"7","low":"-4","precip":"","wind_direction":"?T3?D?��??��","wind_direction_degree":"","wind_speed":"20","wind_scale":"4"},{"date":"2017-02-17","text_day":"??","code_day":"0","text_night":"??","code_night":"0","high":"5","low":"-4","precip":"","wind_direction":"??","wind_direction_degree":"180","wind_speed":"10","wind_scale":"2"}],"last_update":"2017-02-15T18:00:00+08:00"}]}
		char *weatherStr = "{\"results\":[{\"location\":{\"id\":\"WX4FBXXFKE4F\",\"name\":\"����??\",\"country\":\"CN\",\"path\": \"����??,����??,?D1��\",\"timezone\":\"Asia/Shanghai\",\"timezone_offset\":\"+08:00\"},\"daily\":[{\"date\":\"2017-02-15\",\"text_day\":\"?��??\",\"code_day\":\"4\",\"text_night\":\"��?\",\"code_night\":\"9\",\"high\":\"8\",\"low\":\"-1\",\"precip\":\"\",\"wind_direction\":\"??\",\"wind_direction_degree\":\"180\",\"wind_speed\":\"10\",\"wind_scale\":\"2\"},{\"date\":\"2017-02-16\",\"text_day\":\"?��??\",\"code_day\":\"4\",\"text_night\":\"?��??\",\"code_night\":\"4\",\"high\":\"7\",\"low\":\"-4\",\"precip\":\"\",\"wind_direction\":\"?T3?D?��??��\",\"wind_direction_degree\":\"\",\"wind_speed\":\"20\",\"wind_scale\":\"4\"},{\"date\":\"2017-02-17\",\"text_day\":\"??\",\"code_day\":\"0\",\"text_night\":\"??\",\"code_night\":\"0\",\"high\":\"5\",\"low\":\"-4\",\"precip\":\"\",\"wind_direction\":\"??\",\"wind_direction_degree\":\"180\",\"wind_speed\":\"10\",\"wind_scale\":\"2\"}],\"last_update\":\"2017-02-15T18:00:00+08:00\"}]}";
		U16 weather_len = (U16)strlen(weatherStr);
		tempBuf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
		tempBuf[NTY_PROTO_DEVTYPE_IDX] = NTY_PROTO_CLIENT_WATCH;
		tempBuf[NTY_PROTO_PROTOTYPE_IDX] = 0x08;
		tempBuf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_WEATHER_ASYNCREQ;
		memcpy(tempBuf+4, &g_devid, sizeof(DEVID));
		memcpy(tempBuf+12, &weather_len, 2);
		memcpy(tempBuf+14, weatherStr, weather_len);
		*/

		/*
		char *electricFenceStr = "{\"results\":[{\"location\":{\"id\":\"WX4FBXXFKE4F\",\"name\":\"����??\",\"country\":\"CN\",\"path\": \"����??,����??,?D1��\",\"timezone\":\"Asia/Shanghai\",\"timezone_offset\":\"+08:00\"},\"daily\":[{\"date\":\"2017-02-15\",\"text_day\":\"?��??\",\"code_day\":\"4\",\"text_night\":\"��?\",\"code_night\":\"9\",\"high\":\"8\",\"low\":\"-1\",\"precip\":\"\",\"wind_direction\":\"??\",\"wind_direction_degree\":\"180\",\"wind_speed\":\"10\",\"wind_scale\":\"2\"},{\"date\":\"2017-02-16\",\"text_day\":\"?��??\",\"code_day\":\"4\",\"text_night\":\"?��??\",\"code_night\":\"4\",\"high\":\"7\",\"low\":\"-4\",\"precip\":\"\",\"wind_direction\":\"?T3?D?��??��\",\"wind_direction_degree\":\"\",\"wind_speed\":\"20\",\"wind_scale\":\"4\"},{\"date\":\"2017-02-17\",\"text_day\":\"??\",\"code_day\":\"0\",\"text_night\":\"??\",\"code_night\":\"0\",\"high\":\"5\",\"low\":\"-4\",\"precip\":\"\",\"wind_direction\":\"??\",\"wind_direction_degree\":\"180\",\"wind_speed\":\"10\",\"wind_scale\":\"2\"}],\"last_update\":\"2017-02-15T18:00:00+08:00\"}]}";
		U16 electricFence_len = (U16)strlen(electricFenceStr);
		tempBuf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
		tempBuf[NTY_PROTO_DEVTYPE_IDX] = NTY_PROTO_CLIENT_WATCH;
		tempBuf[NTY_PROTO_PROTOTYPE_IDX] = 0x04;
		tempBuf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_COMMON_REQ;
		memcpy(tempBuf+4, &g_devid, sizeof(DEVID));
		memcpy(tempBuf+12, &electricFence_len, 2);
		memcpy(tempBuf+14, electricFenceStr, electricFence_len);
		*/

		//1. Config2������
		//char *json_str_config = "{\"IMEI\":\"355637052788650\",\"Action\":\"Get\",\"Category\":\"Config\"}";
		//char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"Schedule\",\"Action\":\"Add\",\"Schedule\":{\"Daily\":\"Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday\",\"Time\":\"18:00:00\",\"Details\":\"chifan\"}}";
		//U16 json_len = (U16)strlen(json_str);
		//tempBuf[NTY_PROTO_VERSION_IDX] = NTY_PROTO_VERSION;
		//tempBuf[NTY_PROTO_DEVTYPE_IDX] = NTY_PROTO_CLIENT_WATCH;
		//tempBuf[NTY_PROTO_PROTOTYPE_IDX] = PROTO_ROUTE;
		//tempBuf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_COMMON_REQ;
		//tempBuf[NTY_PROTO_MSGTYPE_IDX] = NTY_PROTO_DATA_ROUTE;



		//char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"Efence\",\"Efence\":{\"Num\":\"3\",\"Points\":[\"113.2409402,23.1326885\",\"113.2409412,23.1326895\",\"113.2409408,23.1326890\"]}}";

		//char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"RunTime\",\"RunTime\":{\"AutoConnection\":\"1\",\"LossReport\":\"1\",\"LightPanel\":\"30\",\"WatchBell\":\"11,11\",\"TagetStep\":\"5000\"}}";

		//char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"Schedule\",\"Action\":\"Add\",\"Schedule\":{\"Daily\":\"Monday|Tuesday| Wednesday|Thursday|Friday|Saturday|Sunday\",\"Time\":\"18:00:00\",\"Details\":\"3?��1\"}}";
		
		char *json_str1 = "{\"IMEI\":\"355637052788650\",\"Category\":\"TimeTables\",\"TimeTables\":[{\"Daily\":\"Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday\",\"Morning\":{\"Status\":\"On\",\"StartTime\":\"08:00:00\",\"EndTime\":\"12:00:00\"},\"Afternoon\":{\"Status\":\"On\",\"StartTime\":\"13:00:00\",\"EndTime\":\"18:00:00\"}}]}";
		char *json_str = "{\"IMEI\":\"355637052788650\",\"Category\":\"LAB\",\"LAB\":{\"Bts\":\"460,01,40977,2205409,-65\",\"Nearbts\":[{\"Cell\":\"460,01,40977,2205409\",\"Signal\":\"-65\"},{\"Cell\":\"460,01,40977,2205409\",\"Signal\":\"-65\"},{\"Cell\":\"460,01,40977,2205409\",\"Signal\":\"-65\"}]}}";



#if 0
		U8 tempBuf2[4] = {0x01,0x01,0x01,0x01};
		memcpy(tempBuf+4, &g_devid, sizeof(DEVID));
		memcpy(tempBuf+12, tempBuf2, 4);
		memcpy(tempBuf+16, &g_devid, sizeof(DEVID));
		memcpy(tempBuf+24, &json_len, 2);
		memcpy(tempBuf+26, json_str, json_len);
		ntydbg(" json_str:%s\n", json_str);
		ntydbg(" tempBuf:%s\n", tempBuf+26);
		
#endif

		
#endif
#if 0
		if (ntyGetNetworkStatus() == -1) {
			ntydbg("Startup Client\n");
			ntyStartupClient();
			continue;
		}
		if (tempBuf[0] == 'c') {
			ntydbg("Shutdown Client\n");
			ntyShutdownClient();
			continue;
		}
#endif


		//memset(tempBuf, 0, RECV_BUFFER_SIZE);
		//memcpy(tempBuf, url, strlen(url));
		//int len = strlen(tempBuf+26);
		//ntydbg("tempBuf:%s, len:%d\n", tempBuf+26, len);
#if 0
		ntySendMassDataPacket(tempBuf, len-1);
#else
		//ntySendDataPacket(aid, tempBuf, len);
		//ntyCommonReqClient(g_devid, json_str, json_len);
		//ntyDataRouteClient(g_devid,json_str,json_len);

		//char *json_dataroute = "{\"IMEI\":\"352315052834187\",\"Action\":\"Get\",\"Category\":\"Config\"}";
		//ntydbg("Request DataRoute : %s\n", json_dataroute);
		//ntyDataRouteClient(g_devid,json_dataroute,strlen(json_dataroute));
		//sleep(20);
		//break;

		//char *json_iccid = "{\"IMEI\":\"352315052834187\",\"ICCID\":\"89860116425100650036\"}";
		//ntydbg("Request iccid  : %s\n", json_iccid);
		//ntyICCIDReqClient(g_devid, json_iccid, strlen(json_iccid));
		//sleep(20);
		//break;
		//char *json_common = "{\"Action\":\"Update\",\"Category\":\"Schedule\",\"IMEI\":\"355637053995130\",\"Id\":\"41\",\"Schedule\":{\"Daily\":\"Monday|Wednesday|Thursday|Sunday\",\"Details\":\"����\",\"Time\":\"17:30:0tion\":\",,255\"}}}";

		//char *json_common = "{\"Action\":\"Update\",\"Category\":\"Schedule\",\"IMEI\":\"355637053995130\",\"Id\":\"40\",\"Schedule\":{\"Daily\":\"Tuesday|Wednesday|Thursday|Saturday\",\"Details\":\"����\",\"Time\":\"17:28:00\"}}";
		//char *json_common = "{\"Category\":\"RunTime\",\"IMEI\":\"355637053995130\",\"RunTime\":{\"AutoConnection\":\"0\"}}";
		//char *json_common = "{\"IMEI\":\"355637050315845\",\"Category\":\"TimeTables\",\"TimeTables\":[{\"Daily\":\"Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday\",\"Morning\":{\"Status\":\"1\",\"StartTime\":\"08:00:00\",\"EndTime\":\"16:00:00\"},\"Afternoon\":{\"Status\":\"1\",\"StartTime\":\"14:00:00\",\"EndTime\":\"18:00:00\"}}]}";
		//char *json_common = "{\"IMEI\":\"355637052788650\",\"Category\":\"TimeTables\",\"TimeTables\":[{\"Daily\":\"Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday\",\"Morning\":{\"Status\":\"1\",\"StartTime\":\"08:00:00\",\"EndTime\":\"12:00:00\"},\"Afternoon\":{\"Status\":\"1\",\"StartTime\":\"13:00:00\",\"EndTime\":\"18:00:00\"}}]}";
		//char *json_common = "{\"IMEI\":\"352315052834187\",\"Category\":\"Turn\",\"Turn\":{\"Status\":\"0\",\"On\":{\"Time\":\"08:00:00\"},\"Off\":{\"Time\":\"18:00:00\"}}}";
		//char *json_common="{\"Schedule\":{\"Daily\":\"Wednesday|Thursday|Friday|Saturday\",\"Details\":\"%E4%BE%A7%E9%9D%A2\",\"Time\":\"21:14:14\"},\"Category\":\"Schedule\",\"IMEI\":\"352315052834187\",\"Action\":\"Update\",\"Id\":\"94\"}";
		//char *json_common = "{\"IMEI\":\"352315052834187\",\"Category\":\"Schedule\",\"Action\":\"Add\",\"Schedule\":{\"Daily\":\"Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday\",\"Time\":\"18:00:00\",\"Details\":\"3\"}}";

		//char *json_common = "{\"Action\":\"Add\",\"Category\":\"Contacts\",\"Contacts\":{\"Image\":\"8\",\"Name\":\"%E9%BB%84%E6%B5%B7\",\"Tel\":\"13695287465\"},\"IMEI\":\"355637053995130\"}";
		//char *json_common = "{\"Contact\":{\"Tel\":\"15218293347\",\"Name\":\"%E5%A4%96%E5%85%AC\",\"Image\":\"4\"},\"Category\":\"Contacts\",\"Action\":\"Add\",\"IMEI\":\"352315052834187\"}}";

		
		//char *json_common = "{\"IMEI\":\"352315052834187\",\"Category\":\"Schedule\",\"Action\":\"Add\",\"Schedule\":{\"Daily\":\"Monday|Tuesday| Wednesday|Thursday|Friday|Saturday|Sunday\",\"Time\":\"18:00:00\",\"Details\":\"aasdasd\"}}";
		//char *json_common = "{\"IMEI\":\"355637053995130\",\"Action\":\"Delete\",\"Category\":\"Efence\",\"Index\":\"1\"}";
		//char *json_common = "{\"RunTime\":{\"LossReport\":\"0\"},\"IMEI\":\"355637053995130\",\"Category\":\"RunTime\"}";
		//char *json_common = "{\"RunTime\":{\"TagetStep\":\"14725\"},\"IMEI\":\"355637053995130\",\"Category\":\"RunTime\"}";
		//char *json_common = "{\"Action\":\"Update\",\"Category\":\"Schedule\",\"IMEI\":\"355637053995130\",\"Id\":\"48\",\"Schedule\":{\"Daily\":\"Monday|Wednesday|Thursday|Friday|Saturday\",\"Details\":\"%E5%85%AC%E5%8F%B8%E8%B5%B7%E6%88%91\",\"Status\":\"0\",\"Time\":\"14:33:00\"}}";
		//char *json_common = "{\"IMEI\":\"352315052834187\",\"Category\":\"Turn\",\"Turn\":{\"Status\":\"1\",\"On\":{\"Time\":\"08:00:00\"},\"Off\":{\"Time\":\"18:00:00\"}}}";

		//char *json_common = "{\"Action\":\"Add\",\"Category\":\"Contacts\",\"Contacts\":{\"Image\":\"0\",\"Name\":\"%E7%88%B8%E7%88%B8\",\"Tel\":\"15842754258\"},\"IMEI\":\"355637052238805\"}";

		//char *json_common = "{\"Category\":\"TimeTables\",\"IMEI\":\"355637053995130\",\"TimeTables\":[{\"Afternoon\":{\"EndTime\":\"19:51\",\"StartTime\":\"16:50\",\"Status\":\"1\"},\"Daily\":\"Monday|Wednesday\",\"Morning\":{\"EndTime\":\"09:00\",\"StartTime\":\"08:00\",\"Status\":\"0\"}}]}";
		//char *json_common = "{\"Action\":\"Add\",\"Category\":\"Contacts\",\"Contacts\":{\"Image\":\"7\",\"Name\":\"%E5%A7%90%E5%A7%90\",\"Tel\":\"13652847152\"},\"IMEI\":\"355637053995130\"}";
		//char *json_common = "{\"Category\":\"TimeTables\",\"IMEI\":\"355637053995130\",\"TimeTables\":[{\"Afternoon\":{\"EndTime\":\"19:51\",\"StartTime\":\"16:50\",\"Status\":\"1\"},\"Daily\":\"Monday|Wednesday\",\"Morning\":{\"EndTime\":\"09:00\",\"StartTime\":\"08:00\",\"Status\":\"1\"}}]}";
		//char *json_common = "{\"Category\":\"Contacts\",\"Action\":\"Update\",\"IMEI\":\"355637053995130\",\"Contacts\":{\"Userid\":\"11299\",\"Id\":\"229\",\"Tel\":\"15889650380\",\"Admin\":0,\"Name\":\"%E5%B9%B2%E7%88%B9\",\"Image\":\"http:\/\/picture.quanjiakan.com:9080\/quanjiakan\/resources\/missing\/20170329115531_83i472.png\",\"App\":1}}";
		//char *json_common = "{\"IMEI\":\"355637052788650\",\"Category\":\"Efence\",\"Action\":\"Add\", \"Index\":\"1\",\"Efence\":{\"Num\":\"3\",\"Points\":[\"113.2409402,23.1326885\",\"113.2409412,23.1326895\",\"113.2409408,23.1326890\"]}}";

		//char *json_common = "{\"Results\":{\"IMEI\":\"355637052238805\",\"Category\":\"WearStatus\",\"WearStatus\":\"On\"}}";
		//ntyCommonReqClient(g_devid,json_common,strlen(json_common));
		//sleep(30);

		//char *json_common = "{\"IMEI\":\"355637052788450\",\"Category\":\"AuthorizeReply\",\"AuthorizeType\":\"HealthArchives\",\"Answer\":\"Agree\"}";
		//ntyCommonReqClient(AppId,json_common,strlen(json_common));
		//sleep(30);
		char *json_monitor = "{\"IMEI\":\"866104026020852\",\"Category\":\"Mattress\",\"Action\":\"1\"}";
		ntydbg("send msg:%s\n",json_monitor);
		ntyMonitorSleepReqClient(g_devid,json_monitor,strlen(json_monitor));
		sleep(20);
		sleep(100);

		char *json_bloodreport = "{\"Results\":{\"IMEI\":\"352315052834187\",\"Category\":\"BloodReport\",\"BloodReport\":\"86\",\"Type\":\"WIFI\",\"Location\":\"113.2418077,23.1313968\"}}";
		ntydbg("send msg:%s\n",json_bloodreport);
		ntyCommonReqClient(g_devid,json_bloodreport,strlen(json_bloodreport));
		sleep(20);
		sleep(100);

		char *json_heartreport = "{\"Results\":{\"IMEI\":\"352315052834187\",\"Category\":\"HeartReport\",\"HeartReport\":\"58\",\"Type\":\"WIFI\",\"Location\":\"113.2418077,23.1313968\"}}";
		ntydbg("send msg:%s\n",json_heartreport);
		ntyCommonReqClient(g_devid,json_heartreport,strlen(json_heartreport));
		sleep(100);
		
		//char *json_userdata = "{\"IMEI\":\"355637052329596\",\"Category\":\"ICCID\",\"Action\":\"Set\",\"ICCID\":\"1642510065118\",\"PhoneNum\":\"15889650380\"}";
		//ntyUserDataReqClient(json_userdata, strlen(json_userdata));
		//sleep(30);

/*
		char *json_common = "{\"IMEI\":\"352315052834187\",\"Category\":\"Schedule\",\"Action\":\"Select\"}";
		char *json_contacts = "{\"IMEI\":\"352315052834187\",\"Category\":\"Contacts\",\"Action\":\"Select\"}";
		char *json_turn = "{\"IMEI\":\"352315052834187\",\"Category\":\"Turn\",\"Action\":\"Select\"}";
		char *json_runtime = "{\"IMEI\":\"352315052834187\",\"Category\":\"RunTime\",\"Action\":\"Select\"}";
		char *json_timetables = "{\"IMEI\":\"352315052834187\",\"Category\":\"TimeTables\",\"Action\":\"Select\"}";
		char *json_efence = "{\"IMEI\":\"352315052834187\",\"Category\":\"Efence\",\"Action\":\"Select\"}";
		char *json_location = "{\"IMEI\":\"352315052834187\",\"Category\":\"Location\",\"Action\":\"Select\"}";
		ntydbg("send msg:%s\n",json_common);
		ntyUserDataReqClient(json_common,strlen(json_common));
		sleep(5);
		ntydbg("send msg:%s\n",json_contacts);
		ntyUserDataReqClient(json_contacts,strlen(json_contacts));
		sleep(5);
		ntydbg("send msg:%s\n",json_turn);
		ntyUserDataReqClient(json_turn,strlen(json_turn));
		sleep(5);
		ntydbg("send msg:%s\n",json_runtime);
		ntyUserDataReqClient(json_runtime,strlen(json_runtime));
		sleep(5);
		ntydbg("send msg:%s\n",json_timetables);
		ntyUserDataReqClient(json_timetables,strlen(json_timetables));	
		sleep(5);
		ntydbg("send msg:%s\n",json_efence);
		ntyUserDataReqClient(json_efence,strlen(json_efence));
		sleep(5);
		ntydbg("send msg:%s\n",json_efence);
		ntyUserDataReqClient(json_location,strlen(json_location));
*/

		//char *json_url = "{\"IMEI\":\"352315052834187\",\"Category\":\"URL\",\"Action\":\"Select\"}";
		//ntydbg("send msg:%s\n",json_url);
		//ntyUserDataReqClient(json_url,strlen(json_url));

		
		

		//char *json_location = "{\"IMEI\":\"355637052788650\",\"Category\":\"LAB\",\"LAB\":{\"Bts\":\"460,01,40977,2205409,-65\",\"Nearbts\":[{\"Cell\":\"460,01,40977,2205409\",\"Signal\":\"-65\"},{\"Cell\":\"460,01,40977,2205409\",\"Signal\":\"-65\"},{\"Cell\":\"460,01,40977,2205409\",\"Signal\":\"-65\"}]}}";
		//char *json_location = "{\"IMEI\":\"135790246811220\",\"Category\":\"WIFI\",\"WIFI\":[{\"SSID\":\"TP-LINK_42ED\",\"MAC\":\"F4:83:CD:3A:42:ED\",\"V\":\"-37\"},{\"SSID\":\"EXT\",\"MAC\":\"04:95:E6:22:DA:48\",\"V\":\"-52\"}]}";
		//ntydbg("%s\n", json_location);
		//ntyLocationReqClient(g_devid, json_location, strlen(json_location));
		//sleep(20);

		//char *json_weather = "{\"IMEI\":\"355637052788650\",\"Category\":\"Weather\",\"Bts\":\"460,01,40977,2205409,-65\"}";
		//ntydbg("Request Weather : %s\n", json_weather);
		//ntyWeatherReqClient(g_devid, json_weather, strlen(json_weather));
		//sleep(20);


		//char *json_bindreq = "{\"IMEI\":\"355637052788650\",\"Category\":\"Weather\",\"Bts\":\"460,01,40977,2205409,-65\"}";
		//char *json_bindreq = "{\"BindReq\":{\"WatchName\":\"%E6%B5%8B%E8%AF%95\",\"UserName\":\"%E5%93%A5%E5%93%A5\",\"WatchImage\":\"http:\/\/picture.quanjiakan.com:9080\/quanjiakan\/resources\/device\/20170327175708_fcko80.png\",\"UserImage\":\"7\"},\"IMEI\":\"352315052834187\",\"Category\":\"BindReq\"}";
		//ntyCommonReqClient(g_devid,json_bindreq,strlen(json_bindreq));
		//sleep(30);

		/*
		char *json_bindreq = "{\"IMEI\":\"355637053837001\",\"Category\":\"BindConfirmReq\",\"Answer\":\"Agree\",\"MsgId\":\"2\"}";
		long long proposerId = 10833;
		long long devId = 0x355637053837001; 
		U32 msgId = 2;
		ntyBindConfirmReqClient(proposerId, devId, msgId, json_bindreq, strlen(json_bindreq));
		*/
		
		//char *json_bindreq = " {\"IMEI\":\"355637050957620\",\"Category\":\"BindReq\",\"BindReq\":{\"WatchName\":\"nihs\",\"WatchImage\":\"http:\\/\\/picture.quanjiakan.com:9080\\/quanjiakan\\/resources\\/device\\/20170408162703_4ezfc34wzqi5d3kcxqju.png\",\"UserName\":\"%E7%88%B7%E7%88%B7\",\"UserImage\":\"3\"}}";
		//int length = strlen(json_bindreq);

		//ntydbg(" length : %d\n", length);
		//ntyBindClient(g_devid, json_bindreq, strlen(json_bindreq));
		//sleep(30);


		

		break;
		//sleep(3000);
#endif
	}
#endif
	getchar();
}


