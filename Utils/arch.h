/*
 * $Id: config.h,v 1.5 2017/02/23 14:31:38 grimrath Exp $
 * This is an unpublished work copyright (c) 2019 HELIOS Software GmbH
 * 30827 Garbsen, Germany
 */
#ifndef __ARCH_H__
#define __ARCH_H__

#ifdef __cplusplus
#define _extern_c extern "C"
#else
#define _extern_c
#endif

// --------------------------------------------------------------------------------------------------------------------
// Definitions to adapt between POSIX and MBED
//
#ifdef __MBED__

#include <mbed_assert.h>
#include <mbed_debug.h>
#include <cstdint>
#include <cstdlib>

#ifdef TARGET_DEBUG
#define DEBUG 1
#define STATIC_ASSERT	MBED_STATIC_ASSERT
#define ASSERT		MBED_ASSERT
#else
#define ASSERT(x)	((void)0)
#endif

#define STATIC_ASSERT	MBED_STATIC_ASSERT

#ifndef TOOLCHAIN_GCC
#ifdef __cplusplus
using std::size_t;
using std::va_list;
using std::abort;
#endif

_extern_c size_t strnlen(const char *s, size_t maxlen);
_extern_c char *strdup(const char *s);
_extern_c char *stpcpy(char *dest, const char *src);

#endif

struct iovec {
    void *  iov_base;
    size_t  iov_len;
};

//static inline unsigned read_systicker_us(void) {
//	extern uint32_t us_ticker_read(void);	// I do not want to include us_ticker_api.h here
//
//	return us_ticker_read();
//}

#define FlashFileSysMount	"Flash"
// #define ESPFileSysMount     "ESP"
#define PseudoFileSysMount	"pseudo"

#else // __MBED__

#include <assert.h>
#include <time.h>

#ifdef __cplusplus
#define STATIC_ASSERT(condition, msg) ((void)sizeof(char[1 - 2*!(condition)]))
#else
#define STATIC_ASSERT(expr, msg)	_Static_assert(expr, msg)  // C11 feature
#endif
#define ASSERT				assert

//static inline unsigned read_systicker_us(void) {
//	struct timespec ts;
//	clock_gettime(CLOCK_MONOTONIC, &ts);
//	return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
//}

#endif // __MBED__


// --------------------------------------------------------------------------------------------------------------------
// synchronize memory contents with external peripherals and interrupt handlers
//
// __ATOMIC_RELAXED should be ok since we are only dealing with irq handlers on exactly one CPU/MCU
//
// __atomic_load_n is not available in the online IDE (yet)
//
#if defined(__ATOMIC_RELAXED)

#define help_atomic_load_relaxed(ptr) __atomic_load_n((ptr), __ATOMIC_RELAXED)

#define help_atomic_store_relaxed(ptr, val) __atomic_store_n((ptr), (val), __ATOMIC_RELAXED)

#define help_atomic_readclr_relaxed(ptr) __atomic_exchange_n((ptr), 0, __ATOMIC_RELAXED)

#define help_atomic_or_relaxed(ptr, val) __atomic_fetch_or((ptr), (val), __ATOMIC_RELAXED)

#ifdef __cplusplus
template<typename T> inline bool help_atomic_compare_and_swap(T *ptr, T checkval, T newval) {
    return __atomic_compare_exchange_n(ptr, &checkval, newval, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}
#else
#define help_atomic_compare_and_swap(ptr, checkval, newval) __sync_bool_compare_and_swap((ptr), (checkval), (newval))
#endif

#define sync_memory(mem) do { \
	asm volatile("" : "=m" (mem)); \
	__atomic_thread_fence(__ATOMIC_SEQ_CST); \
} while (0)

#define irq_barrier() __atomic_signal_fence(__ATOMIC_SEQ_CST)

#define sync_memory_all() do { \
	asm volatile("" : : : "memory"); \
	__atomic_thread_fence(__ATOMIC_SEQ_CST); \
} while (0)

#else // defined(__ATOMIC_RELAXED)

#define help_atomic_load_relaxed(ptr) (*(ptr))

#define help_atomic_store_relaxed(ptr, val) ((void)(*(ptr) = (val)))

#define help_atomic_readclr_relaxed(ptr) __sync_fetch_and_and((ptr), 0)

#define help_atomic_or_relaxed(ptr, val) __sync_fetch_and_or((ptr), (val))

#define help_atomic_compare_and_swap(ptr, checkval, newval) __sync_bool_compare_and_swap((ptr), (checkval), (newval))

#define sync_memory(mem) __sync_synchronize()

#define sync_memory_all() __sync_synchronize()

#define irq_barrier() __sync_synchronize()

#endif


#define help_atomic_init(ptr, initval) do { *(ptr) = (initval); } while (0)

// --------------------------------------------------------------------------------------------------------------------
// other

#define ispowerof2(x) (((x) & ((x) - 1)) == 0)

static inline uint32_t alignup32(uint32_t size, uint32_t next) {
    uint32_t next1 = next - 1;
    ASSERT((next & next1) == 0);	// 2^n check
    return (size + next1) & ~next1;
}


// --------------------------------------------------------------------------------------------------------------------
// typesafe macros to get the number of compile-time known array elements.
//
#ifdef __cplusplus

template< typename T, std::size_t N > char(&COUNTOF_REQUIRES_ARRAY_ARGUMENT(T(&)[N]))[N];
#define ARRAYLEN(x) sizeof(COUNTOF_REQUIRES_ARRAY_ARGUMENT(x))

#else

// MBED OS online compiler does not support unnamed and zero sized bitfields as GCC does
// #define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int:-!!(e); }))	// used by Linux kernel
#define __must_be_zero(e) (sizeof(struct { char dummy:(1 - 2*!!(e)); }) - 1)

// __builtin_types_compatible_p: gcc extension, but understood by Intel, clang and ARM compilers too
// __builtin_types_compatible_p is not available in C++
#define __must_be_array(arr) __must_be_zero(__builtin_types_compatible_p(typeof(arr), typeof(&(arr)[0])))

#define ARRAYLEN(arr) (sizeof(arr) / sizeof(0[arr])) + __must_be_array(arr)

#endif


// --------------------------------------------------------------------------------------------------------------------
// quick int32 -> int conversion mainly for printf. Shorter than static_cast<int> and works with C too.
//
static inline int itoi(int val) { return val; }
static inline long long lltoll(long long val) { return val; }


// --------------------------------------------------------------------------------------------------------------------
// Byte order
//
#ifdef __MBED__

#if BYTE_ORDER == LITTLE_ENDIAN

static inline uint16_t htole16(uint16_t x) { return x; }
static inline uint16_t le16toh(uint16_t x) { return x; }
static inline uint32_t htole32(uint32_t x) { return x; }
static inline uint32_t le32toh(uint32_t x) { return x; }

#else

// unused big endian variants
// static inline uint16_t htobe16(uint16_t x) { return __REV16(x); }
// static inline uint16_t be16toh(uint16_t x) { return __REV16(x); }
// static inline uint32_t htobe32(uint32_t x) { return __REV(x); }
// static inline uint32_t be32toh(uint32_t x) { return __REV(x); }

#endif

#elif defined(__linux__)

#include <endian.h>

#elif defined(__APPLE__)

#include <libkern/OSByteOrder.h>
#define htole16 OSSwapHostToLittleInt16
#define le16toh OSSwapHostToLittleInt16
#define htole32 OSSwapHostToLittleInt32
#define le32toh OSSwapHostToLittleInt32

#endif


// --------------------------------------------------------------------------------------------------------------------
// memory debugging
//
#ifdef DEBUG

#ifdef USE_VALGRIND

#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>
#include <string.h>

#define CHECKDEFINED(obj) VALGRIND_CHECK_MEM_IS_DEFINED(&(obj), sizeof(obj))

#endif // ! valgrind

static inline void POISONMEM(void *ptr, size_t sz) {
	memset(ptr, 0x55, sz);
#ifdef USE_VALGRIND
	VALGRIND_MAKE_MEM_UNDEFINED(ptr, sz);
#endif
}

#define POISON(obj) POISONMEM(&(obj), sizeof(obj))

#else // ! DEBUG

#ifdef USE_VALGRIND
#error valgrind features only useable in debug builds
#endif

static inline void POISONMEM(void *ptr, size_t sz) { (void)ptr; (void)sz; }

#define POISON(obj)	((void)0)

#endif // DEBUG

// --------------------------------------------------------------------------------------------------------------------
// Macros to live bookmark code
//
#ifdef DEBUG

_extern_c void dbg_fail_handler(const char *file, int line, const char *func, const char *msg) __attribute__((noreturn));

#define TODO(...)	dbg_fail_handler(__FILE__, __LINE__, __func__, "TODO" __VA_ARGS__)
#define UNTESTED()	dbg_fail_handler(__FILE__, __LINE__, __func__, "UNTESTED")
#define UNREACHABLE()	dbg_fail_handler(__FILE__, __LINE__, __func__, "UNREACHABLE")
#if defined(__x86_64__) || defined(__i386__)
#define BREAKPOINT()	asm("int $3")
#elif defined(__arm__)
#define BREAKPOINT()	__BKPT(0)
#else
#error no compile time breakpoints supplied for this architecture - add them if needed
#endif

#else // ! debug

#define TODO(...)	abort()
#define UNTESTED()	((void)0)
#ifdef __MBED__
#	define UNREACHABLE()	MBED_UNREACHABLE
#else
#	define UNREACHABLE() __builtin_unreachable()
#endif
// no BREAKPOINT() - these must be removed in release builds

#endif // debug



// --------------------------------------------------------------------------------------------------------------------
// Tracing
//
#ifdef DEBUG

// Do not call these directly, use DTRC
_extern_c void trc_dbg(const char *file, int line, const char *func, const char *fmt, ...) __attribute__((format(printf,4,5)));
_extern_c void trc_vdbg(const char *file, int line, const char *func, const char *fmt, va_list ap);

#define DTRC(_fmt, ...) trc_dbg(__FILE__, __LINE__, __func__, (_fmt), ## __VA_ARGS__)


#else // DEBUG

#define DTRC(_fmt, ...)	((void)0)

#endif

// Do not call these directly, use TRC_* macros
_extern_c void trc_printf(const char *fmt, ...) __attribute__((format(printf,1,2)));
_extern_c void trc_vprintf(const char *fmt, va_list ap);

// These exists even in release builds
#define TRC_INF(_fmt, ...) trc_printf((_fmt), ## __VA_ARGS__)
#define TRC_WRN TRC_INF
#define TRC_ERR TRC_INF
#define TRC_VERR(fmt, ap) trc_vprintf((fmt), (ap))

#endif // __ARCH_H__
