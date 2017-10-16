/*#include <vxWorks.h>
#include <wdLib.h>
#include <stdio.h>
#include <semLib.h> */
#include "OAM_gw.h"
#include "../EPONV2R1/include/V2R1General.h"
int OAM_TEST_ENABLE = 1;
char V2R1EponCli[][50] = {
					/*"igmp",*/
					"igmpsnooping",
					"igmpsnooping ver",
					
					"igmpsnooping enable all",
					"igmpsnooping enable 2 0",
					"igmpsnooping enable 2",
					"igmpsnooping enable 2 1",
					"igmpsnooping enable 2",					

					"igmpsnooping enable 1 0",
					"igmpsnooping enable 1",
					"igmpsnooping enable 1 1",
					"igmpsnooping enable 1",	

					"igmpsnooping enable 3 0",
					"igmpsnooping enable 3",
					"igmpsnooping enable 3 1",
					"igmpsnooping enable 3",	

					"igmpsnooping enable 4 0",
					"igmpsnooping enable 4",
					"igmpsnooping enable 4 1",
					"igmpsnooping enable 4",	

					/**/
					/*"igmpsnooping channel",
					"igmpsnooping channel 16",
					"igmpsnooping channel",
					"igmpsnooping channel 1",
					"igmpsnooping channel",
					
					"igmpsnooping aging",
					"igmpsnooping aging 239",
					"igmpsnooping aging",
					"igmpsnooping aging 15",
					"igmpsnooping aging",
					"igmpsnooping aging 3825",
					"igmpsnooping aging",
					"igmpsnooping aging 543",
					"igmpsnooping aging",					
					"igmpsnooping aging 0",
					"igmpsnooping aging",*/
					

					/*"igmpsnooping gda_add <addr> [<port_list>]",
					"igmpsnooping gda_del <addr>",*/
					"igmpsnooping gda_add 0100.5e00.0501 1-4",
					"igmpsnooping gda_show",
					"igmpsnooping gda_add 0100.5e00.0502 1,4,3",
					"igmpsnooping gda_show",
					/*"igmpsnooping gda_add 0100.5e00.0503",
					"igmpsnooping gda_show",	*/				
										
					/*"igmpsnooping last_member",
					"igmpsnooping last_member 1",
					"igmpsnooping last_member",
					"igmpsnooping last_member 0",
					"igmpsnooping last_member",	

					"igmpsnooping param qry_cnt",
					"igmpsnooping param qry_cnt 3",
					"igmpsnooping param qry_cnt",
					"igmpsnooping param qry_cnt 6",
					"igmpsnooping param qry_cnt",

					"igmpsnooping param lm_q_cnt",
					"igmpsnooping param lm_q_cnt 1",
					"igmpsnooping param lm_q_cnt",
					"igmpsnooping param lm_q_cnt 8",
					"igmpsnooping param lm_q_cnt",

					"igmpsnooping param lm_resp_t",
					"igmpsnooping param lm_resp_t 1",
					"igmpsnooping param lm_resp_t",
					"igmpsnooping param lm_resp_t 3",
					"igmpsnooping param lm_resp_t",
					"igmpsnooping param lm_resp_t 2",
					"igmpsnooping param lm_resp_t",*/
					/*vlan*/
					"vlan",
					"vlan dot1q",
					"vlan dot1q 1",
					"vlan dot1q",
					"vlan dot1q 0",
					"vlan dot1q",
					"vlan dot1q 1",
					"vlan dot1q",

					"vlan pvid 0",
					"vlan pvid 1",
					"vlan pvid 2",
					"vlan pvid 3",
					"vlan pvid 4",
					"vlan pvid 1 1022",
					"vlan pvid 1",
					"vlan pvid 2 1011",
					"vlan pvid 2",
					"vlan pvid 3 434",
					"vlan pvid 3",
					"vlan pvid 4 567",
					"vlan pvid 4",
					
					"vlan acceptable_frame_types 0",
					"vlan acceptable_frame_types 1",
					"vlan acceptable_frame_types 2",
					"vlan acceptable_frame_types 3",
					"vlan acceptable_frame_types 4",
					"vlan acceptable_frame_types 1 all",
					"vlan acceptable_frame_types 1",
					"vlan acceptable_frame_types 1 tagged",
					"vlan acceptable_frame_types 1",
					"vlan acceptable_frame_types 2 all",
					"vlan acceptable_frame_types 2",
					"vlan acceptable_frame_types 2 tagged",
					"vlan acceptable_frame_types 2",

					"vlan acceptable_frame_types 3 all",
					"vlan acceptable_frame_types 3",
					"vlan acceptable_frame_types 3 tagged",
					"vlan acceptable_frame_types 3",

					"vlan acceptable_frame_types 4 all",
					"vlan acceptable_frame_types 4",
					"vlan acceptable_frame_types 4 tagged",
					"vlan acceptable_frame_types 4",

					"vlan ingress_filtering 0",
					"vlan ingress_filtering 1",
					"vlan ingress_filtering 2",
					"vlan ingress_filtering 3",
					"vlan ingress_filtering 4",
					"vlan ingress_filtering 1 0",
					"vlan ingress_filtering 1",
					"vlan ingress_filtering 1 1",
					"vlan ingress_filtering 1",

					"vlan ingress_filtering 2 0",
					"vlan ingress_filtering 2",
					"vlan ingress_filtering 2 1",
					"vlan ingress_filtering 2",					

					"vlan ingress_filtering 3 0",
					"vlan ingress_filtering 3",
					"vlan ingress_filtering 3 1",
					"vlan ingress_filtering 3",

					/*"vlan dot1q_add 1",
					"vlan dot1q_add 234",
					"vlan dot1q_show",
					"vlan dot1q_add 2",
					"vlan dot1q_show",
					"vlan dot1q_add 23",
					"vlan dot1q_show",

					"vlan dot1q_del 234",
					"vlan dot1q_show",
					"vlan dot1q_del 23",
					"vlan dot1q_show",*/

					"vlan port_isolate",
					"vlan port_isolate 1",
					"vlan port_isolate",
					"vlan port_isolate 0",
					"vlan port_isolate",

					/*atu*/
					"atu",
					/*"atu size",
					"atu size 1",
					"atu size",
					"atu size 0",
					"atu size",
					"atu size 2",
					"atu size",*/

					"atu aging",
					"atu aging 15",
					"atu aging",
					"atu aging 300",
					"atu aging",
					"atu aging 9630",
					"atu aging",
					"atu aging 0",
					"atu aging",

					"atu static_add 0000.3455.1234 0 1", 
					"atu show static",
					"atu static_add 0000.3455.1234 0 1,2,3,4",
					"atu show",
					"atu static_add 0000.3455.1235 0 1-3",
					"atu show",
					"atu static_add 0100.5e05.2345 1 1",
					"atu show 1",
					"atu static_add 0000.5e05.2345 1 1",
					"atu show",
					"atu static_add 0000.3455.1236 1 1",
					"atu show",					

					"atu static_del 0000.5e05.2345",
					"atu show",
					"atu static_del 0000.3455.1234",
					"atu show",
					"atu static_del 0000.3455.1236",
					"atu show",

					/*event*/
					/*"event oam_alarm sym",
					"event oam_alarm sym 1 10e-8 10e-10",
					"event oam_alarm sym",
					"event oam_alarm sym 0 10e-10 10e-3",
					"event oam_alarm sym",
					
					"event oam_alarm frm",
					"event oam_alarm frm 1 0xefff000 0x11110000",
					"event oam_alarm frm",
					"event oam_alarm frm 0 0xefff000 0x11110000",
					"event oam_alarm frm",
					
					"event oam_alarm frp",
					"event oam_alarm frp 1 0x7777000 0x88889900",
					"event oam_alarm frp",
					"event oam_alarm frp 0 0x7777000 0x88889900",
					"event oam_alarm frp",
					
					"event oam_alarm sec",
					"event oam_alarm sec 1 0x6662 0x0212",
					"event oam_alarm sec",
					"event oam_alarm sec 0 0x6662 0x0212",
					"event oam_alarm sec",
					
					"event show 5",
					"event show"*/

					/*bandwidth----------------------------------------*/
					"bandwidth set up 0",
					"bandwidth set up 1",
					"bandwidth set up 2",
					"bandwidth set up 3",
					"bandwidth set up 4",
					"bandwidth set up 5",
					"bandwidth set up 6",
					"bandwidth set up 7",
					"bandwidth set up 0 1 10 100000",
					"bandwidth set up 0",
					"bandwidth set up 0 0 10 100000",
					"bandwidth set up 0",					
					"bandwidth set up 1 1 20 200000",
					"bandwidth set up 1",
					"bandwidth set up 1 0 20 200000",
					"bandwidth set up 1",					
					"bandwidth set up 2 1 30 300000",
					"bandwidth set up 2",
					"bandwidth set up 2 0 30 300000",
					"bandwidth set up 2",	
					"bandwidth set up 3 1 40 400000",
					"bandwidth set up 3",
					"bandwidth set up 3 0 40 400000",
					"bandwidth set up 3",	

					"bandwidth set up 4 1 50 500000",
					"bandwidth set up 4",
					"bandwidth set up 4 0 50 500000",
					"bandwidth set up 4",
					"bandwidth set up 5 1 60 600000",
					"bandwidth set up 5",
					"bandwidth set up 5 0 60 600000",
					"bandwidth set up 5",
					"bandwidth set up 6 1 70 700000",
					"bandwidth set up 6",
					"bandwidth set up 6 0 70 700000",
					"bandwidth set up 6",
					"bandwidth set up 7 1 80 800000",
					"bandwidth set up 7",
					"bandwidth set up 7 0 80 800000",
					"bandwidth set up 7",	

					"bandwidth broadcast 1",
					"bandwidth broadcast 1 0",
					"bandwidth broadcast 1",
					"bandwidth broadcast 1 50",
					"bandwidth broadcast 1",
					"bandwidth broadcast 1 100",
					"bandwidth broadcast 1",

					
					
					/*FE port*/
					"port",
					"port en 1",
					"port en 1 0",
					"port en 1",
					"port en 1 1",
					"port en 1",
					"port en 2 0",
					"port en 2",
					"port en 2 1",
					"port en 2",
					"port en 3 0",
					"port en 3",
					"port en 3 1",
					"port en 3",
					"port en 4 0",
					"port en 4",
					"port en 4 1",
					"port en 4",
					
					"port mode 1",
					"port mode 1 8",
					"port mode 1",
					"port mode 1 9",
					"port mode 1",
					"port mode 1 10",
					"port mode 1",
					"port mode 1 11",
					"port mode 1",
					"port mode 1 0",
					"port mode 1",					
					
					"port fc 1 0"
					"port fc 1",
					"port fc 1 1",
					"port fc 1",
					"port fc 2 0",
					"port fc 2",
					"port fc 2 1",
					"port fc 2",
					"port fc 3 0",
					"port fc 3",
					"port fc 3 1",
					"port fc 3",
					"port fc 4 0",
					"port fc 4",
					"port fc 4 1",
					"port fc 4",
					
					"port rate 1 2 10",
					"port rate 1 2",
					"port rate 1 1 8",
					"port rate 1 1",
					"port rate 2 2 7",
					"port rate 2 2",
					"port rate 2 1 6",
					"port rate 2 1",
					"port rate 3 2 5",
					"port rate 3 2",
					"port rate 3 1 10",
					"port rate 3 1",
					"port rate 4 2 20",
					"port rate 4 2",
					"port rate 4 1 10",
					"port rate 4 1",
					
					"port maclimit 1 8",
					"port maclimit 1",
					"port maclimit 1 1",
					"port maclimit 1",
					"port maclimit 1 16",
					"port maclimit 2",
					"port maclimit 2 5",
					"port maclimit 2",
					"port maclimit 2 1",
					"port maclimit 2",
					"port maclimit 2 16",
					"port maclimit 2",
					"port maclimit 3 4",
					"port maclimit 3",
					"port maclimit 3 1",
					"port maclimit 3",
					"port maclimit 3 16",
					"port maclimit 3",
					"port maclimit 4 8",
					"port maclimit 4",
					"port maclimit 4 1",
					"port maclimit 4",
					"port maclimit 4 16",
					"port maclimit 4",
					
					
					/*Qos*/
					"qos",
					"qos def_pri 3 5",
					"qos def_pri 3",
					"qos def_pri 3 0",
					"qos def_pri 3",
					"qos def_pri 3 7",
					"qos def_pri 3",
					"qos def_pri 1 1",
					"qos def_pri 1",
					"qos def_pri 1 2",
					"qos def_pri 1",
					"qos def_pri 1 3",
					"qos def_pri 1",
					"qos def_pri 2 4",
					"qos def_pri 2",
					"qos def_pri 2 5",
					"qos def_pri 2",
					"qos def_pri 2 6",
					"qos def_pri 2",
					"qos def_pri 4 7",
					"qos def_pri 4",
					"qos def_pri 4 0",
					"qos def_pri 4",
					"qos def_pri 4 1",
					"qos def_pri 4",

					/*"qos def_port_tc 2 3",
					"qos def_port_tc 2",
					"qos def_port_tc 2 0",
					"qos def_port_tc 2",
					"qos def_port_tc 2 2",
					"qos def_port_tc 2",
					"qos def_port_tc 1 3",
					"qos def_port_tc 1",
					"qos def_port_tc 1 0",
					"qos def_port_tc 1",
					"qos def_port_tc 1 1",
					"qos def_port_tc 1",
					"qos def_port_tc 3 3",
					"qos def_port_tc 3",
					"qos def_port_tc 3 0",
					"qos def_port_tc 3",
					"qos def_port_tc 3 1",
					"qos def_port_tc 3",	
					"qos def_port_tc 4 2",
					"qos def_port_tc 4",
					"qos def_port_tc 4 0",
					"qos def_port_tc 4",
					"qos def_port_tc 4 1",
					"qos def_port_tc 4",	*/				
					
					"qos user_pri_reg 1 0",
					"qos user_pri_reg 1 0 7",
					"qos user_pri_reg 1 0",
					"qos user_pri_reg 1 0 0",
					"qos user_pri_reg 1 0",
					"qos user_pri_reg 2 0 6",
					"qos user_pri_reg 2 0",
					"qos user_pri_reg 2 1 5",
					"qos user_pri_reg 2 1",
					"qos user_pri_reg 3 3 4",
					"qos user_pri_reg 3 3",
					"qos user_pri_reg 4 5 6",
					"qos user_pri_reg 4 5",
					
					
					"qos user_pri_tc  3 1",
					"qos user_pri_tc  3",
					"qos user_pri_tc  3 0",
					"qos user_pri_tc  3",
					"qos user_pri_tc  0 3",
					"qos user_pri_tc  0",
					"qos user_pri_tc  3 0",
					"qos user_pri_tc  3",
					"qos user_pri_tc  4 1",
					"qos user_pri_tc  4",
					"qos user_pri_tc  5 2",
					"qos user_pri_tc  5",
					"qos user_pri_tc  6 3",
					"qos user_pri_tc  6",
					"qos user_pri_tc  7 0",
					"qos user_pri_tc  7",
					
					"qos dscp_tc  6 3",
					"qos dscp_tc  6",
					"qos dscp_tc  1 0",
					"qos dscp_tc  1",
					"qos dscp_tc  1 1",
					"qos dscp_tc  1",
					"qos dscp_tc  2 2",
					"qos dscp_tc  2",
					"qos dscp_tc  3 3",
					"qos dscp_tc  3",
					"qos dscp_tc  4 1",
					"qos dscp_tc  4",
					"qos dscp_tc  6 3",
					"qos dscp_tc  6",
					"qos dscp_tc  7 0",
					"qos dscp_tc  50",
					"qos dscp_tc  51 1",
					"qos dscp_tc  61",
					"qos dscp_tc  32 2",
					"qos dscp_tc  42",
					"qos dscp_tc  53 3",
					"qos dscp_tc  23",
					"qos dscp_tc  34 1",
					"qos dscp_tc  14",	
					
					"qos algorithm",
					"qos algorithm wrr", 
					"qos algorithm",
					"qos algorithm spq", 
					"qos algorithm",
					

					
					/*stat*/
					"stat",
					/*"stat port_histogram 1",
					"stat port_histogram",
					"stat port_histogram 0",
					"stat port_histogram",
					"stat port_histogram 2",
					"stat port_histogram",*/
					
					"stat port_flush 2",
					"stat port_show 2 4",
					"stat port_flush 3",
					"stat port_show 3",
					"stat port_flush 1",
					"stat port_show 1",
					"stat port_flush 4",
					"stat port_show 4",
					"stat port_flush",


					"mgt laser",
					"mgt laser 1",
					"mgt laser",
					"mgt laser 0",
					"mgt laser",

					"mgt self_check_result",
					"mgt self_check_result 1",
					"mgt self_check_result 2",
					"mgt self_check_result 3",
					"mgt self_check_result 4",
					"mgt self_check_result 5",
					"mgt self_check_result 6",
					"mgt self_check_result 7",
					"mgt self_check_result 8",
					"mgt self_check_result 9",
					"mgt self_check_result 10",
					"mgt self_check_result 11",
					"mgt self_check_result 12",
					"mgt self_check_result 13",
					"mgt self_check_result 14",
					"mgt self_check_result 15",
					
					
					
					
					/*mgt*/
					/*"mgt temperature",
					"mgt temperature 49",

					"mgt config save",
					"mgt config clear",



					"mgt reset def",
					"mgt reset"*/
					/*"NULL",*/
					/*loopback*/
					"loop",
					
					"loopback port_en 1 1",
					"loopback port_en 1",
					"loopback port_en 1 0",
					"loopback port_en 1",
					"loopback port_en 4 1",
					"loopback port_en 4",
					"loopback port_en 4 0",
					"loopback port_en 4",					
					};
char V2R1EponClistats[][50] = {
							"stat port_show 3 0",
							"stat port_show 3 1",
							"stat port_show 3 2",
							"stat port_show 3 3",
							"stat port_show 3 4",
							"stat port_show 3 5",
							"stat port_show 3 6",
							"stat port_show 3 7",
							"stat port_show 3 8",
							"stat port_show 3 9",
							"stat port_show 3 10",
							"stat port_show 3 11",
							"stat port_show 3 12",
							"stat port_show 3 13",
							"stat port_show 3 14",
							"stat port_show 3 15",
							"stat port_show 3 16",
							"stat port_show 3 17",
							"stat port_show 3 18",
							"stat port_show 3 19",
							"stat port_show 3 20",
							"stat port_show 3 21",
							"stat port_show 3 22",
							"stat port_show 3 23",
							"stat port_show 3 24",
							"stat port_show 3 25",
							"stat port_show 3 26",
							"stat port_show 3 27",
							"stat port_show 3 28",
							"stat port_show 3 29",
							
							"stat port_flush 3", /*清楚统计*/
							
							"stat port_show 3 0",
							"stat port_show 3 1",
							"stat port_show 3 2",
							"stat port_show 3 3",
							"stat port_show 3 4",
							"stat port_show 3 5",
							"stat port_show 3 6",
							"stat port_show 3 7",
							"stat port_show 3 8",
							"stat port_show 3 9",
							"stat port_show 3 10",
							"stat port_show 3 11",
							"stat port_show 3 12",
							"stat port_show 3 13",
							"stat port_show 3 14",
							"stat port_show 3 15",
							"stat port_show 3 16",
							"stat port_show 3 17",
							"stat port_show 3 18",
							"stat port_show 3 19",
							"stat port_show 3 20",
							"stat port_show 3 21",
							"stat port_show 3 22",
							"stat port_show 3 23",
							"stat port_show 3 24",
							"stat port_show 3 25",
							"stat port_show 3 26",
							"stat port_show 3 27",
							"stat port_show 3 28",
							"stat port_show 3 29",	
							"NULL",
							};

char V2R1EponCliSelfCheck[][50] = {
							"mgt self_check_result 0",
							"mgt self_check_result 1",
							"mgt self_check_result 2",
							"mgt self_check_result 3",
							"mgt self_check_result 4",
							"mgt self_check_result 5",
							"mgt self_check_result 6",
							"mgt self_check_result 7",
							"mgt self_check_result 8",
							"mgt self_check_result 9",
							"mgt self_check_result 10",
							"mgt self_check_result 11",
							
							"mgt self_check_result",
							"NULL"
};

char EuqFrameInfo[] = {
0x01 , 0x80 , 0xc2 , 0x00 , 0x00 , 0x02 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
0x22 , 0x88 , 0x09 , 0x03 , 0x00 , 0x50 , 0xfe , 0x00 , 0x0f  
         , 0xe9 , 0x14 , 0xa3 , 0x00 , 0x00 , 0x00 , 0xb7 , 0x00 , 0x00 , 
0x00 , 0xb7 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00
         , 0x01 , 0x00 , 0x01 , 0xff , 0xff , 0xff , 0x0f , 0x50 , 0x57 , 
0x33 , 0x3a , 0x54 , 0x30 , 0x31 , 0x31 , 0x37 , 0x30 , 0x30 , 0x39 , 0x2d
         , 0x50 , 0x4c , 0x07 , 0x31 , 0x2e , 0x30 , 0x2e , 0x31 , 0x2e , 
0x34 , 0x09 , 0x30 , 0x2e , 0x31 , 0x2e , 0x34 , 0x30 , 0x34 , 0x2e , 0x30 
         , 0x07 , 0x31 , 0x2e , 0x31 , 0x2e , 0x31 , 0x2e , 0x33 , 0x23 , 
0x53 , 0x68 , 0x61 , 0x6e , 0x67 , 0x68 , 0x61 , 0x69 , 0x20 , 0x44 , 0x41
         , 0x52 , 0x45 , 0x20 , 0x54 , 0x45 , 0x43 , 0x48 , 0x4e , 0x4f , 
0x4c , 0x4f , 0x47 , 0x49 , 0x45 , 0x53 , 0x20 , 0x43 , 0x6f , 0x2e , 0x2c
         , 0x20 , 0x4c , 0x54 , 0x44 , 0x03 , 0x34 , 0x46 , 0x45 , 0x3a , 
0x32 , 0x32 , 0x46 , 0x2c , 0x20 , 0x49 , 0x6e , 0x66 , 0x6f , 0x20 , 0x54
         , 0x65 , 0x63 , 0x68 , 0x20 , 0x42 , 0x75 , 0x69 , 0x6c , 0x64 , 
0x69 , 0x6e , 0x67 , 0x2c , 0x20 , 0x4e , 0x6f , 0x2e , 0x31 , 0x35 , 0x35 
};


int cli_flag = 0;
int cli_printf = 1;
extern int  Comm_Cli_request_transmit( struct vty * vty,
				const unsigned short OltId, 
				const unsigned short OnuId,
				unsigned char  *pClibuf,
				const unsigned short cli_data_size,
				unsigned char *psessionIdField);
extern int  CommOltMsgRvcCallbackInit(unsigned char GwProId, void *Function);
void CommHadleCli(unsigned short   PonId,
                       unsigned short	OnuId,
                       unsigned short	llid,
                       unsigned short   Length,
                       unsigned char	*pFrame,
                       unsigned char	*pSessionField)
{
	/*char cliTemp[Length+1] = {0};*/
	char *pCli = NULL;
	pCli = (char *)VOS_Malloc((ULONG)(Length+1), MODULE_OAM);
	if (pCli == NULL)
		{
		printf("  pCli = NULL !\r\n");
		sys_console_printf("  pCli = NULL !\r\n");
		return;
		}
	memset(pCli, 0, Length+5);	
	memcpy(pCli, pFrame, Length);
	if (pFrame == NULL)
	{
		if (pSessionField != NULL)
			VOS_Free(pSessionField);
		
		pSessionField = NULL;
		return;
	}

	if (pSessionField == NULL)
	{
		if (pFrame != NULL)
			VOS_Free(pFrame);
		pFrame = NULL;
		return;
	}
	
	if(cli_flag == 1)
	{
		int iLoop = 0;
		printf("  Rev Content(len %d) : \r\n",Length);
		printf("  ");
		for(iLoop=0;iLoop<Length;iLoop++)
		{
			printf("%02x ",pFrame[iLoop]);
			if( (iLoop+1) % 20 == 0 ) printf("\r\n  ");
		}
		printf("\r\n\r\n");  
		/*printf("  ----------------------------------------------------------------\r\n");*/
	}
		
	
	if (cli_printf == 1)
		{
			pCli[Length] = '\0';
			pCli[Length+1] = '\0';
			pCli[Length+2] = '\0';
			pCli[Length+3] = '\0';
			pCli[Length+4] = '\0';
			printf("  Rev daya`s info : \r\n");
			printf("  %s",pCli);
			printf("\r\n  ----------------------------------------------------------------\r\n\r\n");
			/*sys_console_printf("  cli cmd : %s\r\n\r\n",pCli);*/
			/*sys_console_printf("-----------------------------------------------\r\n");*/	
		}
	
	VOS_Free(pCli);
	if (pFrame != NULL)
		VOS_Free(pFrame);
	pFrame = NULL;
	if (pSessionField != NULL)
		VOS_Free(pSessionField);
	pSessionField = NULL;
		return;
}

static int cli(unsigned short OltId, 
				unsigned short OnuId,
				unsigned char  *pClibuf)
{
	char *pTemp = NULL;
	char *pSeField = NULL;
	unsigned short length = 0;
	/*输出到串口*/
	sys_console_printf("\r\n  ----------------------------------------------------------------\r\n");
	sys_console_printf("  cli len %d\r\n",strlen(pClibuf));
	/*sys_console_printf("  %s\r\n",pClibuf);*/


	/*一下printf 输出到shell*/
	
	printf("  cli len %d\r\n",strlen(pClibuf));
	/*printf("  %s\r\n",pClibuf);*/
	length = (strlen(pClibuf));

	
	pTemp = (char *)VOS_Malloc((ULONG)length, (ULONG)MODULE_OAM);
	if(pTemp == NULL)
		return -1;
	pSeField = (char *)VOS_Malloc((ULONG)8, (ULONG)MODULE_OAM);
	if(pSeField == NULL)
		return -1;	
	memset(pTemp, 0, length);

	
	memcpy((pTemp), pClibuf, strlen(pClibuf));
	memset(pSeField, 0x5a, 8);
	if (OAM_TEST_ENABLE)
	{
		int iLoop = 0;
		printf("  Send Content(len %d):\r\n",length);
		printf("  ");
		for(iLoop=0;iLoop<length;iLoop++)
		{
			printf("%02x ",pTemp[iLoop]);
			if( (iLoop+1) % 20 == 0 ) printf("\r\n  ");
		}
		printf("\r\n\r\n");  

	}
	
	/*这里假定psefield域由该测试函数给出*/
	return (Comm_Cli_request_transmit(0, OltId, OnuId,pTemp, length, pSeField));

}

int oam_test_delay = 20;
int cliall(short int ponId , short int onuId)
{
	int count = 0;
	int result = 0;
	/*char Temp[60] = {0};*/
	/*cli(ponId, onuId,&(V2R1EponCli[count][0]));*/
	while(0 != memcmp(&(V2R1EponCli[count][0]), "NULL", 4))
		{
		result = cli(ponId, onuId, &(V2R1EponCli[count][0]));
		if(result != 0)
			{
			
			return result;
			}
		count++;
		taskDelay(oam_test_delay);
		}
	sys_console_printf("  end of cli test\r\n");
	return 0;
}

int cliser(short int ponId , short int onuId,int flag)
{
	int count = 0;
	int result = 0;
	/*char Temp[60] = {0};*/
	/*cli(ponId, onuId,&(V2R1EponCli[count][0]));*/
	for(count = 0; count<flag; count++)
		{
		result = cli(ponId, onuId, &(V2R1EponCli[count][0]));
		printf("  (count: %d) ",count);
		if(result != 0)
			{
			return result;
			}
		taskDelay(300);
		}
	sys_console_printf("  end of cli test\r\n");
	return 0;
}

int cliflag(short int ponId , short int onuId,int startflag, int endflag)
{
	int count = 0;
	int result = 0;
	/*char Temp[60] = {0};*/
	/*cli(ponId, onuId,&(V2R1EponCli[count][0]));*/
	for(count = startflag; count<endflag; count++)
		{
		result = cli(ponId, onuId, &(V2R1EponCli[count][0]));
		printf("  (count: %d) ",count);
		if(result != 0)
			{
			return result;
			}
		taskDelay(300);
		}
	sys_console_printf("  end of cli test\r\n");
	return 0;
}

int cliInit(void)
{
	return (CommOltMsgRvcCallbackInit((unsigned char)GW_CALLBACK_CLI_1, (void *)CommHadleCli));

}

int clistats(short int ponId , short int onuId)
{
	int count = 0;
	int result = 0;
	/*char Temp[60] = {0};*/
	/*cli(ponId, onuId,&(V2R1EponCli[count][0]));*/
	while(0 != memcmp(&(V2R1EponClistats[count][0]), "NULL", 4))
		{
		result = cli(ponId, onuId, &(V2R1EponClistats[count][50]));
		if(result != 0)
			{
			
			return result;
			}
		count++;
		taskDelay(300);
		}
	sys_console_printf("  end of cli stats test\r\n");
	return 0;
	
}

int cliselfcheck(short int ponId , short int onuId)
{
	int count = 0;
	int result = 0;
	/*char Temp[60] = {0};*/
	/*cli(ponId, onuId,&(V2R1EponCli[count][0]));*/
	while(0 != memcmp(&(V2R1EponCliSelfCheck[count][0]), "NULL", 4))	
		{
		result = cli(ponId, onuId, &(V2R1EponCliSelfCheck[count][50]));
		if(result != 0)
			{
			
			return result;
			}
		count++;
		taskDelay(300);
		}
	sys_console_printf("  end of cli self check test\r\n");
	return 0;
}



void CommOlt_HadleCli(unsigned short   PonId,
                       unsigned short	OnuId,
                       unsigned short   Length,
                       unsigned char	*pFrame,
                       unsigned char	*pSessionField)
{
	/*char cliTemp[Length+1] = {0};*/
	char *pCli = NULL;
	pCli = (char *)VOS_Malloc((ULONG)(Length+1), MODULE_OAM);
	if (pCli == NULL)
		{
		printf("  pCli = NULL !\r\n");
		sys_console_printf("  pCli = NULL !\r\n");
		return;
		}
	memset(pCli, 0, Length+5);	
	memcpy(pCli, pFrame, Length);
	if (pFrame == NULL)
	{
		if (pSessionField != NULL)
			VOS_Free(pSessionField);
		
		pSessionField = NULL;
		return;
	}

	if (pSessionField == NULL)
	{
		if (pFrame != NULL)
			VOS_Free(pFrame);
		pFrame = NULL;
		return;
	}
	
	if (1 == 1)
		{
			pCli[Length] = '\0';
			pCli[Length+1] = '\0';
			pCli[Length+2] = '\0';
			pCli[Length+3] = '\0';
			pCli[Length+4] = '\0';
			/*printf("  Rev daya`s info : \r\n",pCli);*/
			printf("  %s",pCli);
		}
	
	VOS_Free(pCli);
	if (pFrame != NULL)
		VOS_Free(pFrame);
	pFrame = NULL;
	if (pSessionField != NULL)
		VOS_Free(pSessionField);
	pSessionField = NULL;
		return;
}              


int EuqOamFrametest()
{

	CommEhtFrameReveive (0,0,1,233,EuqFrameInfo );
	CommEhtFrameReveive (0,0,2,233,EuqFrameInfo );
	
}

int EuqoamSend(unsigned long count)
{
	int temp = 0;
	for(temp = 0;temp<count;temp++)
	{
		EuqOamFrametest();
		taskDelay(10);
	}
	return 0;
}


int testSCB(short int PonId,char *pClibuf, unsigned short cli_data_size,unsigned char *psessionIdField)
{
	int iRes = 0;
	/*unsigned short llid = 0;*/
	unsigned long lSendSerNo = 0;
	/*short int opcode = 0x10;*/
	short int broadcast_llid = -32767;
		
	if(pClibuf == NULL)
		return -4 ;
	if (NULL == psessionIdField)
		return -4 ;	
	if (0 != CommOamPadSendsernoGet(&lSendSerNo))
		return -1;	


	iRes = COMM_OAM_frame_send( 	PonId, 
									0,
									0x10,
									broadcast_llid,
									pClibuf, 
									cli_data_size, 
									psessionIdField,
									0, 
									lSendSerNo);	

}
int  CliScb(
				const unsigned short PonId, 
				unsigned char  *pClibuf )
{
	char *pTemp = NULL;
	char *pSeField = NULL;
	unsigned short length = 0;
	length = (strlen(pClibuf));
	pTemp = (char *)VOS_Malloc((ULONG)length, (ULONG)MODULE_OAM);
	if(pTemp == NULL)
		return -1;
	pSeField = (char *)VOS_Malloc((ULONG)8, (ULONG)MODULE_OAM);
	if(pSeField == NULL)
		return -1;	
	testSCB(PonId,pClibuf, length,pSeField);
	return(0);
}
