#include "GetMacByCmd.h"
#include "SunDay.h"

// ��λ�ؼ���DHCP ,����ַ�������Ӣ������һ���ġ�
BOOL ParseMac(std::string& szBuf,std::string &szRet)
{
	unsigned char* pBuf = (unsigned char*)szBuf.c_str();
	const char* pStart = szBuf.c_str();
	int nLen = szBuf.length();
	int nSearLen = nLen;
	unsigned char* pKeyWord = (unsigned char*)"DHCP ";
	int nKeyLen = strlen("DHCP ");
	unsigned char* pFindResult = NULL;
	char szMacBuf[50] ={0}; // 00-12-34-56-78-80,	00-01-00-01-1F-BE-33-6E-4C-72-B9-E3-39-98
	do 
	{
		// Խ��
		if ((char*)pBuf > pStart+nLen) break;
		// ���ٲ���DHCO
		pFindResult = SUNDAY(pBuf,pKeyWord,nKeyLen,nSearLen);
		if(!pFindResult) break;

		// ��ǰ���ҷֺ�
		unsigned char* pFindFenHao = pFindResult;
		BOOL bFind = FALSE;
		do 
		{
			if (pFindFenHao[0]==':')
			{
				bFind = TRUE;
				break;
			}
			// Խ��
			if ((char*)pFindFenHao<pStart)
				break;
			pFindFenHao--;
		} while (1);

		// �ֺ�������ҿո�
		if (bFind)
		{
			bFind = FALSE;
			// �����ֺź��β��ո�
			unsigned char* pFindKongGe = pFindFenHao+2;
			do 
			{
				if ((char)pFindKongGe[0]==0x20)
				{
					bFind = TRUE;
					break;
				}
				// Խ��
				if (pFindKongGe>pFindResult) break;
				pFindKongGe++;
			} while (1);

			if (bFind)
			{
				// 22-12-34-56-78-9A�ĳ���Ϊ17��nCount�����ո�
				// IPv6 Ϊ41
				int nCount = (char*)pFindKongGe-(char*)pFindFenHao;
				if (nCount>17&&nCount<45)
				{
					memcpy(szMacBuf,pFindFenHao+2,nCount-1);
					szRet.append(szMacBuf);
					szRet.append("\n");
				}
			}
		}
		nSearLen = nLen - ((char*)pFindResult- (char*)pStart);
		pBuf = pFindResult + nKeyLen;
	} while (1);

	return TRUE;
}

BOOL GetMacByCmd(std::string& macOUT)
{
	BOOL ret = false;
	std::string result;
	FILE * pipe =_popen("ipconfig /all","r");
	if (pipe)
	{
		char buffer[128];  
		while(!feof(pipe)) 
		{  
			if(fgets(buffer, 128, pipe))
			{
				//���ܵ������result��,��¼��Ч��   
				if (strlen(buffer)>40)
					result.append(buffer);
			}  
		}
		_pclose(pipe); 
		ret = ParseMac(result, macOUT);//��ȡMAC��ַ��
	}
	return ret;
}