#ifndef _FOREACH_H_
#define _FOREACH_H_

#define FOREACH_IMPL(TYPE,ITER,OBJ,ITER_TYPE,BEGIN,END) \
    for ( TYPE::ITER_TYPE ITER = (OBJ).BEGIN(); ITER != (OBJ).END(); ++ITER )

#define FOREACH(TYPE,ITER,OBJ) \
    FOREACH_IMPL(TYPE,ITER,OBJ,iterator,begin,end)

#define FOREACH_CONST(TYPE,ITER,OBJ) \
    FOREACH_IMPL(TYPE,ITER,OBJ,const_iterator,begin,end)

#endif //_FOREACH_H_
