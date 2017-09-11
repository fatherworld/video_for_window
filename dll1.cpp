
#include "stdafx.h"
#include <string>
#include<iostream>
#include "afxdialogex.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h> // for String... functions
#include <crtdbg.h> // for _ASSERTE 
#include <atlstr.h>
#include "ImageHelper/include/BmpHelper.h"
#include "ImageHelper/src/BmpHelper.c"
#include "ImageHelper/include/Dump.h"
#include "ImageHelper/include/Dump.c"

#define DLL1_API extern "C" _declspec(dllexport)
#include "Dll1.h"
using namespace std;
#pragma warning(disable:4996)
#pragma once 
typedef struct _IplImage
{
    int  nSize;             /* sizeof(IplImage) */
    int  ID;                /* version (=0)*/
    int  nChannels;         /* Most of OpenCV functions support 1,2,3 or 4 channels */
    int  alphaChannel;      /* Ignored by OpenCV */
    int  depth;             /* Pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
                               IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported.  */
    char colorModel[4];     /* Ignored by OpenCV */
    char channelSeq[4];     /* ditto */
    int  dataOrder;         /* 0 - interleaved color channels, 1 - separate color channels.
                               cvCreateImage can only create interleaved images */
    int  origin;            /* 0 - top-left origin,
                               1 - bottom-left origin (Windows bitmaps style).  */
    int  align;             /* Alignment of image rows (4 or 8).
                               OpenCV ignores it and uses widthStep instead.    */
    int  width;             /* Image width in pixels.                           */
    int  height;            /* Image height in pixels.                          */
    struct _IplROI *roi;    /* Image ROI. If NULL, the whole image is selected. */
    struct _IplImage *maskROI;      /* Must be NULL. */
    void  *imageId;                 /* "           " */
    struct _IplTileInfo *tileInfo;  /* "           " */
    int  imageSize;         /* Image data size in bytes
                               (==image->height*image->widthStep
                               in case of interleaved data)*/
    char *imageData;        /* Pointer to aligned image data.         */
    int  widthStep;         /* Size of aligned image row in bytes.    */
    int  BorderMode[4];     /* Ignored by OpenCV.                     */
    int  BorderConst[4];    /* Ditto.                                 */
    char *imageDataOrigin;  /* Pointer to very origin of image data
                               (not necessarily aligned) -
                               needed for correct deallocation */
}
IplImage;
HWND   capWnd;
VIDEOHDR* hdr;
DWORD  fourcc;
int width, height;
int widthSet, heightSet;
HIC    hic;
IplImage* frame;
CAPDRIVERCAPS caps;// CAPDRIVERCAPS 结构，定义驱动器性能
CAPTUREPARMS m_Parms;// CAPTURE PARMS 结构，定义捕捉参数
// 
// int* YV16ToRGB(byte[] src, int width, int height){  
//     int numOfPixel = width * height;  
//     int positionOfU = numOfPixel;  
//     int positionOfV = numOfPixel/2 + numOfPixel;  
//     int[] rgb = new int[numOfPixel*3];  
//     for(int i=0; i<height; i++){  
//         int startY = i*width;  
//         int step = i*width/2;  
//         int startU = positionOfU + step;  
//         int startV = positionOfV + step;  
//         for(int j = 0; j < width; j++){  
//             int Y = startY + j;  
//             int U = startU + j/2;  
//             int V = startV + j/2;  
//             int index = Y*3;  
//             rgb[index+R] = (int)((src[Y]&0xff) + 1.4075 * ((src[V]&0xff)-128));  
//             rgb[index+G] = (int)((src[Y]&0xff) - 0.3455 * ((src[U]&0xff)-128) - 0.7169*((src[V]&0xff)-128));  
//             rgb[index+B] = (int)((src[Y]&0xff) + 1.779 * ((src[U]&0xff)-128));  
//         }  
//     }  
//     return rgb;  
// }  


//DWORD转string
void DwordToString(string &output,DWORD input)
{
    CString tempS;
    tempS.Format(_T("%d"),input);
    output=tempS.GetBuffer(0);
}

int convert_yuv_to_rgb_pixel(int y, int u, int v)  
{  
    unsigned int pixel32 = 0;  
    unsigned char *pixel = (unsigned char*)&pixel32;  
    int r, g, b;  
    r = y + (1.370705 * (v-128));  
    g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));  
    b = y + (1.732446 * (u-128));  
    if(r > 255) r = 255;  
    if(g > 255) g = 255;  
    if(b > 255) b = 255;  
    if(r < 0) r = 0;  
    if(g < 0) g = 0;  
    if(b < 0) b = 0;  
//     pixel[0] = r * 220 / 256;  
//     pixel[1] = g * 220 / 256;  
//     pixel[2] = b * 220 / 256;  
    pixel[0] = r ;
    pixel[1] = g ;
    pixel[2] = b ;
    return pixel32;  
}  

BYTE clip255(long Value)
{
    BYTE retValue;

    if (Value > 255)
        retValue = 255;
    else if (Value < 0)
        retValue = 0;
    else
        retValue = (BYTE)Value;

    return retValue;
}

/*yuv格式转换为rgb格式*/
//win7采到的数据默认YUY格式，一个像素用2位表示，这里将YUY转换为RGB格式（3个字节），便于显示，不转换会出现黑框框
void YUY2_RGB2_ljh(unsigned char* YUY2buff,unsigned char  *RGBbuff,long dwSize)
{
    unsigned char *orgRGBbuff = RGBbuff;
    for( long count = 0; count < dwSize; count += 4 )
    {
        unsigned char Y0 = *YUY2buff;
        unsigned char U = *(++YUY2buff);
        unsigned char Y1 = *(++YUY2buff);
        unsigned char V = *(++YUY2buff);
        ++YUY2buff;
        long Y,C,D,E;
        unsigned char R,G,B;
        Y = Y0;
        C = Y - 16;
        D = U - 128;
        E = V - 128;
        R = clip255(( 298 * C           + 409 * E + 128) >> 8);
        G = clip255(( 298 * C - 100 * D - 208 * E + 128)>> 8);
        B = clip255(( 298 * C + 516 * D           + 128) >> 8);
        *(RGBbuff)   = B;
        *(++RGBbuff) = G;
        *(++RGBbuff) = R;
        Y = Y1;
        C = Y-16;
        D = U-128;
        E = V-128;
        R = clip255(( 298 * C           + 409 * E + 128) >> 8);
        G = clip255(( 298 * C - 100 * D - 208 * E + 128)>> 8);
        B = clip255(( 298 * C + 516 * D           + 128) >> 8);
        *(++RGBbuff) = B;
        *(++RGBbuff) = G;
        *(++RGBbuff) = R;
        ++RGBbuff;
    }
}


int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width,unsigned int height)  
{  
    unsigned int in, out = 0;  
    unsigned int pixel_16;  
    unsigned char pixel_24[3];  
    unsigned int pixel32;  
    int y0, u, y1, v;  
    for(in = 0; in < width * height * 2; in += 4) {  
        pixel_16 =  
            yuv[in + 3] << 24 |  
            yuv[in + 2] << 16 |  
            yuv[in + 1] <<  8 |  
            yuv[in + 0];//YUV422每个像素2字节，每两个像素共用一个Cr,Cb值，即u和v，RGB24每个像素3个字节  
        y0 = (pixel_16 & 0x000000ff);  
        u  = (pixel_16 & 0x0000ff00) >>  8;  
        y1 = (pixel_16 & 0x00ff0000) >> 16;  
        v  = (pixel_16 & 0xff000000) >> 24;  
        pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);  
        pixel_24[0] = (pixel32 & 0x000000ff);  
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;  
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;  
        rgb[out++] = pixel_24[0];  
        rgb[out++] = pixel_24[1];  
        rgb[out++] = pixel_24[2];//rgb的一个像素  
        pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);  
        pixel_24[0] = (pixel32 & 0x000000ff);  
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;  
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;  
        rgb[out++] = pixel_24[0];  
        rgb[out++] = pixel_24[1];  
        rgb[out++] = pixel_24[2];  
    }  
    return 0;  
}  

//version1
int save_bmp(unsigned char * data,int data_size,int w,int h,const char* file)  
{  
    BITMAPINFO m_bitmapinfo;
    capGetVideoFormat(capWnd,&m_bitmapinfo,sizeof(m_bitmapinfo));
    string bitCount;
    DwordToString(bitCount,m_bitmapinfo.bmiHeader.biBitCount);
    cout<<"此时的biBitCount: "<<bitCount<<endl;

    BMP in_img;
    unsigned char* rgb;
    memset(&in_img,0,sizeof(BMP));

    in_img.channels=3;
    in_img.width=w;
    in_img.height=h;

    in_img.data=(unsigned char*)malloc(3*640*480);
    memset(in_img.data,0,(in_img.channels)*(in_img.width)*in_img.height);
    YUY2_RGB2_ljh(data,in_img.data,data_size);
  //  convert_yuv_to_rgb_buffer(data,in_img.data,in_img.width,in_img.height);
//     WriteFileHeader(out, &in_img, BIT24);
//     WriteFileInfoHead(out,&in_img,BIT24);
    
    int retl=WriteBMP(file,&in_img,BIT24);
   // int ret1= dump_bmp((char*)file,(char*)in_img.data,in_img.width,in_img.height,in_img.channels,DUMP_UCHAR);
   
    /*
    位图文件头  
        BITMAPFILEHEADER bmpheader;   
        BITMAPINFO bmpinfo; 
        bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);   
        bmpinfo.bmiHeader.biWidth = w;   
        bmpinfo.bmiHeader.biHeight = 0-h;   
        bmpinfo.bmiHeader.biPlanes = 1;   
        bmpinfo.bmiHeader.biBitCount = 24;   
        bmpinfo.bmiHeader.biCompression = BI_RGB;   
        bmpinfo.bmiHeader.biSizeImage = 0;   
        bmpinfo.bmiHeader.biXPelsPerMeter = 100;   
        bmpinfo.bmiHeader.biYPelsPerMeter = 100;   
        bmpinfo.bmiHeader.biClrUsed = 0;   
        bmpinfo.bmiHeader.biClrImportant = 0;  
        
    
        bmpheader.bfType = ('M' <<8)|'B';   
        bmpheader.bfReserved1 = 0;   
        bmpheader.bfReserved2 = 0;   
    
        bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmpheader.bfType=0x4d42;
        bmpheader.bfOffBits=(DWORD)sizeof(BITMAPCOREHEADER)+bmpinfo.bmiHeader.biSize;
       bmpheader.bfSize = bmpheader.bfOffBits + w*h*bit/8;  
        bmpheader.bfSize=sizeof(BITMAPFILEHEADER)+bmpinfo.bmiHeader.biSize+bmpinfo.bmiHeader.biSizeImage;
    
        
    
        fwrite(&bmpheader,sizeof(BITMAPFILEHEADER),1,out);   
        fwrite(&bmpinfo.bmiHeader,sizeof(BITMAPINFOHEADER),1,out);  
        fwrite(data,data_size,1,out);  
        */
    return retl;
} 
   



/**
	 * 获取相机预览时的图片高度
	 * @return 图片高度
	 */
int getCameraPreviewHeight()
{
    BITMAPINFO m_bitmapinfo;
    capGetVideoFormat(capWnd,&m_bitmapinfo,sizeof(m_bitmapinfo));
    return m_bitmapinfo.bmiHeader.biHeight;
}

/**
	 * 获取相机预览时的图片宽度
	 * @return 图片宽度
	 */
int getCameraPreviewWidth()
{
    BITMAPINFO m_bitmapinfo;
    capGetVideoFormat(capWnd,&m_bitmapinfo,sizeof(m_bitmapinfo));
    return m_bitmapinfo.bmiHeader.biWidth;
}

/**
	 * 获取相机预览时的图片格式
	 * @return 格式类型
	 */
int getCameraPreviewFormat()
{
    BITMAPINFO m_bitmapinfo;
    capGetVideoFormat(capWnd,&m_bitmapinfo,sizeof(m_bitmapinfo));
    cout<<"此时摄像头的预览图片格式为"<<m_bitmapinfo.bmiHeader.biCompression<<endl;
    return m_bitmapinfo.bmiHeader.biCompression;
}




//定义一个回调函数，在流捕捉的时候执行
long  EncodeCallback(HWND hWnd,LPVIDEOHDR lpVHdr)
{
    
//     buffer=new unsigned char[lpVHdr->dwBytesUsed+1];
//     memset(buffer,0,lpVHdr->dwBytesUsed+1);
//     memcpy(buffer,lpVHdr->lpData,lpVHdr->dwBytesUsed);
    unsigned char* buffer1=new unsigned char[lpVHdr->dwBytesUsed];
    memset(buffer1,0,lpVHdr->dwBytesUsed);
    memcpy(buffer1,lpVHdr->lpData,lpVHdr->dwBytesUsed);
    string test_;
    DwordToString(test_,lpVHdr->dwBufferLength);
    int retl=save_bmp(buffer1,lpVHdr->dwBytesUsed,getCameraPreviewWidth(),getCameraPreviewHeight(),"C:\\Users\\User\\Desktop\\pic\\test1.bmp");
    if(lpVHdr->dwBufferLength == sizeof(lpVHdr->lpData))
    {
        free(lpVHdr->lpData);
    }
    return 1;
}

bool grapFram()
{
    capGrabFrameNoStop(capWnd);// 截获当前图像
    capEditCopy(capWnd);// 将图像拷贝到剪贴板
    return true;
}

/**
	 * Camera 初始化
	 * @param module 模块类型    无模块：-1 ，IR模块：0，SLAM模块：1，CR模块：2，MARKER模块：3，WP模块：4
	 * @param mCameraID 相机ID  后置相机：0，前置相机：1
	 * @return false失败，true成功
	 */

bool init(int module, int mCameraID)
{
    capWnd = 0;
    hdr = 0;
    fourcc = 0;
    hic = 0;
    frame = 0;
    width = height = -1;
    widthSet = heightSet = 0;
    char  szDeviceName[100];
    char szDeviceVersion[80];
    HWND hWndC = 0;

    LPWSTR device_name,device_version;
    
    device_name=CA2W(szDeviceName);
    device_version = CA2W(szDeviceVersion);
    if( (unsigned)mCameraID >= 10 )
        mCameraID = 0;
    for( ; mCameraID < 10; mCameraID++ )
    {
        if( capGetDriverDescription( mCameraID,szDeviceName,
            sizeof (szDeviceName), szDeviceVersion,
            sizeof (szDeviceVersion)))
        {
            hWndC = capCreateCaptureWindow ( _T("My Own Capture Window"),
                WS_POPUP | WS_CHILD, 0, 0, 320, 240, 0, 0);
            if( capDriverConnect (hWndC, mCameraID))
                break;
            DestroyWindow( hWndC );
            hWndC = 0;
        }
    }
    if( hWndC )
    {
        capWnd = hWndC;
        hdr = 0;
        hic = 0;
        fourcc = (DWORD)-1;
        memset( &caps, 0, sizeof(caps));
        capDriverGetCaps( hWndC, &caps, sizeof(caps));
        CAPSTATUS status = {};
        capGetStatus(hWndC, &status, sizeof(status));
        ::SetWindowPos(hWndC, NULL, 0, 0, status.uiImageWidth, status.uiImageHeight, SWP_NOZORDER|SWP_NOMOVE);
    //    capSetUserData( hWndC, (size_t)this);
        capSetCallbackOnFrame( hWndC, EncodeCallback);
        capCaptureGetSetup(hWndC,&m_Parms,sizeof(CAPTUREPARMS));
        m_Parms.dwRequestMicroSecPerFrame = 66667/2; // 30 FPS
        m_Parms.fYield=TRUE;
        capCaptureSetSetup(hWndC,&m_Parms,sizeof(CAPTUREPARMS));
        //capPreview( hWndC, 1 );
        capPreviewScale(hWndC,FALSE);
        capPreviewRate(hWndC,1);

        // Get frame initial parameters.
        const DWORD size = capGetVideoFormatSize(capWnd);
        if( size > 0 )
        {
            unsigned char *pbi = new unsigned char[size];
            if( pbi )
            {
                if( capGetVideoFormat(capWnd, pbi, size) == size )
                {
                    BITMAPINFOHEADER& vfmt = ((BITMAPINFO*)pbi)->bmiHeader;
                    widthSet = vfmt.biWidth;
                    heightSet = vfmt.biHeight;
                    fourcc = vfmt.biCompression;
                }
                delete []pbi;
            }
        }
        // And alternative way in case of failure.
        if( widthSet == 0 || heightSet == 0 )
        {
            widthSet = status.uiImageWidth;
            heightSet = status.uiImageHeight;
        }
    }
    return capWnd != 0;
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}



/**
	 * 更新相机预览画面
*/
void UpdatePreview()
{


}

/**
	 * 开始预览
	 * @return false失败，true成功
     */
bool startPreview()
{
    bool suc=false;
    cout<<"此时的参数值为"<<m_Parms.fYield<<endl;
    if(capCaptureSetSetup(capWnd, &m_Parms,sizeof(m_Parms))==TRUE)
    {
        suc = capGrabFrameNoStop(capWnd);// 截获当前图像
        suc = capEditCopy(capWnd);// 将图像拷贝到剪贴板
    }
    return suc;
}

/**
	 * 停止相机的预览和画面的渲染
	 */
void stop()
{
    capCaptureStop(capWnd);// 停止捕捉
}
