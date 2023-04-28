#ifndef __TRYCATCH_H__
#define __TRYCATCH_H__

#define try do{  
#define throw(X) goto __EXCEPTION_ROUTE_##X
#define throwif(C,X) if(C) goto __EXCEPTION_ROUTE_##X
#define catch(X) break; \
__EXCEPTION_ROUTE_##X: 
#define finally }while(0);

/*  Usage

    try
    {
        throwif({cond},ERR_CODE_1);
        
        if({cond})
            throw(ERR_CODE_2);
    }
    catch(ERR_CODE_1)
        printf("CATCH(1)\n");
        ..
        ..
        ..
    catch(ERR_CODE_2)
        printf("CATCH(2)\n");
    finally
        printf("end\n");
}
*/

#endif // __TRYCATCH_H__
