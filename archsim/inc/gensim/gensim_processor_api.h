/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#include <stdint.h>

// allow a prefix to allow use of macros out of the context of the cpu object
#ifndef PREFIX
#define PREFIX
#endif

#define registers() (reg_state)

#ifndef __ROTATE_FUNS
#define __ROTATE_FUNS
static inline uint32_t __ror32 (uint32_t x, uint32_t n)
{
	return (x<<n) | (x>>(32-n));
}
static inline uint32_t __ror64 (uint64_t x, uint32_t n)
{
	return (x<<n) | (x>>(64-n));
}
#endif

#ifndef __BITMNP_FUNS
#define __BITMNP_FUNS

// I don't like these double underscored functions very much at all. -H

#define __SEXT64(v, from) (((int64_t)(((uint64_t)(v)) << (64 - (from)))) >> (64 - (from)))

#define __ONES(a) (~0ULL >> (64 - (a)))
#define __CLZ32(a) __builtin_clz((a))

static inline uint64_t __REPLICATE(uint64_t __v, unsigned int __e)
{
	while (__e < 64) {
		__v |= __v << __e;
		__e *= 2;
	}

	return __v;
}

#endif

#ifndef __BITCAST_FUNS
#define __BITCAST_FUNS
static inline float bitcast_u32_float(uint32_t x)
{
	union {
		float f;
		uint32_t u;
	};
	u = x;
	return f;
}
static inline uint32_t bitcast_float_u32(float x)
{
	union {
		float f;
		uint32_t u;
	};
	f = x;
	return u;
}
static inline double bitcast_u64_double(uint64_t x)
{
	union {
		double f;
		uint64_t u;
	};
	u = x;
	return f;
}
static inline uint64_t bitcast_double_u64(double x)
{
	union {
		double f;
		uint64_t u;
	};
	f = x;
	return u;
}
#endif

#define float_is_snan(x) ((bitcast_float_u32(x) & 0x7fc00000) == 0x7f800000)
#define float_is_qnan(x) ((bitcast_float_u32(x) & 0x7fc00000) == 0x7fc00000)

#define double_is_snan(x) ((bitcast_double_u64(x) & 0x7ffc000000000000ULL) == 0x7ff8000000000000ULL)
#define double_is_qnan(x) ((bitcast_double_u64(x) & 0x7ffc000000000000ULL) == 0x7ffc000000000000ULL)

#define double_sqrt(x) (sqrt((double)x))
#define float_sqrt(x) (sqrt((float)x))

#define double_abs(x) (fabs((double)x))
#define float_abs(x) (fabs((float)x))

#ifndef __GENC_FUNS
#define __GENC_FUNS

extern "C" {

	uint32_t genc_bswap32(uint32_t);
	uint64_t genc_bswap64(uint64_t);

	uint16_t genc_adc8_flags(uint8_t lhs, uint8_t rhs, uint8_t carry_in);
	uint16_t genc_adc16_flags(uint16_t lhs, uint16_t rhs, uint8_t carry_in);
	uint16_t genc_adc_flags(uint32_t lhs, uint32_t rhs, uint8_t carry_in);
	uint16_t genc_adc64_flags(uint64_t lhs, uint64_t rhs, uint8_t carry_in);
	uint16_t genc_sbc_flags(uint32_t lhs, uint32_t rhs, uint8_t carry_in);
	uint16_t genc_sbc8_flags(uint8_t lhs, uint8_t rhs, uint8_t carry_in);
	uint16_t genc_sbc16_flags(uint16_t lhs, uint16_t rhs, uint8_t carry_in);
	uint16_t genc_sbc64_flags(uint64_t lhs, uint64_t rhs, uint8_t carry_in);

	uint32_t genc_adc(uint32_t lhs, uint32_t rhs, uint8_t carry_in);
	uint64_t genc_adc64(uint64_t lhs, uint64_t rhs, uint8_t carry_in);
	uint32_t genc_sbc(uint32_t lhs, uint32_t rhs, uint8_t carry_in);
	uint64_t genc_sbc64(uint64_t lhs, uint64_t rhs, uint8_t carry_in);

}

#endif
