#include "fsr.h"
#include "stm32u5xx_nucleo.h" //temp code until fsr ready

void fsr_init(void){

}

bool fsr_detect_instability(void){
    return (BSP_PB_GetState(BUTTON_USER) == BUTTON_PRESSED); //temp code until fsr ready
}