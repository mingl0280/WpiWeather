/* Some embedded simulators crash unless the file has .bss and .data sections.
   We provide both by introducing two dummy variables below. */

#include "WpiWeather.h"

int main(int argc, char* argv[])
{
	using namespace cmdline;
	parser argParser;
	argParser.add<string>("location", 'l', "The location of weather report", false, "Albuquerque");
	argParser.add<string>("format", 'f', "the data format of report, SI(International standard units) or Imperial units ", false, "c", oneof<string>("c", "f"));
	argParser.parse_check(argc, argv);
	string loc = argParser.get<string>("location");
	string fmt = argParser.get<string>("format");
	char *l = const_cast<char*>(loc.c_str());
	char *u = const_cast<char*>(fmt.c_str());
	InpLocation = loc;
	doDataFetchAndDisplay(l, u);
	return 0;
}

void doDataFetchAndDisplay(char location[], char unittype[])
{
	CURL *curl;
	CURLcode res;
	string queryContent = "q=select * from weather.forecast where woeid in (select woeid from geo.places(1) where text = '" + string(location) + "') and u='" + string(unittype) + "'&format=xml";
	char* qStr = const_cast<char*>(queryContent.c_str());
	struct  curl_slist * headers = NULL;
	headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; rv:49.0) Gecko/20100101 Firefox/49.0");
	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
	headers = curl_slist_append(headers, "Accept: */*");
	headers = curl_slist_append(headers, "Host: query.yahooapis.com");
	char *buf[16384] = { 0 };
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, "http://query.yahooapis.com/v1/public/yql");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, qStr);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onDataFetched);
		processer = "";
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	int lastchr = processer.find("</query>") + 8;
	processer.resize(lastchr + 1);
	DisplayData(processer);
}

size_t onDataFetched(char *ptr, size_t size, size_t nmemb, void *stream)
{
	processer += ptr;
	return size*nmemb;
}

string&   replace_all(string&   str, const   string&   old_value, const   string&   new_value)
{
	while (true) {
		string::size_type   pos(0);
		if ((pos = str.find(old_value)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}
string& replace_all_distinct(string& str, const string& old_value, const string& new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}
#define USE_XML
#ifndef USE_XML
void DisplayData(string content)
{
	string wind_dir, wind_spd, weather_txt, temp, condcode;
	int pos0 = content.find("<yweather:wind", 0) + 14;
	int pos1 = content.find("direction=\"", pos0) + 11;
	int pos2 = content.find("speed=\"", pos0) + 7;
	int pos3 = content.find("\"", pos2);
	int lenspd = pos3 - pos2;
	int cond0 = content.find("<yweather:condition", pos0) + 19;
	int cond1 = content.find("code=\"", cond0) + 6;
	int cond2 = content.find("\"", cond1);
	int lencondcode = cond2 - cond1;
	condcode = content.substr(cond1, lencondcode);
	int cond3 = content.find("temp=\"", cond0) + 6;
	int cond4 = content.find("\"", cond3);
	int lentemp = cond4 - cond3;
	temp = content.substr(cond3, lentemp);
	int cond5 = content.find("text=\"", cond0) + 6;
	int cond6 = content.find("\"", cond5);
	int lentxt = cond6 - cond5;
	weather_txt = content.substr(cond5, lentxt);
	//int loc0 = content.find("");


	wind_dir = content.substr(pos1, 3);
	wind_spd = content.substr(pos2, lenspd);
	tftDisplay("Albuquerque", wind_dir, wind_spd, temp, weather_txt, condcode, true);
	//cout << "Wind Direction: " << wind_dir << " Wind Speed:" << wind_spd << " m/s" << endl;
	//cout << "Temperature:" << temp << "deg.C Weather status:" << weather_txt << endl;
}
#else

#ifdef USE_ADV
void DisplayData(string cont)
{
	string content;
	content = replace_all_distinct(cont, "yweather:", "");
	char *ptr = const_cast<char*>(content.c_str());

	xmlDocPtr doc = xmlParseMemory(ptr, content.length());
	xmlXPathContextPtr xPathContext;
	
	//cout << xmlXPathRegisterNs(xPathContext, (xmlChar*) "yweather", (xmlChar*)"http://xml.weather.yahoo.com/ns/rss/1.0") << " ";
	//cout << xmlXPathRegisterNs(xPathContext, (xmlChar*)"geo", (xmlChar*) "http://www.w3.org/2003/01/geo/wgs84_pos#") << endl;
	xmlChar xPath0[] = "/query/results/channel/item/condition";
	xmlChar xPath1[] = "/query/results/channel/wind";
	xmlChar xPath2[] = "/query/results/channel/location";
	xmlChar xPath3[] = "/query/results/channel/atmosphere";
	list<string> attribs0, attribs1, attribs2, attribs3;
	attribs0.push_back("code");
	attribs0.push_back("temp");
	attribs0.push_back("text");
	attribs1.push_back("direction");
	attribs1.push_back("speed");
	attribs2.push_back("city");
	attribs3.push_back("humidity");

	xPathSet AllXPaths[4];
	setAllXPaths(AllXPaths[0], xPath0, attribs0);
	setAllXPaths(AllXPaths[1], xPath1, attribs1);
	setAllXPaths(AllXPaths[2], xPath2, attribs2);
	setAllXPaths(AllXPaths[3], xPath3, attribs3);

	xPathContext = xmlXPathNewContext(doc);
	map<string, string> *results;
	if (xPathContext)
	{
		results = GetXMLValuesByXPath(xPathContext, AllXPaths, 4);
	}
	map<string, string>::iterator mIter;
	for (mIter = results->begin(); mIter != results->end(); mIter++)
	{
		cout << string(mIter->first) << " " << string(mIter->second) << endl;
	}
}
void setAllXPaths(xPathSet &item, xmlChar* stmt, list<string> lst)
{
	item.xPathStmt = stmt;
	item.GAttribList = lst;
}

map<string, string> *GetXMLValuesByXPath(xmlXPathContextPtr xPathContext, xPathSet xPath[], int counts)
{
	map<string, string> *retValues;
	retValues = new map<string, string>[counts];
	for (int c = 0; c < counts; c++)
	{
		//retValues[c] = new map<string, string>();
		xmlXPathObjectPtr xPathResult;
		xPathResult = xmlXPathEvalExpression(xPath[c].xPathStmt, xPathContext);
		xmlXPathFreeContext(xPathContext);
		if (xPathResult)
		{
			if (xmlXPathNodeSetIsEmpty(xPathResult->nodesetval))
			{
				xmlXPathFreeObject(xPathResult);
				break;
			}
			else {
				xmlNodeSetPtr xNodes = xPathResult->nodesetval;
				for (int i = 0; i < xNodes->nodeNr; i++)
				{
					int j = 0;
					list<string>::iterator x;
					for (x = xPath[c].GAttribList.begin(); x != xPath[c].GAttribList.end(); x++)
					{
						string sTarget = string(*x);
						xmlChar *xTarget = (xmlChar*)const_cast<char*>(sTarget.c_str());
						xmlChar* xResult = xmlGetProp(xNodes->nodeTab[i], xTarget);
						if (xResult == NULL) xResult = BAD_CAST"no value, Attrib not found.";
						string sResult =(char*) xResult;
						retValues[c].insert(pair<string, string>(sTarget, sResult));
					}
				}
				xmlXPathFreeObject(xPathResult);
			}
		}
		else {
			break;
		}
	}
	return retValues;
}
#else
void DisplayData(string cont)
{
	string content;
	content = replace_all_distinct(cont, "yweather:", "");
	if (content.length() <= 200)
	{
		cerr << "Undefined Location!" << endl;
		return;
	}
	tftInit();
	iconMapInit();
	char *ptr = const_cast<char*>(content.c_str());
	xmlChar xPath0[] = "/query/results/channel/item/condition";
	xmlChar xPath1[] = "/query/results/channel/wind";
	xmlChar xPath2[] = "/query/results/channel/location";
	xmlChar xPath3[] = "/query/results/channel/atmosphere";
	string Location, wDir, wVel, wTemp, wText, wID;
	xmlDocPtr doc = xmlParseMemory(ptr, content.length());
	xmlXPathContextPtr xPathContext = xmlXPathNewContext(doc);
	bool isIntl;
	if (content.find("speed=\"mph\"")!=content.npos)isIntl = false; else isIntl = true;
	
	xmlChar **params1 = new xmlChar*[3];
	xmlChar **params2 = new xmlChar*[2];
	xmlChar **params3 = new xmlChar*[1];
	xmlChar **params4 = new xmlChar*[1];

	params1[0] = new xmlChar[4]; params1[0] = (xmlChar*)"code";
	params1[1] = new xmlChar[4]; params1[1] = (xmlChar*)"temp";
	params1[2] = new xmlChar[4]; params1[2] = (xmlChar*)"text";
	params2[0] = new xmlChar[9]; params2[0] = (xmlChar*)"direction";
	params2[1] = new xmlChar[5]; params2[1] = (xmlChar*)"speed";
	params3[0] = new xmlChar[4]; params3[0] = (xmlChar*)"city";
	params4[0] = new xmlChar[8]; params4[0] = (xmlChar*)"humidity";

	xmlXPathObjectPtr R1, R2, R3, R4;
	R1 = xmlXPathEvalExpression(xPath0, xPathContext);
	R2 = xmlXPathEvalExpression(xPath1, xPathContext);
	R3 = xmlXPathEvalExpression(xPath2, xPathContext);
	R4 = xmlXPathEvalExpression(xPath3, xPathContext);

	string *cond = getPropByXPath(xPath0, xPathContext, params1, 3);
	string *wind = getPropByXPath(xPath1, xPathContext, params2, 2);
	string *loca = getPropByXPath(xPath2, xPathContext, params3, 1);
	string *humd = getPropByXPath(xPath3, xPathContext, params4, 1);
	
	xmlXPathFreeContext(xPathContext);
	tftDisplay(loca[0], wind[0], wind[1], cond[1], cond[2], cond[0], humd[0], isIntl);
}

string* getPropByXPath(xmlChar *xPath, xmlXPathContextPtr xPathContext, xmlChar **params, int count)
{
	
	string *ret = new string[count];
	xmlXPathObjectPtr xPathResult;
	xPathResult = xmlXPathEvalExpression(xPath, xPathContext);
	if (xPathResult)
	{
		if (xmlXPathNodeSetIsEmpty(xPathResult->nodesetval))
		{
			xmlXPathFreeObject(xPathResult);
		}
		else {
			xmlNodeSetPtr xNodes = xPathResult->nodesetval;
			for (int i = 0; i < xNodes->nodeNr; i++)
			{
				for (int j = 0; j < count; j++)
				{
					ret[j] = string((char*)xmlGetProp(xNodes->nodeTab[i], params[j]));
					
				}
			}
		}
	}
	xmlXPathFreeObject(xPathResult);
	return ret;
}
	
#endif

#endif
/// <summary>
/// Send the data to screen to display the content
/// </summary>
/// <param name="location">Location</param>
/// <param name="wind_dir">Wind direction</param>
/// <param name="wind_vel">Wind speed(m/s)</param>
/// <param name="temp">Current Temperature</param>
/// <param name="weather">Weather description</param>
/// <param name="weatherID">Weather ID</param>
void tftDisplay(string location, string wind_dir, string wind_vel, string temp, string weather, string weatherID, string humd, bool is_intl_unit)
{
	string img = iconMap[atoi(weatherID.c_str())];
	string locationstr = location + " Weather";
	string l1str = temp + ((is_intl_unit)?" C":" F")+", " + weather + ", " + humd + "%";
	char msstr[6];
	sprintf(msstr, "%2.2f", atof(const_cast<char*>(wind_vel.c_str())) / 3.6);
	string l2str = "Wind:" + wind_dir + " "  + ((is_intl_unit)? msstr + (string(" m/s")): wind_vel + " mph");
	(*(tFields["Title"])).setValue(const_cast<char*>(locationstr.c_str()));
	(*(tFields["Line1"])).setValue(const_cast<char*>(l1str.c_str()));
	(*(tFields["Line2"])).setValue(const_cast<char*>(l2str.c_str()));
	tManager.refresh();
	cout << InpLocation << "天气：" << weather << "，温度：" << temp << ((is_intl_unit) ? "℃" : "℉") << "，湿度：" << humd << "%" << endl
		<< "风向：" << wind_dir << "，风速：" << ((is_intl_unit) ? msstr + string("米每秒") : wind_vel + "英里每小时" )<<endl;
	tftWriteImage(img);
}

void tftWriteImage(string filename)
{
	PngProc img;
	img.pngInit(filename);

	unsigned int **pBitMap = img.get565BitMap();
	int w, h = 0;
	w = img.getPNGInfo()->width;
	h = img.getPNGInfo()->height;
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			tft.drawPixel(x + 46, y + 30, pBitMap[y][x]);
		}
	}
}

void tftInit()
{
	tFields.insert(pair<string, TFT_field*>("Title", new TFT_field(tft, 5, 1, 118, 16, Color565(255, 255, 255), 1, TFT_BLACK, false)));
	tFields.insert(pair<string, TFT_field*>("Line1", new TFT_field(tft, 5, 68, 118, 16, Color565(255, 255, 255), 1, TFT_BLACK, false)));
	tFields.insert(pair<string, TFT_field*>("Line2", new TFT_field(tft, 5, 96, 118, 16, Color565(255, 255, 255), 1, TFT_BLACK, false)));
	tManager.add(tFields["Title"]);
	tManager.add(tFields["Line1"]);
	tManager.add(tFields["Line2"]);
	wiringPiSetupGpio();
	tft.commonInit();
	tft.initR();
	tft.setRotation(DEGREE_180);
	tft.setBackground(TFT_BLACK);
	tft.clearScreen();
	(*(tFields["Title"])).setValue("  Location Test");
	(*(tFields["Line1"])).setValue("Init test text...");
	(*(tFields["Line2"])).setValue("Init test text 2...");
	tManager.refresh();
}
/*
void  do_shortInit(unsigned char _channel, unsigned char _rs, unsigned char _rst, int _speed)
{
	pinMode(_rs, OUTPUT);

	// 0 is SPI0_CE0_N which is GPIO8 (first chip select line) => Channel 0
	if (wiringPiSPISetup(_channel, _speed) < 0)
	{
		// Handle error
		printf("TFT_ST7735::commonInit Error setting up SPI\n");
	}

	// toggle RST low to reset; CS low so it'll listen to us
	if (_rst)
	{
		pinMode(_rst, OUTPUT);
		digitalWrite(_rst, HIGH);
		delay(100);
		digitalWrite(_rst, LOW);
		delay(100);
		digitalWrite(_rst, HIGH);
		delay(100);
	}
}*/


/// <summary>
/// initializes icon file map
/// </summary>
void iconMapInit()
{
	iconMap.insert(pair<int, string>(tornado, "Resources/tornado.png"));
	iconMap.insert(pair<int, string>(tropical_storm, "./Resources/tropical_storm.png"));
	iconMap.insert(pair<int, string>(hurricane, "./Resources/hurricane.png"));
	iconMap.insert(pair<int, string>(severe_thunderstorms, "./Resources/thunderstorm.png"));
	iconMap.insert(pair<int, string>(thunderstorms, "./Resources/thunderstorm.png"));
	iconMap.insert(pair<int, string>(mixed_rain_and_snow, "./Resources/snowrain.png"));
	iconMap.insert(pair<int, string>(mixed_rain_and_sleet, "./Resources/snowrain.png"));
	iconMap.insert(pair<int, string>(mixed_snow_and_sleet, "./Resources/snowrain.png"));
	iconMap.insert(pair<int, string>(freezing_drizzle, "./Resources/ice.png"));
	iconMap.insert(pair<int, string>(drizzle, "./Resources/drizzle.png"));
	iconMap.insert(pair<int, string>(freezing_rain, "./Resources/ice.png"));
	iconMap.insert(pair<int, string>(showers, "./Resources/drizzle.png"));
	iconMap.insert(pair<int, string>(12, "./Resources/drizzle.png"));
	iconMap.insert(pair<int, string>(snow_flurries, "./Resources/flurries.png"));
	iconMap.insert(pair<int, string>(light_snow_showers, "./Resources/lightsnow.png"));
	iconMap.insert(pair<int, string>(blowing_snow, "./Resources/snow.png"));
	iconMap.insert(pair<int, string>(snow, "./Resources/snow.png"));
	iconMap.insert(pair<int, string>(hail, "./Resources/hail.png"));
	iconMap.insert(pair<int, string>(sleet, "./Resources/snowrain.png"));
	iconMap.insert(pair<int, string>(dust, "./Resources/dust.png"));
	iconMap.insert(pair<int, string>(foggy, "./Resources/fog.png"));
	iconMap.insert(pair<int, string>(haze, "./Resources/haze.png"));
	iconMap.insert(pair<int, string>(smoky, "./Resources/haze.png"));
	iconMap.insert(pair<int, string>(blustery, "./Resources/wind.png"));
	iconMap.insert(pair<int, string>(windy, "./Resources/wind.png"));
	iconMap.insert(pair<int, string>(cold, "./Resources/lowtemp.png"));
	iconMap.insert(pair<int, string>(cloudy, "./Resources/clouds.png"));
	iconMap.insert(pair<int, string>(mostly_cloudy_night_, "./Resources/clouds.png"));
	iconMap.insert(pair<int, string>(mostly_cloudy_day_, "./Resources/clouds.png"));
	iconMap.insert(pair<int, string>(partly_cloudy_night_, "./Resources/clouds.png"));
	iconMap.insert(pair<int, string>(partly_cloudy_day_, "./Resources/clouds.png"));
	iconMap.insert(pair<int, string>(clear_night_, "./Resources/clearnight.png"));
	iconMap.insert(pair<int, string>(sunny, "./Resources/sun.png"));
	iconMap.insert(pair<int, string>(fair_night_, "./Resources/clearnight.png"));
	iconMap.insert(pair<int, string>(fair_day_, "./Resources/sun.png"));
	iconMap.insert(pair<int, string>(mixed_rain_and_hail, "./Resources/lightning.png"));
	iconMap.insert(pair<int, string>(hot, "./Resources/hightemp.png"));
	iconMap.insert(pair<int, string>(isolated_thunderstorms, "./Resources/thunderstorm.png"));
	iconMap.insert(pair<int, string>(scattered_thunderstorms, "./Resources/thunderstorm.png"));
	iconMap.insert(pair<int, string>(scattered_thunderstorms, "./Resources/thunderstorm.png"));
	iconMap.insert(pair<int, string>(scattered_showers_, "./Resources/drizzle.png"));
	iconMap.insert(pair<int, string>(scattered_showers, "./Resources/rain.png"));
	iconMap.insert(pair<int, string>(heavy_snow, "./Resources/snow.png"));
	iconMap.insert(pair<int, string>(scattered_snow_showers, "./Resources/snow.png"));
	iconMap.insert(pair<int, string>(heavy_snow, "./Resources/snow.png"));
	iconMap.insert(pair<int, string>(partly_cloudy, "./Resources/suncloud.png"));
	iconMap.insert(pair<int, string>(thundershowers, "./Resources/lightning.png"));
	iconMap.insert(pair<int, string>(snow_showers, "./Resources/snow.png"));
	iconMap.insert(pair<int, string>(isolated_thundershowers, "./Resources/lightning.png"));
	iconMap.insert(pair<int, string>(not_available, "./Resources/unknown.png"));
}

/// <summary>
/// Initialize PNG file
/// </summary>
/// <param name="filename">PNG file name</summary>
/// <returns>if the initialize succeed.</returns>
int PngProc::pngInit(char *filename)
{
	this->m_pngFile = fopen(filename, "rb");
	if (this->m_pngFile == NULL)
	{
		fclose(this->m_pngFile);
		return PNG_FILE_OPEN_ERR;
	}
	this->m_pngData = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (this->m_pngData == NULL)
	{
		fclose(this->m_pngFile);
		return PNG_STRUCT_CREATE_ERR;
	}
	this->m_pngInfo = png_create_info_struct(this->m_pngData);
	if (this->m_pngInfo == NULL)
	{
		fclose(this->m_pngFile);
		png_destroy_read_struct(&(this->m_pngData), NULL, NULL);
		return PNG_GET_INFO_ERR;
	}
	if (setjmp(png_jmpbuf(this->m_pngData)))
	{
		fclose(this->m_pngFile);
		png_destroy_read_struct(&(this->m_pngData), &(this->m_pngInfo), NULL);
	}
	png_init_io(this->m_pngData, this->m_pngFile);
	png_read_png(this->m_pngData, this->m_pngInfo, PNG_TRANSFORM_IDENTITY, 0);
	png_get_IHDR(this->m_pngData, this->m_pngInfo, &this->m_width, &this->m_height, &this->m_bitDepth, &(this->m_color_type), NULL, NULL, NULL);

	png_bytep* row_pointers = png_get_rows(this->m_pngData, this->m_pngInfo);

	unsigned int bufSize = 0;

	const int w = m_pngInfo->width;
	const int h = m_pngInfo->height;
	m_color565BitMap = new unsigned int*[h];
	m_colorRgbBitMap = new RGBColor*[h];
	for (int i = 0; i < h; i++)
	{
		m_color565BitMap[i] = new unsigned int[w];
		m_colorRgbBitMap[i] = new RGBColor[w];
	}

	if (this->m_color_type = PNG_COLOR_TYPE_RGBA)
	{
		for (int y0 = 0; y0 < this->m_height; y0++)
		{
			int x = 0;
			for (int x0 = 0; x0 < this->m_width * 4; x0 += 4)
			{
				unsigned char R, G, B = 0;
				unsigned char a, r, g, b;
				r = row_pointers[y0][x0];
				g = row_pointers[y0][x0 + 1];
				b = row_pointers[y0][x0 + 2];
				a = row_pointers[y0][x0 + 3];
				double A = 0;
				A = (row_pointers[y0][x0 + 3]) / 256;
				R = row_pointers[y0][x0] * A;
				G = row_pointers[y0][x0 + 1] * A;
				B = row_pointers[y0][x0 + 2] * A;

				int Rx, Gx, Bx;
				Rx = r * a / 256;
				Gx = g * g / 256;
				Bx = b * a / 256;
				if (a <= 32) { Rx = 0; Gx = 0; Bx = 0; }
				this->m_color565BitMap[y0][x] = Color565(Rx, Gx, Bx);

				RGBColor tmpRgb = { R,G,B };
				this->m_colorRgbBitMap[y0][x] = tmpRgb;
				x++;
			}
		}
	}
	else
	{
		return PNG_INVALID_FILE_FORMAT;
	}
	return PNG_INIT_SUCCESS;
}

PngProc::PngProc()
{
	m_pngData = NULL;
	m_bitDepth = 0;
	m_color565BitMap = NULL;
	m_colorRgbBitMap = NULL;
	m_color_type = 0;
	m_height = 0;
	m_pngData = NULL;
	m_pngFile = NULL;
	m_pngInfo = NULL;
}

PngProc::PngProc(char *filename)
{
	pngInit(filename);
}


PngProc::PngProc(string filename)
{
	pngInit(const_cast<char*>(filename.c_str()));
}

int PngProc::pngInit(string filename)
{
	pngInit(const_cast<char*>(filename.c_str()));
}

int PngProc::getRowBytes(int width) {
	if ((width * 3) % 4 == 0) {
		return width * 3;
	}
	else {
		return ((width * 3) / 4 + 1) * 4;
	}
}

png_infop PngProc::getPNGInfo()
{
	return this->m_pngInfo;
}

unsigned int **PngProc::get565BitMap()
{
	return this->m_color565BitMap;
}

RGBColor **PngProc::getRGBBitMap()
{
	return this->m_colorRgbBitMap;
}