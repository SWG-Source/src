// MD5.cpp: implementation of the MD5 class.
//
//////////////////////////////////////////////////////////////////////

#include "MD5.h"
#include "Types.h"

using namespace std;


#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{


    MD5::State::State() :
        state(4,0),
        count(2,0),
        buffer(64,0)
    {
        state[0] = 0x67452301;
        state[1] = 0xefcdab89;
        state[2] = 0x98badcfe;
        state[3] = 0x10325476;
    }

    static char const init[64] = 
    {
        -128, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    vector<char> MD5::padding(init,init+sizeof(init)/sizeof(init[0]));

    MD5::MD5()
    {
        Init();
    }

    MD5::MD5(const string & input)
    {
        Init();
        Update(input);
    }

    vector<int> MD5::Decode(const vector<char> &achar0, int i, int j)
    {
        vector<int> ai(16,0);
        int l;
        int k = l = 0;
        for(; l < i; l += 4)
        {
            ai[k] = (achar0[l + j] & 0xff) | (achar0[l + 1 + j] & 0xff) << 8 | (achar0[l + 2 + j] & 0xff) << 16 | (achar0[l + 3 + j] & 0xff) << 24;
            k++;
        }

        return ai;
    }

    vector<char> MD5::Encode(vector<int> &ai, int i)
    {
        vector<char> achar0(i,0);
        int k;
        int j = k = 0;
        for(; k < i; k += 4){
            achar0[k] = (char)(ai[j] & 0xff);
            achar0[k + 1] = (char)(ai[j] >> 8 & 0xff);
            achar0[k + 2] = (char)(ai[j] >> 16 & 0xff);
            achar0[k + 3] = (char)(ai[j] >> 24 & 0xff);
            j++;
        }

        return achar0;
    }

    int MD5::FF(int i, int j, int k, int l, int i1, int j1, int k1)
    {
        i = uadd(i, (j & k) | (~j & l), i1, k1);
        return uadd(rotate_left(i, j1), j);
    }

    vector<char> MD5::Final()
    {
        if (finalsNull)
        {
            State &state1 = state;
            vector<char> achar0 = Encode(state1.count, 8);
            int i = state1.count[0] >> 3 & 0x3f;
            int j = i >= 56 ? 120 - i : 56 - i;
            Update(state1, padding, 0, j);
            Update(state1, achar0, 0, 8);
            finals = state1;
            finalsNull = false;
        }
        return Encode(finals.state, 16);
    }

    int MD5::GG(int i, int j, int k, int l, int i1, int j1, int k1)
    {
        i = uadd(i, (j & l) | (k & ~l), i1, k1);
        return uadd(rotate_left(i, j1), j);
    }

    int MD5::HH(int i, int j, int k, int l, int i1, int j1, int k1)
    {
        i = uadd(i, j ^ k ^ l, i1, k1);
        return uadd(rotate_left(i, j1), j);
    }

    int MD5::II(int i, int j, int k, int l, int i1, int j1, int k1)
    {
        i = uadd(i, k ^ (j | ~l), i1, k1);
        return uadd(rotate_left(i, j1), j);
    }

    void MD5::Init()
    {
        if (padding.size() == 0)
        {
            padding.resize(64,0);
            padding[0] = -128;
        }
        finalsNull = true;
    }

    void MD5::Transform(State & state1, const vector<char> &achar0, int i)
    {
        int j = state1.state[0];
        int k = state1.state[1];
        int l = state1.state[2];
        int i1 = state1.state[3];
        vector<int> ai = Decode(achar0, 64, i);
        j = FF(j, k, l, i1, ai[0], 7, 0xd76aa478);
        i1 = FF(i1, j, k, l, ai[1], 12, 0xe8c7b756);
        l = FF(l, i1, j, k, ai[2], 17, 0x242070db);
        k = FF(k, l, i1, j, ai[3], 22, 0xc1bdceee);
        j = FF(j, k, l, i1, ai[4], 7, 0xf57c0faf);
        i1 = FF(i1, j, k, l, ai[5], 12, 0x4787c62a);
        l = FF(l, i1, j, k, ai[6], 17, 0xa8304613);
        k = FF(k, l, i1, j, ai[7], 22, 0xfd469501);
        j = FF(j, k, l, i1, ai[8], 7, 0x698098d8);
        i1 = FF(i1, j, k, l, ai[9], 12, 0x8b44f7af);
        l = FF(l, i1, j, k, ai[10], 17, -42063);
        k = FF(k, l, i1, j, ai[11], 22, 0x895cd7be);
        j = FF(j, k, l, i1, ai[12], 7, 0x6b901122);
        i1 = FF(i1, j, k, l, ai[13], 12, 0xfd987193);
        l = FF(l, i1, j, k, ai[14], 17, 0xa679438e);
        k = FF(k, l, i1, j, ai[15], 22, 0x49b40821);
        j = GG(j, k, l, i1, ai[1], 5, 0xf61e2562);
        i1 = GG(i1, j, k, l, ai[6], 9, 0xc040b340);
        l = GG(l, i1, j, k, ai[11], 14, 0x265e5a51);
        k = GG(k, l, i1, j, ai[0], 20, 0xe9b6c7aa);
        j = GG(j, k, l, i1, ai[5], 5, 0xd62f105d);
        i1 = GG(i1, j, k, l, ai[10], 9, 0x2441453);
        l = GG(l, i1, j, k, ai[15], 14, 0xd8a1e681);
        k = GG(k, l, i1, j, ai[4], 20, 0xe7d3fbc8);
        j = GG(j, k, l, i1, ai[9], 5, 0x21e1cde6);
        i1 = GG(i1, j, k, l, ai[14], 9, 0xc33707d6);
        l = GG(l, i1, j, k, ai[3], 14, 0xf4d50d87);
        k = GG(k, l, i1, j, ai[8], 20, 0x455a14ed);
        j = GG(j, k, l, i1, ai[13], 5, 0xa9e3e905);
        i1 = GG(i1, j, k, l, ai[2], 9, 0xfcefa3f8);
        l = GG(l, i1, j, k, ai[7], 14, 0x676f02d9);
        k = GG(k, l, i1, j, ai[12], 20, 0x8d2a4c8a);
        j = HH(j, k, l, i1, ai[5], 4, 0xfffa3942);
        i1 = HH(i1, j, k, l, ai[8], 11, 0x8771f681);
        l = HH(l, i1, j, k, ai[11], 16, 0x6d9d6122);
        k = HH(k, l, i1, j, ai[14], 23, 0xfde5380c);
        j = HH(j, k, l, i1, ai[1], 4, 0xa4beea44);
        i1 = HH(i1, j, k, l, ai[4], 11, 0x4bdecfa9);
        l = HH(l, i1, j, k, ai[7], 16, 0xf6bb4b60);
        k = HH(k, l, i1, j, ai[10], 23, 0xbebfbc70);
        j = HH(j, k, l, i1, ai[13], 4, 0x289b7ec6);
        i1 = HH(i1, j, k, l, ai[0], 11, 0xeaa127fa);
        l = HH(l, i1, j, k, ai[3], 16, 0xd4ef3085);
        k = HH(k, l, i1, j, ai[6], 23, 0x4881d05);
        j = HH(j, k, l, i1, ai[9], 4, 0xd9d4d039);
        i1 = HH(i1, j, k, l, ai[12], 11, 0xe6db99e5);
        l = HH(l, i1, j, k, ai[15], 16, 0x1fa27cf8);
        k = HH(k, l, i1, j, ai[2], 23, 0xc4ac5665);
        j = II(j, k, l, i1, ai[0], 6, 0xf4292244);
        i1 = II(i1, j, k, l, ai[7], 10, 0x432aff97);
        l = II(l, i1, j, k, ai[14], 15, 0xab9423a7);
        k = II(k, l, i1, j, ai[5], 21, 0xfc93a039);
        j = II(j, k, l, i1, ai[12], 6, 0x655b59c3);
        i1 = II(i1, j, k, l, ai[3], 10, 0x8f0ccc92);
        l = II(l, i1, j, k, ai[10], 15, 0xffeff47d);
        k = II(k, l, i1, j, ai[1], 21, 0x85845dd1);
        j = II(j, k, l, i1, ai[8], 6, 0x6fa87e4f);
        i1 = II(i1, j, k, l, ai[15], 10, 0xfe2ce6e0);
        l = II(l, i1, j, k, ai[6], 15, 0xa3014314);
        k = II(k, l, i1, j, ai[13], 21, 0x4e0811a1);
        j = II(j, k, l, i1, ai[4], 6, 0xf7537e82);
        i1 = II(i1, j, k, l, ai[11], 10, 0xbd3af235);
        l = II(l, i1, j, k, ai[2], 15, 0x2ad7d2bb);
        k = II(k, l, i1, j, ai[9], 21, 0xeb86d391);
        state1.state[0] += j;
        state1.state[1] += k;
        state1.state[2] += l;
        state1.state[3] += i1;
    }

    void MD5::Update(char char0)
    {
        vector<char> achar0(1,0);
        achar0[0] = char0;
        Update(achar0, 1);
    }

    void MD5::Update(State & state1, const vector<char> &achar0, int i, int j)
    {
        finalsNull = true;
        if (j - i > (int)achar0.size())
            j = achar0.size() - i;
        int k = state1.count[0] >> 3 & 0x3f;
        if ((state1.count[0] += j << 3) < j << 3)
            state1.count[1]++;
        state1.count[1] += j >> 29;
        int l = 64 - k;
        int j1;
        if(j >= l)
        {
            for(int i1 = 0; i1 < l; i1++)
                state1.buffer[i1 + k] = achar0[i1 + i];

            Transform(state1, state1.buffer, 0);
            for(j1 = l; j1 + 63 < j; j1 += 64)
                Transform(state1, achar0, j1);

            k = 0;
        } 
        else
        {
            j1 = 0;
        }
        if (j1 < j)
        {
            int k1 = j1;
            for(; j1 < j; j1++)
                state1.buffer[(k + j1) - k1] = achar0[j1 + i];
        }
    }

    void MD5::Update(const string &s)
    {
        vector<char> achar(s.size(),0);
        for (int i=0; i<(int)s.size(); i++)
            achar[i] = s[i];
        Update( achar, achar.size() );
    }

    void MD5::Update(const vector<char> &achar0)
    {
        Update(achar0, 0, achar0.size());
    }

    void MD5::Update(const vector<char> &achar0, int i)
    {
        Update(state, achar0, 0, i);
    }

    void MD5::Update(const vector<char> &achar0, int i, int j)
    {
        Update(state, achar0, i, j);
    }

    string MD5::asHex()
    {
        return asHex(Final());
    }

    string MD5::asHex(const vector<char> &achar0)
    {
        const string hex = "0123456789abcdef";

        string stringbuffer;
        for (int i = 0; i < (int)achar0.size(); i++)
        {
            stringbuffer += hex.substr((achar0[i] >> 4) & 0x0f,1);
            stringbuffer += hex.substr(achar0[i] & 0x0f,1);
        }

        return stringbuffer;
    }

    int MD5::rotate_left(int i, int j)
    {
        unsigned i1 = i;
        unsigned j1 = j;
        return i1 << j1 | i1 >> (32 - j1);
    }

    int MD5::uadd(int i, int j)
    {
        int64 l = (int64)i & 0x0ffffffffL;
        int64 l1 = (int64)j & 0x0ffffffffL;
        l += l1;
        return (int)(l & 0x0ffffffffL);
    }

    int MD5::uadd(int i, int j, int k)
    {
        return uadd(uadd(i, j), k);
    }

    int MD5::uadd(int i, int j, int k, int l)
    {
        return uadd(uadd(i, j, k), l);
    }

};
#ifdef EXTERNAL_DISTRO
};
#endif


