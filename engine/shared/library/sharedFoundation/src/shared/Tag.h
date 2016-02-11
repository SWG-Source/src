// ======================================================================
//
// Tag.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef TAG_H
#define TAG_H

// ======================================================================

#define TAG_DIGIT_NULL   00
#define TAG_DIGIT_SPACE  20

#define TAG_DIGIT_0      30
#define TAG_DIGIT_1      31
#define TAG_DIGIT_2      32
#define TAG_DIGIT_3      33
#define TAG_DIGIT_4      34
#define TAG_DIGIT_5      35
#define TAG_DIGIT_6      36
#define TAG_DIGIT_7      37
#define TAG_DIGIT_8      38
#define TAG_DIGIT_9      39

#define TAG_DIGIT_A      41
#define TAG_DIGIT_B      42
#define TAG_DIGIT_C      43
#define TAG_DIGIT_D      44
#define TAG_DIGIT_E      45
#define TAG_DIGIT_F      46
#define TAG_DIGIT_G      47
#define TAG_DIGIT_H      48
#define TAG_DIGIT_I      49
#define TAG_DIGIT_J      4A
#define TAG_DIGIT_K      4B
#define TAG_DIGIT_L      4C
#define TAG_DIGIT_M      4D
#define TAG_DIGIT_N      4E
#define TAG_DIGIT_O      4F
#define TAG_DIGIT_P      50
#define TAG_DIGIT_Q      51
#define TAG_DIGIT_R      52
#define TAG_DIGIT_S      53
#define TAG_DIGIT_T      54
#define TAG_DIGIT_U      55
#define TAG_DIGIT_V      56
#define TAG_DIGIT_W      57
#define TAG_DIGIT_X      58
#define TAG_DIGIT_Y      59
#define TAG_DIGIT_Z      5A

#define TAG_DIGIT__      5f

#define TAG_DIGIT_a      61
#define TAG_DIGIT_b      62
#define TAG_DIGIT_c      63
#define TAG_DIGIT_d      64
#define TAG_DIGIT_e      65
#define TAG_DIGIT_f      66
#define TAG_DIGIT_g      67
#define TAG_DIGIT_h      68
#define TAG_DIGIT_i      69
#define TAG_DIGIT_j      6A
#define TAG_DIGIT_k      6B
#define TAG_DIGIT_l      6C
#define TAG_DIGIT_m      6D
#define TAG_DIGIT_n      6E
#define TAG_DIGIT_o      6F
#define TAG_DIGIT_p      70
#define TAG_DIGIT_q      71
#define TAG_DIGIT_r      72
#define TAG_DIGIT_s      73
#define TAG_DIGIT_t      74
#define TAG_DIGIT_u      75
#define TAG_DIGIT_v      76
#define TAG_DIGIT_w      77
#define TAG_DIGIT_x      78
#define TAG_DIGIT_y      79
#define TAG_DIGIT_z      7A

// ======================================================================

typedef uint32 Tag;

// ======================================================================

#define TAG_B(a,b,c,d) 0x ## a ## b ## c ## d
#define TAG_A(a,b,c,d) static_cast<Tag>(TAG_B(a,b,c,d))
#define TAG(a,b,c,d)   TAG_A(TAG_DIGIT_ ## a, TAG_DIGIT_ ## b, TAG_DIGIT_ ## c, TAG_DIGIT_ ## d)

#define TAG3(a,b,c)    TAG_A(TAG_DIGIT_ ## a, TAG_DIGIT_ ## b, TAG_DIGIT_ ## c, TAG_DIGIT_SPACE)
#define TAG2(a,b)      TAG_A(TAG_DIGIT_ ## a, TAG_DIGIT_ ## b, TAG_DIGIT_SPACE, TAG_DIGIT_SPACE)
#define TAG1(a)        TAG_A(TAG_DIGIT_ ## a, TAG_DIGIT_SPACE, TAG_DIGIT_SPACE, TAG_DIGIT_SPACE)

#define TAG03(a,b,c)   TAG_A(TAG_DIGIT_ ## a, TAG_DIGIT_ ## b, TAG_DIGIT_ ## c, TAG_DIGIT_NULL)
#define TAG02(a,b)     TAG_A(TAG_DIGIT_ ## a, TAG_DIGIT_ ## b, TAG_DIGIT_NULL, TAG_DIGIT_NULL)
#define TAG01(a)       TAG_A(TAG_DIGIT_ ## a, TAG_DIGIT_NULL, TAG_DIGIT_NULL, TAG_DIGIT_NULL)

// ======================================================================
/**
 * Convert a text string to a Tag
 *
 * This routine will convert a text string, such as "ABCD", to the tag value 'ABCD'.
 * If the string is less than 4 characters long, it will be padded with spaces.
 * If the string is greater than 4 characters long, it will be truncated.
 *
 * @param value string to convert to a Tag.
 */

inline Tag ConvertStringToTag(const char *value)
{
	Tag result = 0;
	
	const int length = strlen(value);
	for (int i = 0; i < 4; ++i)
	{
		const uint32 ch = static_cast<uint32>((i >= length) ? ' ' : value[i]);
		result = (result << 8) | ch;
	}

	return result;
}

// ----------------------------------------------------------------------
/**
 * Convert an integer to a Tag
 *
 * This routine will convert an int, such as 4, to the tag value '0004'.
 *
 * @param value value to convert to tag format
 */

inline Tag ConvertIntToTag(int value)
{
	Tag result = 0;
	
	DEBUG_FATAL(value < 0 || value > 9999, ("value out of range 0/%d/9999", value));

	for (int i = 0; i < 4; ++i)
	{
		const uint digit = static_cast<uint>(static_cast<int>(value % 10));
		value /= 10;
		result |= (digit + '0') << (i * 8);
	}

	return result;
}

// ----------------------------------------------------------------------
/**
 * Print the specified tag into a text buffer.
 * 
 * This routine will fill the first four characters of the specified character
 * array with the name of the specified tag.  If a character of the tag
 * is not printable, it will be replaced with a question mark.
 * 
 * A nullptr-character will be appended to the output buffer.
 * 
 * This routine assumes the specified character buffer is at least 5 characters
 * in length.
 * 
 * @param tag  Tag to format
 * @param buffer  Character array to format the tag into
 */

inline void ConvertTagToString(Tag tag, char *buffer)
{
	int  i, j, ch;

	DEBUG_FATAL(!buffer, ("buffer is nullptr"));

	for (i = 0, j = 24; i < 4; ++i, j -= 8)
	{
		ch = (static_cast<int>(tag) >> j) & 0xff; //lint !e702 // shift right of a signed quantity
		if (isprint(ch))
			buffer[i] = static_cast<char>(ch);
		else
			buffer[i] = '?';
	}

	buffer[4] = 0;
}

// ----------------------------------------------------------------------
/**
 * Convert the specified tag to an int.
 * 
 * This routine will attempt to create an integer using as
 * many of least significant bytes of the tag as possible.
 *
 * Example: TAG(0,1,0,0) == 100
 * Example: TAG(1,R,1,0) == 10
 * Example: TAG(1,R,1,X) == 0
 * 
 * @param tag  Tag to format
 * @return Integer form of tag string.
 */

inline int ConvertTagSuffixToInt(Tag tag)
{
	int returnValue=0;
	for (int i=24;i>=0;i-=8)
	{
		const int c = (tag>>i) & 0xff;
		if (c>='0' && c<='9')
		{
			returnValue = returnValue*10 + int(c) -'0';
		}
		else
		{
			returnValue=0;
		}
	}
	return returnValue;
}

// ----------------------------------------------------------------------
/**
 * Convert the specified tag to an int.
 * 
 * This routine will attempt to create an integer from 
 * a tag containing only numeric characters. If non-numeric
 * characters are detected, 0 is returned.
 *
 * Example: TAG(0,1,0,0) == 100
 * Example: TAG(1,R,1,0) == 0
 * Example: TAG(1,R,1,X) == 0
 * 
 * @param tag  Tag to format
 * @return Integer form of tag string.
 */

inline int ConvertTagToInt(Tag tag)
{
	int returnValue=0;
	for (int i=24;i>=0;i-=8)
	{
		const int c = (tag>>i) & 0xff;
		if (c>='0' && c<='9')
		{
			returnValue = returnValue*10 + int(c) -'0';
		}
		else
		{
			return 0;
		}
	}
	return returnValue;
}

// ======================================================================

const Tag TAG_0000 = TAG(0,0,0,0);
const Tag TAG_0001 = TAG(0,0,0,1);
const Tag TAG_0002 = TAG(0,0,0,2);
const Tag TAG_0003 = TAG(0,0,0,3);
const Tag TAG_0004 = TAG(0,0,0,4);
const Tag TAG_0005 = TAG(0,0,0,5);
const Tag TAG_0006 = TAG(0,0,0,6);
const Tag TAG_0007 = TAG(0,0,0,7);
const Tag TAG_0008 = TAG(0,0,0,8);
const Tag TAG_0009 = TAG(0,0,0,9);
const Tag TAG_0010 = TAG(0,0,1,0);
const Tag TAG_0011 = TAG(0,0,1,1);
const Tag TAG_0012 = TAG(0,0,1,2);
const Tag TAG_0013 = TAG(0,0,1,3);
const Tag TAG_0014 = TAG(0,0,1,4);
const Tag TAG_0015 = TAG(0,0,1,5);

const Tag TAG_DATA = TAG(D,A,T,A);
const Tag TAG_ENTR = TAG(E,N,T,R);
const Tag TAG_FORM = TAG(F,O,R,M);
const Tag TAG_INFO = TAG(I,N,F,O);

const Tag TAG_ARRY = TAG(A,R,R,Y);
const Tag TAG_NAME = TAG(N,A,M,E);
const Tag TAG_PARM = TAG(P,A,R,M);

//-- please do not add any Tags to this file unless they are generic
//-- and expected to be used in many different contexts

// ======================================================================

#endif

