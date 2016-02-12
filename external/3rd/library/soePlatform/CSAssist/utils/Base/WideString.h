#ifndef BASE__UCS2_H
#define BASE__UCS2_H


#include <algorithm>
#include <string>
#include <vector>
#include "Base/Archive.h"


namespace ucs2
{

    class string
    {
		public:
			typedef unsigned short  char_type;
			typedef unsigned        size_type;
    
        public:
            string();
            string(const string & copy);
            string(const char * copy);
            string(const char_type * copy);
            explicit string(const std::string & copy);
            ~string();

            string & operator=(const char * rhs);
            string & operator=(const std::string & rhs);
            string & operator=(const char_type * rhs);
            string & operator=(const string & rhs);

            string & operator+=(const char * rhs);
            string & operator+=(const std::string & rhs);
            string & operator+=(const char_type * rhs);
            string & operator+=(const string & rhs);

            const char_type & at(size_type index) const;
            char_type & at(size_type index);

            const char_type & operator[](size_type index) const;
            char_type & operator[](size_type index);

            bool operator==(const string & rhs) const;
            bool operator!=(const string & rhs) const;
            bool operator<(const string & rhs) const;
            bool operator<=(const string & rhs) const;
            bool operator>(const string & rhs) const;
            bool operator>=(const string & rhs) const;

            string & assign(const char_type * rhs);
            string & assign(const char_type * rhs, size_type count);
            string & assign(const string & rhs, size_type position, size_type count);
            string & assign(const string & rhs);
            string & assign(size_type count, char_type c);
            string & assign(const char_type * first, const char_type * last);

            string & append(const char_type * rhs);
            string & append(const char_type * rhs, size_type count);
            string & append(const string & rhs, size_type position, size_type count);
            string & append(const string & rhs);
            string & append(size_type count, char_type c);
            string & append(const char_type * first, const char_type * last);

            std::string narrow() const;
            const char_type * c_str() const;
            const char_type * data() const;

            size_type length() const;
            size_type size() const;
            size_type max_size() const;
            void resize(size_type n, char_type c = 0x0032);
            size_type capacity() const;
            void reserve(size_type n = 0);
            bool empty() const;

        private:
            size_type              mLength;
            std::vector<char_type> mData;
			static char_type	   mOutOfRangeCharacter;
    };

	inline char WideToNarrow(string::char_type c) { return (char)c; }
	inline string::char_type NarrowToWide(char c) { return (string::char_type)c; }

	string::char_type string::mOutOfRangeCharacter(0);

	////////////////////////////////////////
	//	default constructor allocates 8 characters and sets length to zero
    inline string::string() :
        mLength(0),
        mData(8,0)
    {
    }

	////////////////////////////////////////
	//	target string is a nullptr-terminated C string
    inline string::string(const char * copy) :
        mLength(0),
        mData(8,0)
    {
		//	(1) protect from nullptr pointer
		if (!copy)
		{
			return;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (*copy)
		{
			reserve(mLength+1);
			mData[mLength++] = *copy++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
    }

	////////////////////////////////////////
	//	target string is a C string that may or may not include nullptr characters
    inline string::string(const std::string & copy) :
        mLength(copy.length()),
        mData(8,0)
    {
		//	(1) ensure we have the storage for the entire string
		reserve(mLength);
		//	(2) perform transform to copy the narrow string to our
		//		wide string
		std::transform(copy.begin(), copy.end(), mData.begin(), NarrowToWide);
		//	(3) ensure nullptr termination
		mData[mLength] = 0;
	}

	////////////////////////////////////////
	//	target string is a wide nullptr-terminated C string
    inline string::string(const char_type * copy) :
        mLength(0),
        mData(8,0)
    {
		//	(1) protect from nullptr pointer
		if (!copy)
		{
			return;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (*copy)
		{
			reserve(mLength+1);
			mData[mLength++] = *copy++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
    }

	////////////////////////////////////////
	//	copy constructor
    inline string::string(const string & copy) :
        mLength(copy.mLength),
        mData(8,0)
    {
		reserve(mLength);
		mData.assign(copy.mData.begin(), copy.mData.begin()+copy.mLength+1);
    }

    inline string::~string()
    {
    }

    inline string & string::operator=(const char * rhs)
	{
		mLength = 0;
		//	(1) protect from nullptr pointer
		if (!rhs)
		{
			mData[0] = 0;
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (*rhs)
		{
			reserve(mLength+1);
			mData[mLength++] = *rhs++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
	}

    inline string & string::operator=(const std::string & rhs)
	{
		mLength = rhs.length();
		//	(1) ensure we have the storage for the entire string
		reserve(mLength);
		//	(2) perform transform to copy the narrow string to our
		//		wide string
		std::transform(rhs.begin(), rhs.end(), mData.begin(), NarrowToWide);
		//	(3) ensure nullptr termination
		mData[mLength] = 0;
		return *this;
	}

    inline string & string::operator=(const char_type * rhs)
	{
		mLength = 0;
		//	(1) protect from nullptr pointer
		if (!rhs)
		{
			mData[0] = 0;
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (*rhs)
		{
			reserve(mLength+1);
			mData[mLength++] = *rhs++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
	}

    inline string & string::operator=(const string & rhs)
	{
		//	(1) protect from assigning to self
		if (&rhs != this)
		{
			mLength = rhs.mLength;
			reserve(mLength);
			mData.assign(rhs.mData.begin(), rhs.mData.begin()+rhs.mLength+1);
		}
		return *this;
	}

    inline string & string::operator+=(const char * rhs)
	{
		//	(1) protect from nullptr pointer
		if (!rhs)
		{
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (*rhs)
		{
			reserve(mLength+1);
			mData[mLength++] = *rhs++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
	}

    inline string & string::operator+=(const std::string & rhs)
	{
		//	(1) ensure we have the storage for the entire string
		reserve(mLength+rhs.length());
		//	(2) perform transform to copy the narrow string to our
		//		wide string
		std::transform(rhs.begin(), rhs.end(), mData.begin()+mLength, NarrowToWide);
		//	(3) ensure nullptr termination
		mLength += rhs.length();
		mData[mLength] = 0;
		return *this;
	}

    inline string & string::operator+=(const char_type * rhs)
	{
		//	(1) protect from nullptr pointer
		if (!rhs)
		{
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (*rhs)
		{
			reserve(mLength+1);
			mData[mLength++] = *rhs++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
	}

    inline string & string::operator+=(const string & rhs)
	{
		reserve(mLength+rhs.mLength);
		std::copy(rhs.mData.begin(), rhs.mData.begin()+rhs.mLength+1, mData.begin()+mLength);
		mLength += rhs.mLength;

        return *this;
	}

    inline const string::char_type & string::at(string::size_type index) const
	{
		return (index > mLength) ? mOutOfRangeCharacter : mData[index];
	}

    inline string::char_type & string::at(string::size_type index)
	{
		return (index > mLength) ? mOutOfRangeCharacter : mData[index];
	}

    inline const string::char_type & string::operator[](string::size_type index) const
	{
		return (index > mLength) ? mOutOfRangeCharacter : mData[index];
	}

    inline string::char_type & string::operator[](string::size_type index)
	{
		return (index > mLength) ? mOutOfRangeCharacter : mData[index];
	}

    inline bool string::operator==(const string & rhs) const
	{
		if (mLength != rhs.mLength)
		{
			return false;
		}
		else
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*mLength) == 0);
		}
	}

    inline bool string::operator!=(const string & rhs) const
	{
		if (mLength != rhs.mLength)
		{
			return true;
		}
		else
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*mLength) != 0);
		}
	}

    inline bool string::operator<(const string & rhs) const
	{
		if (mLength <= rhs.mLength)
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*mLength) < 0);
		}
		else
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*rhs.mLength) < 0);
		}
	}

    inline bool string::operator<=(const string & rhs) const
	{
		if (mLength <= rhs.mLength)
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*mLength) <= 0);
		}
		else
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*rhs.mLength) <= 0);
		}
	}

    inline bool string::operator>(const string & rhs) const
	{
		if (mLength <= rhs.mLength)
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*mLength) > 0);
		}
		else
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*rhs.mLength) > 0);
		}
	}

    inline bool string::operator>=(const string & rhs) const
	{
		if (mLength <= rhs.mLength)
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*mLength) >= 0);
		}
		else
		{
			return (memcmp(&mData[0], &rhs.mData[0], sizeof(size_type)*rhs.mLength) >= 0);
		}
	}

    inline string & string::assign(const char_type * rhs)
    {
		mLength = 0;
		//	(1) protect from nullptr pointer
		if (!rhs)
		{
			mData[0] = 0;
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (*rhs)
		{
			reserve(mLength+1);
			mData[mLength++] = *rhs++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
    }

    inline string & string::assign(const char_type * rhs, size_type count)
    {
		mLength = 0;
		//	(1) protect from nullptr pointer
		if (!rhs)
		{
			mData[0] = 0;
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (count--)
		{
			reserve(mLength+1);
			mData[mLength++] = *rhs++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
    }

    inline string & string::assign(const string & rhs, size_type position, size_type count)
    {
		//	(1) protect from assigning to self
		if (&rhs != this)
		{
			mLength = count;
			reserve(mLength);
			mData.assign(rhs.mData.begin()+position, rhs.mData.begin()+position+count+1);
		}
		return *this;
    }

    inline string & string::assign(const string & rhs)
    {
		//	(1) protect from assigning to self
		if (&rhs != this)
		{
			mLength = rhs.mLength;
			reserve(mLength);
			mData.assign(rhs.mData.begin(), rhs.mData.begin()+rhs.mLength+1);
		}
		return *this;
    }

    inline string & string::assign(size_type count, char_type c)
    {
		mLength = count;
		reserve(mLength);
		std::fill(mData.begin(), mData.end(), c);
		mData[mLength]=0;
		return *this;
    }

    inline string & string::assign(const char_type * first, const char_type * last)
    {
		mLength = 0;
		//	(1) protect from nullptr pointer
		if (!first || !last)
		{
			mData[0] = 0;
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (first != last)
		{
			reserve(mLength+1);
			mData[mLength++] = *first++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
    }

    inline string & string::append(const char_type * rhs)
    {
		//	(1) protect from nullptr pointer
		if (!rhs)
		{
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (*rhs)
		{
			reserve(mLength+1);
			mData[mLength++] = *rhs++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
    }

    inline string & string::append(const char_type * rhs, size_type count)
    {
		//	(1) protect from nullptr pointer
		if (!rhs)
		{
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (count--)
		{
			reserve(mLength+1);
			mData[mLength++] = *rhs++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
    }

    inline string & string::append(const string & rhs, size_type position, size_type count)
    {
		//	(1)	protect from invalid position value
		if (rhs.mLength <= position)
		{
			return *this;
		}
		//	(2) make sure we copy count charatcers, or to the end of the string
		if (rhs.mLength < position+count)
		{
			count = rhs.mLength-position;
		}
		//	(3) perform copy
		reserve(mLength+count);
		std::copy(rhs.mData.begin()+position, rhs.mData.begin()+position+count+1, mData.begin()+mLength);
		mLength += count;
        return *this;
    }

    inline string & string::append(const string & rhs)
    {
		//	(1) perform copy
		reserve(mLength+rhs.mLength);
		std::copy(rhs.mData.begin(), rhs.mData.begin()+rhs.mLength+1, mData.begin()+mLength);
		mLength += rhs.mLength;
        return *this;
    }

    inline string & string::append(size_type count, char_type c)
    {
		reserve(mLength+count);
		std::fill(mData.begin()+mLength, mData.begin()+mLength+count, c);
		mLength += count;
		mData[mLength]=0;
		return *this;
    }

    inline string & string::append(const char_type * first, const char_type * last)
    {
		//	(1) protect from nullptr pointer
		if (!first || !last)
		{
			return *this;
		}
		//	(2) linear copy the string, must reserve before each character
		//		because the string length is unknown
		while (first != last)
		{
			reserve(mLength+1);
			mData[mLength++] = *first++;
		}
		//	(3) ensure nullptr termination
		mData[mLength]=0;
		return *this;
    }

    inline std::string string::narrow() const
	{
		std::string narrow;
		narrow.resize(mLength);
		std::transform(mData.begin(), mData.begin()+mLength+1, narrow.begin(), WideToNarrow);
		return narrow;
	}

    inline const string::char_type * string::c_str() const
	{
		return &mData[0];
	}

    inline const string::char_type * string::data() const
	{
		return &mData[0];
	}

    inline string::size_type string::length() const
    {
        return mLength;
    }

    inline string::size_type string::size() const
    {
        return mLength;
    }

    inline void string::resize(string::size_type n, string::char_type c)
    {
        reserve(n);
		
		while (mLength<n)
		{
			mData[mLength++] = c;
		}
    }

    inline string::size_type string::capacity() const
    {
        return mData.size()-1;
    }

    inline void string::reserve(string::size_type n)
    {
        while (n > capacity())
            mData.resize((capacity()+1)*2,0x0000);
    }

    inline bool string::empty() const
    {
        return mLength == 0;
    }


}

namespace Base
{


    inline void get(ByteStream::ReadIterator & source, ucs2::string & target)
    {
        unsigned int size = 0;
        get(source, size);

        const unsigned char * const buf = source.getBuffer();
        const ucs2::string::char_type * const ubuf = reinterpret_cast<const ucs2::string::char_type *>(buf);

        target.assign(ubuf, ubuf + size);

        const unsigned int readSize = size * sizeof(ucs2::string::char_type);
        source.advance(readSize);
    }

    inline void put(ByteStream & target, const ucs2::string & source)
    {
        const unsigned int size = source.size();
        put(target, size);
        put(target, (const unsigned char *)source.data(), size*sizeof(ucs2::string::char_type));
    }


}


#endif
