   .global  OS_CPU_SR_Save
   .global  OS_CPU_SR_Restore

   .text
   .align 2
   .thumb
   .syntax unified

@                     OS_ENTER_CRITICAL();             /* cpu_sr = OS_CPU_SR_Save(new_basepri);    */
@                     OS_EXIT_CRITICAL();              /* OS_CPU_RestoreSR(cpu_sr);                */


.thumb_func
OS_CPU_SR_Save:
    CPSID   I                                   @ Cortex-M7 errata notice. See Note #2
    PUSH   {R1}
    MRS     R1, BASEPRI
    MSR     BASEPRI, R0
    DSB
    ISB
    MOV     R0, R1
    POP    {R1}
    CPSIE   I
    BX      LR

.thumb_func
OS_CPU_SR_Restore:
    CPSID   I                                   @ Cortex-M7 errata notice. See Note #2
    MSR     BASEPRI, R0
    DSB
    ISB
    CPSIE   I
    BX      LR
