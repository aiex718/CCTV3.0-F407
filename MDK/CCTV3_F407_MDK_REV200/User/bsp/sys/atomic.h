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

#define __U8_LDREX  __LDREXB
#define __U16_LDREX __LDREXH
#define __U32_LDREX __LDREXW
//STREX return 0 if success, 1 if fail
#define __U8_STREX   __STREXB
#define __U16_STREX  __STREXH
#define __U32_STREX  __STREXW

#define __ATOMIC_MACRO_CAT(a,b) a##b

#define __ATOMIC_GENERIC_EXPAND_INSTANCE(__func_name)   \
_GEN##__func_name(uint32_t,__U32_STREX,__U32_LDREX)     \
_GEN##__func_name(uint16_t,__U16_STREX,__U16_LDREX)     \
_GEN##__func_name(uint8_t ,__U8_STREX ,__U8_LDREX)    

#define __ATOMIC_GENERIC_EXPAND_SIZE(__func,ptr,ptr_size,...)       \
{                                                                   \
    switch (ptr_size)                                               \
    {                                                               \
        case 1:return __func##Type(uint8_t ,ptr,##__VA_ARGS__);     \
        case 2:return __func##Type(uint16_t,ptr,##__VA_ARGS__);     \
        case 4:return __func##Type(uint32_t,ptr,##__VA_ARGS__);     \
        default:return (uint32_t)-1;                                \
    }                                                               \
}

/**
 * @brief Adds two integers and replaces the first integer with the sum, 
 *        as an atomic operation.
 * @param ptr A variable pointer containing the first value to be added.
 *            The result is stored in ptr.
 * @param val The value to be added to the integer at ptr.
 * @return The new value that was stored at ptr by this operation.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_ADD_TEMPLATE(__type,__str_func,__ldr_func)                 \
__STATIC_INLINE __type Atomic_Add_##__type( __IO __type* ptr,__type val)        \
{                                                                               \
    __type snap;                                                                \
    do                                                                          \
    {                                                                           \
        snap = __ldr_func(ptr)+val;                                             \
    } while (__str_func(snap,ptr));                                             \
    __DMB();                                                                    \
    return snap;                                                                \
}
//Generate all data type functions
__ATOMIC_GENERIC_EXPAND_INSTANCE(__ATOMIC_ADD_TEMPLATE)
//Generate function for generic data type and size
#define Atomic_Add_Type(_type,ptr,val) \
    __ATOMIC_MACRO_CAT(Atomic_Add_, _type) ## (ptr,val)
__STATIC_INLINE uint32_t Atomic_Add_Size(__IO void *ptr,uint8_t ptr_size,uint32_t val) \
    __ATOMIC_GENERIC_EXPAND_SIZE(Atomic_Add_,ptr,ptr_size,val);
//Generic function decl
#define Atomic_Add(ptr,val) Atomic_Add_Size((ptr),sizeof(*(ptr)),(val))


/**
 * @brief Subtracts two integers and replaces the first integer with the result, 
 *        as an atomic operation.
 * @param ptr A variable pointer containing the first value to be subtracted.
 *            The result is stored in ptr.
 * @param val The value to subtract on the integer at ptr.
 * @return The new value that was stored at ptr by this operation.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_SUB_TEMPLATE(__type,__str_func,__ldr_func)                 \
__STATIC_INLINE __type Atomic_Sub_##__type( __IO __type* ptr,__type val)        \
{                                                                               \
    __type snap;                                                                \
    do                                                                          \
    {                                                                           \
        snap = __ldr_func(ptr)-val;                                             \
    } while (__str_func(snap,ptr));                                             \
    __DMB();                                                                    \
    return snap;                                                                \
}
__ATOMIC_GENERIC_EXPAND_INSTANCE(__ATOMIC_SUB_TEMPLATE)

#define Atomic_Sub_Type(_type,ptr,val) \
    __ATOMIC_MACRO_CAT(Atomic_Sub_, _type) ## (ptr,val)
__STATIC_INLINE uint32_t Atomic_Sub_Size(__IO void *ptr,uint8_t ptr_size,uint32_t val) \
    __ATOMIC_GENERIC_EXPAND_SIZE(Atomic_Sub_,ptr,ptr_size,val);
#define Atomic_Sub(ptr,val) Atomic_Sub_Size((ptr),sizeof(*(ptr)),(val))

/**
 * @brief Bitwise "ands" two integers and replaces the first integer with the 
 *        result, as an atomic operation.
 * @param ptr A variable pointer containing the first value to be combined. 
 *            The result is stored in ptr.
 * @param val The value to be combined with the integer at ptr.
 * @return The original value in ptr.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_AND_TEMPLATE(__type,__str_func,__ldr_func)                 \
__STATIC_INLINE __type Atomic_And_##__type(__IO __type* ptr,__type val)         \
{                                                                               \
    __type snap;                                                                \
    do                                                                          \
    {                                                                           \
        snap = __ldr_func(ptr);                                                 \
    } while (__str_func((snap&val),ptr));                                       \
    __DMB();                                                                    \
    return snap;                                                                \
}
__ATOMIC_GENERIC_EXPAND_INSTANCE(__ATOMIC_AND_TEMPLATE)

#define Atomic_And_Type(_type,ptr,val) \
    __ATOMIC_MACRO_CAT(Atomic_And_, _type) ## (ptr,val)
__STATIC_INLINE uint32_t Atomic_And_Size(__IO void *ptr,uint8_t ptr_size,uint32_t val) \
    __ATOMIC_GENERIC_EXPAND_SIZE(Atomic_And_,ptr,ptr_size,val);
#define Atomic_And(ptr,val) Atomic_And_Size((ptr),sizeof(*(ptr)),(val))

/**
 * @brief Bitwise "ors" two integers and replaces the first integer with the result,
 *        as an atomic operation.
 * @param ptr A variable containing the first value to be combined. 
 *            The result is stored in ptr.
 * @param val The value to be combined with the integer at ptr.
 * @return The original value in ptr.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_OR_TEMPLATE(__type,__str_func,__ldr_func)                  \
__STATIC_INLINE __type Atomic_Or_##__type(__IO __type* ptr,__type val)          \
{                                                                               \
    __type snap;                                                                \
    do                                                                          \
    {                                                                           \
        snap = __ldr_func(ptr);                                                 \
    } while (__str_func((snap|val),ptr));                                       \
    __DMB();                                                                    \
    return snap;                                                                \
}
__ATOMIC_GENERIC_EXPAND_INSTANCE(__ATOMIC_OR_TEMPLATE)

#define Atomic_Or_Type(_type,ptr,val) \
    __ATOMIC_MACRO_CAT(Atomic_Or_, _type) ## (ptr,val)
__STATIC_INLINE uint32_t Atomic_Or_Size(__IO void *ptr,uint8_t ptr_size,uint32_t val) \
    __ATOMIC_GENERIC_EXPAND_SIZE(Atomic_Or_,ptr,ptr_size,val);
#define Atomic_Or(ptr,val) Atomic_Or_Size((ptr),sizeof(*(ptr)),(val))

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
__ATOMIC_GENERIC_EXPAND_INSTANCE(__ATOMIC_INCREMENT_TEMPLATE)

#define Atomic_Increment_Type(_type,ptr) \
    __ATOMIC_MACRO_CAT(Atomic_Increment_, _type) ## (ptr)
__STATIC_INLINE uint32_t Atomic_Increment_Size(__IO void *ptr,uint8_t ptr_size) \
    __ATOMIC_GENERIC_EXPAND_SIZE(Atomic_Increment_,ptr,ptr_size);
#define Atomic_Increment(ptr) Atomic_Increment_Size((ptr),sizeof(*(ptr)))

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
__ATOMIC_GENERIC_EXPAND_INSTANCE(__ATOMIC_DECREMENT_TEMPLATE)

#define Atomic_Decrement_Type(_type,ptr) \
    __ATOMIC_MACRO_CAT(Atomic_Decrement_, _type) ## (ptr)
__STATIC_INLINE uint32_t Atomic_Decrement_Size(__IO void *ptr,uint8_t ptr_size) \
    __ATOMIC_GENERIC_EXPAND_SIZE(Atomic_Decrement_,ptr,ptr_size);
#define Atomic_Decrement(ptr) Atomic_Decrement_Size((ptr),sizeof(*(ptr)))

/**
 * @brief Sets a variable to a specified value as an atomic operation.
 * @param ptr The variable to set to the specified value.
 * @param val The value to which the ptr parameter is set.
 * @return The original value in ptr.
 * @note This operation never failed.
 */
#define _GEN__ATOMIC_EXCHANGE_TEMPLATE(__type,__str_func,__ldr_func)                \
__STATIC_INLINE __type Atomic_Exchange_##__type(__IO __type* ptr,__type val)        \
{                                                                                   \
    __type snap;                                                                    \
    do                                                                              \
    {                                                                               \
        snap = __ldr_func(ptr);                                                     \
    } while (__str_func(val,ptr));                                                  \
    __DMB();                                                                        \
    return snap;                                                                    \
}
__ATOMIC_GENERIC_EXPAND_INSTANCE(__ATOMIC_EXCHANGE_TEMPLATE)

#define Atomic_Exchange_Type(_type,ptr,val) \
    __ATOMIC_MACRO_CAT(Atomic_Exchange_, _type) ## (ptr,val)
__STATIC_INLINE uint32_t Atomic_Exchange_Size(__IO void *ptr,uint8_t ptr_size,uint32_t val) \
    __ATOMIC_GENERIC_EXPAND_SIZE(Atomic_Exchange_,ptr,ptr_size,val);
#define Atomic_Exchange(ptr,val) Atomic_Exchange_Size(ptr,sizeof(*(ptr)),val)

/**
 * @brief Compares two integers for equality and, if they are equal, 
 *        replaces the ptr with val.
 * @param ptr The destination, whose value is compared with expect and
 *            possibly replaced.
 * @param val The value that replaces the destination value if the comparison 
 *            results in equality.
 * @param expect The value that is compared to the value at ptr.
 * @return The original value in ptr.
 */
#define _GEN__ATOMIC_CMPXCHG_TEMPLATE(__type,__str_func,__ldr_func)                                 \
__STATIC_INLINE __type Atomic_Cmpxchg_##__type(__IO __type* ptr,__type val,__type expect)           \
{                                                                                                   \
    __type snap;                                                                                    \
    do                                                                                              \
    {                                                                                               \
        snap = __ldr_func(ptr);                                                                     \
        if(snap != expect)                                                                          \
            break;                                                                                  \
    } while (__str_func(val,ptr));                                                                  \
    __DMB();                                                                                        \
    return snap;                                                                                    \
}
__ATOMIC_GENERIC_EXPAND_INSTANCE(__ATOMIC_CMPXCHG_TEMPLATE)

#define Atomic_Cmpxchg_Type(_type,ptr,val,expect) \
    __ATOMIC_MACRO_CAT(Atomic_Cmpxchg_, _type) ## (ptr,val,expect)
__STATIC_INLINE uint32_t Atomic_Cmpxchg_Size(__IO void *ptr,uint8_t ptr_size,uint32_t val,uint32_t expect)\
    __ATOMIC_GENERIC_EXPAND_SIZE(Atomic_Cmpxchg_,ptr,ptr_size,val,expect);
#define Atomic_Cmpxchg(ptr,val,expect)  Atomic_Cmpxchg_Size(ptr,sizeof(*(ptr)),val,expect)

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
    __DSB();
}

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
