#ifndef __MACADDR_H__
#define __MACADDR_H__

#include <windows.h>

#define MACADDRESS_BYTELEN  6                       // MAC��ַ�ֽڳ���

typedef struct _MACADDRESS
{
    BYTE    SrcMacAddr[MACADDRESS_BYTELEN];         // ԭ��MAC��ַ
    BYTE    CurMacAddr[MACADDRESS_BYTELEN];         // ��ǰMAC��ַ
} MACADDRESS;

#ifdef __cplusplus
extern "C"{
#endif

    /*

    */
    /*
        ����     �� ���WMI��DeviceIoControl��ȡ����ԭ��MAC��ַ�͵�ǰMAC��ַ
        ��ڲ���  ��
                   iQueryType  ��Ҫ��ȡ����������
                               0 �� ����USB����
                               1 �� ������USB����
                   pMacAddress �洢����MAC��ַ
                   uSize       �ɴ洢�����������Ŀ
        ����ֵ��
                   -1  ��֧�ֵ��豸����ֵ
                   -2  WMI����ʧ��
                   -3  ����ȷ��WQL��ѯ���
                   >=0 ��ȡ��������Ŀ
    */
    INT WDK_MacAddress(INT iQueryType, MACADDRESS * pMacAddress, INT iSize);

#ifdef __cplusplus
}
#endif

#endif