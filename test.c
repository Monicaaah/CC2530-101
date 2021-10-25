/* 包含头文件 */
#include "ioCC2530.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* 四个LED灯的端口定义 */
#define D3 P1_0
#define D4 P1_1
#define D5 P1_3
#define D6 P1_4
/* 按键的端口定义 */
#define KEY1 P1_2


/* 变量定义 */
unsigned char workMode=0;     //系统工作模式:0：流水灯模式  1：自动模式
unsigned int  counter=0;      //定时器中断计数器
unsigned int  sensorValue=0;  //火焰传感器数值
unsigned char temp;//从串口接收一个字符
char uart_rx_buf[20];//串口接收缓冲区
char uart_rx_len=0;//串口接收字符长度

/**********LED端口初始化************************/
void InitLED(void)
{
   
   /*.......答题区1开始：LED灯IO端口初始化 4个LED默认为熄灭状态....................*/
  P1SEL &=~0X1B;
  P1DIR |=0X1B;
  P1 &=~0X1B;
  /*.......答题区1 结束.......................................*/
}

/************流水灯模式**************/
void warterLedMode()
{
    //由考生编写 实现流水灯效果，定时器定时10ms产生一此中断，要求每个灯时间约为250ms
    /*.......答题区2 开始：....................*/
    if(counter<25)
    {
      D5=1;D3=D4=D6=0;
    }
    else if(counter<50)
    {
      D6=1;D3=D4=D5=0;
    }
    else if(counter<75)
    {
      D3=1;D4=D5=D6=0;
    }
    else if(counter<100)
    {
      D4=1;D3=D5=D6=0;
    }
    else
    {
    counter=0;
    }

   /*.......答题区2 结束.......................................*/
}




void InitKey(void)
{

   /*.......答题区3 开始：设置KEY相应引脚模式，普通IO口，输入，上下拉模式....................*/
   P1SEL &=~0X04;
   P1DIR &=~0X04;
   P1INP &=~0X04;
   P2INP &=~0X40;
   IEN2|=0x10;
   P1IEN|=0x04;
   PICTL|=0x02;
   
   /*.......答题区3 结束.......................................*/
}
/***********定时器初始化************************/
void InittTimer1(void)
{
    T1IF=0;                  //清除timer1中断标志
    T1STAT &= ~0x01;         //清除通道0中断标志
    
    T1CTL = 0x0A;            //配置32分频，模比较计数工作模式 
    
    T1CCTL0 |= 0x04;         //设定timer1通道0比较模式
    T1CC0L = 10000&0xFF;     //把10000的低8位写入T1CC0L
    T1CC0H = (10000>>8)&0xFF;//把10000的高8位写入T1CC0H
    
    T1IE = 1;                //使能定时器1的中断，或者写为IEN1 |= 0x02;
   }

void ScanKey(void)
{
  
  /*.......答题区4 开始：要求实现按键扫描 
  实现默认上电是流水灯模式，
  按1次 进入自动报警模式
  再按1次，进入流水灯模式  
  ....................*/

      if(KEY1==0)
      {
	workMode=~workMode;
      }

   
     /*.......答题区4 结束.......................................*/
}
/**********串口通信初始化************************/
void InitUART0(void)
{
    PERCFG = 0x00; //usart0 使用备用位置1 TX-P0.3 RX-P0.2  
    P0SEL |= 0x0c; //  P0.2 P0.3 用于外设功能
    U0CSR |= 0xC0; //uart模式 允许接收
    P2DIR &= ~0xC0;   //P0优先作为UART方式  
   
   /*.......答题区5 开始： 波特率115200bps....................*/
    U0GCR =11;
    U0BAUD =216;
    
     /*.......答题区5 结束.......................................*/
    
    U0UCR |= 0x80;//流控无 无奇偶校验 8位数据位  1位停止位
    URX0IF = 0;         //清零UART0 RX中断标志
    UTX0IF = 0;         //清零UART0 TX中断标志
    URX0IE = 1;         //使能接收中断
}


/************定时器T1中断服务子程序**************/
#pragma vector = T1_VECTOR //中断服务子程序
__interrupt void T1_ISR(void)
{   
    counter++;
    ScanKey();
}



/*************** 往串口发送指定长度的数据  ***************/
void uart_tx_string(char *data_tx,int len)  
{   
  unsigned int j;  
  for(j=0;j<len;j++)  
  {   
    U0DBUF = *data_tx++;  // 将要发送的1字节数据写入U0DBUF
    while(UTX0IF == 0);  // 等待TX中断标志，即U0DBUF就绪
    UTX0IF = 0;   // 清零TX中断标志
  }
} 



/************UART0 接收中断服务子程序**************/
#pragma vector = URX0_VECTOR //中断服务子程序
__interrupt void UART0_RX_ISR(void)
{
   
    URX0IF=0;
    temp = U0DBUF;
  
    if(temp !='#')
    {
      /*.......答题区6 开始：..
      把从串口接收到的数据存放到uart_rx_buf中，用uart_rx_len控制住下标..................*/
      uart_rx_buf[uart_rx_len++]=temp;
      
       /*.......答题区6 结束.......................................*/
    }
    else
    {
      U0CSR &= ~0x40;//禁止接收
      
      uart_tx_string("火焰ADC值=",sizeof("火焰ADC值="));
      /*.......答题区8 开始：..把接收到的uart_rx_buf内容发送回串口..................*/
      uart_tx_string(uart_rx_buf,uart_rx_len);
      
       /*.......答题区8 结束.....................*/
   
      sensorValue=atoi(uart_rx_buf);//将火焰传感数据转换成整数并存放到火焰传感器变量sensorValue中
      memset(uart_rx_buf,0,sizeof(uart_rx_buf));
      uart_rx_len=0;      
      
      U0CSR |= 0x40;//允许接收
    }
}



/***************自动模式*****************/
void autoControl()
{
    //由考生编写 自动模式下的状态灯是D3灭 D4亮。
    //根据火焰传感器的数值自动控制D5 D6的输出状态
    //sensorValue>=200时，实现D5 D6灯亮，否则D5 D6灯灭
    /*.......答题区7 开始：....................*/
    D3=0;D4=1;
    if(sensorValue>=200)
    {
      D5=D6=1;
    }
    else
    {
      D5=D6=0;
    }

   /*.......答题区7 结束.......................................*/
}

/************main函数入口**************************/
void main(void)
{
    //时钟初始化,速度设置为32MHz
    CLKCONCMD &= 0X80;
    while(CLKCONSTA&0X40);
    
    InitLED();
    InitKey();
    InittTimer1();      //初始化Timer1
    InitUART0();        //UART0初始化

    EA = 1;             //使能全局中断
    
    while(1)
    {
        if(workMode==0)
            warterLedMode();//流水灯模式
        else if(workMode==1)
             autoControl();//自动报警模式
       
    }
}
