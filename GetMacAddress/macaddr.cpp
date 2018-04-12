#include <tchar.h>
#include <algorithm>
#include "macaddr.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include <ntddndis.h>
#include <setupapi.h>
//#include <hidsdi.h>
#ifdef __cplusplus
}
#endif

#pragma comment (lib, "setupapi.lib")
//#pragma comment (lib, "hid.lib")

const GUID GUID_QUERYSET[] = {
	// ����ԭ��MAC��ַ������USB������
	{0xAD498944, 0x762F, 0x11D0, 0x8D, 0xCB, 0x00, 0xC0, 0x4F, 0xC3, 0x35, 0x8C},

	// ����ԭ��MAC��ַ���޳�USB������-- δ���
	{0xAD498944, 0x762F, 0x11D0, 0x8D, 0xCB, 0x00, 0xC0, 0x4F, 0xC3, 0x35, 0x8C},
};

// ��ȡ����ԭ��MAC��ַ
static BOOL WDK_GetMacAddress(TCHAR * DevicePath, MACADDRESS * pMacAddress, INT iIndex, BOOL isIncludeUSB)
{
	HANDLE  hDeviceFile;
	BOOL    isOK = FALSE;

	// �޳���������
	if (_tcsnicmp(DevicePath + 4, TEXT("root"), 4) == 0)
	{
		return FALSE;
	}

	if (!isIncludeUSB)
	{   // �޳�USB����
		if (_tcsnicmp(DevicePath + 4, TEXT("usb"), 4) == 0)
		{
			return FALSE;
		}
	}

	// ��ȡ�豸���
	hDeviceFile = CreateFile(DevicePath,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hDeviceFile != INVALID_HANDLE_VALUE)
	{
		ULONG   dwID;
		BYTE    ucData[8];
		DWORD   dwByteRet;

		// ��ȡ��ǰMAC��ַ
		dwID = OID_802_3_CURRENT_ADDRESS;
		isOK = DeviceIoControl(hDeviceFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwID, sizeof(dwID), ucData, sizeof(ucData), &dwByteRet, NULL);
		if (isOK)
		{
			memcpy(pMacAddress[iIndex].CurMacAddr, ucData, dwByteRet);

			// ��ȡԭ��MAC��ַ
			dwID = OID_802_3_PERMANENT_ADDRESS;
			isOK = DeviceIoControl(hDeviceFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwID, sizeof(dwID), ucData, sizeof(ucData), &dwByteRet, NULL);
			if (isOK)
			{
				memcpy(pMacAddress[iIndex].SrcMacAddr, ucData, dwByteRet);
			}
		}
		CloseHandle(hDeviceFile);
	}
	return isOK;
}

static BOOL WDK_GetProperty(TCHAR* DevicePath, INT iQueryType, MACADDRESS *pMacAddress, INT iIndex)
{
	BOOL isOK = FALSE;

	switch (iQueryType)
	{
	case 0:     // ����ԭ��MAC��ַ������USB������       
		isOK = WDK_GetMacAddress(DevicePath, pMacAddress, iIndex, TRUE);
		break;

	case 1:     // ����ԭ��MAC��ַ���޳�USB������       
		isOK = WDK_GetMacAddress(DevicePath, pMacAddress, iIndex, FALSE);
		break;

	default:
		break;
	}
	return isOK;
}

INT WDK_MacAddress(INT iQueryType, MACADDRESS * pMacAddress, INT iSize)
{
	HDEVINFO hDevInfo;
	DWORD MemberIndex, RequiredSize;
	SP_DEVICE_INTERFACE_DATA            DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA    DeviceInterfaceDetailData;
	INT iTotal = 0;

	// �жϲ�ѯ�����Ƿ�֧��
	if ((iQueryType < 0) || (iQueryType >= sizeof(GUID_QUERYSET) / sizeof(GUID)))
	{
		return -2;  // ��ѯ���Ͳ�֧��
	}

	// ��ȡ�豸��Ϣ��
	hDevInfo = SetupDiGetClassDevs(GUID_QUERYSET + iQueryType, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	// ö���豸��Ϣ���������豸
	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	for (MemberIndex = 0; ((pMacAddress == NULL) || (iTotal < iSize)); MemberIndex++)
	{   // ��ȡ�豸�ӿ�
		if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, GUID_QUERYSET + iQueryType, MemberIndex, &DeviceInterfaceData))
		{   // �豸ö�����
			break;
		}

		// ��ȡ���ջ�������С����������ֵΪFALSE��GetLastError()=ERROR_INSUFFICIENT_BUFFER
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, NULL, 0, &RequiredSize, NULL);

		// ������ջ�����
		DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
		DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// ��ȡ�豸ϸ����Ϣ
		if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, DeviceInterfaceDetailData, RequiredSize, NULL, NULL))
		{
			if (pMacAddress != NULL)
			{
				if (WDK_GetProperty(DeviceInterfaceDetailData->DevicePath, iQueryType, pMacAddress, iTotal))
				{
					iTotal++;
				}
			}
			else
			{
				iTotal++;
			}
		}

		free(DeviceInterfaceDetailData);
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return iTotal;
}
