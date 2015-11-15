#ifndef COMMON_API__MESSAGE_MACROS_H
#define COMMON_API__MESSAGE_MACROS_H


#define DefineMessageBegin(ClassName,BaseClass,MessageID)                                                   \
	class DECLSPEC ClassName : public BaseClass                                                       \
	{                                                                                                       \
		enum { MESSAGE_ID = MessageID };                                                                    \
		private:                                                                                            \
			class Entry																						\
			{																								\
			public:																							\
				Entry();																					\
				virtual ~Entry();																			\
			};																								\
																											\
			static Entry ms_classEntry;																		\
			void InitializeMembers();                                                                       \
		public:                                                                                             \
			ClassName();																					\
			ClassName(const unsigned messageId);															\
			ClassName(const ClassName & source);															\
			virtual Basic * Clone() const;																	\
			virtual Basic * Clone(StorageVector_t &storageVector) const;									\
			virtual const char * MessageName() const { return #ClassName; }									\
			virtual const char * MessageIDString() const { return #MessageID; }

#if defined(PRINTABLE_MESSAGES) || defined(TRACK_READ_WRITE_FAILURES)
#	define DefineMessageMember(MemberName,Type)                                                             \
		public:																								\
			class Print##MemberName : public soe::AutoVariable<Type >										\
			{																								\
				public:																						\
					virtual const char * VariableName() const { return #MemberName; }						\
			};																								\
		public:																								\
			Print##MemberName m##MemberName;																\
		public:                                                                                             \
			const Type & Get##MemberName() const                                                            \
				{ return m##MemberName.Get(); }																\
			Type & Get##MemberName()			                                                            \
				{ return m##MemberName.Get(); }																\
			void Set##MemberName(const Type & value)                                                        \
				{ m##MemberName.Set((const Type &)value); }

#else
#	define DefineMessageMember(MemberName,Type)                                                             \
		public:	                                                                                            \
			soe::AutoVariable<Type > m##MemberName;                                                         \
		public:                                                                                             \
			const Type & Get##MemberName() const                                                            \
				{ return m##MemberName.Get(); }																\
			Type & Get##MemberName()			                                                            \
				{ return m##MemberName.Get(); }																\
			void Set##MemberName(const Type & value)                                                        \
				{ m##MemberName.Set((const Type &)value); }

#endif

#define DefineMessageEnd                                                                                    \
	};

#define ImplementMessageBegin(ClassName,BaseClass)                                                          \
	ClassName::ClassName() : BaseClass(MESSAGE_ID)															\
	{                                                                                                       \
		InitializeMembers();                                                                                \
	}                                                                                                       \
	ClassName::ClassName(const unsigned messageId) : BaseClass(messageId)									\
	{                                                                                                       \
		InitializeMembers();                                                                                \
	}                                                                                                       \
	ClassName::ClassName(const ClassName & source)															\
	{																										\
		InitializeMembers();																				\
																											\
		MemberVector_t::iterator iterator;																	\
		MemberVector_t::const_iterator sourceIterator = source.mMembers.begin();							\
		for(iterator=mMembers.begin(); iterator!=mMembers.end(); iterator++, sourceIterator++)				\
		{																									\
			iterator->data->Copy(sourceIterator->data);														\
			iterator->size = sourceIterator->size;															\
		}																									\
	}																										\
	Basic * ClassName::Clone() const																		\
	{																										\
		ClassName * msg = new ClassName(*this);																\
		/*msg->InitializeMembers();		*/																	\
		return msg;																							\
	}																										\
	Basic * ClassName::Clone(StorageVector_t & storageVector) const											\
	{																										\
		ClassName * msg = new(storageVector) ClassName(*this);												\
		/*msg->InitializeMembers();		*/																	\
		return msg;																							\
	}																										\
	ClassName::Entry::Entry()																				\
	{																										\
		ClassName msg;																						\
																											\
		AddClassMapEntry(msg.GetMsgId(), &msg);																\
	}																										\
	ClassName::Entry::~Entry()																				\
	{																										\
	}																										\
	ClassName::Entry ClassName::ms_classEntry;																\
	void ClassName::InitializeMembers()                                                                     \
	{

#define ImplementMessageMember(MemberName,DefaultValue)                                                     \
		m##MemberName.Set(DefaultValue);                                                                    \
		Insert(m##MemberName);

#define ImplementMessageMemberEx(MemberName,DefaultValue,MaxLen)                                            \
		m##MemberName.Set(DefaultValue);                                                                    \
		Insert(m##MemberName, MaxLen);

#define ImplementVersionAddedMessageMember(MemberName,DefaultValue,Version)                             \
	m##MemberName.Set(DefaultValue);                                                                    \
	Insert(m##MemberName, 1, Version, eAdded);

#define ImplementVersionDroppedMessageMember(MemberName,DefaultValue,Version)                           \
	m##MemberName.Set(DefaultValue);                                                                    \
	Insert(m##MemberName, 1, Version, eDropped);

#define ImplementVersionAddedMessageMemberEx(MemberName,DefaultValue,MaxLen,Version)                  \
	m##MemberName.Set(DefaultValue);                                                                    \
	Insert(m##MemberName, MaxLen, Version, eAdded);

#define ImplementVersionDroppedMessageMemberEx(MemberName,DefaultValue,MaxLen,Version)                \
	m##MemberName.Set(DefaultValue);                                                                    \
	Insert(m##MemberName, MaxLen, Version, eDropped);

#define ImplementMessageMemberOptional(MemberName,DefaultValue)                                             \
		m##MemberName.Set(DefaultValue);                                                                    \
		InsertOptional(m##MemberName);

#define ImplementMessageMemberOptionalEx(MemberName,DefaultValue,MaxLen)                                    \
		m##MemberName.Set(DefaultValue);                                                                    \
		InsertOptional(m##MemberName, MaxLen);

#define ImplementMessageEnd                                                                                 \
	}

#endif

