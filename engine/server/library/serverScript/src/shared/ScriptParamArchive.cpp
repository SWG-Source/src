// ScriptParamArchive.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverScript/ScriptParamArchive.h"

#include "Archive/Archive.h"
#include "UnicodeUtils.h"
#include "localizationArchive/StringIdArchive.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/AttribMod.h"
#include "sharedGame/AttribModArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ObjectMenuRequestData.h"
#include "sharedNetworkMessages/ObjectMenuRequestDataArchive.h"
#include "sharedObject/CachedNetworkIdArchive.h"
#include "sharedUtility/ValueDictionary.h"
#include "sharedUtility/ValueDictionaryArchive.h"
#include "unicodeArchive/UnicodeArchive.h"
#include <deque>

//-----------------------------------------------------------------------

namespace Archive
{

void get(ReadIterator & source, std::vector<const std::vector<NetworkId> *> & target)
{
	target.clear();
	signed int length = 0;
	get(source, length);
	for (int i = 0; i < length; ++i)
	{
		std::vector<NetworkId> * inner = new std::vector<NetworkId>();
		signed int innerLength = 0;
		get(source, innerLength);
		NetworkId id;
		for (int j = 0; j < innerLength; ++j)
		{
			get(source, id);
			inner->push_back(id);
		}
		target.push_back(inner);
	}
}

void put(ByteStream & target, const std::vector<const std::vector<NetworkId> *> & source)
{
	signed int length = source.size();
	put(target, length);
	for (int i = 0; i < length; ++i)
	{
		const std::vector<NetworkId> * inner = source[i];
		signed int innerLength = 0;
		if (inner != nullptr)
			innerLength = inner->size();
		put(target, innerLength);
		for (int j = 0; j < innerLength; ++j)
		{
			put(target, inner->at(j));
		}
	}
}

//-----------------------------------------------------------------------

void get(ReadIterator & source, Param & target)
{
	Param * p = &target;
	p->m_owned = true;
	int paramType;
	get(source, paramType);
	p->m_type = static_cast<Param::ParamType>(paramType);
	get(source, p->m_name);
	switch(paramType)
	{
	case Param::BOOL:
		{
			get(source, p->m_param.bParam);
		}
		break;
	case Param::BOOL_ARRAY:
		{
			std::deque<bool> * b = new std::deque<bool>;
			get(source, *b);
			p->m_param.baParam = b;
		}
		break;
	case Param::INT:
		{
			get(source, p->m_param.iParam);
		}
		break;
	case Param::INT_ARRAY:
		{
			std::vector<int> * v = new std::vector<int>;
			get(source, *v);
			p->m_param.iaParam = v;
		}
		break;
	case Param::FLOAT:
		{
			get(source, p->m_param.fParam);
		}
		break;
	case Param::FLOAT_ARRAY:
		{
			std::vector<float> * v = new std::vector<float>;
			get(source, *v);
			p->m_param.faParam = v;
		}
		break;
	case Param::STRING:
		{
			std::string s;
			get(source, s);
			char * str = new char[s.length() + 1];
			strcpy(str, s.c_str());
			p->m_param.sParam = str;
		}
		break;
	case Param::STRING_ARRAY:
		{
			std::vector<std::string> v;
			get(source, v);
			std::vector<std::string>::const_iterator si;
			std::vector<const char *> * s = new std::vector<const char *>;
			for(si = v.begin(); si != v.end(); ++si)
			{
				char * str = new char[(*si).length() + 1];
				strcpy(str, (*si).c_str());
				s->push_back(str);
			}
			p->m_param.saParam = s;
		}
		break;
	case Param::UNICODE:
		{
			Unicode::String * s = new Unicode::String;
			get(source, *s);
			p->m_param.uParam = s;
		}
		break;
	case Param::UNICODE_ARRAY:
		{
			std::vector<const Unicode::String *> * s = new std::vector<const Unicode::String *>;
			int count;
			get(source, count);
			int index;
			for(index = 0; index < count; ++index)
			{
				Unicode::String tmp;
				get(source, tmp);
				Unicode::String * entry = new Unicode::String(tmp);
				s->push_back(entry);
			}
			p->m_param.uaParam = s;
		}
		break;
	case Param::OBJECT_ID:
		{
			NetworkId * id = new NetworkId;
			get(source, *id);
			p->m_param.oidParam = id;
		}
		break;
	case Param::OBJECT_ID_ARRAY:
		{
			std::vector<NetworkId> * ids = new std::vector<NetworkId>;
			get(source, *ids);
			p->m_param.oidaParam = ids;
		}
		break;
	case Param::CACHED_OBJECT_ID_ARRAY:
		{
			std::vector<CachedNetworkId> * ids = new std::vector<CachedNetworkId>;
			get(source, *ids);
			p->m_param.coidaParam = ids;
		}
		break;
	case Param::OBJECT_ID_ARRAY_ARRAY:
		{
			std::vector<const std::vector<NetworkId> *> * ids = new std::vector<const std::vector<NetworkId> *>;
			get(source, *ids);
			p->m_param.oidaaParam = ids;
		}
		break;
	case Param::STRING_ID:
		{
			StringId * s = new StringId;
			get(source, *s);
			p->m_param.sidParam = s;
		}
		break;
	case Param::STRING_ID_ARRAY:
		{
			int count;
			get(source, count);
			std::vector<const StringId *> * s = new std::vector<const StringId *>;
			int index;
			for(index = 0; index < count; ++index)
			{
				StringId * id = new StringId;
				get(source, *id);
				s->push_back(id);
			}
			p->m_param.sidaParam = s;
		}
		break;
	case Param::ATTRIB_MOD:
		{
			AttribMod::AttribMod * m = new AttribMod::AttribMod;
			get(source, *m);
			p->m_param.amParam = m;
		}
		break;
	case Param::ATTRIB_MOD_ARRAY:
		{
			std::vector<AttribMod::AttribMod> * m = new std::vector<AttribMod::AttribMod>;
			int count;
			get(source, count);
			int index;
			for(index = 0; index < count; ++index)
			{
				AttribMod::AttribMod mod;
				get(source, mod);
				m->push_back(mod);
			}
			p->m_param.amaParam = m;
		}
		break;
	case Param::MENTAL_STATE_MOD:
		{
			ServerObjectTemplate::MentalStateMod * m = new ServerObjectTemplate::MentalStateMod;
			int t ;
			get(source, t);
			m->target = static_cast<ServerObjectTemplate::MentalStates>(t);
			get(source, m->value);
			get(source, m->timeAtValue);
			get(source, m->decay);
			p->m_param.msmParam = m;
		}
		break;
	case Param::MENTAL_STATE_MOD_ARRAY:
		{
			std::vector<ServerObjectTemplate::MentalStateMod> * m = new std::vector<ServerObjectTemplate::MentalStateMod>;
			int count;
			get(source, count);
			int index;
			for(index = 0; index < count; ++index)
			{
				ServerObjectTemplate::MentalStateMod mod;
				int t;
				get(source, t);
				mod.target = static_cast<ServerObjectTemplate::MentalStates>(t);
				get(source, mod.value);
				get(source, mod.timeAtValue);
				get(source, mod.decay);
				m->push_back(mod);
			}
			p->m_param.amsmParam = m;
		}
		break;
	case Param::OBJECT_MENU_INFO:
		{
			MenuDataVector * v = new MenuDataVector;
			get(source, *v);
			p->m_param.aomrdParam = v;
		}
		break;
	case Param::MANUFACTURE_SCHEMATIC:
		{
			DEBUG_FATAL(true, ("This NEVER is sent over the network! --sjakab\n"));
		}
		break;
	case Param::INGREDIENT_SLOT:
		{
			DEBUG_FATAL(true, ("This NEVER is sent over the network! --sjakab\n"));
		}
		break;
	case Param::LOCATION:
		{
			Vector * v = new Vector;
			get(source, *v);
			p->m_param.lParam = v;
		}
		break;
	case Param::LOCATION_ARRAY:
		{
			std::vector<const Vector *> * v = new std::vector<const Vector *>;
			get_ptr<Vector>(source, *v);
			p->m_param.laParam = v;
		}
		break;
	case Param::DICTIONARY:
		{
			ValueDictionary *  dictionary = new ValueDictionary;
			get(source, *dictionary);
			p->m_param.valueDictionaryParam = dictionary;
		}
		break;
	case Param::DICTIONARY_ARRAY:
		{
			int32 count;
			get(source, count);

			std::vector<ValueDictionary> * dictionaryArray = new std::vector<ValueDictionary>;
			ValueDictionary dictionary;

			int index;
			for(index = 0; index < static_cast<int>(count); ++index)
			{
				get(source, dictionary);
				dictionaryArray->push_back(dictionary);
			}

			p->m_param.valueDictionaryArrayParam = dictionaryArray;
		}
		break;
	case Param::BYTE_ARRAY:
		{
			std::vector<unsigned char> * v = new std::vector<unsigned char>;
			get(source, *v);
			p->m_param.byteArrayParam = v;
		}
		break;
	default:
		{
			DEBUG_FATAL(true, ("Unknown or unhandled ScriptParam type!!!!\n"));
		}
		break;
	}
}

//-----------------------------------------------------------------------

void put(ByteStream & target, const Param & source)
{
		Archive::put(target, source.m_type);
		Archive::put(target, source.m_name);
		switch(source.m_type)
		{
		case Param::BOOL:
			{
				Archive::put(target, source.m_param.bParam);
			}
			break;
		case Param::BOOL_ARRAY:
			{
				Archive::put(target, *source.m_param.baParam);
			}
			break;
		case Param::INT:
			{
				Archive::put(target, source.m_param.iParam);
			}
			break;
		case Param::INT_ARRAY:
			{
				Archive::put(target, *source.m_param.iaParam);
			}
			break;
		case Param::FLOAT:
			{
				Archive::put(target, source.m_param.fParam);
			}
			break;
		case Param::FLOAT_ARRAY:
			{
				Archive::put(target, *source.m_param.faParam);
			}
			break;
		case Param::STRING:
			{
				std::string s = source.m_param.sParam;
				Archive::put(target, s);
			}
			break;
		case Param::STRING_ARRAY:
			{
				std::vector<std::string> s;
				std::vector<const char *>::const_iterator c;
				for(c = source.m_param.saParam->begin(); c != source.m_param.saParam->end(); ++c)
				{
					if (*c)
					{
						s.push_back(std::string(*c));
					}
					else
					{
						s.push_back(std::string(""));
					}
				}
				Archive::put(target, s);
			}
			break;
		case Param::UNICODE:
			{
				Archive::put(target, *source.m_param.uParam);
			}
			break;
		case Param::UNICODE_ARRAY:
			{
				std::vector<const Unicode::String *>::const_iterator ui;
				int count = source.m_param.uaParam->size();
				Archive::put(target, count);
				for(ui = source.m_param.uaParam->begin(); ui != source.m_param.uaParam->end(); ++ui)
				{
					if ((*ui)) 
					{
						Archive::put(target, *(*ui));
					}
					else
					{
						static const Unicode::String emptyStr = 
							Unicode::narrowToWide("");
						Archive::put(target, emptyStr);
					}
				}
				
			}
			break;
		case Param::OBJECT_ID:
			{
				Archive::put(target, *source.m_param.oidParam);
			}
			break;
		case Param::OBJECT_ID_ARRAY:
			{
				Archive::put(target, *source.m_param.oidaParam);
			}
			break;
		case Param::CACHED_OBJECT_ID_ARRAY:
			{
				Archive::put(target, *source.m_param.coidaParam);
			}
			break;
		case Param::OBJECT_ID_ARRAY_ARRAY:
			{
				Archive::put(target, *source.m_param.oidaaParam);
			}
			break;
		case Param::STRING_ID:
			{
				Archive::put(target, *source.m_param.sidParam);
			}
			break;
		case Param::STRING_ID_ARRAY:
			{
				int count = source.m_param.sidaParam->size();
				Archive::put(target, count);
				std::vector<const StringId *>::const_iterator si;
				for(si = source.m_param.sidaParam->begin(); si != source.m_param.sidaParam->end(); ++si)
				{
					Archive::put(target, *(*si));
				}
			}
			break;
		case Param::ATTRIB_MOD:
			{
				const AttribMod::AttribMod * m = source.m_param.amParam;
				Archive::put(target, *m);
			}
			break;
		case Param::ATTRIB_MOD_ARRAY:
			{
				std::vector<AttribMod::AttribMod>::const_iterator mi;
				int count = source.m_param.amaParam->size();
				Archive::put(target, count);
				for (mi = source.m_param.amaParam->begin(); mi != source.m_param.amaParam->end(); ++mi)
				{
					Archive::put(target, *mi);
				}
			}
			break;
		case Param::MENTAL_STATE_MOD:
			{
				const ServerObjectTemplate::MentalStateMod * m = source.m_param.msmParam;
				int t = m->target;
				Archive::put(target, t);
				Archive::put(target, m->value);
				Archive::put(target, m->timeAtValue);
				Archive::put(target, m->decay);
			}
			break;
		case Param::MENTAL_STATE_MOD_ARRAY:
			{
				std::vector<ServerObjectTemplate::MentalStateMod>::const_iterator mi;
				int count = source.m_param.amsmParam->size();
				Archive::put(target, count);
				for(mi = source.m_param.amsmParam->begin(); mi != source.m_param.amsmParam->end(); ++mi)
				{
					int t = (*mi).target;
					Archive::put(target, t);
					Archive::put(target, (*mi).value);
					Archive::put(target, (*mi).timeAtValue);
					Archive::put(target, (*mi).decay);
				}
			}
			break;
		case Param::OBJECT_MENU_INFO:
			{
				Archive::put(target, *source.m_param.aomrdParam);
			}
			break;
		case Param::MANUFACTURE_SCHEMATIC:
			{
				DEBUG_FATAL(true, ("This NEVER is sent over the network! --sjakab\n"));
			}
			break;
		case Param::INGREDIENT_SLOT:
			{
				DEBUG_FATAL(true, ("This NEVER is sent over the network! --sjakab\n"));
			}
			break;
		case Param::LOCATION:
			{
				Archive::put(target, *source.m_param.lParam);
			}
			break;
		case Param::LOCATION_ARRAY:
			{
				Archive::put(target, *source.m_param.laParam);
			}
			break;
		case Param::DICTIONARY:
			{
				Archive::put(target, *source.m_param.valueDictionaryParam);
			}
			break;
		case Param::DICTIONARY_ARRAY:
			{
				int32 count = static_cast<int32>(source.m_param.valueDictionaryArrayParam->size());
				Archive::put(target, count);
				std::vector<ValueDictionary>::const_iterator iter;
				for(iter = source.m_param.valueDictionaryArrayParam->begin(); iter != source.m_param.valueDictionaryArrayParam->end(); ++iter)
				{
					Archive::put(target, (*iter));
				}
			}
			break;
		case Param::BYTE_ARRAY:
			{
				Archive::put(target, *source.m_param.byteArrayParam);
			}
			break;
		default:
			DEBUG_FATAL(true, ("Unknown or unhandled ScriptParam type!!!!\n"));
			break;
		}
}
	
//-----------------------------------------------------------------------

}//namespace Archive

//-----------------------------------------------------------------------

