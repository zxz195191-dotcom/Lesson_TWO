#include "ti_msp_dl_config.h"


#define UART_0_INST UART0
#define UART_0_INST_IRQHandler UART0_IRQHandler
#define UART_0_INST_INT_IRQN UART0_INT_IRQn

// SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
// {
//     DL_GPIO_reset(GPIOA);看不懂怎么实现的 

// void LED_Init(){和之前的gpio初始化也不一样
//     DL_GPIO_initDigitalOutput(IOMUX_PINCM31);
//     DL_GPIO_clearPins(GPIOB,DL_GPIO_PIN_14);
//     DL_GPIO_enableOutput(GPIOB,DL_GPIO_PIN_14);
// }

// __STATIC_INLINE void DL_GPIO_reset(GPIO_Regs* gpio)
// {
//     gpio->GPRCM.RSTCTL =
//         (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR |
//             GPIO_RSTCTL_RESETASSERT_ASSERT);
// }


//     DL_GPIO_reset(GPIOB);
//     DL_UART_Main_reset(UART_0_INST);

//     DL_GPIO_enablePower(GPIOA);
//     DL_GPIO_enablePower(GPIOB);
//     DL_UART_Main_enablePower(UART_0_INST);
//     delay_cycles(POWER_STARTUP_DELAY);
// }

// #define DL_UART_Main_reset DL_UART_reset

void UART_Init(){
    DL_GPIO_reset(GPIOA);
    DL_UART_Main_init(UART0_INT_IRQn);
    
    DL_GPIO_enablePower(GPIOA);
    DL_UART_Main_enablePower(UART0_INT_IRQn);
    
    DL_GPIO_initDigitalOutput(IOMUX_PINCM10);
    DL_GPIO_initDigitalInput(IOMUX_PINCM11);

    DL_GPIO_clearPins(GPIOA , (DL_GPIO_PIN_10 | DL_GPIO_PIN_11));
    DL_GPIO_enableOutput(GPIOA , (DL_GPIO_PIN_10 | DL_GPIO_PIN_11));

    DL_GPIO_initPeripheralOutputFunction//感觉这样做没意义 我不理解api代表的是什么 只是对着工程抄写

}

void UART0_IRQHandler(){

}

int main(void)
{
    SYSCFG_DL_init();

    while (1) {
    }
}
