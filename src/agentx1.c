/*
 ============================================================================
 Name        : agentx1.c
 Author      : CrazyBoyFeng
 Copyright   : GNU General Public License
 Description : Agent X One in C, ANSI-style
 ============================================================================
 */
#include "agentx1.h"
void about(void) { //显示软件产品相关信息
	puts("Agent X One [Version: 5]"); //XXX 发布之前改版本号
	puts("Homepage: http://bitbucket.org/CrazyBoyFeng/agentx1"); //为了保证可持续的反馈与维护，请不要修改网址
	puts("GNU General Public License: http://gnu.org/licenses/gpl.html"); //衍生请不要修改协议
	puts("Copyright (C) 2013-2015 CrazyBoyFeng. All rights reserved."); //狂男风
	puts("This is dedicated to my friends, for the passing good times...");	//非常希望这句话在各种衍生中也能够得以保留
}
void help(void) { //显示帮助相关信息
	puts("Usage: agentx1 [-h] Help to use");
	puts("\t[-L <interface-name>] LAN (default br-lan)");
	puts("\t[-W <interface-name>] WAN (default br-wan)");
	puts("\t[-p (default)NONUSE|LOCAL|BOTH] Promiscuous mode");
	puts("\t[-a (default)NONE|AFTER|BEFORE|BETWEEN] DHCP function");
	puts("\t[-i <address>] Binding static (default by local) IP");
	puts("\t[-n <address>] Binding static (default by local) netmask");
	puts("\t[-g <address>] Binding (default 0.0.0.0) gateway");
	puts("\t[-d <address>] Binding (default 0.0.0.0) DNS");
	puts("\t[-u <account>] Binding (default by client) account");
	puts(
			"For more information, visit: http://bitbucket.org/CrazyBoyFeng/agentx1/wiki");
}
void config(int argc, char **argv) { //配置
	about(); //输出软件产品相关信息
	promiscuous = 0;	//混杂模式
	dhcp_wan = 0; //不使用0，之后1，两次2，之前3
	ip_wan = 0;
	netmask_wan = 0;
	gateway_wan = 0;
	dns_wan = 0;
	account_wan = "\0";	//用户绑定账户
	opterr = 0; //错误操作
	char *lan = "br-lan";
	char *wan = "br-wan";
	int option; //操作符
	while ((option = getopt(argc, argv, "hL:W:p:u:a:i:n:g:d:")) != -1) { //大写参数是对工作有关键性影响的
		switch (option) { //操作符
		case 'L': //lan
			lan = optarg;
			break;
		case 'W': //wan
			wan = optarg;
			break;
		case 'p': //wan
			if (strcmp("NONUSE", optarg) == 0) {
				promiscuous = 0; //不混杂
			} else if (strcmp("LOCAL", optarg) == 0) {
				promiscuous = 1; //本地混杂
			} else if (strcmp("BOTH", optarg) == 0) {
				promiscuous = 2; //双向混杂
			} else {
				printf("The promiscious mode %s is invalid, so abort!\n",
						optarg);
				opterr = 2;
			}
			break;
		case 'a': //dhcp
			if (strcmp("NONE", optarg) == 0) {
				dhcp_wan = 0;
			} else if (strcmp("AFTER", optarg) == 0) {
				dhcp_wan = 1;
			} else if (strcmp("BETWEEN", optarg) == 0) {
				dhcp_wan = 2;
			} else if (strcmp("BEFORE", optarg) == 0) {
				dhcp_wan = 3;
			} else {
				printf("The DHCP mode %s is invalid, so abort!\n", optarg);
				opterr = 2;
			}
			break;
		case 'i': //ip
			ip_wan = inet_addr(optarg);
			if (ip_wan == 0xffffffff) {
				printf("The IP %s is invalid, so abort!\n", optarg);
				opterr = 2;
			}
			break;
		case 'n': //mask
			netmask_wan = inet_addr(optarg);
			if (netmask_wan == 0xffffffff) {
				printf("The netmask %s is invalid, so abort!\n", optarg);
				opterr = 2;
			}
			break;
		case 'g': //gate
			gateway_wan = inet_addr(optarg);
			if (gateway_wan == 0xffffffff) {
				printf("The gateway %s is invalid, so abort!\n", optarg);
				opterr = 2;
			}
			break;
		case 'd': //dns
			dns_wan = inet_addr(optarg);
			if (dns_wan == 0xffffffff) {
				printf("The DNS %s is invalid, so abort!\n", optarg);
				opterr = 2;
			}
			break;
		case 'u': //用户账户
			account_wan = optarg;
			break;
		case 'h': //help
			opterr = -1;
			break;
		default:
			printf("The option -%c is invalid, so abort!\n", option);
			opterr = 1;
			break;
		}
	}
	switch (opterr) {
	case -1: //help
		printf("The help is in use, so interrupt!\n");
		opterr++; //退出标志
		/* no break */
	case 1: //操作符错误
		help(); //显示帮助
		/* no break */
	case 2: //参数错误
		opterr--;
		exit(1);
		break;
	}
	find_lan(lan); //打开网卡
	find_wan(wan); //打开网卡
	printf("\tPromiscUous mode: ");
	switch (promiscuous % 3) { //比较promiscuous_lan
	case 0: //关闭
		printf("Out of use\n");
		break;
	case 1: //单播
		printf("LAN network\n");
		break;
	case 2: //组播
		printf("LAN and WAN\n");
		break;
	}
	printf("\tDHCP function: ");
	switch (dhcp_wan % 4) { //比较dhcp
	case 0: //关闭
		printf("Disabled\n");
		print_wan(); //取出并打印地址
		break;
	case 1: //之后
		printf("After authentication\n");
		break;
	case 2: //两次
		printf("Between authentications\n");
		break;
	case 3: //之前
		printf("Before authentication\n");
		break;
	}
	if (strlen(account_wan) > 1) { //设置了account
		printf("\tAccount: %s\n", account_wan);
	}
}
void finish(void) { //进程关闭回调
	if (state == X_RE) {
		puts("Sending the EAPOL-Logoff packet to server...");
		set_head(data_buffer, size_buffer);	//修改加密位
		send_wan(data_buffer, size_buffer);	//发logoff
	}
	fflush(stdout);
}
void error(char *msg) { //处理错误信息
	perror(msg); //输出错误
	exit(1); //错误退出
}
int main(int argc, char **argv) { //主函数
	config(argc, argv); //初始化参数
	puts("Turning the work mode to Initialization...");
	state = X_PRE; //初始状态
	puts("Opening the main work threads...");
	unsigned long int tid_lan, tid_wan;	//线程
	if (pthread_create(&tid_lan, NULL, (void *) work_lan, NULL) < 0
			|| pthread_create(&tid_wan, NULL, (void *) work_wan, NULL) < 0) { //创建线程
		error("pthread_create() error"); //出错提示
	}
	atexit(finish); //进程关闭回调
	fflush(stdout); //刷新输出缓冲
	pthread_join(tid_wan, NULL); //等待wan线程
	pthread_join(tid_lan, NULL); //等待lan线程
	return 0;
}
