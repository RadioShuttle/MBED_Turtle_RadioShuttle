/*
 * $Id: $
 * This is an unpublished work copyright (c) 2019 HELIOS Software GmbH
 * 30827 Garbsen, Germany
 */
#include <mbed.h>
#include "arch.h"

// --------------------------------------------------------------------------------------------------------------------
#ifndef TOOLCHAIN_GCC

_extern_c size_t strnlen(const char *s, size_t maxlen) {
    const char *endp = static_cast<const char *>(memchr(s, 0, maxlen));
    if (endp == NULL) {
        return maxlen;
    } else {
        return endp - s;
    }
}

_extern_c char *strdup(const char *s) {
    size_t sz = strlen(s) + 1;
    char *news = static_cast<char *>(malloc(sz));
    if (news) {
        memcpy(news, s, sz);
    }
    return news;
}

// _extern_c char *stpcpy(char *dest, const char *src) {
//     size_t l = strlen(src);
//     memcpy(dest, src, l+1);
//     return dest + l;
// }

#endif

// --------------------------------------------------------------------------------------------------------------------
#ifdef TARGET_STM32L0
#ifdef TOOLCHAIN_GCC
_extern_c unsigned int __atomic_fetch_or_4(volatile void *mem, unsigned int val, int model) {
    volatile unsigned int *ptr = static_cast<volatile unsigned int *>(mem);
    core_util_critical_section_enter();
    unsigned int tmp = *ptr;
    *ptr = tmp | val;
    core_util_critical_section_exit();
    return tmp;
}

_extern_c unsigned int __atomic_exchange_4(volatile void *mem, unsigned int val, int model) {
    volatile unsigned int *ptr = static_cast<volatile unsigned int *>(mem);
    core_util_critical_section_enter();
    unsigned int tmp = *ptr;
    *ptr = val;
    core_util_critical_section_exit();
    return tmp;
}

#else
_extern_c bool __user_cmpxchg_1(unsigned char *ptr, unsigned char oldp, unsigned char newp)
{
    return !core_util_atomic_cas_u8(ptr, &oldp, newp);
//     core_util_critical_section_enter();
//     bool r = (*ptr == oldp);
//     if (r) {
//         *ptr = newp;
//     }
//     core_util_critical_section_exit();
//     return !r;
}

_extern_c bool __user_cmpxchg_4(unsigned int *ptr, unsigned int oldp, unsigned int newp)
{
    return !core_util_atomic_cas_u32(ptr, &oldp, newp);
//     core_util_critical_section_enter();
//     bool r = (*ptr == oldp);
//     if (r) {
//         *ptr = newp;
//     }
//     core_util_critical_section_exit();
//     return !r;
}
#endif
#endif
