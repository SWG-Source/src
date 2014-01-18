#if defined (linux) && ((__GNUC__ == 3 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL == 3) || (__GNUC__ == 2 && __GNUC_MINOR__ > 96))
#include <bits/char_traits.h>
#include "Unicode.h"

#ifdef EXTERNAL_DISTRO
using namespace NAMESPACE::Plat_Unicode;
#else
using namespace Plat_Unicode;
#endif

namespace std
{

void char_traits<unicode_char_t>::assign(unicode_char_t &__c1, const unicode_char_t &__c2)
{
  __c1 = __c2;
}

bool char_traits<unicode_char_t>::eq(const unicode_char_t &__c1, const unicode_char_t &__c2)
{
   return __c1 == __c2;
}

bool char_traits<unicode_char_t>::lt(const unicode_char_t &__c1, const unicode_char_t &__c2)
{
  return __c1 < __c2;
}

unicode_char_t *char_traits<unicode_char_t>::copy(unicode_char_t *__s1, const unicode_char_t *__s2, size_t __n)
{
   return(__n == 0 ? __s1 : (unicode_char_t*)memcpy(__s1, __s2, __n * sizeof(unicode_char_t)));
}

unicode_char_t *char_traits<unicode_char_t>::move(unicode_char_t *__s1, const unicode_char_t* __s2, size_t _Sz)
{
  return (_Sz == 0 ? __s1 : (unicode_char_t*)memmove(__s1, __s2, _Sz * sizeof(unicode_char_t)));
}

const unicode_char_t *char_traits<unicode_char_t>::find(const unicode_char_t* __s, size_t __n, const unicode_char_t & __c)
{
  for( ; __n > 0; ++__s, --__n)
    if(eq(*__s, __c))
      return __s;
  return 0;
}

size_t char_traits<unicode_char_t>::length(const unicode_char_t *__s)
{
  size_t __i;
  for(__i = 0; !eq(__s[__i], 0); ++__i);
  return __i;
}

unicode_char_t *char_traits<unicode_char_t>::assign(unicode_char_t *__s, size_t __n, unicode_char_t __c)
{
  for(size_t __i = 0; __i < __n; ++__i)
    __s[__i] = __c;
  return __s;
}

int char_traits<unicode_char_t>::compare(const unicode_char_t *__s1, const unicode_char_t *__s2, size_t __n)
{
        for(size_t __i = 0; __i < __n; ++__i)
                if(!eq(__s1[__i], __s2[__i]))
                        return __s1[__i] < __s2[__i] ? -1 : 1;
        return 0;
}

}

#endif
