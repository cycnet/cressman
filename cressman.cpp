// cressman.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cressman.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorIThm>
#include <math.h>
#define ULONG_PTR ULONG
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;
using namespace Gdiplus;
CLSID encoderClsid;
template<typename T>
struct TPoint3D
{
	TPoint3D() : x(0), y(0), z(0) {}
	TPoint3D(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
	T x;
	T y;
	T z;
};
typedef TPoint3D<double> Point3D;
struct lnglat
{
	float lng;
	float lat;
	bool operator < (const lnglat&lgt)const
	{
		return lng<lgt.lng;
	}
	bool operator > (const lnglat&lgt)const
	{
		return lng>lgt.lng;
	}
};
struct posz
{
	int x;
	int y; 
	double z;
};
struct colorz
{
	int x;
	int y;
	int z;
	bool operator <(const colorz&drs)const	
	{  
		return   x < drs.x;
	}
    bool   operator >  (const  colorz&   drs   )const
	{  
		return   x > drs.x;
    }
};
static COLORREF rr_color[7]={
	RGB(255,255,255),
		RGB(166,242,143),
		RGB(61,186,61),
		RGB(97,184,255),
		RGB(0,0,255),
		RGB(255,0,255),
		RGB(128,0,64)
};

static COLORREF tt_color[24] = {
	RGB(40,93,253),
	RGB(32,125,253),
	RGB(28,141,253),
	RGB(24,157,253),
	RGB(20,173,253),
	RGB(16,189,253),
	RGB(12,205,253),
	RGB(8,222,253),
	RGB(4,237,253),
	RGB(0,253,253),
	RGB(122,255,254),
	RGB(255,255,255),
	RGB(254,254,205),
	RGB(254,254,156),
	RGB(255,220,100),
	RGB(255,210,35),
	RGB(255,186,30),
	RGB(255,163,25),
	RGB(251,140,52),
	RGB(247,128,79),
	RGB(250,110,65),
	RGB(255,89,25),
	RGB(255,45,14),
	RGB(255,0,0)
};

float l_lng=109.0;
float r_lng=118.1;
float b_lat=20.0;
float t_lat=26.1;
double lng=0.0,lat=0.0;
double fbl=0.01;
int nWidth=0;
int nHeight=0;
double count_z=0.0;

vector<Point3D> m_vecPoints;
TCHAR szPath[MAX_PATH],drive[MAX_PATH],dir[MAX_PATH],fname[MAX_PATH],ext[MAX_PATH],pp [MAX_PATH],wind_u[MAX_PATH],wind_v[MAX_PATH];
void setPath();
void Read(string filename);
void Dump();
void cressman();
double jisuan(float c_lng,float c_lat,float mz);
void readmap(string txt,Graphics *g1);
int getsize(ifstream *read_file);

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num= 0;
	UINT size= 0;
	ImageCodecInfo* pImageCodecInfo= NULL;
	GetImageEncodersSize(&num, &size);
	if(size== 0)
	{
		return -1;
	}
	pImageCodecInfo= (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo== NULL)
	{
		return -1;
	}
	
	GetImageEncoders(num, size, pImageCodecInfo);
	
	for(int j=0; j< num; ++j)
	{
		if(wcscmp(pImageCodecInfo[j].MimeType, format)== 0)
		{
			*pClsid= pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}
	
	free(pImageCodecInfo);
	return -1;
	
}
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	char filepath[MAX_PATH];
	memset(filepath,0,MAX_PATH);
	setPath();
	sprintf(filepath,"%sstation.txt",szPath);

	Read(filepath);
	Dump();
	cressman();

	return 0;
}

void setPath()
{
	GdiplusStartupInput gdiplusstartupinput;
	ULONG_PTR gdiplustoken;
	GdiplusStartup(&gdiplustoken, &gdiplusstartupinput, NULL);
	
	GetModuleFileName( NULL, szPath, MAX_PATH );
	_tsplitpath( szPath,drive,dir,fname,ext );
	strcpy( szPath, drive );
	strcat( szPath, dir );
	return;
}

void Read(string filename)
{
	ifstream in(filename.c_str());
	if(!in) {
		cout<<"打开样本文件失败！"<< endl;
		return;
	}
	
	double x, y, z;
	m_vecPoints.clear();
	count_z=0.0;
	while(!in.eof()) {
		string line;
		getline(in, line);
		istringstream iss(line);
		iss >>x >> y >> z;
		count_z+=z;
		m_vecPoints.push_back(Point3D(x, y, z));
	}

	in.close();
	in.clear();
	
}

void Dump()// const throw()
{
	for(int i=0; i<m_vecPoints.size(); i++)
		cout << m_vecPoints[i].x << "\t" << m_vecPoints[i].y << "\t" << m_vecPoints[i].z << endl;
	cout << count_z/m_vecPoints.size() << endl;
}

void cressman()
{
	nWidth=(r_lng-l_lng)/fbl;
	nHeight=(t_lat-b_lat)/fbl;
	float* cressman_z = new float[nWidth*nHeight];

	int cressman_size=nWidth*nHeight;
	float chushi=count_z/m_vecPoints.size();

	float R=1.0;
/*
 	for(int y=0;y<nHeight;y++)
	{
		for(int x=0;x<nWidth;x++)
		{
			int count=0;
			float myz=0.0;
			float min=R+10,outz=chushi;
			float weight=0.0,fengzhi=0.0,fengmu=0.0;
			for(int j=0;j<m_vecPoints.size();j++)
			{
				float lng=x*fbl+l_lng;
				float lat=t_lat-y*fbl;
				float r=::sqrt(::pow((lng - m_vecPoints[j].x), 2) + ::pow(( lat - m_vecPoints[j].y), 2));
				if(r<R)
				{
					myz+=m_vecPoints[j].z;
					count++;
				}
				
			}
			if(count>0)
			{
				cressman_z[y*nWidth+x]=myz/count;
			}else
			{
				cressman_z[y*nWidth+x]=chushi;
			}
		}
		}
	*/
	for(int nn=0;nn<cressman_size;nn++)
	{
		cressman_z[nn]=chushi;
	}
	
	vector<double> vecZs;
	vector<posz> vecZs2;
	
	for(int mm=0;mm<5;mm++)
	{
		for(int y=0;y<nHeight;y++)
		{
			for(int x=0;x<nWidth;x++)
			{
				float z=0.0;
				z=jisuan(l_lng+x*fbl,b_lat+y*fbl,cressman_z[y*nWidth+x]);
				cressman_z[y*nWidth+x]=z;
			}
		}
	}


	
	for(int y=0;y<nHeight;y++)
	{
		for(int x=0;x<nWidth;x++)
		{
			float z=cressman_z[y*nWidth+x];
			vecZs.push_back(z);	
			posz tmpposz;
			tmpposz.x=x;
			tmpposz.y=y;
			tmpposz.z=z;
			vecZs2.push_back(tmpposz);
		}
	}


	int sbcount=24;
	int len=vecZs.size();
	vector<double>::iterator iter;
	iter = max_element(vecZs.begin(), vecZs.end());
	double max = *iter;
	iter = min_element(vecZs.begin(), vecZs.end());
	double min = *iter;
	double interval = (max - min) / (sbcount - 1);
	cout <<interval<<",m_dThickMax:"<<max <<",m_dThickMin:"<< min<< endl;


	delete cressman_z;


	int mHeight=nHeight+100;
	int mWidth=nWidth+150;

	BYTE* picData = new BYTE[mWidth*mHeight*4];
	memset(picData,0,mWidth*mHeight*4);
	
	Bitmap bm(mWidth,mHeight,mWidth*4,PixelFormat32bppARGB,picData);//创建bitmap  

	Graphics g1(&bm);
	g1.SetSmoothingMode(SmoothingModeHighQuality);
	CRect rect;
	Pen m_Pen(Color::Black,1.0f);
	Pen m_PenRed(Color::Red,1.0f);
	PointF points[10] ;
	SolidBrush backbrush(Color(255,255,255,255));
	
	g1.FillRectangle(&backbrush,0,0,mWidth,mHeight);


	int*m_color=new int[mWidth*mHeight];
	memset(m_color,0,mWidth*mHeight*4);

	char provicePath[MAX_PATH];
	sprintf(provicePath,"%smapinfo\\广东省界.txt",szPath);
	readmap(provicePath,&g1);
	//sprintf(provicePath,"%smapinfo\\县边界.txt",szPath);
	//readmap(provicePath,&g1);
	
	cout <<"地理信息完成"<< endl;


	for(int i=0;i<vecZs2.size();i++)
	{
		posz tmpposz=vecZs2[i];

		double z =tmpposz.z;
		int index = (int)((z - min) / interval);
		int row=tmpposz.y+30;
		int col=tmpposz.x+45;
	//	if(picData[row*mWidth*4+col*4+0] == 254 &&picData[row*mWidth*4+col*4+1] == 254 && picData[row*mWidth*4+col*4+2] == 254)
		{
			picData[row*mWidth*4+col*4+0] = GetBValue(tt_color[index]);    //B
			picData[row*mWidth*4+col*4+1] = GetGValue(tt_color[index]);
			picData[row*mWidth*4+col*4+2] = GetRValue(tt_color[index]);
			picData[row*mWidth*4+col*4+3] = 255;    //Alpha
			m_color[row*mWidth+col]=index;
		
		}
		
	}
	cout << "色斑完成"<<endl;


	colorz t_colorz;
	vector< vector<colorz> >vv_cz;
	vector<colorz> v_cz;
	vv_cz.clear();
	v_cz.clear();


	for(int t=0;t<sbcount;t++)
	{
		vv_cz.push_back(v_cz);
	}
	
	for( y=30;y<nHeight+30;y++)
	{
		for(int x=45;x<nWidth+45;x++)
		{
			int i1=m_color[(y-1)*mWidth+x];
			int i2=m_color[(y+1)*mWidth+x];
			int i3=m_color[y*mWidth+x-1];
			int i4=m_color[y*mWidth+x+1];
			int i5=m_color[(y-1)*mWidth+x-1];
			int i6=m_color[(y+1)*mWidth+x+1];
			int i7=m_color[(y-1)*mWidth+x+1];
			int i8=m_color[(y+1)*mWidth+x-1];
			int i9=m_color[y*mWidth+x];
			//if(i1==i2 ||||i3==i4 || i5==i6 || i7== i8 )
			if(i1==i9 &&i2==i9 && i3==i9 && i4== i9 && i5== i9&& i6== i9 && i7== i9 && i8== i9)
			{

			}else
			{
				if(i9-i1==1 ||i9-i2==1  ||i9-i3==1  ||i9-i4==1  ||i9-i5==1  ||i9-i6==1  ||i9-i7==1  ||i9-i8==1 )
				{

					t_colorz.x=x;
					t_colorz.y=y;
					t_colorz.z=i9;
					for(int t=0;t<sbcount;t++)
					{
						if(i9==t)
						{
							vv_cz[t].push_back(t_colorz);	
						}
					}
				}
				
			}
		}
	}

	PointF b_p;
	PointF e_p;
	PointF  tpoint;
	char t_char[256];
	CString t_string="";
	FontFamily fontfamily(L"宋体");  
	Gdiplus::Font font(&fontfamily,12,FontStyleRegular,UnitPixel); 
	Gdiplus::Font font1(&fontfamily,12,FontStyleRegular,UnitPixel); 
	Gdiplus::Font font10(&fontfamily,10,FontStyleRegular,UnitPixel); 
	SolidBrush gzbrush(Color(255,0,0,0));
	SolidBrush redbrush(Color(255,255,0,0));

	rect.left=45;
	rect.right=nWidth+45;
	rect.top=30;
	rect.bottom=nHeight+30;
	
//	vector<int>v_x;

	for(t=0;t<vv_cz.size()-1;t++)
	{
		v_cz=vv_cz[t];
		sort(v_cz.begin(),v_cz.end());//升序
		colorz t_colorz;
		int md=v_cz.size()/10;
		int mcount=0;

	//	v_x.clear();
	//	if(v_cz.size()>200)
		{
			for(int k=0;k<v_cz.size();k++)
			{
				
				t_colorz=v_cz[k];
				int x=t_colorz.x;
				int y=t_colorz.y;
				int z=t_colorz.z;
				
				
				if( k%md==1  && k>md  && k<(v_cz.size()-md) && v_cz.size()>200)
				{
					
					if(mcount%2==0)
					{
						memset(t_char,0,256);
						sprintf(t_char,"%0.1f",z*interval+min);
						t_string=t_char;
						tpoint.X=x-6;
						tpoint.Y=y-4;
						if(y>rect.top+2 && y<rect.bottom-2)
						{
							g1.DrawString(t_string.AllocSysString(),-1,&font,tpoint,&gzbrush);
							//v_x.push_back(x);
						}
						
					}
					
					mcount++;
				}else
				{
					picData[y*mWidth*4+x*4+0] = 0;    //B
					picData[y*mWidth*4+x*4+1] = 0;
					picData[y*mWidth*4+x*4+2] = 0;
					picData[y*mWidth*4+x*4+3] = 255;    //Alpha
				}
				
			}
		}
	
		
	}
	

	cout << "等值线完成"<<endl;
	delete m_color;  


	for( i=0; i<m_vecPoints.size(); i++)
	{
		memset(t_char,0,256);
		sprintf(t_char,"%0.1f",m_vecPoints[i].z);
		t_string=t_char;
		tpoint.X=(m_vecPoints[i].x-l_lng)/fbl+30;
		tpoint.Y=(t_lat-m_vecPoints[i].y)/fbl+45;
		g1.DrawEllipse(&m_PenRed,(int)tpoint.X-3,(int)tpoint.Y-3,2,2);
		g1.DrawString(t_string.AllocSysString(),-1,&font10,tpoint,&redbrush);
	}


	g1.FillRectangle(&backbrush,-1,-1,mWidth+1,rect.top+1);
	g1.FillRectangle(&backbrush,-1,rect.bottom,mWidth+1,mHeight-rect.bottom+1);
	g1.FillRectangle(&backbrush,-1,0,rect.left+1,mHeight+1);
	g1.FillRectangle(&backbrush,rect.right,-1,mWidth-rect.right+1,mHeight+1);

	points[0].X=rect.left;
	points[0].Y=rect.top;
	points[1].X=rect.right;
	points[1].Y=rect.top;
	
	points[2].X=rect.right;
	points[2].Y=rect.bottom;
	
	points[3].X=rect.left;
	points[3].Y=rect.bottom;
	points[4]=points[0];
 	g1.DrawLines(&m_Pen, points, 5);
	cout <<"边框完成"<< endl;

	

	for(int n=0; n<=nWidth; n=n+100) 
	{
		float i=l_lng+n*0.01;
	//	float j=t_lat-m*0.01;
		b_p.Y=rect.bottom;
		e_p.Y=rect.bottom+8;
		
		b_p.X=rect.left+n;
		e_p.X=b_p.X;
		g1.DrawLine(&Pen(Color::Black, 1),b_p,e_p);

		memset(t_char,0,256);
		sprintf(t_char,"%0.0f゜E",i);
		t_string=t_char;
		tpoint.X=b_p.X-3;
		tpoint.Y=e_p.Y+3;
		g1.DrawString(t_string.AllocSysString(),-1,&font,tpoint,&gzbrush);

	}

	for(int  m=0; m<=nHeight; m=m+100)
	{
		float j=b_lat+m*0.01;
		b_p.X=rect.left-8;
		e_p.X=rect.left;
	
		b_p.Y=rect.bottom-m;
		e_p.Y=b_p.Y;
		g1.DrawLine(&Pen(Color::Black, 1),b_p,e_p);

		memset(t_char,0,256);
		sprintf(t_char,"%0.0f゜N",j);
		t_string=t_char;
		tpoint.X=rect.left-43;
		tpoint.Y=rect.bottom-m-10;
		g1.DrawString(t_string.AllocSysString(),-1,&font,tpoint,&gzbrush);
	}
	
	memset(t_char,0,256);
	
	sprintf(t_char,"%s", "test");
	t_string=t_char;
	tpoint.X=rect.left+100;
	tpoint.Y=rect.bottom+38;
	g1.DrawString(t_string.AllocSysString(),-1,&font,tpoint,&gzbrush);
	cout<<"底部信息完成"<< endl;

	for(int k=0;k<sbcount;k++)
	{
		SolidBrush backbrush2(Color(255,GetRValue(tt_color[k]),GetGValue(tt_color[k]),GetBValue(tt_color[k])));
		g1.FillRectangle(&backbrush2,rect.right+8,rect.bottom-30-k*20,20,20);
		g1.DrawRectangle(&m_Pen,rect.right+8,rect.bottom-30-k*20,20,20);
		
		if(k<sbcount-1)
		{
			memset(t_char,0,256);
			sprintf(t_char,"%0.1f",min+interval*k);
			t_string=t_char;
			tpoint.X=rect.right+30;
			tpoint.Y=rect.bottom-34-k*20;
			g1.DrawString(t_string.AllocSysString(),-1,&font,tpoint,&gzbrush);
		}
	
		//m_dThickMax
		//double interval = (m_dThickMax - m_dThickMin) / (24 - 1);
	}
	cout<<"色标完成"<<endl;

	

	char outpng[MAX_PATH];
	sprintf(outpng,"%sout.png",szPath);
	CString outfile=outpng;
	BSTR outbstr;
	outbstr=outfile.AllocSysString();
	GetEncoderClsid(L"image/png", &encoderClsid);
	bm.Save(outbstr,&encoderClsid);


}

double jisuan(float c_lng,float c_lat,float mz)
{
	float weight=0.0,m_z=0.0,fengzhi=0.0,fengmu=0.0;
	float R=2.0;
	float R_pow=::pow(R,2);
	float r_pow=0.0,r=0.0;
	for(int i=0; i<m_vecPoints.size(); i++)
	{
		r_pow=::pow((c_lng - m_vecPoints[i].x), 2) + ::pow(( c_lat - m_vecPoints[i].y), 2);
		r=::sqrt(r_pow);
		
		if(r<R && r>=0)
		{
			weight=(R_pow-r_pow)/(R_pow+r_pow);
// 			int x=(m_vecPoints[i].x-l_lng)/fbl;
// 			int y=(t_lat-m_vecPoints[i].y)/fbl;
// 			int pos=y*nWidth+x;
// 			m_z=cressman_z[pos]-m_vecPoints[i].z;
			m_z=mz-m_vecPoints[i].z;

		//	m_z=m_vecPoints[i].z;

			fengzhi+=weight*weight*m_z;
			fengmu+=weight;
		}
	}
	double z=mz;
	if(fengmu>0)
	{
		z=fengzhi/fengmu;
		if(abs(z)<10)
		{
			z+=mz;
		}else
		{
			z=mz;
		}
		
	}

	return z;
}

int getsize(ifstream *read_file)
{
	string line,input_ret;
	int ret=0;
	vector<string> vString;
	getline(*read_file, line);
	
	stringstream input(line);
	vString.clear();
	while(input>>input_ret)
	{
		vString.push_back(input_ret);
	}
	ret=atof(vString[0].c_str());
	return ret;
}
void readmap(string txt,Graphics *g1)
{
	int nWidth=(r_lng-l_lng)/fbl+100;
	int nHeight=(t_lat-b_lat)/fbl+150;
	
	ifstream read_file;
	string line,input_ret;
	vector<string> vString;
	lnglat m_lnglat;
	vector<lnglat> v_lnglat;
	
	Pen m_Pen(Color(255, 0, 0, 0),0.8f);
	SolidBrush m_Brush(Color(255, 254, 254, 254));
	read_file.open(txt.c_str());
	if(!read_file.is_open())
	{
		return;
	}
	v_lnglat.clear();
	int count=0,count2=0;
	int pos=0;
	for(int i=0;i<3;i++)
	{
		getline(read_file, line);
	}
	count=getsize(&read_file);
	
	int allcount=0;
	for(i=0;i<count;i++)
	{
		count2=getsize(&read_file);
		PointF *p=new PointF[count2];
		for(int j=0;j<count2;j++)
		{
			getline(read_file, line);
			stringstream input(line);
			vString.clear();
			while(input>>input_ret)
			{
				vString.push_back(input_ret);
			}
			
			lng=atof(vString[0].c_str());
			lat=atof(vString[1].c_str());
			int x=(lng-l_lng)/fbl;
			int y=(t_lat-lat)/fbl;
			
			p[j].X=x+45;
			p[j].Y=y+30;
			
			
		}
		
		GraphicsPath path;
		path.AddLines(p,count2);
		g1->FillPath(&m_Brush,&path);
		g1->DrawLines(&m_Pen, p, count2);
		if(p)delete p;
	}
	
	read_file.close();
	read_file.clear();
	return;
}
