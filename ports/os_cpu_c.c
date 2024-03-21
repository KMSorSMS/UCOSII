#include <ucos_ii.h>

INT32U  OS_KA_BASEPRI_Boundary;                                 /* Base Priority boundary.                              */


void OSTaskSwHook(void){
    // Do nothing
}

void OSInitHookBegin(void){
    OS_KA_BASEPRI_Boundary = (INT32U)(CPU_CFG_KA_IPL_BOUNDARY << (8u - CPU_CFG_NVIC_PRIO_BITS));
}