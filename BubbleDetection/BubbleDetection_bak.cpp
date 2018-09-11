// BubbleDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

#define MAX_WIDTH	1000
#define MAX_HEIGHT	800
#define THRES_PERCENT 60
//#define Resize

enum Item_Direction {
	LTR = 0,	// left to right
	RTL = 1,	// right to left
	TTB = 2,	//top to bottom
	BTT = 3		//bottom to top
};

struct Field
{
	float lw;	//label widht
	float fw;	//field width
};

struct Item
{
	float l;	//left of item
	float t;	//top of item
	float w;	//item width
	float h;	//item height
	int q;		//index of item
};

struct Section
{
	string name;	
	float l;		//left of section
	float t;		//top of section
	float w;		//section width
	float h;		//section height
	float cw;		//column width
	float hw;		//column height
	float ofs;		//offset of item
	Field m;	//field
	Item_Direction d; //direction
	char f[6] = "ABCDE";
	vector<Item> items;
};


void InitializeDefinition(vector<Section> &sections) {
	int nItems = 42;
	int nItemRows = 7;

	Section section;

	section.name = "Section 1";
	section.l = 0.076;
	section.t = 0.663;
	section.w = 0.825;
	section.h = 0.11;
	section.cw = 1.0 / 6;
	section.hw = 0.14;
	section.ofs = 0.05;
	section.d = LTR;

	Field field;
	field.fw = 0.2;
	field.lw = 0.16;

	section.m = field;

	for (int i = 0; i < nItems; i++) {
		int row = i % nItemRows;
		int col = i / nItemRows;
		
		Item item;

		item.q = i;
		item.w = 0.117;
		item.h = 0.14;
		item.l = section.cw * col;
		item.t = section.hw * row;

		section.items.push_back(item);
	}

	sections.push_back(section);
}

void detectItems(Section section, Mat &secImg, vector<Rect> &rcItem) {
	Rect rc;
	int width = secImg.cols;
	int height = secImg.rows;

	int fieldCnt = strlen(section.f);

	for (int i = 0; i < section.items.size(); i++) {
		rc.x = width * (section.items[i].l + section.ofs);
		rc.y = height * section.items[i].t;
		rc.width = width * section.items[i].w;
		rc.height = height * section.items[i].h;

		rcItem.push_back(rc);

	}
}

void detectFields(Section section, Mat &img, vector<Rect> &rcItem, vector<string> &section_result_binary, vector<string> &section_result_string) {

	int fieldCnt = strlen(section.f);
	
	string marked = "1";
	string unmarked = "0";

	
	for (int i = 0; i < rcItem.size(); i++) {
		string result_binary = "";
		string result_string = "";

		vector<Rect> rcFields;
		switch (section.d)
		{
		case LTR:
			for (int j = 0; j < fieldCnt; j++) {
				Rect rc;
				int width = rcItem[i].width / fieldCnt;
				rc.x = rcItem[i].x + width * j;
				rc.y = rcItem[i].y;
				rc.width = width;
				rc.height = rcItem[i].height;
				rcFields.push_back(rc);
			}
			break;
		case RTL:
			break;
		case TTB:
			break;
		case BTT:
			break;
		default:
			break;
		}

		for (int i = 0; i < rcFields.size(); i++) {
			int w = rcFields[i].width * 0.25;
			int h = rcFields[i].height * 0.25;
			int xStart = rcFields[i].x + w;
			int xEnd = xStart + w * 2;
			int yStart = rcFields[i].y + h;
			int yEnd = yStart + h * 2;

			int marked_pixel_cnt = 0;
			for (int x = xStart; x < xEnd; x++) {
				for (int y = yStart; y < yEnd; y++) {
					int val = img.at<unsigned char>(y, x);
					if (val < 1) {
						marked_pixel_cnt++;
					}
				}
			}

			double percent = marked_pixel_cnt / ((double)w * h * 4) * 100;

			if (percent > THRES_PERCENT) {
				result_binary += marked;
				result_string = section.f[i];
			}
			else {
				result_binary += unmarked;
			}
		}

		section_result_binary.push_back(result_binary);
		section_result_string.push_back(result_string);
	}
}

void drawItems(Mat &secImg, vector<Rect> &rcItem) {
	for (int i = 0; i < rcItem.size(); i++) {
		rectangle(secImg, rcItem[i], Scalar(0, 255, 0), 2, 8, 0);
	}
}

void showResult(vector<string> rlt_bin, vector<string> rlt_str) {
	for (int i = 0; i < rlt_bin.size(); i++) {
		cout << to_string(i + 1) + " - " + rlt_bin[i] + "	" + rlt_str [i]<< endl;
	}
}

int main()
{
	Mat src_img = imread("images/test.jpg");

	if (src_img.empty()) {
		cout << "Error, cannot load test.jpg image" << endl;
		system("PAUSE");
		return 0;
	}

	int src_width = src_img.cols;
	int src_height = src_img.rows;

	Mat re_img = src_img.clone();

#ifdef Resize
	double ratio = 1.0;
	if (src_width > MAX_WIDTH) {
		if (src_width > src_height) {
			ratio = MAX_WIDTH / (double)src_width;
		}
		else {
			ratio = MAX_HEIGHT / (double)src_height;
		}
	}

	int re_width = src_width * ratio;
	int re_height = src_height * ratio;

	resize(src_img, re_img, Size(re_width, re_height));
#endif // Resize

	int width = re_img.cols;
	int height = re_img.rows;

	vector<Section> sections;
	vector<Rect> rcItems;
	vector<string> secResults_binary;
	vector<string> secResults_string;

	InitializeDefinition(sections);

	//imshow("source", src_img);

	for (int i = 0; i < sections.size(); i++) {
		int left = width * sections[i].l;
		int top = height * sections[i].t;

		Rect secROI = Rect(left, top, width * sections[i].w, height * sections[i].h);
		Mat sec_img = re_img(secROI);
		
		detectItems(sections[i], sec_img, rcItems);

		Mat gray, thres;
		cvtColor(sec_img, gray, CV_RGB2GRAY);
		threshold(gray, thres, 230, 255, CV_THRESH_BINARY);

		drawItems(sec_img, rcItems);

		detectFields(sections[i], thres, rcItems, secResults_binary, secResults_string);

		showResult(secResults_binary, secResults_string);

		namedWindow(sections[i].name, WINDOW_NORMAL);
		imshow(sections[i].name, sec_img);
	}

	//imshow("rlt", re_img);

	waitKey(0);

    return 0;
}

