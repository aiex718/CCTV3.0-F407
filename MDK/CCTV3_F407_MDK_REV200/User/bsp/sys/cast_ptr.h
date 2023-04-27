#ifndef __CAST_PTR_H__
#define __CAST_PTR_H__


#define __ANY_CAST_VAR(type) (type*)&
#define __ANY_CAST_CONST(type) (const type*)&
#define __VAR_CAST_VAR(type) (type*)&(type)
#define __VAR_ARRAY_CAST_VAR(type,len) (type*)&(type[len])
#define __CONST_CAST_VAR(type) (type*)&(const type)
#define __CONST_ARRAY_CAST_VAR(type) (type*)&(const type[])

#endif
