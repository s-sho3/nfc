//gcc pcse_test.c -lWinSCard
#ifdef WIN32
#undef UNICODE
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include 
#include <PCSC/wintypes.h>
#else
#include <winscard.h>

#endif

#ifdef WIN32
static char *pcsc_stringify_error(LONG rv)
{

 static char out[20];
 sprintf_s(out, sizeof(out), "0x%08X", rv);

 return out;
}
#endif

#define CHECK(f, rv) \
 if (SCARD_S_SUCCESS != rv) \
 { \
  printf(f ": %s\n", pcsc_stringify_error(rv)); \
  return -1; \
 }

int main(void)
{
 LONG rv;

 SCARDCONTEXT hContext;
 LPTSTR mszReaders;
 SCARDHANDLE hCard;
 DWORD dwReaders, dwActiveProtocol, dwRecvLength;

 SCARD_IO_REQUEST pioSendPci;
 BYTE pbRecvBuffer[258];

BYTE cmd1[] = { 0xff, 0x42, 0x01, 0x00, 0x02, 0x00, 0x00};
BYTE cmd2[] = { 0xff, 0x46, 0x01, 0x01, 0x04, 0x00, 0x00, 0x80, 0x00};
BYTE services1[] = {0x09, 0x09, 0x09, 0x0B, 0x0D,0x49};
BYTE services2[] = {0x01, 0x30, 0x40, 0x01, 0x30, 0x30};
	
unsigned int i, j, k, l, m;

 rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
 CHECK("SCardEstablishContext", rv)

#ifdef SCARD_AUTOALLOCATE
 dwReaders = SCARD_AUTOALLOCATE;

 rv = SCardListReaders(hContext, NULL, (LPTSTR)&mszReaders, &dwReaders);
 CHECK("SCardListReaders", rv)
#else
 rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
 CHECK("SCardListReaders", rv)

 mszReaders = calloc(dwReaders, sizeof(char));
 rv = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
 CHECK("SCardListReaders", rv)
#endif
 printf("reader name: %s\n", mszReaders);

 rv = SCardConnect(hContext, mszReaders, SCARD_SHARE_SHARED,
  SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
 CHECK("SCardConnect", rv)

 switch(dwActiveProtocol)
 {
  case SCARD_PROTOCOL_T0:
   pioSendPci = *SCARD_PCI_T0;
   break;

  case SCARD_PROTOCOL_T1:
   pioSendPci = *SCARD_PCI_T1;
   break;
 }
 dwRecvLength = sizeof(pbRecvBuffer);

/*for searching services*/
for(j=0;j<256;j++){
	cmd1[5] = j;
	for(k=0;k<256;k++){
		cmd1[6] = k;
		rv = SCardTransmit(hCard, &pioSendPci, cmd1, sizeof(cmd1),NULL, pbRecvBuffer, &dwRecvLength);
		CHECK("SCardTransmit", rv)

		if(!((pbRecvBuffer[9] == 255) && (pbRecvBuffer[10] == 255))){
			//grep -v "IDm8bytes + 01 FF FF 90 00" dump_result.txt > dump_result2.txt
			//grep -B 1 respon ./dump_result2.txt	
			printf("send: ");
			for(i=0; i<sizeof(cmd1); i++){
				printf("%02X ", cmd1[i]);
			}
			printf("\n");
			printf("response: ");
			for(i=0; i<dwRecvLength; i++){
 			printf("%02X ", pbRecvBuffer[i]);
			}
			printf("\n");
			
			for(l=0;l<6;l++){
				//ff 46 01 01 04 %s 80 %02x
//				cmd2[5] = services1[l];
//				cmd2[6] = services2[l];
				cmd2[5] = cmd1[5];
				cmd2[6] = cmd1[6];
				for(m=0;m<256;m++){
					cmd2[8] = m;
					rv = SCardTransmit(hCard, &pioSendPci, cmd2, sizeof(cmd2),NULL, pbRecvBuffer, &dwRecvLength);
					CHECK("SCardTransmit", rv)

					//IDm8bytes + 01 A8 90 00
					if(!((pbRecvBuffer[9] == 168) && (pbRecvBuffer[10] == 144)) && !((pbRecvBuffer[9] == 165) && (pbRecvBuffer[10] == 144))){
						printf("send: ");
						for(i=0; i<sizeof(cmd2); i++){
							printf("%02X ", cmd2[i]);
						}
						printf("\n");

						printf("response: ");
						for(i=0; i<dwRecvLength; i++){
							printf("%02X ", pbRecvBuffer[i]);
						}
						printf("\n");
					}
				}
			}
		}
	}
//						printf("j:%d",j);
}
rv = SCardDisconnect(hCard, SCARD_LEAVE_CARD);
CHECK("SCardDisconnect", rv)

#ifdef SCARD_AUTOALLOCATE
 rv = SCardFreeMemory(hContext, mszReaders);
 CHECK("SCardFreeMemory", rv)

#else
 free(mszReaders);
#endif

 rv = SCardReleaseContext(hContext);

 CHECK("SCardReleaseContext", rv)

 return 0;
}

