// oCamDemoApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "wImage.h"
#include "colorCorrection.hpp"
#include "ImgProc.h"
#include "libCamCap.h"
#include <string>
#include <vector>
#include <iterator>
#include <ctime>

#define FLOAT_SCALER    100 
#define SAVE_SCALE      0x73617665 
#define ERASE_SCALE     0x78787878  
#define LOAD_SCALE      0x89898989  
#define ERASE_FIRMWARE  0x30303030

static const int DEFAULT_EXPOSURE = -6;
static const int DEFAULT_GAIN = 64;
static const int DEFAULT_WB_COMP = 100;


struct CamDevice
{
	int id;
	string model;
	string serialNumber;
	string usbType;
	string fw;
};

vector<CamDevice> camDeviceList;

wImage		m_ImageSrc;
wImage		m_Image;
CAMPTR      m_pCam;

int			m_Width = 320;
int			m_Height = 240;
double		m_FPS = 160;

bool inSubMenu = false;
bool initialized = false;
int cnt = 0;

void saveImage(CamDevice* device, int ImageIndex)
{
	wImage image;

	if (device->model == "oCam-1MGN-U")
	{
		image = m_ImageSrc;
	}
	else
	{
		image = m_Image;
	}
	stringstream ss;
	ss << "image" << cnt << ".bmp";
	image.Save(ss.str());
}

void convertToRGB(CamDevice *device, BYTE *src, BYTE *dest)
{
	if (device->model == "oCam-1CGN-U")
	{
		Bayer2RGB((char*)src, (char*)dest, m_Width, m_Height, BayerGR2RGB);
	}
}

void CallbackFunction(void *Para, void *Data)
{
	if (initialized)
	{
		//Copy data into image source
		BYTE* img = (BYTE*)m_ImageSrc.GetPtr1D();
		memcpy(img, Data, m_ImageSrc.GetSize());

		// Convert images to RGB
		BYTE* src = (BYTE*)m_ImageSrc.GetPtr1D();
		BYTE* dst = (BYTE*)m_Image.GetPtr1D();
		convertToRGB((CamDevice *)(Para), src, dst);
		saveImage((CamDevice *)(Para), cnt++);
		
	}
}

void showMenu()
{
	printf("Menu:\n");
	printf("1. List connected devices\n");
	printf("2. Exit\n");

	printf("Enter selection: ");
}

void showConnectedDevices()
{
	printf("\n******Camera List******\n");
	int camNum = GetConnectedCamNumber();
	for (int i = 0; i<camNum; i++)
	{
		CamDevice device;
		device.id = i;
		device.model.assign(CamGetDeviceInfo(i, INFO_MODEL_NAME));
		device.serialNumber.assign(CamGetDeviceInfo(i, INFO_SERIAL_NUM));
		device.usbType.assign(CamGetDeviceInfo(i, INFO_USB_TYPE));
		device.fw.assign(CamGetDeviceInfo(i, INFO_DATE_TIME));
		printf("%d. Camera Model: %s\n   Serial Number: %s\n   USB Type: %s\n   Firmware: %s\n\n", 
			(i+1), 
			device.model.c_str(), 
			device.serialNumber.c_str(), 
			device.usbType.c_str(), 
			device.fw.c_str());
		camDeviceList.push_back(device);
	}

	printf("Select device: ");
}
void control_command(CAMPTR ptrCam, unsigned int value)
{
	unsigned char cmd[8];

	cmd[0] = (value >> 28) & 0xf;
	cmd[1] = (value >> 24) & 0xf;
	cmd[2] = (value >> 20) & 0xf;
	cmd[3] = (value >> 16) & 0xf;
	cmd[4] = (value >> 12) & 0xf;
	cmd[5] = (value >> 8) & 0xf;
	cmd[6] = (value >> 4) & 0xf;
	cmd[7] = (value >> 0) & 0xf;


	CamSetCtrl(ptrCam, CTRL_GAIN, cmd[0]);
	CamSetCtrl(ptrCam, CTRL_GAIN, cmd[1]);
	CamSetCtrl(ptrCam, CTRL_GAIN, cmd[2]);
	CamSetCtrl(ptrCam, CTRL_GAIN, cmd[3]);
	CamSetCtrl(ptrCam, CTRL_GAIN, cmd[4]);
	CamSetCtrl(ptrCam, CTRL_GAIN, cmd[5]);
	CamSetCtrl(ptrCam, CTRL_GAIN, cmd[6]);
	CamSetCtrl(ptrCam, CTRL_GAIN, cmd[7]);
}

void setColorDefault(CAMPTR ptrCam)
{
	CamSetCtrl(ptrCam, CTRL_EXPOSURE, DEFAULT_EXPOSURE);
	CamSetCtrl(ptrCam, CTRL_GAIN, DEFAULT_GAIN);
	CamSetCtrl(ptrCam, CTRL_WHITEBALANCE_COMPONENT_BLUE, DEFAULT_WB_COMP);
	CamSetCtrl(ptrCam, CTRL_WHITEBALANCE_COMPONENT_RED, DEFAULT_WB_COMP);

	/*
	* erase scale trigger
	*/
	control_command(ptrCam, ERASE_SCALE);

	/*
	* load scale trigger
	*/
	control_command(ptrCam, LOAD_SCALE);

	// ²¯´ÙÅ°´Â¹æ¹ý
	CamStop(ptrCam);
	CamStart(ptrCam);

	// save trigger ÀÌÈÄ, Gain °ªÀÌ º¯ÇßÀ¸¹Ç·Î ²°´Ù°¡ Ä×À»¶§ 64·Î ½ÃÀÛ ÇÒ ¼ö ÀÖµµ·Ï
	CamSetCtrl(ptrCam, CTRL_GAIN, DEFAULT_GAIN);
}

void colorCorrection(CAMPTR ptrCam, int width, int height)
{
	//wImage src(width, height, MV_Y8);
	//wImage dst(width, height, MV_RGB24);
	//CamGetImage(ptrCam, (BYTE*)src.GetPtr1D());

	///*
	//*  calculate the white balance
	//*/
	//Bayer2RGB((char*)src.GetPtr1D(), (char*)dst.GetPtr1D(), width, height, BayerGR2RGB);

	//double normList[3];
	//calNormOfImage(normList, (unsigned char*)dst.GetPtr1D(), width, height);

	//double scaleRed = normList[1] / normList[0];
	//double scaleBlue = normList[1] / normList[2];

	//int settingValueRed = static_cast<int>(round(scaleRed * FLOAT_SCALER));
	//int settingValueBlue = static_cast<int>(round(scaleBlue * FLOAT_SCALER));

	int settingValueRed = 128;
	int settingValueBlue = 180;

	CamSetCtrl(ptrCam, CTRL_WHITEBALANCE_COMPONENT_BLUE, settingValueBlue);

	CamSetCtrl(ptrCam, CTRL_WHITEBALANCE_COMPONENT_RED, settingValueRed);

	/*
	* save trigger
	*/
	control_command(ptrCam, SAVE_SCALE);

	CamStop(ptrCam);
	CamStart(ptrCam);

	CamSetCtrl(ptrCam, CTRL_GAIN, DEFAULT_GAIN);
}

void startCapturing(unsigned int camId)
{
	if (camId > camDeviceList.size())
	{
		printf("Device specified does not exist!\n\n");
		return;
	}
	CamDevice selectedDevice = camDeviceList.at(camId - 1);
	m_pCam = CamOpen(selectedDevice.id, m_Width, m_Height, m_FPS, CallbackFunction, &selectedDevice);

	if (m_pCam == NULL)
	{
		printf("Unable to open connection to camera\n\n");
		return;
	}

	if (selectedDevice.model == "oCam-1MGN-U" || selectedDevice.model == "oCam-1CGN-U")
	{
		m_ImageSrc.Alloc(m_Width, m_Height, MV_Y8);
	}
	else
	{
		m_ImageSrc.Alloc(m_Width, m_Height, MV_YUV422);
	}

	m_Image.Alloc(m_Width, m_Height, MV_RGB24);

	Sleep(50);

	if (CamStart(m_pCam) == 0)
	{
		printf("Unable to start camera successfully\n\n");
		return;
	}

	//Set to default colors
	setColorDefault(m_pCam);

	Sleep(50);

	// Correct the colors
	colorCorrection(m_pCam, m_Width, m_Height);
	initialized = true;

	Sleep(50);

	clock_t begin = clock();

	double timeElapsed = 0;
	do
	{
		timeElapsed = clock() - begin;
		printf("Capturing for %02f\n", (timeElapsed/CLOCKS_PER_SEC));

	} while (timeElapsed < 5000); //For 10 seconds

	CamStop(m_pCam);
	CamClose(m_pCam);
	cnt = 0;
	
	printf("\n");
}

int main()
{
	printf("oCam-1CGN-U Demo\n");
	showMenu();

	string selection;
	while (cin >> selection)
	{
		if (selection == "\n")
		{
			continue;
		}
		if (inSubMenu) 
		{
			int deviceId = atoi(selection.c_str());
			startCapturing(deviceId);
			showMenu();
			inSubMenu = false;
		}
		else
		{	
			if (selection == "1")
			{
				//Clear out list
				camDeviceList.clear();

				//Get connected devices
				showConnectedDevices();
				inSubMenu = true;
			}
			else if (selection == "2")
			{
				printf("Exiting...\n");
				exit(0);
			}
			else
			{
				printf("Invalid input! Please select again\n\n");
				showMenu();
			}
		}
	}
    return 0;
}

