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

    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM21, IOMUX_PINCM21_PF_UART0_TX);
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM22, IOMUX_PINCM2_PF_UART0_RX);

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
    DL_GPIO_initDigitalOutput(IOMUX_PINCM31);
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
    DL_SYSTICK_config(CPUCLK_FREQ / 1000U);
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
// volatile uint8_t rx_data = 0;
volatile uint32_t rx_irq_count = 0;



#define BUF_SIZE 8

uint8_t buffer[BUF_SIZE];

volatile uint8_t write = 0;//head
volatile uint8_t read = 0;//tile
// volatile bool full = false;

bool Buffer_Push(uint8_t data){
    
    if((write = (write + 1) % BUF_SIZE) == read) return false;

    buffer[write] = data ;

    write = (write + 1) % BUF_SIZE;//0开始 ++ -> 1%8=1 ... 7%8=7 -> 8%8 = 0 ; 0 1 2 3 4 5 6 7
    //                                                         t             h ( h++ == t )   
    return true;
}


bool Buffer_Pop(uint8_t *data)//确实不理解为什么* 因为这个“data”在某处实例化之后 会在push被赋值 然后直接访问变量位置就可以省出来位置吗
{
     if(read == write) return false;

    *data = buffer[read];//data是存储存储数据地址的变量 &data是访问地址本身 *data是访问数据内容
    
    // DL_UART_Main_transmitData(UART_0_INST,&data);

    read = (read + 1) % BUF_SIZE;

    return true;
}


uint32_t rx_main_count = 0;
// bool tx_busy = false , rx_busy = false;

volatile uint8_t data = 0;
int main(void)
{
    SYSCFG_DL_init();
    
    UART_Init();
    LED_Init();
    Btn_Init();
    SysTick_Init();

    uint8_t data ;
    while (1) 
    {

        // if(rx_ready){
        //     rx_ready = false;

        //     // DL_UART_Main_transmitData(UART_0_INST,rx_data);
        //     rx_main_count++;
        //     delay_cycles(CPUCLK_FREQ / 100);

        // }
        if(Buffer_Pop(&data)){
            DL_UART_Main_transmitData(UART_0_INST,data);
            rx_main_count++;
            delay_cycles(CPUCLK_FREQ / 100);
        }


    }
}



void UART0_IRQHandler(){
    switch(DL_UART_Main_getPendingInterrupt(UART_0_INST)){
        case DL_UART_MAIN_IIDX_RX:

            data = DL_UART_Main_receiveData(UART_0_INST);
            Buffer_Push(data);
            // rx_ready = true;
            rx_irq_count++;

        break;
        
        default:
            break;        
    }
}


/*
为什么先设初始电平再enable output？ 养成好习惯 之后初始未定义电平可能会带来意料外的故障



为什么CPUCLK/1000？一秒钟翻转3200000次 现在需要每1ms反馈一次 那就需要1s来回1000次 3200000/1000=3200？所以不应该是3200吗
应该是 CPUCLK_FREQ/（CPUCLK_FREQ/1000） -> 3200000/(3200000/1000)= 3200000/3200=1000

是 32,000,000 Hz = 32 MHz，不是 3,200,000。

数 32000 个 CPU tick
↓
过去约 1 ms
↓
产生一次 SysTick IRQ
↓
system_ms++




为什么system_ms volatile 目前浅显的分类办法是 是否会被主函数调用 纯粹的iqr 之类的控制的变量需要volatile （话说const又是？）

真正判断的是：

一个变量会不会在当前代码执行流“看不到的地方”发生改变，同时另一个执行上下文还会访问它？

const 的意思就是：

这份配置创建以后，不允许普通 C 代码再修改它。




为什么ISR里只++？中断内部简单逻辑 防止卡死

ISR 占 CPU 太久，会：

延迟其他中断；
增大系统响应抖动；
严重时丢事件；
整个程序表现得像“卡”。






为什么还额外包一层millis()？一个变量 虽然我还是觉得直接用没啥 但是看你的意思是防止多处调用造成冲突？
dl库里有好多同一个引脚define了不同 但是实际对象一样的别名 为了语境贴切很好理解目的 
多包一层最大意义就sys_ms不可能被millis修改 或者说防止被修改？

它真正的价值是：

把“时间从哪里来”藏起来，给上层一个稳定接口。

按钮只知道：

millis()

它不需要知道：

现在是 SysTick
以后换 Timer
还是 RTOS tick

假设以后底层改成：

uint32_t millis(){
    return Timer_GetMilliseconds();
}

按钮代码：

if(millis() - start >= 600)

完全不用改。

这叫封装 / abstraction

*/