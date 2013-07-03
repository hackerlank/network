#include "clientsocket.h" 
/**
 * ���ӵ� ip.port �ķ�����
 */
bool ClientSocket::connect(const char *ip,unsigned short port)
{
	sys::Socket socket;
	socket.setHandle(::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP));
	struct hostent *host;
	if((host=gethostbyname(ip))==NULL){
		socket.close();
		return false;
	}
	struct sockaddr_in serv_addr;
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port);
	serv_addr.sin_addr=*((struct in_addr *)host->h_addr);
	bzero(&(serv_addr.sin_zero),8);
	if (socket.connect((struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) == -1) 	
	{
		socket.close();
		return false;		
	}
	if (!connection)
		connection = new Connection();
	connection->setHandle(socket);
	return true;
}

/**
 * ��ȡ����
 * \reeturn ����
 */
Connection* ClientSocket::getConnection()
{
	return connection;
}

/**
 * �Ͽ�����
 */
void ClientSocket::disconnect()
{
	if (connection)
		connection->setInvalid();
	if (connection)
		delete connection;
	connection = NULL;
}