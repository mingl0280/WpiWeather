#pragma once
#ifndef _WPIWEATHER_H
#define _WPIWEATHER_H


#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <curl/curl.h>
#include <ctime>
#include <cmath>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlstring.h>
#include <libxml/xpathInternals.h>
#include <tft_st7735.h>
#include <tft_manager.h>
#include <tft_field.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <png.h>
#include "cmdline.h"
#define PNG_FILE_OPEN_ERR 400
#define PNG_STRUCT_CREATE_ERR 402
#define PNG_GET_INFO_ERR 404
#define PNG_SET_JMP_ERR 406
#define PNG_INVALID_FILE_FORMAT 408
#define PNG_INIT_SUCCESS 0

using namespace std;

enum conditions
{
	tornado = 0,
	tropical_storm = 1,
	hurricane = 2,
	severe_thunderstorms = 3,
	thunderstorms = 4,
	mixed_rain_and_snow = 5,
	mixed_rain_and_sleet = 6,
	mixed_snow_and_sleet = 7,
	freezing_drizzle = 8,
	drizzle = 9,
	freezing_rain = 10,
	showers = 11,
	snow_flurries = 13,
	light_snow_showers = 14,
	blowing_snow = 15,
	snow = 16,
	hail = 17,
	sleet = 18,
	dust = 19,
	foggy = 20,
	haze = 21,
	smoky = 22,
	blustery = 23,
	windy = 24,
	cold = 25,
	cloudy = 26,
	mostly_cloudy_night_ = 27,
	mostly_cloudy_day_ = 28,
	partly_cloudy_night_ = 29,
	partly_cloudy_day_ = 30,
	clear_night_ = 31,
	sunny = 32,
	fair_night_ = 33,
	fair_day_ = 34,
	mixed_rain_and_hail = 35,
	hot = 36,
	isolated_thunderstorms = 37,
	scattered_thunderstorms = 38,
	scattered_showers_ = 39,
	scattered_showers = 40,
	heavy_snow = 41,
	scattered_snow_showers = 42,
	partly_cloudy = 44,
	thundershowers = 45,
	snow_showers = 46,
	isolated_thundershowers = 47,
	not_available = 3200
};

struct RGBColor
{
	unsigned int R;
	unsigned int G;
	unsigned int B;
};

struct xPathSet
{
	xmlChar *xPathStmt;
	list<string> GAttribList;
};

class PngProc
{
private:
	png_structp m_pngData;
	png_infop m_pngInfo;
	int m_bitDepth;
	FILE* m_pngFile;


	RGBColor **m_colorRgbBitMap;
	unsigned int **m_color565BitMap;
	int getRowBytes(int width);

	png_uint_32 m_width, m_height;
	int m_color_type;
public:
	PngProc();
	PngProc(char *filename);
	PngProc(string filename);
	int pngInit(char *filename);
	int pngInit(string filename);
	unsigned int **get565BitMap();
	RGBColor **getRGBBitMap();
	png_infop getPNGInfo();
}pngfile;



size_t onDataFetched(char *ptr, size_t size, size_t nmemb, void *stream);
void DisplayData(string content);
void tftDisplay(string location, string wind_dir, string wind_vel, string temp, string weather, string weatherID, string humd, bool is_intl_unit);
void tftInit();
void iconMapInit();
void tftWriteImage(string filename);
void doDataFetchAndDisplay(char location[], char unittype[]);
string& replace_all(string& str, const string& old_value, const string& new_value);
string& replace_all_distinct(string& str, const string& old_value, const string& new_value);
map<string, string> *GetXMLValuesByXPath(xmlXPathContextPtr xPathContext, xPathSet xPath[], int counts);
void setAllXPaths(xPathSet &item, xmlChar* stmt, list<string> lst);
string* getPropByXPath(xmlChar *xPath, xmlXPathContextPtr xPathContext, xmlChar **params, int count);
string* getPropByXPath(xmlXPathObjectPtr xPathResult, xmlChar **params, int count);
//void do_shortInit(unsigned char, unsigned char, unsigned char, int);
string InpLocation = "";

TFT_ST7735 tft = *new TFT_ST7735(0, 24, 25, 32000000);
TFT_manager tManager = *new TFT_manager();

map<string, TFT_field*> tFields;
map<int, string> iconMap;

string processer = "";

#endif // !_WPIWEATHER_H