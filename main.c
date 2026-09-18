#include "ti_msp_dl_config.h"


#define UART_0_INST UART0
#define UART_0_INST_IRQHandler UART0_IRQHandler
#define UART_0_INST_INT_IRQN UART0_INT_IRQn

static const DL_UART_Main_Config UART_0_Cfg = {
    .mode = DL_UART_MAIN_MODE_NORMAL,
    .wordLength = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .stopBits = DL_UART_MAIN_STOP_BITS_ONE,
    .parity = DL_UART_MAIN_PARITY_NONE,
    .direction = DL_UART_MAIN_DIRECTION_TX_RX
};

static const DL_UART_Main_ClockConfig UART_0_Clk = {
    .clockSel = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

void UART_Init(){

    DL_UART_Main_enablePower(UART_0_INST);

    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM21, GPIO_UART_0_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(GPIO_UART_0_IOMUX_RX, GPIO_UART_0_IOMUX_RX_FUNC);

    DL_UART_Main_setClockConfig(UART_0_INST,(DL_UART_Main_ClockConfig  *) &UART_0_Clk);
    DL_UART_Main_init(UART_0_INST,(DL_UART_Main_Config *) &UART_0_Cfg);//要先设置时钟再初始化串口

    //DL_UART_Main_setOversampling(UART_0_INST, DL_UART_OVERSAMPLING_RATE_3X);这个不用吗<--下面的自动分配波特率已经解决了
    DL_UART_Main_configBaudRate(UART_0_INST,CPUCLK_FREQ,115200);//CPUCLK_FREQ 和 UART 当前实际使用的 BUSCLK 概念上不是一回事

    DL_UART_Main_enableInterrupt(UART_0_INST,DL_UART_MAIN_INTERRUPT_RX);
    DL_UART_Main_enable(UART_0_INST);

    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);

}

void LED_Init(){
    DL_GPIO_initPeripheralOutput(IOMUX_PINCM31);
    DL_GPIO_setPins(GPIOB, DL_GPIO_PIN_14);
    DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_14);
}

void Btn_Init(){
    DL_GPIO_initDigitalInputFeatures(IOMUX_PINCM49, 
    DL_GPIO_INVERSION_DISABLE,
    DL_GPIO_RESISTOR_PULL_UP,
    DL_GPIO_HYSTERESIS_DISABLE,
    DL_GPIO_WAKEUP_DISABLE);
}

void SysTick_Init(){
    CPUCLK_FREQ(CPUCLK_FREQ / 1000U);
}

volatile uint32_t sys_ms = 0;
void SysTick_Handler(){
    sys_ms++;
}

uint32_t millis(){
    return sys_ms;
}

// volatile uint8_t EchoData = 0;错误 历程是iqr内部消化不涉及while 所以要防止优化
uint8_t EchoData = 0;
volatile bool rx_ready = false;

void UART0_IRQHandler(){
    switch(DL_UART_Main_getPendingInterrupt(UART_0_INST)){
        case DL_UART_MAIN_IIDX_RX:
        
            EchoData = DL_UART_Main_receiveData(UART_0_INST);
        break;
        
        default:
            break;        
    }
}

int main(void)
{
    SYSCFG_DL_init();
    
    UART_Init();
    LED_Init();
    Btn_Init();
    SysTick_Init()

    while (1) 
    {
        if(rx_ready){
            rx_ready = false;
        }

    }
}

/*
为什么先设初始电平再enable output？ 养成好习惯 之后初始未定义电平可能会带来意料外的故障

为什么CPUCLK/1000？一秒钟翻转3200000次 现在需要每1ms反馈一次 那就需要1s来回1000次 3200000/1000=3200？所以不应该是3200吗
应该是 CPUCLK_FREQ/（CPUCLK_FREQ/1000） -> 3200000/(3200000/1000)= 3200000/3200=1000

为什么system_ms volatile目前浅显的分类办法是 是否会被主函数调用 纯粹的iqr 之类的控制的变量需要volatile （话说const又是？）

为什么ISR里只++？中断内部简单逻辑 防止卡死

为什么还额外包一层millis()？一个变量 虽然我还是觉得直接用没啥 但是看你的意思是防止多处调用造成冲突？
dl库里有好多同一个引脚define了不同 但是实际对象一样的别名 为了语境贴切很好理解目的 
多包一层最大意义就sys_ms不可能被millis修改 或者说防止被修改？
*/