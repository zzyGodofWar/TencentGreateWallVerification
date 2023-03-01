#include <Windows.h>
#include <gdiplus.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace Gdiplus;

#pragma comment(lib,"gdiplus.lib")

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

typedef struct ResultLine
{
	int x1;
	int x2;
	int midx;
	int y;
}ResultLine;

typedef struct ResultPoint
{
	int midx;
	int xtol;
	int xcount;
	int ymin, ymax;
}ResultPoint;

int main()
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;


	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


	CLSID bmpClsid;
	GetEncoderClsid(L"image/bmp", &bmpClsid);

	WCHAR filename[32];
	
	Color ncol[3] = { Color(255, 0, 0),Color(0, 255, 0),Color(0, 0, 255) };

	vector<ResultLine>ResultList;
	vector<ResultPoint>ResultPointList;

	for (int mapid = 1; mapid <= 5;mapid++) {
		wsprintf(filename, L"hk%d.png", mapid);


		ResultList.resize(0);
		ResultPointList.resize(0);

		Bitmap *rbmp = new Bitmap(filename);
		Color dotcol;

		char lineflag;
		int lastx = 0;
		BYTE pr, pg, pb;
		ResultLine tline;

		//cout << rbmp->GetWidth() << " x " << rbmp->GetHeight() << "\n";

		//Bitmap *nmap = new Bitmap(rbmp->GetWidth(), rbmp->GetHeight());
		Bitmap *nmap = rbmp;
		int nidx = 0;
		int reset = 1;

		for (int y = 0; y < rbmp->GetHeight(); y++) {
			lineflag = 0;
			reset = 1;
			for (int x = 0; x < rbmp->GetWidth(); x++) {
				rbmp->GetPixel(x, y, &dotcol);
				pr = dotcol.GetR();
				pg = dotcol.GetG();
				pb = dotcol.GetB();

				if (pr > 115 && pg > 115 && pb > 115
					&& pr < 220 && pg < 220 && pb < 220) {
					if (lineflag == 0) {
						lineflag = 1;
						lastx = x;
					}
					else if (lineflag == 2) {
						if (x - lastx > 120 || x - lastx < 30) {
							continue;
						}

						for (int i = lastx; i <= x; i++) {
							nmap->SetPixel(i, y, ncol[nidx]);
						}

						nidx = (nidx + 1) % 3;
						//cout << "line:" << y << " x1:" << lastx << " x2:" << x << endl;


						tline.x1 = lastx;
						tline.x2 = x;
						tline.y = y;
						tline.midx = (lastx + x) / 2;
						ResultList.push_back(tline);

						lineflag = 0;
					}
					else if (lineflag == 1) {
						if (x - lastx > 5) {
							lineflag = 0;
						}
					}
				}
				else if (pr < 110 && pg < 110 && pb < 110) {
					if (lineflag == 1) {
						lineflag = 2;
					}
				}
				else if (x - lastx > 5) {
					lineflag = 0;
				}
			}
		}

		ResultPoint rpl;
		bool flag;
		for (auto rline = ResultList.begin(); rline != ResultList.end();rline++) {
			if (!ResultPointList.size()) {
				rpl.xcount = 1;
				rpl.xtol = rpl.midx = (*rline).midx;
				rpl.ymin = rpl.ymax = (*rline).y;

				ResultPointList.push_back(rpl);
			}
			else {
				flag = 1;
				for (auto rpoint = ResultPointList.begin(); rpoint != ResultPointList.end();rpoint++) {
					if (abs((*rpoint).midx - (*rline).midx) <= 3) {
						if (((*rline).y >= (*rpoint).ymin - 3) && ((*rline).y <= (*rpoint).ymax + 3)) {
							(*rpoint).xcount++;
							(*rpoint).xtol += (*rline).midx;
							(*rpoint).midx = (*rpoint).xtol / (*rpoint).xcount;
							(*rpoint).ymin = min((*rpoint).ymin, (*rline).y);
							(*rpoint).ymax = max((*rpoint).ymax, (*rline).y);
							flag = 0;
							break;
						}
					}
				}
				if (flag) {
					rpl.xcount = 1;
					rpl.xtol = rpl.midx = (*rline).midx;
					rpl.ymin = rpl.ymax = (*rline).y;

					ResultPointList.push_back(rpl);
				}
			}
		}
		
		int max = 0;
		int xpos;
		//cout << "size:" << ResultPointList.size() << endl;
		for (auto rpoint = ResultPointList.begin(); rpoint != ResultPointList.end(); rpoint++) {
			/*cout << "Point:" << (*rpoint).xcount << " " << (*rpoint).midx 
				<< " " << (*rpoint).ymax << " " << (*rpoint).ymin << endl;*/
			if (max < (*rpoint).xcount) {
				max = (*rpoint).xcount;
				xpos = (*rpoint).midx;
			}
		}
		
		for (int y = 0; y < nmap->GetHeight();y++) {
			nmap->SetPixel(xpos, y, ncol[0]);
		}

		wsprintf(filename, L"f%d.png", mapid);
		nmap->Save(filename, &bmpClsid);
	}
}


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders  
	UINT  size = 0;         // size of the image encoder array in bytes  

	ImageCodecInfo* pImageCodecInfo = NULL;

	//2.获取GDI+支持的图像格式编码器种类数以及ImageCodecInfo数组的存放大小  
	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure  

	 //3.为ImageCodecInfo数组分配足额空间  
	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure  

	 //4.获取所有的图像编码器信息  
	GetImageEncoders(num, size, pImageCodecInfo);

	//5.查找符合的图像编码器的Clsid  
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success  
		}
	}

	//6.释放步骤3分配的内存  
	free(pImageCodecInfo);
	return -1;  // Failure  
}