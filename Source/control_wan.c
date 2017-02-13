/*
 ============================================================================
 Name        : control_wan.c
 Author      : CrazyBoyFeng
 Copyright   : GNU General Public License
 Description : Agent X One WAN controller in C, ANSI-style
 ============================================================================
 */
#include "agentx1.h"
int sock_wan; //连接
struct ifreq if_wan; //网络接口
void find_wan(char *interface) { //建立wan连接
	if ((sock_wan = socket(PF_PACKET, SOCK_RAW, htons(0x888e))) < 0) { //建立连接
		error("WAN socket() error"); //出错提示
	}
	strcpy(if_wan.ifr_name, interface); //指定wan接口
	printf("\tWAN interface: %s\n", if_wan.ifr_name);
	if (ioctl(sock_wan, SIOCGIFINDEX, &if_wan) < 0) { //控制接口
		error("WAN ioctl() error"); //出错提示
	}
	struct sockaddr_ll sll_wan; //地址
	sll_wan.sll_family = AF_PACKET; //设置wan协议族
	sll_wan.sll_ifindex = if_wan.ifr_ifindex; //设置wan接口
	sll_wan.sll_protocol = htons(0x888e); //设置wan协议类型
	if (bind(sock_wan, (struct sockaddr *) &sll_wan, sizeof(sll_wan))) { //将接口与地址绑定
		error("WAN bind() error"); //出错提示
	}
}
void print_wan(void) { //取出并打印地址
	if (ip_wan == 0 || netmask_wan == 0) {
		if (ioctl(sock_wan, SIOCGIFADDR, &if_wan) < 0) { //控制接口
			error("WAN ioctl() error"); //出错提示
		}
		ip_wan = ((struct sockaddr_in *) &if_wan.ifr_addr)->sin_addr.s_addr; //得到IP

		if (ioctl(sock_wan, SIOCGIFNETMASK, &if_wan) < 0) { //控制接口
			error("WAN ioctl() error"); //出错提示
		}
		netmask_wan =
				((struct sockaddr_in *) &if_wan.ifr_addr)->sin_addr.s_addr; //得到掩码
		//没有较通用或标准的方式获得网关和DNS，请自行指定。
	}
	unsigned char *addr; //输出地址指针
	addr = (unsigned char *) (&ip_wan); //ip地址
	printf("\tIP address: %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
	addr = (unsigned char *) (&netmask_wan); //子网掩码
	printf("\tNetmask: %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
	addr = (unsigned char *) (&gateway_wan); //网关地址
	printf("\tGateway: %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
	addr = (unsigned char *) (&dns_wan); //dns
	printf("\tDNS: %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
}
void refresh_wan(void) { //dhcp及获得网络信息//XXX 调查二次认证的过程
	if (dhcp_wan == 0) { //不使用dhcp
		return;
	} else if (dhcp_wan == 1 && state == X_ON) { //on状态不会在认证后dhcp
		return;
	} else if (dhcp_wan == 3 && (state == X_PRE || state == X_RE)) { //pre和re状态不会在认证前dhcp
		return;
	}
	puts("Refreshing the network interfaces...");
	ip_wan = 0; //重新初始化
	netmask_wan = 0; //重新初始化
	char command[100] = "udhcpc -i "; //dhcp命令
	strcat(command, if_wan.ifr_name); //wan
	strcat(command, ">/dev/null"); //屏蔽输出
	system(command); //更换dhcp
	print_wan();	//取出并打印地址
}

void open_wan(void) { //获得mac
	if (ioctl(sock_wan, SIOCGIFFLAGS, &if_wan) < 0) { //准备混杂模式
		error("WAN ioctl() error"); //出错提示
	}
	if (promiscuous == 2) { //双向混杂模式
		if_wan.ifr_flags |= IFF_PROMISC; //混杂模式
	} else {
		if_wan.ifr_flags &= ~IFF_PROMISC; //混杂模式
	}
	if (ioctl(sock_wan, SIOCGIFFLAGS, &if_wan) < 0) { //以混杂模式打开网卡
		error("WAN ioctl() error"); //出错提示
	}
	if (ioctl(sock_wan, SIOCGIFHWADDR, &if_wan) < 0) { //查询mac
		error("WAN ioctl() error"); //出错提示
	}
	memcpy(mac_wan, if_wan.ifr_hwaddr.sa_data, 6); //获得mac地址
	printf("\tWAN MAC address: %02x-%02x-%02x-%02x-%02x-%02x\n", mac_wan[0],
			mac_wan[1], mac_wan[2], mac_wan[3], mac_wan[4], mac_wan[5]);
}

void filter_wan(unsigned char *buffer) { //锁定服务器
	memcpy(server_wan, buffer + 6, 6);	//取出server
	printf("\tServer MAC address: %02x-%02x-%02x-%02x-%02x-%02x\n",
			server_wan[0], server_wan[1], server_wan[2], server_wan[3],
			server_wan[4], server_wan[5]);
}
void send_wan(unsigned char *buffer, int length) {	//wan发包
	if (promiscuous != 2) {	//非混杂模式修改dst地址
		memcpy(buffer, server_wan, 6); //修改dst为server
	}
	memcpy(buffer + 6, mac_wan, 6);	//修改src为wan
	if (sendto(sock_wan, buffer, length, 0, NULL, 0) < 0) {
		error("WAN sendto() error");	//错误提示
	}
	printf("\tPacket over WAN: %ld\n", time(NULL)); //输出响应时间
}
void work_wan(void) { //wan线程
	puts("Opening the WAN socket connection...");
	open_wan(); //查询接口
	puts("Receiving the packets from WAN...");
	int len_wan; //包长度
	unsigned char buf_wan[1024]; //缓冲区
	while ((len_wan = recvfrom(sock_wan, buf_wan, 1024, 0, NULL, NULL)) > 0) { //循环接收
		if (buf_wan[0x0f] != 0x00 || memcmp(mac_wan, buf_wan, 6) != 0) { //不是eap包或者不是发给自己的包
			continue; //丢弃
		} else if (state == X_ON) { //转发状态
			if (buf_wan[0x12] == 0x01) { //request
				if (buf_wan[0x16] == 0x01) { //id
					puts("Receiving a EAP-Request/Identity packet from WAN!");
					puts("Reading the server MAC address...");
					filter_wan(buf_wan); //锁定服务器
					puts(
							"Sending the EAP-Request/Identity packet to client...");
					send_lan(buf_wan, len_wan); //发送
				} else if (buf_wan[0x16] == 0x04
						&& memcmp(server_wan, buf_wan + 6, 6) == 0) { //md5
					puts("Receiving a EAP-Request packet from server!");
					puts("Sending the EAP-Request packet to client...");
					send_lan(buf_wan, len_wan); //发送
				}
			} else if (buf_wan[0x12] == 0x03
					&& memcmp(server_wan, buf_wan + 6, 6) == 0) { //success
				puts("Receiving a EAP-Success packet from server!");
				puts("Refreshing the network interfaces...");
				refresh_wan(); //dhcp并输出
				puts("Storing the EAP-Success packet...");
				size_buffer = len_wan;
				memcpy(data_buffer, buf_wan, size_buffer);	//复制数据
				puts("Reading the repeat parameters...");
				get_success(buf_wan); //读取hello_key和hello_count
				puts("Sending the EAP-Success packet to client...");
				send_lan(buf_wan, len_wan); //发送
			} else if (buf_wan[0x12] == 0x04
					&& memcmp(server_wan, buf_wan + 6, 6) == 0) { //failure
				puts("Receiving a EAP-Failure packet from server!");
				puts("Turning the work mode to Initialization...");
				state = X_PRE; //初始模式
				puts("Sending the EAP-Failure packet to client...");
				send_lan(buf_wan, len_wan); //发送
			}
		} else if (state == X_OFF && memcmp(server_wan, buf_wan + 6, 6) == 0) { //等待状态且已获得服务器
			switch (buf_wan[0x12]) {	//type
			case 0x03:	//success
				puts("Receiving a EAP-Success packet from server!");
				puts("Reading the repeat parameters...");
				get_success(buf_wan); //读取hello_key和hello_count
				puts("Sending the EAP-Success packet to client...");
				send_lan(buf_wan, len_wan); //发送
				break;
			case 0x04: //failure
				puts("Receiving a EAP-Failure packet from server!");
				puts("Turning the work mode to Initialization...");
				state = X_PRE;	//初始模式
				puts("Sending the EAP-Failure packet to client...");
				send_lan(buf_wan, len_wan);	//发送
				break;
			}	//type
		} else if (state == X_RE && memcmp(server_wan, buf_wan + 6, 6) == 0) {//中继状态
			switch (buf_wan[0x12]) {	//type
			case 0x03:	//success
				puts("Receiving a EAP-Success packet from server!");
				puts("Reading the repeat parameters...");
				get_success(buf_wan); //读取hello_key和hello_count
				break;
			case 0x04: //failure 被动掉线
				puts("Receiving a EAP-Failure packet from server!");
				puts("Turning the work mode to Initialization...");
				state = X_PRE;	//初始模式
				break;
			}	//type
		} else if (state == X_PRE && memcmp(server_wan, buf_wan + 6, 6) == 0) {	//准备状态
			switch (buf_wan[0x12]) {	//type
			case 0x04: //failure //主动掉线（客户端发送start之后转发缓存的logoff得到的回应）
				puts("Receiving a EAP-Failure packet from server!");
				puts("Modifying the EAPOL-Start packet...");
				set_head(data_buffer, size_buffer); //修改加密位
				puts("Turning the work mode to Initialization...");
				state = X_ON;	//转发模式
				puts("Sending the EAPOL-Start packet to WAN...");
				send_wan(data_buffer, size_buffer);	//发start
				break;
			} //type
		}	//state
		fflush(stdout); //刷新输出缓冲区
	}	//while
	error("recvfrom() error");	//监听失败
}
