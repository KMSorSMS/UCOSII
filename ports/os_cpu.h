#ifndef  OS_CPU_H
#define  OS_CPU_H
typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned int   INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   int   INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */
typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */

#define  OS_CRITICAL_METHOD   3u
#if OS_CRITICAL_METHOD == 3u                     /* See OS_CPU_A.ASM                                   */
OS_CPU_SR  OS_CPU_SR_Save         (OS_CPU_SR  new_basepri);
void       OS_CPU_SR_Restore      (OS_CPU_SR  cpu_sr);
#endif
#if OS_CRITICAL_METHOD == 3u
                                                 /* Save current BASEPRI priority lvl for exception... */
                                                 /* .. and set BASEPRI to CPU_CFG_KA_IPL_BOUNDARY      */
#define  OS_ENTER_CRITICAL()  do { cpu_sr = OS_CPU_SR_Save(CPU_CFG_KA_IPL_BOUNDARY << (8u - CPU_CFG_NVIC_PRIO_BITS));} while (0)
                                                 /* Restore CPU BASEPRI priority level.                */
#define  OS_EXIT_CRITICAL()   do { OS_CPU_SR_Restore(cpu_sr);} while (0)
#endif

#endif
