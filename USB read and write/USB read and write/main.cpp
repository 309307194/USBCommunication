#ifdef _DEBUG
#pragma comment(lib,"setupapi.lib")
#pragma comment(lib,"hid.lib")
#endif

#include<stdio.h>
#include<Windows.h>
#include<SetupAPI.h>
#include<hidsdi.h>
//#include<initguid.h>
//#include<stdint.h>


void ShowGUID(LPGUID HidGuid);
BOOL FindDevice(UINT32 PID, UINT32 VID);
BOOL CustomReadFile(void);
BOOL CustomWriteFile(void);

HANDLE hUserHidFileHandle;

int main()
{
	printf("Please chose your operate\n");
	printf("1.Read Mode\n");
	printf("2.Write Mode\n");
	char c = getchar();
	if ('1' == c)
	{

		CustomReadFile();	//���Դ˴��ύ�Ƿ�ɹ�
	}
	if ('2' == c)
	{
		CustomWriteFile();
	}
	return 0;
}

void ShowGUID(LPGUID HidGuid)
{
	printf("%X-", HidGuid->Data1);
	printf("%X-", HidGuid->Data2);
	printf("%X-", HidGuid->Data3);
	for (size_t i = 0; i < 8; i++)
	{
		printf("%02X", HidGuid->Data4[i]);
	}
	printf("\n");
}

BOOL FindDevice(UINT32 PID, UINT32 VID)
{
	GUID hid_guid;
	BOOL bGetDeviceInterface = FALSE;
	BOOL bGetDeviceInterfaceDetail = FALSE;
	BOOL bGetAttributes = FALSE;
	UINT32 nDeviceIndex = 0;
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	DWORD nRequiredSize = 0;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData;
	HANDLE hFileHandle;
	HIDD_ATTRIBUTES hid_attributes;

	//��ȡ�豸��HIDGUID
	HidD_GetHidGuid(&hid_guid);
	ShowGUID(&hid_guid);

	//��ȡ�豸��Ϣ���Ͼ��
	HDEVINFO device_info = SetupDiGetClassDevs(
		&hid_guid,									//ָ���豸��HIDGUID
		NULL,										//ָ���豸ʵ���ַ�����NULLΪ��ָ��
		NULL,										//���ھ��
		DIGCF_DEVICEINTERFACE | DIGCF_PRESENT		//�豸�ӿ�����豸�ӿڵ��豸|ϵͳ�е�ǰ���ڵ��豸
	);

	do
	{
		//��ȡ�豸�ӿ���Ϣ
		DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		bGetDeviceInterface = SetupDiEnumDeviceInterfaces(
			device_info,							//�豸��Ϣ����
			NULL,									//ָ��ĳ���豸��Ϣ
			&hid_guid,								//GUID
			nDeviceIndex,							//�豸index
			&DeviceInterfaceData					//��������÷��Ļ�������ָ��
		);

		if (bGetDeviceInterface)
		{
			printf("GetDeviceInterfaces success\n");
			printf("the device index = %d\n", nDeviceIndex);
			//��ȡ�豸�ӿ���ϸ��Ϣ
			//1.Get the required buffer size
			bGetDeviceInterfaceDetail = SetupDiGetDeviceInterfaceDetail(
				device_info,
				&DeviceInterfaceData,
				NULL,
				0,
				&nRequiredSize,
				NULL
			);

			printf("required buffer size = %ld\n", nRequiredSize);

			//2.Allocate an appropriately sized buffer and call the function again to get the interface details

			pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(nRequiredSize);
			if (pDeviceInterfaceDetailData)
			{
				pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			}

			bGetDeviceInterfaceDetail = SetupDiGetDeviceInterfaceDetail(
				device_info,
				&DeviceInterfaceData,
				pDeviceInterfaceDetailData,
				nRequiredSize,
				NULL,
				NULL
			);

			if (bGetDeviceInterfaceDetail)
			{
				printf("\tGetDeviceDetailDataSuccess!\n");

				hFileHandle = CreateFile(
					pDeviceInterfaceDetailData->DevicePath,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL
				);

				if (hFileHandle == INVALID_HANDLE_VALUE)
				{
					printf("\t\tCreateFile Error\n");
				}
				else
				{
					printf("\t\tCreateFile Success\n");

					bGetAttributes = HidD_GetAttributes(
						hFileHandle,
						&hid_attributes
					);
					if (bGetAttributes)
					{
						printf("\t\tPID=0x%X\n", hid_attributes.ProductID);
						printf("\t\tVID=0x%X\n", hid_attributes.VendorID);
						if (hid_attributes.ProductID == PID && hid_attributes.VendorID == VID)
						{
							hUserHidFileHandle = hFileHandle;
							printf("\t\t\tFindDeviceSuccess\n");
							return TRUE;
						}
					}
				}
			}

		}

		nDeviceIndex++;

	} while (bGetDeviceInterface);

	return TRUE;
}

BOOL CustomReadFile(void)
{
	BOOL bReadFile = FALSE;
	BYTE readbuf[0x41] = { 0 };
	DWORD nNumberOfBytesRead;

	printf("User HID device software!\n");
	if (FindDevice(0x7654, 0x4567))
	{
		while (true)
		{
			Sleep(10);
			bReadFile = ReadFile(
				hUserHidFileHandle,
				readbuf,
				0x41,
				&nNumberOfBytesRead,
				NULL
			);

			if (bReadFile)
			{
				printf("read data byte:%d\n", nNumberOfBytesRead);
				for (UINT i = 0; i < nNumberOfBytesRead; i++)
				{
					printf("%02x\t", readbuf[i]);
					if (i % 8 == 0)
					{
						printf("\n");
					}
				}
			}
		}
	}

	return TRUE;
}


BOOL CustomWriteFile(void)
{
	BOOL bWriteFile = FALSE;
	BYTE writebuf[0x41] = { 0 };
	DWORD nNumberOfBytesWrite;
	printf("User HID device software!\n");
	if (FindDevice(0x7654, 0x4567))
	{
		while (true)
		{
			Sleep(1000);
			writebuf[1] = 0x02;
			bWriteFile = WriteFile(
				hUserHidFileHandle,
				writebuf,
				0x41,
				&nNumberOfBytesWrite,
				NULL
			);
			if (bWriteFile)
			{
				printf("write data byte:%d\n", nNumberOfBytesWrite);
				for (size_t i = 0; i < nNumberOfBytesWrite; i++)
				{
					printf("%02X\t", writebuf[i]);
					if (i % 8 == 0)
					{
						printf("\n");
					}
				}
			}
		}
	}

	return TRUE;
}