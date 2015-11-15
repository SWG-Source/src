#ifndef SERVER_PROFILE_H
#define SERVER_PROFILE_H


#include <string>
#include <list>
#include <map>
#include "types.h"


class Profiler
{
	struct NodeData;
	struct Node;
	typedef std::map<std::string,NodeData> DataMap_t;
	typedef std::map<std::string,Node> NodeMap_t;
	typedef std::list<NodeData *> DataStack_t;
	typedef std::list<Node *> NodeStack_t;
	struct NodeData
	{
		NodeData();
		soe::uint64	mStartTimer;
		double mTotalLatency;
		double mChildLatency;
		double mTotalChildLatency;
		double mMaxLatency;
		double mMinLatency;
		unsigned mTotalCalls;
	};
	struct Node
	{
		NodeData mData;
		NodeMap_t mNodeMap;
	};

	public:
		Profiler();
		~Profiler();

		void AddNode(Node & lhs, const Node & rhs);
		void AddData(const Profiler & rhs, bool hierarchy=true);

		bool IsActive() { return mActive; }
		void Start() { mActive = true; }
		void Stop() { mActive = false; }

		void Open(const std::string & description);
		void Close();
		void Clear();

		void GetOutput(std::string & output, bool hierarchy=true);
		void GetXmlOutput(std::string & output, bool hierarchy=true);

	private:
		void GetNodeOutput(std::string & output, const NodeMap_t & nodeMap, double profileDuration, int level = 0);
		void GetXmlNodeOutput(std::string & output, const NodeMap_t & nodeMap, double profileDuration, int level = 0);
		void MakeValidXmlTag(std::string& tag);

	private:
		bool mActive;
		unsigned mStartTime;
		soe::uint64 mStartTimer;
		DataMap_t mDataMap;			//	stores linear call totals
		DataStack_t mDataStack;		//	stores linear call stack
		NodeMap_t mNodeMap;			//	stores hierarchical call totals
		NodeStack_t mNodeStack;		//	stores hierarchical call stack
};

extern Profiler globalProfiler;

class Profile
{
	public:
		Profile(const char * description, Profiler & profile = globalProfiler);
		~Profile();
	private:
		Profiler &		mProfiler;
};


#endif

