#pragma once
#pragma pack(1)
/**
 * ����Ϣ
 **/
struct stNullCmd{
	unsigned char msgType; // ��Ϣͷ
	unsigned char type; // ��Ϣ����1
	unsigned short param; // ��Ϣ����2
	stNullCmd(){
		msgType = 0x80; // ���λ����Ϊ1
		type = 0;
		param = 0;
	};
};
#pragma pack()

