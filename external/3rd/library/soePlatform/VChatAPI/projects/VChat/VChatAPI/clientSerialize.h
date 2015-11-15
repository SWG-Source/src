#ifndef _VCHAT_CLIENT_SERIALIZE_H_
#define _VCHAT_CLIENT_SERIALIZE_H_

#include "common.h"

namespace soe
{

	inline unsigned Write(unsigned char * stream, unsigned size, const VChatSystem::Channel & data, unsigned version = 0)
	{			
		return data.Write(stream, size);
	}

	inline unsigned Read(const unsigned char * stream, unsigned size, VChatSystem::Channel & data, unsigned, unsigned version = 0)
	{
		return data.Read(stream, size);
	}

	inline unsigned Write(unsigned char * stream, unsigned size, const VChatSystem::CharacterChannel & data, unsigned version = 0)
	{			
		return data.Write(stream, size);
	}

	inline unsigned Read(const unsigned char * stream, unsigned size, VChatSystem::CharacterChannel & data, unsigned, unsigned version = 0)
	{
		return data.Read(stream, size);
	}

	inline unsigned Write(unsigned char * stream, unsigned size, const VChatSystem::ChannelVec_t & data, unsigned version = 0)
	{
		unsigned bytesTotal = 0;
		unsigned bytes = 0;
		bytesTotal += (bytes = soe::Write(stream + bytesTotal, size - bytesTotal, (unsigned)data.size()));
		if (!bytes) return 0;

		for (size_t i = 0; i < data.size(); i++)
		{
			bytesTotal += (bytes = soe::Write(stream + bytesTotal, size - bytesTotal, data[i]));
			if (!bytes) return 0;
		}

		return bytesTotal;
	}


	inline unsigned Read(const unsigned char * stream, unsigned size, VChatSystem::ChannelVec_t & data, unsigned maxLen = 1, unsigned version = 0)
	{
		unsigned bytesTotal = 0;
		unsigned bytes = 0;

		data.clear();
		int vsize = 0;

		bytesTotal += (bytes = soe::Read(stream + bytesTotal, size - bytesTotal, vsize));
		if (!bytes) return 0;

		for (int i = 0; i < vsize; i++)
		{
			VChatSystem::Channel tmpData;
			bytesTotal += (bytes = soe::Read(stream + bytesTotal, size - bytesTotal, tmpData, 1)); 
			if (!bytes) 
			{
				return 0;
			}
			data.push_back(tmpData);
		}

		return bytesTotal;
	}

	inline unsigned Write(unsigned char * stream, unsigned size, const VChatSystem::CharacterChannelVec_t & data, unsigned version = 0)
	{
		unsigned bytesTotal = 0;
		unsigned bytes = 0;
		bytesTotal += (bytes = soe::Write(stream + bytesTotal, size - bytesTotal, (unsigned)data.size()));
		if (!bytes) return 0;

		for (size_t i = 0; i < data.size(); i++)
		{
			bytesTotal += (bytes = soe::Write(stream + bytesTotal, size - bytesTotal, data[i]));
			if (!bytes) return 0;
		}

		return bytesTotal;
	}


	inline unsigned Read(const unsigned char * stream, unsigned size, VChatSystem::CharacterChannelVec_t & data, unsigned maxLen = 1, unsigned version = 0)
	{
		unsigned bytesTotal = 0;
		unsigned bytes = 0;

		data.clear();
		int vsize = 0;

		bytesTotal += (bytes = soe::Read(stream + bytesTotal, size - bytesTotal, vsize));
		if (!bytes) return 0;

		for (int i = 0; i < vsize; i++)
		{
			VChatSystem::CharacterChannel tmpData;
			bytesTotal += (bytes = soe::Read(stream + bytesTotal, size - bytesTotal, tmpData, 1)); 
			if (!bytes) 
			{
				return 0;
			}
			data.push_back(tmpData);
		}

		return bytesTotal;
	}
};

#endif //_VCHAT_CLIENT_SERIALIZE_H_
