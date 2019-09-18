//*******************************************************************************
//* 函数名: UDP_Init()
//* 功能:   UDP初始化
//* 返回值: 
//*			-1: 初始化失败
//*			0 : 初始化成功
//*******************************************************************************
int UDP_Init()
{
	int optval = 1;
	int ret;
	pthread_t id1;

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == socket_fd)
	{
		printf("socket fail\n");
		return -1;
	}
	
	//本地，用于接收数据
	bzero(&local_addr,sizeof(struct sockaddr_in));
	local_addr.sin_family=AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(LOCAL_PORT);
	bind(socket_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	
	setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &optval, sizeof(int));
	
	//远端，用于发送收据
	memset(&remote_addr,0,sizeof(struct sockaddr_in));
	remote_addr.sin_family=AF_INET;
	//remote_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);
	inet_pton(AF_INET, "192.168.1.252", &remote_addr.sin_addr);
	remote_addr.sin_port=htons(REMOTE_PORT);
	
	
	//创建获取数据线程
	ret= pthread_create(&id1,NULL,(void*)Read_Data,NULL ); 
	if(ret)
	{
		printf("create Read_Data pthread error\n");
		return -1;
	}
	
	return 0;

}

//*******************************************************************************
//* 函数名: Read_Data()
//* 功能:   读取数据线程
//* 返回值: 
//*******************************************************************************
void *Read_Data(void *arg)
{
	while(1)
	{
		UDP_Read_Data();
		sleep(1);
	}
}



//*******************************************************************************
//* 函数名: UDP_Send_Data()
//* 功能:   UDP发送数据
//* 返回值: 
//*			-1: 失败
//*			0 : 成功
//*******************************************************************************
static int UDP_Send_Data(char *msg)
{			
	int i;
	
	printf("\n");
	for(i = 0 ;i<CMD_LEN;i++)
	{
		printf("%02x \t",msg[i]);
	}
	printf("\n");
	
	if( -1 == sendto(socket_fd, msg, strlen(msg), 0,(struct sockaddr *)&remote_addr, sizeof(struct sockaddr)))
	{
		printf("UDP_Send_Data fail\n");
		return -1;
	}
	
	return 0;
}

//*******************************************************************************
//* 函数名: UDP_Read_Data()
//* 功能:   UDP接收数据
//*******************************************************************************
static int UDP_Read_Data()
{
	printf("UDP_Read_Data\n");
	int i;
	float liveTime = 0;
	int slowCounts;
	int inputCounts;
	int dataLenSave = XMA_Channel_num;
	int dataLen = XMA_Channel_num;
	char *data = (char *)malloc(dataLen);
	socklen_t local_len = sizeof(struct sockaddr);
	
	//如果是512总道数，只读前面512个字节
	if(dataLen == READ_LEN_512_CHANNEL)
	{
		if(-1 == recvfrom(socket_fd, data, READ_LEN_512_CHANNEL, 0, (struct sockaddr *)&local_addr, &local_len))
		{
			printf("UDP_Read_Data fail\n");
			return -1;
		}
	}
	else //如果是1024以上的总道数，则循环读，直到读完
	{
		while(dataLen != 0)
		{
			if(-1 == recvfrom(socket_fd, data, READ_DATA_LEN, 0, (struct sockaddr *)&local_addr, &local_len))
			{
				printf("UDP_Read_Data fail\n");
				return -1;
			}
			data += READ_DATA_LEN;    //循环读取时，需要将指针偏移，以免覆盖数据
			dataLen -= READ_DATA_LEN;
		}
		data -= dataLenSave;
	}
	for(i = 0 ; i <dataLenSave ; i++)
	{
		printf("%02x  ",data[i]);
	}
	
	Analy_Data(data,XMA_Channel_num,liveTime);


	//数据解析完释放内存
	if(data != NULL)
	{
		free(data);
		data = NULL;
	}
	return 0;
}
