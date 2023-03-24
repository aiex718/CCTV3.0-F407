#ifndef __ATOMIC_CM4_H__
#define __ATOMIC_CM4_H__

//atomic wrapper for cortex-m4

#pragma diag_suppress 3731

#ifdef __cplusplus
extern "C" {
#endif

//Api Reference
//https://learn.microsoft.com/zh-tw/dotnet/api/system.threading.interlocked?view=net-6.0

#include "bsp/platform/platform_defs.h"

#define __LOCK_VALUE 0x01

#define __U8_LDREX  __LDREXB
#define __U16_LDREX __LDREXH
#define __U32_LDREX __LDREXW

#define __U8_STREX   __STREXB
#define __U16_STREX  __STREXH
#define __U32_STREX  __STREXW

#define __ATOMIC_MACRO_CAT(a,b) a##b


/**
 * @brief Adds two integers and replaces the first integer with the sum, 
 *        as an atomic operation.
 * @param ptr A variable pointer containing the first value to be added.
 *            The result is stored in ptr.
 * @param val The value to be added to the integer at ptr.
 * @return The new value that was stored at ptr by this operation.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_ADD_TEMPLATE(__type,__str_func,__ldr_func)            \
__STATIC_INLINE __type Atomic_Add_##__type( __IO __type* ptr,__type val)   \
{                                                                               \
    __type snap;                                                                \
    do                                                                          \
    {                                                                           \
        snap = __ldr_func(ptr)+val;                                             \
    } while (__str_func(snap,ptr));                                             \
    __DMB();                                                                    \
    return snap;                                                                \
}


/**
 * @brief Subtracts two integers and replaces the first integer with the result, 
 *        as an atomic operation.
 * @param ptr A variable pointer containing the first value to be subtracted.
 *            The result is stored in ptr.
 * @param val The value to subtract on the integer at ptr.
 * @return The new value that was stored at ptr by this operation.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_SUB_TEMPLATE(__type,__str_func,__ldr_func)            \
__STATIC_INLINE __type Atomic_Sub_##__type( __IO __type* ptr,__type val)   \
{                                                                               \
    __type snap;                                                                \
    do                                                                          \
    {                                                                           \
        snap = __ldr_func(ptr)-val;                                             \
    } while (__str_func(snap,ptr));                                             \
    __DMB();                                                                    \
    return snap;                                                                \
}

/**
 * @brief Bitwise "ands" two integers and replaces the first integer with the 
 *        result, as an atomic operation.
 * @param ptr A variable pointer containing the first value to be combined. 
 *            The result is stored in ptr.
 * @param val The value to be combined with the integer at ptr.
 * @return The original value in ptr.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_AND_TEMPLATE(__type,__str_func,__ldr_func)            \
__STATIC_INLINE __type Atomic_And_##__type(__IO __type* ptr,__type val)    \
{                                                                               \
    __type snap;                                                                \
    do                                                                          \
    {                                                                           \
        snap = __ldr_func(ptr);                                                 \
    } while (__str_func((snap&val),ptr));                                       \
    __DMB();                                                                    \
    return snap;                                                                \
}

/**
 * @brief Bitwise "ors" two integers and replaces the first integer with the result,
 *        as an atomic operation.
 * @param ptr A variable containing the first value to be combined. 
 *            The result is stored in ptr.
 * @param val The value to be combined with the integer at ptr.
 * @return The original value in ptr.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_OR_TEMPLATE(__type,__str_func,__ldr_func)             \
__STATIC_INLINE __type Atomic_Or_##__type(__IO __type* ptr,__type val)     \
{                                                                               \
    __type snap;                                                                \
    do                                                                          \
    {                                                                           \
        snap = __ldr_func(ptr);                                                 \
    } while (__str_func((snap|val),ptr));                                       \
    __DMB();                                                                    \
    return snap;                                                                \
}

/**
 * @brief Sets a variable to a specified value as an atomic operation.
 * @param ptr The variable to set to the specified value.
 * @param val The value to which the ptr parameter is set.
 * @return The original value in ptr.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_EXCHANGE_TEMPLATE(__type,__str_func,__ldr_func)           \
__STATIC_INLINE __type Atomic_Exchange_##__type(__IO __type* ptr,__type val)   \
{                                                                                   \
    __type snap;                                                                    \
    do                                                                              \
    {                                                                               \
        snap = __ldr_func(ptr);                                                     \
    } while (__str_func(val,ptr));                                                  \
    __DMB();                                                                        \
    return snap;                                                                    \
}

/**
 * @brief Compares two integers for equality and, if they are equal, 
 *        replaces the ptr with val.
 * @param ptr The destination, whose value is compared with comparand and
 *            possibly replaced.
 * @param val The value that replaces the destination value if the comparison 
 *            results in equality.
 * @param comparand The value that is compared to the value at ptr.
 * @return The original value in ptr.
 */
#define _GEN__ATOMIC_CMPXCHG_TEMPLATE(__type,__str_func,__ldr_func)                            \
__STATIC_INLINE __type Atomic_Cmpxchg_##__type(__IO __type* ptr,__type val,__type comparand)   \
{                                                                                                   \
    __type snap;                                                                                    \
    do                                                                                              \
    {                                                                                               \
        snap = __ldr_func(ptr);                                                                     \
        if(snap != comparand)                                                                       \
            break;                                                                                  \
    } while (__str_func(val,ptr));                                                                  \
    __DMB();                                                                                        \
    return snap;                                                                                    \
}
#define Atomic_Cmpxchg(type,ptr,val,comparand) __ATOMIC_MACRO_CAT(Atomic_Cmpxchg_, type) ## (ptr,val,comparand)

/**
 * @brief Block process until successfully lock ptr.
 * @param ptr The pointer to be locked.
 * @return None.
 * @note This operation exclusively set the value at ptr to __LOCK_VALUE
 */
#define _GEN__ATOMIC_LOCK_TEMPLATE(__type,__str_func,__ldr_func)   \
__STATIC_INLINE void Atomic_Lock_##__type(__IO __type* ptr)        \
{                                                                       \
    do                                                                  \
    {                                                                   \
        while(__ldr_func(ptr));                                         \
    } while(__str_func(__LOCK_VALUE,ptr));                              \
    __DMB();                                                            \
}

/**
 * @brief Try to lock ptr.
 * @param ptr The pointer to lock.
 * @return 0 if lock success, 1 if failed.
 * @note This operation try exclusively set the value at ptr to __LOCK_VALUE
 */
#define _GEN__ATOMIC_TRYLOCK_TEMPLATE(__type,__str_func,__ldr_func)    \
__STATIC_INLINE uint8_t Atomic_TryLock_##__type(__IO __type* ptr)      \
{                                                                           \
    uint8_t ret;                                                            \
    if(__ldr_func(ptr)) ret=1; /*already locked*/                           \
    else ret = __str_func(__LOCK_VALUE,ptr);                                \
    __DMB();                                                                \
    return ret;                                                             \
}

/**
 * @brief Unlock ptr.
 * @param ptr The pointer to be unlocked.
 * @return None.
 * @note This operation set the value at ptr to 0
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_UNLOCK_TEMPLATE(__type,__str_func,__ldr_func) \
__STATIC_INLINE void Atomic_Unlock_##__type(__IO __type* ptr)      \
{                                                                       \
    *ptr=0;                                                             \
    __DMB();                                                            \
}

/**
 * @brief Increments a specified variable by 1 and stores the result,
 *        as an atomic operation.
 * @param ptr The variable whose value is to be incremented.
 * @return The incremented value.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_INCREMENT_TEMPLATE(__type,__str_func,__ldr_func)   \
__STATIC_INLINE __type Atomic_Increment_##__type(__IO __type* ptr)      \
{                                                                       \
    return Atomic_Add_##__type(ptr,1);                                  \
}
#define Atomic_Increment(type,ptr) __ATOMIC_MACRO_CAT(Atomic_Increment_, type) ## (ptr)


/**
 * @brief Decrements a specified variable by 1 and stores the result,
 *        as an atomic operation.
 * @param ptr The variable whose value is to be decremented.
 * @return The decremented value.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_DECREMENT_TEMPLATE(__type,__str_func,__ldr_func)   \
__STATIC_INLINE __type Atomic_Decrement_##__type(__IO __type* ptr)      \
{                                                                       \
    return Atomic_Sub_##__type(ptr,1);                                  \
}

/**
 * @brief Disable interrupt.
 * @return None.
 * @note This operation never failed.
 */
__STATIC_INLINE void Atomic_EnterCritical(void)
{
    __disable_irq();
    __DSB();
    __ISB();
}

/**
 * @brief Enable interrupt.
 * @return None.
 * @note This operation never failed.
 */
__STATIC_INLINE void Atomic_ExitCritical(void)
{
    __enable_irq();
}

#define __GENERIC__EXPAND_(__func_name)             \
_GEN##__func_name(uint32_t,__U32_STREX,__U32_LDREX) \
_GEN##__func_name(uint16_t,__U16_STREX,__U16_LDREX) \
_GEN##__func_name(uint8_t ,__U8_STREX ,__U8_LDREX)    

__GENERIC__EXPAND_(__ATOMIC_ADD_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_SUB_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_AND_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_OR_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_EXCHANGE_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_CMPXCHG_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_LOCK_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_TRYLOCK_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_UNLOCK_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_INCREMENT_TEMPLATE)
__GENERIC__EXPAND_(__ATOMIC_DECREMENT_TEMPLATE)

//undef
#undef __GENERIC__EXPAND_

#undef __U8_LDREX  
#undef __U16_LDREX 
#undef __U32_LDREX 

#undef __U8_STREX  
#undef __U16_STREX 
#undef __U32_STREX 


#ifdef __cplusplus
}
#endif

#endif // __ATOMIC_CM4_H__
