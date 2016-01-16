#ifdef WIN32
#pragma warning (disable: 4786)
#endif


#include <stdio.h>
#include <time.h>
#include "profile.h"
#include "timer.h"


#ifdef WIN32
#define snprintf _snprintf
#endif


Profiler globalProfiler;


Profiler::NodeData::NodeData() :
	mStartTimer(0),
	mTotalLatency(0),
	mChildLatency(0),
	mTotalChildLatency(0),
	mMaxLatency(0),
	mMinLatency(0),
	mTotalCalls(0)
{
}


Profiler::Profiler() :
	mActive(false),
	mStartTime(0),
	mStartTimer(0),
	mDataMap(),
	mDataStack(),
	mNodeMap(),
	mNodeStack()
{
	mStartTime = (unsigned)time(0);
	mStartTimer = soe::GetTimer();
}

Profiler::~Profiler()
{
}

void Profiler::AddNode(Node & lhs, const Node & rhs)
{
	lhs.mData.mTotalLatency += rhs.mData.mTotalLatency;
	lhs.mData.mTotalChildLatency += rhs.mData.mTotalChildLatency;
	lhs.mData.mTotalCalls += rhs.mData.mTotalCalls;
	if (rhs.mData.mMaxLatency > lhs.mData.mMaxLatency)
		lhs.mData.mMaxLatency = rhs.mData.mMaxLatency;
	if (lhs.mData.mMinLatency == 0 || rhs.mData.mMinLatency < lhs.mData.mMinLatency)
		lhs.mData.mMinLatency = rhs.mData.mMinLatency;

	NodeMap_t::const_iterator iter;
	for (iter = rhs.mNodeMap.begin(); iter != rhs.mNodeMap.end(); iter++)
	{
		Node & node = lhs.mNodeMap[iter->first];
		AddNode(node, iter->second);
	}
}

void Profiler::AddData(const Profiler & rhs, bool hierarchy)
{
	if (hierarchy)
	{
		NodeMap_t::const_iterator nodeIter;
		for (nodeIter = rhs.mNodeMap.begin(); nodeIter != rhs.mNodeMap.end(); nodeIter++)
		{
			Node & node = mNodeMap[nodeIter->first];
			AddNode(node, nodeIter->second);
		}
	}

	DataMap_t::const_iterator iter;
	for (iter = rhs.mDataMap.begin(); iter != rhs.mDataMap.end(); iter++)
	{
		NodeData & lhs = mDataMap[iter->first];
		const NodeData & rhs = iter->second;
		
		lhs.mTotalLatency += rhs.mTotalLatency;
		lhs.mTotalChildLatency += rhs.mTotalChildLatency;
		lhs.mTotalCalls += rhs.mTotalCalls;
		if (rhs.mMaxLatency > lhs.mMaxLatency)
			lhs.mMaxLatency = rhs.mMaxLatency;
		if (lhs.mMinLatency == 0 || rhs.mMinLatency < lhs.mMinLatency)
			lhs.mMinLatency = rhs.mMinLatency;
	}
}

void Profiler::Open(const std::string & description)
{
	NodeData & data = mDataMap[description];
	data.mStartTimer = soe::GetTimer();
	data.mChildLatency = 0;
	mDataStack.push_front(&data);

	if (mNodeStack.empty())
	{
		Node & node = mNodeMap[description];
		node.mData.mStartTimer = data.mStartTimer;
		node.mData.mChildLatency = 0;
		mNodeStack.push_front(&node);
	}
	else
	{
		Node & node = mNodeStack.front()->mNodeMap[description];
		node.mData.mStartTimer = data.mStartTimer;
		node.mData.mChildLatency = 0;
		mNodeStack.push_front(&node);
	}
}

void Profiler::Close()
{
	if (mNodeStack.empty() || mDataStack.empty())
		return;

	double latency;
	double trueLatency;

	NodeData & data = *mDataStack.front();
	Node & node = *mNodeStack.front();
	mDataStack.pop_front();
	mNodeStack.pop_front();
	
	latency = soe::GetTimerLatency(data.mStartTimer);

	trueLatency = latency - data.mChildLatency;
	if (data.mMaxLatency < trueLatency)
		data.mMaxLatency = trueLatency;
	if (data.mMinLatency == 0 || data.mMinLatency > trueLatency)
		data.mMinLatency = trueLatency;
	data.mTotalLatency += latency;
	data.mTotalCalls++;

	trueLatency = latency - node.mData.mChildLatency;
	if (node.mData.mMaxLatency < trueLatency)
		node.mData.mMaxLatency = trueLatency;
	if (node.mData.mMinLatency == 0 || node.mData.mMinLatency > trueLatency)
		node.mData.mMinLatency = trueLatency;
	node.mData.mTotalLatency += latency;
	node.mData.mTotalCalls++;

	if (!mNodeStack.empty() && !mDataStack.empty())
	{
		NodeData & parentData = *mDataStack.front();
		Node & parentNode = *mNodeStack.front();
		parentData.mChildLatency += latency;
		parentData.mTotalChildLatency += latency;
		parentNode.mData.mChildLatency += latency;
		parentNode.mData.mTotalChildLatency += latency;
	}
}

void Profiler::Clear()
{
	mStartTime = (unsigned)time(0);
	mStartTimer = soe::GetTimer();
	mDataMap.clear();
	mDataStack.clear();
	mNodeMap.clear();
	mNodeStack.clear();
}

void Profiler::GetOutput(std::string & output, bool hierarchy)
{
	char buffer[256];
	double profileDuration = soe::GetTimerLatency(mStartTimer);
	
	//	print header
	struct tm timeStruct = *localtime((time_t *)&mStartTime);
	strftime(buffer,128,"%m/%d/%Y %H:%M:%S", &timeStruct);
	output = buffer;
	snprintf(buffer, sizeof(buffer), " profile duration: %.1fms\n", profileDuration*1000);
	output += buffer;

	if (hierarchy && !mNodeMap.empty())
	{
		//	print legend
		snprintf(buffer, sizeof(buffer), "prcnt   latency     max     min     avg count prcnt   latency description\n");
		output += buffer;

		//  recursively print node map (hierarchical call profile)
		GetNodeOutput(output, mNodeMap, profileDuration);
	}

	//	print legend
	snprintf(buffer, sizeof(buffer), "prcnt   latency     max     min     avg count prcnt   latency description\n");
	output += buffer;

	//  print data map (linear call profile)
	DataMap_t::const_iterator iter;
	for (iter = mDataMap.begin(); iter != mDataMap.end(); iter++)
	{
		const NodeData & data = iter->second;
		double trueLatency = data.mTotalLatency-data.mTotalChildLatency;
		snprintf(buffer, sizeof(buffer), "%5.1f %9.3f %7.3f %7.3f %7.3f %5u %5.1f %9.3f ", trueLatency/profileDuration*100, trueLatency*1000, data.mMaxLatency*1000, data.mMinLatency*1000, trueLatency/data.mTotalCalls*1000, data.mTotalCalls, data.mTotalLatency/profileDuration*100, data.mTotalLatency*1000);
		output += buffer;
		snprintf(buffer, sizeof(buffer), "%s\n", iter->first.c_str());
		output += buffer;
	}
}

void Profiler::GetXmlOutput(std::string & output, bool hierarchy)
{
	char buffer[256];
	double profileDuration = soe::GetTimerLatency(mStartTimer);
	
	output += "<General>";

	//	print header
	struct tm timeStruct = *localtime((time_t *)&mStartTime);
	strftime(buffer,128,"%m/%d/%Y %H:%M:%S", &timeStruct);
	output += "<DateTime>";
	output += buffer;
	output += "</DateTime>";
	
	snprintf(buffer, sizeof(buffer), "%.1f\n", profileDuration*1000);
	output += "<Duration>";
	output += buffer;
	output += "</Duration>";

	output += "</General>";

	if (hierarchy && !mNodeMap.empty())
	{
		//	print legend
		//sprintf(buffer,"prcnt   latency     max     min     avg count prcnt   latency description\n");
		//output += buffer;

		//  recursively print node map (hierarchical call profile)
		//GetNodeOutput(output, mNodeMap, profileDuration);
		output += "<DataMapNested>";
		GetXmlNodeOutput(output, mNodeMap, profileDuration);
		output += "</DataMapNested>";
	}

	//	print legend
	//sprintf(buffer,"prcnt   latency     max     min     avg count prcnt   latency description\n");
	//output += buffer;

	output += "<DataMap>";

	output += "<DataMapCount>";
	snprintf(buffer, sizeof(buffer), "%u", mDataMap.size());
	output += buffer;
	output += "</DataMapCount>";

	//  print data map (linear call profile)
	int i = 1;
	DataMap_t::const_iterator iter;
	for (iter = mDataMap.begin(); iter != mDataMap.end(); iter++)
	{
		const NodeData & data = iter->second;
		double trueLatency = data.mTotalLatency-data.mTotalChildLatency;
		
		/*
		sprintf(buffer, "%5.1f %9.3f %7.3f %7.3f %7.3f %5u %5.1f %9.3f ", 
			trueLatency/profileDuration*100, 
			trueLatency*1000, 
			data.mMaxLatency*1000, 
			data.mMinLatency*1000, 
			trueLatency/data.mTotalCalls*1000, 
			data.mTotalCalls, 
			data.mTotalLatency/profileDuration*100, 
			data.mTotalLatency*1000);
		*/
		snprintf(buffer, sizeof(buffer), "<DataMap%u>", i);
		output += buffer;

		output += "<Description>";
		output += iter->first;
		output += "</Description>";

		snprintf(buffer, sizeof(buffer), "%5.1f", trueLatency/profileDuration*100);
		output += "<Percent>";
		output += buffer;
		output += "</Percent>";

		snprintf(buffer, sizeof(buffer), "%9.3f", trueLatency*1000);
		output += "<Latency>";
		output += buffer;
		output += "</Latency>";

		snprintf(buffer, sizeof(buffer), "%7.3f", data.mMaxLatency*1000);
		output += "<Max>";
		output += buffer;
		output += "</Max>";

		snprintf(buffer, sizeof(buffer), "%7.3f", data.mMinLatency*1000);
		output += "<Min>";
		output += buffer;
		output += "</Min>";

		snprintf(buffer, sizeof(buffer), "%7.3f", trueLatency/data.mTotalCalls*1000);
		output += "<Average>";
		output += buffer;
		output += "</Average>";

		snprintf(buffer, sizeof(buffer), "%5u", data.mTotalCalls);
		output += "<Count>";
		output += buffer;
		output += "</Count>";

		snprintf(buffer, sizeof(buffer), "%5.1f", data.mTotalLatency/profileDuration*100);
		output += "<TotalPercent>";
		output += buffer;
		output += "</TotalPercent>";

		snprintf(buffer, sizeof(buffer), "%9.3f", data.mTotalLatency*1000);
		output += "<TotalLatency>";
		output += buffer;
		output += "</TotalLatency>";

		//sprintf(buffer,"%s\n",iter->first.c_str());
		//output += buffer;

		snprintf(buffer, sizeof(buffer), "</DataMap%u>", i++);
		output += buffer;
	}
	output += "</DataMap>";
}

void Profiler::GetNodeOutput(std::string & output, const NodeMap_t & nodeMap, double profileDuration, int level)
{
	char buffer[256];
	
	//  recursively print node map
	NodeMap_t::const_iterator iter;
	for (iter = nodeMap.begin(); iter != nodeMap.end(); iter++)
	{
		//	output node description
		const Node & node = iter->second;
		double trueLatency = node.mData.mTotalLatency-node.mData.mTotalChildLatency;
		snprintf(buffer, sizeof(buffer), "%5.1f %9.3f %7.3f %7.3f %7.3f %5u %5.1f %9.3f ", trueLatency/profileDuration*100, trueLatency*1000, node.mData.mMaxLatency*1000, node.mData.mMinLatency*1000, trueLatency/node.mData.mTotalCalls*1000, node.mData.mTotalCalls, node.mData.mTotalLatency/profileDuration*100, node.mData.mTotalLatency*1000);
		output += buffer;
		for (int i=0; i<level; i++)
			output += " ";
		snprintf(buffer, sizeof(buffer), "+%s\n", iter->first.c_str());
		output += buffer;

		//	recursively print node's node map
		GetNodeOutput(output, node.mNodeMap, profileDuration, level+1);
	}
}

void Profiler::GetXmlNodeOutput(std::string & output, const NodeMap_t & nodeMap, double profileDuration, int level)
{
	char buffer[256];

	output += "<NodeCount>";
	snprintf(buffer, sizeof(buffer), "%u", nodeMap.size());
	output += buffer;
	output += "</NodeCount>";

	//  recursively print node map
	int i = 1;
	NodeMap_t::const_iterator iter;
	for (iter = nodeMap.begin(); iter != nodeMap.end(); iter++)
	{
		snprintf(buffer, sizeof(buffer), "<Node%d>", i);
		output += buffer;

		output += "<Description>";
		output += iter->first;
		output += "</Description>";

		//	output node description
		const Node & node = iter->second;
		double trueLatency = node.mData.mTotalLatency-node.mData.mTotalChildLatency;
		/*
		sprintf(buffer, "%5.1f %9.3f %7.3f %7.3f %7.3f %5u %5.1f %9.3f ", 
			trueLatency/profileDuration*100, 
			trueLatency*1000, 
			node.mData.mMaxLatency*1000, 
			node.mData.mMinLatency*1000, 
			trueLatency/node.mData.mTotalCalls*1000, 
			node.mData.mTotalCalls, 
			node.mData.mTotalLatency/profileDuration*100, 
			node.mData.mTotalLatency*1000);
		*/
		snprintf(buffer, sizeof(buffer), "%5.1f", trueLatency/profileDuration*100);
		output += "<Percent>";
		output += buffer;
		output += "</Percent>";

		snprintf(buffer, sizeof(buffer), "%9.3f", trueLatency*1000);
		output += "<Latency>";
		output += buffer;
		output += "</Latency>";

		snprintf(buffer, sizeof(buffer), "%7.3f", node.mData.mMaxLatency*1000);
		output += "<Max>";
		output += buffer;
		output += "</Max>";

		snprintf(buffer, sizeof(buffer), "%7.3f", node.mData.mMinLatency*1000);
		output += "<Min>";
		output += buffer;
		output += "</Min>";

		snprintf(buffer, sizeof(buffer), "%7.3f", trueLatency/node.mData.mTotalCalls*1000);
		output += "<Average>";
		output += buffer;
		output += "</Average>";

		snprintf(buffer, sizeof(buffer), "%5u", node.mData.mTotalCalls);
		output += "<Count>";
		output += buffer;
		output += "</Count>";

		snprintf(buffer, sizeof(buffer), "%5.1f", node.mData.mTotalLatency/profileDuration*100);
		output += "<TotalPercent>";
		output += buffer;
		output += "</TotalPercent>";

		snprintf(buffer, sizeof(buffer), "%9.3f", node.mData.mTotalLatency*1000);
		output += "<TotalLatency>";
		output += buffer;
		output += "</TotalLatency>";

		//	recursively print node's node map
		GetXmlNodeOutput(output, node.mNodeMap, profileDuration, level+1);

		snprintf(buffer, sizeof(buffer), "</Node%d>", i++);
		output += buffer;
	}
}


Profile::Profile(const char * description, Profiler & profile) :
	mProfiler(profile)
{
	if (mProfiler.IsActive())
	{
		mProfiler.Open(description);
	}
}

Profile::~Profile()
{
	if (mProfiler.IsActive())
	{
		mProfiler.Close();
	}
}



