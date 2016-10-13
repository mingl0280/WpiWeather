#ifndef DHT11_H

#define DHT11_H
#pragma once

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <curl/curl.h>
#include <ctime>
#include <cmath>
#include <wiringPi.h>
#include <pthread.h>
#include <sys/types.h>

#define DHT11ERR_NO_RESPONSE 1;

using namespace std;


class DHT11Reader
{
private:
	int DATA = 4;
	int TFGate = 400;
	static const int TIME_START = 20000;
	static const int RETRY = 10;
	static const int MAXCNT = 10000;
	int m_temp = 0;
	int m_wetness = 0;
	int bits[40] = { 0 };
	int data[5] = { 0 };
	static pthread_t dGetThread;
	int readValue();
	static void *dataReader(void *ptr);
	static DHT11Reader *Rder;
public:
	int getTemp();
	int getWetness();
	DHT11Reader();
	DHT11Reader(int Data_Port);
	DHT11Reader(int Data_Port, int TFGateValue);
	int startThread();
	int *getBits();
	void forceRefresh();
};

#endif