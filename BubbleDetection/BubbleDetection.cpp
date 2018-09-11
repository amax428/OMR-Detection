// BubbleDetection.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "jsoncons/json.hpp"

using namespace std;
using namespace cv;
using jsoncons::json;

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
	float l;
	float t;
	float w;
	float h;
};

struct Item
{
	float l;
	float t;
	float w;
	float h;
	float lw;
	float s;
	int q;
	string d;
	string y;
	int oo;
	string f;
	Field m;
	string f_R;
};

struct Block
{
	string n;
	float l;
	float t;
	float w;
	float h;
	vector<Item> I;
};

struct Page
{
	vector<Block> B;
};

struct OMRData
{
	string n;
	vector<Page> pages;
};

void ExeptionThrow() {
	cout << "Parsing Error!" << endl;
	system("PAUSE");
}



#if 0
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
#endif

bool parseJson(OMRData& data) {
	ifstream is("assets/definition_resolved.js");

	if (!is.is_open()) {
		cout << "Cannot open definition_resolved.js File!" << endl;
		return 0;
	}

	json temp;
	is >> temp;

	string temp_P = "";

	if (temp.has_key("n")) {
		data.n = temp["n"].as<string>();
	}

	if (temp.has_key("P")) {
		temp_P = temp["P"].as<string>();
	}
	else {
		ExeptionThrow();
		return 0;
	}

	json j = json::parse(temp_P);

	for (const auto& j_iter_page : j.array_range()) {
		Page pg;
		string temp_B = "";

		if (!j_iter_page.has_key("B")) {
			continue;
		}

		temp_B = j_iter_page["B"].as<string>();

		json j_B = json::parse(temp_B);

		for (const auto& j_iter_block : j_B.array_range()) {
			Block bl;

			bl.n = j_iter_block["n"].as<string>();
			bl.l = j_iter_block["l"].as<float>();
			bl.t = j_iter_block["t"].as<float>();
			bl.w = j_iter_block["w"].as<float>();
			bl.h = j_iter_block["h"].as<float>();

			string temp_I = "";

			if (!j_iter_block.has_key("I")) {
				continue;
			}

			temp_I = j_iter_block["I"].as<string>();

			json j_I = json::parse(temp_I);

			for (const auto& j_iter_item : j_I.array_range()) {
				Item it;

				it.l = j_iter_item["l"].as<float>();
				it.t = j_iter_item["t"].as<float>();
				it.w = j_iter_item["w"].as<float>();
				it.h = j_iter_item["h"].as<float>();
				it.lw = j_iter_item["lw"].as<float>();
				it.s = j_iter_item["s"].as<float>();
				it.q = j_iter_item["q"].as<int>();
				it.d = j_iter_item["d"].as<string>();
				it.y = j_iter_item["y"].as<string>();
				it.oo = j_iter_item["oo"].as<int>();
				it.f = j_iter_item["f"].as<string>();

				string temp_F = "";

				if (!j_iter_item.has_key("m")) {
					continue;
				}

				temp_F = j_iter_item["m"].as<string>();

				json j_F = json::parse(temp_F);

				it.m.l = j_F["l"].as<float>();
				it.m.t = j_F["t"].as<float>();
				it.m.w = j_F["w"].as<float>();
				it.m.h = j_F["h"].as<float>();

				bl.I.push_back(it);
			}
			pg.B.push_back(bl);
		}

		data.pages.push_back(pg);
	}

	return 1;
}

bool isMarked(Mat img) {
	int width = img.cols;
	int height = img.rows;

	float cropRatio = 0.5;
	
	float l = cropRatio * 0.5;
	float t = cropRatio * 0.5;

	Rect roi = Rect(l * width, t * height, width * cropRatio, height * cropRatio);
	Mat rlt_Img = img(roi);

	int marked_pixel_cnt = 0;
	for (int y = 0; y < rlt_Img.rows; y++) {
		unsigned char *data = rlt_Img.ptr(y);
		for (int x = 0; x < rlt_Img.cols; x++) {
			int val = *data++;
			if (val < 1) {
				marked_pixel_cnt++;
			}
		}
	}

	double percent = marked_pixel_cnt / ((double)rlt_Img.rows * rlt_Img.cols) * 100;

	if (percent > THRES_PERCENT) {
		return true;
	}

	return false;
}

int main()
{
	OMRData data;
	if (!parseJson(data)) {
		return 0;
	}


	Mat src_img = imread("assets/test.jpg");

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

	Mat gray, thres;
	cvtColor(re_img, gray, CV_RGB2GRAY);
	threshold(gray, thres, 230, 255, CV_THRESH_BINARY);

	int width = re_img.cols;
	int height = re_img.rows;

	string marked_str = "1";
	string unmarked_str = "0";

	for (auto& page_iter : data.pages) {
		for (auto& block_iter : page_iter.B) {
			cout << block_iter.n << "----------------------" << endl;
			int left = width * block_iter.l;
			int top = height * block_iter.t;

			Rect blockROI = Rect(left, top, width * block_iter.w, height * block_iter.h);
			Mat block_img = thres(blockROI);

			for (auto& item_iter : block_iter.I) {
				int w = block_img.cols;
				int h = block_img.rows;

				Rect itemROI = Rect(w * item_iter.l, h * item_iter.t, w * item_iter.w, h * item_iter.h);
				Mat item_img = block_img(itemROI);

				float l = item_iter.lw;

				w = item_img.cols;
				h = item_img.rows;

				string str = "";
				while (l + item_iter.m.w < 1) {
					Rect fieldROI = Rect(w * l, h * item_iter.m.t, w * item_iter.m.w, h * item_iter.m.h);
					Mat field_img = item_img(fieldROI);

					if (isMarked(field_img)) {
						str += marked_str;
					}
					else {
						str += unmarked_str;
					}

					l += item_iter.m.w;
				}
				item_iter.f_R = str;

				cout << "t:" << item_iter.y << ", " << "q:" << item_iter.q << ", "
					<< "oo:" << item_iter.oo << ", " << "d:" << item_iter.d << ", "
					<< "f:" << item_iter.f_R << endl;
			}

			/*
			namedWindow(block_iter.n, WINDOW_NORMAL);
			imshow(block_iter.n, block_img);
			*/
		}

	}

	namedWindow(data.n, WINDOW_NORMAL);
	imshow(data.n, src_img);
	waitKey(0);

    return 0;
}

