// TaskManagerSysInfo.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstTaskManager.h"
#include "TaskManagerSysInfo.h"
#pragma warning ( disable : 4201)
#include <windows.h>
#include <tlhelp32.h>
//-----------------------------------------------------------------------

TaskManagerSysInfo::TaskManagerSysInfo() :
averageScore()
{
	update();
}

//-----------------------------------------------------------------------

TaskManagerSysInfo::TaskManagerSysInfo(const TaskManagerSysInfo &)
{

}

//-----------------------------------------------------------------------

TaskManagerSysInfo::~TaskManagerSysInfo()
{
}

//-----------------------------------------------------------------------

TaskManagerSysInfo & TaskManagerSysInfo::operator = (const TaskManagerSysInfo & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

const float TaskManagerSysInfo::getScore() const
{
	std::list<float>::const_iterator i;
	float avg = 0.0f;
	for(i = averageScore.begin(); i != averageScore.end(); ++i)
	{
		avg += (*i);
	}
	avg = avg / averageScore.size();
	return avg;
}

//-----------------------------------------------------------------------

void TaskManagerSysInfo::update()
{
	static int64 activeTime[2] = {0};
	static int64 currentTime[2] = {0};

	float currentScore = 0.0f;
	activeTime[0] = activeTime[1];
	currentTime[0] = currentTime[1];
	activeTime[1] = 0;

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	double procAvg = 0.0f;

	MEMORYSTATUS memStat;
	GlobalMemoryStatus(&memStat);
	
	currentScore = static_cast<float>(static_cast<float>(memStat.dwMemoryLoad) * 0.005f);
	if(hProcessSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe32 = {0};
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnap, &pe32)) 
		{ 
			do 
			{
				HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION, false, pe32.th32ProcessID);
				// some stuf with the enumerated processes
				FILETIME createTime = {0};
				FILETIME exitTime = {0};
				FILETIME kernelTime = {0};
				FILETIME userTime = {0};
				GetProcessTimes(proc, &createTime, &exitTime, &kernelTime, &userTime);
				int64 totals;
				
				// SDK docs say:
				// It is not recommended that you add and subtract values 
				// from the FILETIME structure to obtain relative times. Instead, you should 
				// Copy the resulting FILETIME structure to a ULARGE_INTEGER structure. 
				// Use normal 64-bit arithmetic on the ULARGE_INTEGER value. 
				int64 c;
				int64 e;
				int64 k;
				int64 u;
				memcpy(&c, &createTime, sizeof(int64));
				memcpy(&e, &exitTime, sizeof(int64));
				memcpy(&k, &kernelTime, sizeof(int64));
				memcpy(&u, &userTime, sizeof(int64));
				
				totals = k + u;
				
				FILETIME fst;
				SYSTEMTIME st; 
				GetSystemTime(&st);
				SystemTimeToFileTime(&st, &fst);

				int64 runTime;
				memcpy(&runTime, &fst, sizeof(int64));
				runTime = runTime - c;

				if(c || e || k || u)
				{
					activeTime[1] += k + e;
				}

			} 
			while (Process32Next(hProcessSnap, &pe32)); 
		}
	}

	FILETIME fst;
	SYSTEMTIME st; 
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &fst);
	memcpy(&currentTime[1], &fst, sizeof(int64));
	int64 timeSlice = currentTime[1] - currentTime[0];
	int64 activeSlice = activeTime[1] - activeTime[0];
	procAvg = static_cast<double>(static_cast<double>(activeSlice) / timeSlice);
	//REPORT_LOG(true, ("%f\n", procAvg));
	currentScore = currentScore + static_cast<float>(procAvg * 0.5);

	averageScore.insert(averageScore.end(), currentScore);

	if(averageScore.size() > 100)
		averageScore.erase(averageScore.begin());
}

//-----------------------------------------------------------------------

