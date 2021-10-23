/* 包含头文件 */
#include <ioCC2530.h>
#include <stdio.h>
#include <string.h>

/*宏定义*/
#define D3 P1_0 
#define D4 P1_1
#define D5 P1_3 
#define D6 P1_4
#define SW1 P1_2

/*定义变量*/
unsigned char counter = 0; //统计定时器溢出次数
unsigned char LedState = 1;//4个LED灯的工作模式，取值范围1、2、3
unsigned char keyCount = 0;//统计SW1按键按下次数
unsigned char outputStr[100]={0};//存放串口输出数据

/*声明函数*/
void InitCLK(void);//系统时钟初始化函数，为32MHz
void UART0SendByte(unsigned char c);//UART0发送一个字节函数
void UART0SendString(unsigned char *str);//UART0发送整个字符串
void delay(void);//延时函数
/*定义函数*/
void InitCLK(void)
{
  CLKCONCMD &= 0x80;
  while(CLKCONSTA & 0x40);
}

void UART0SendByte(unsigned char c)
{
  U0DBUF = c;// 将要发送的1字节数据写入U0DBUF
  while (!UTX0IF) ;// 等待TX中断标志，即U0DBUF就绪
  UTX0IF = 0;// 清零TX中断标志
}

void UART0SendString(unsigned char *str)
{
  while(*str != '\0')
  {
    UART0SendByte(*str++);
  }
}

void delay(void)
{
  unsigned long t = 50000;
  while(t--);
}

/*主函数*/
void main(void)
{
  InitCLK();
  
  /*.......答题区1开始：LED灯IO端口初始化和熄灭所有灯....................*/
  P1SEL &=~0X1B;
  P1DIR |=0X1B;
  P1 &=~0X1B;//熄灭所有灯
  //D3=D4=D5=D6=0;
  
  /*.......答题区1结束.......................................*/
 
  /*.......答题区2开始：按键中断输入功能初始化...............*/
  P1SEL &=~0X04;
  P1DIR &=~0X04;//设置为输入，清0的操作
  P1INP &=~0X04;
  P2INP &=~0X40;
  P1IEN |=0X04;
  IEN2 |=0X10;
  PICTL |=0x02;
  //下面已经有使能总中断，这里不需要
  
  
  /*.......答题区2结束.......................................*/
  
  /*.......答题区3开始：定时器1初始化........................*/
  T1CTL =0X08;
  TIMIF |=0X40;//使能定时器1的溢出中断
  T1IE =1;
//1.注释 2.试题
  
  /*.......答题区3结束.......................................*/

  /*....... 串口0初始化.........................*/
  PERCFG = 0x00;	
  P0SEL = 0x3c;	
  U0CSR |= 0x80;//设置USART0为UART模式
  U0BAUD = 216;//设置波特率14400
  U0GCR = 8;//设置波特率14400
  U0UCR |= 0x80;
  UTX0IF = 0;  // 清零USART0 TX中断标志 
  /*....... ......................................*/
    
  EA = 1;//使能总中断  
  while(1)
  {
  /*.......答题区4开始：实现LED灯3种工作模式对应的效果.........................*/
      //可先把外围的框架写完，最后再写这里
    if(LedState==1)
    {
      D3=D4=D5=D6=0;
    }
    else if(LedState==2)
    {
     D3=D4=D5=D6=1;
     delay();
     D3=D4=D5=D6=0;
     delay();
    }
    else if(LedState==3)
    {
      D3=D4=0;
      D5=1;
      D6=0;
      delay();
      D5=0;
      D6=1;
      delay();
    }
    
    
    
  /*.......答题区4结束：..................................*/    
  }
}

/*中断服务函数*/

#pragma vector = P1INT_VECTOR
__interrupt void P1_ISR(void)
{
  if(P1IFG & 0x04)
  {
    /*.......答题区5开始：...........*/
     //按键统计次数自加1
        keyCount++;//keyCount+1
     //启动定时器1工作于自由运行模式
        T1CTL =0X09;
     //清除P1_2口中断标志位
        P1IFG &=~0x04;
    /*.......答题区5结束...........*/
  }
  P1IF = 0;//清除P1端口中断标志位
}



//在1秒内，SW1按键按下次数不同代表不同的灯的工作模式
#pragma vector = T1_VECTOR
__interrupt void T1_ISR(void)
{
  counter++;
  if(counter>10)
  {
      /*.......答题区6开始：...........*/
     //定时器1暂停运行
        T1CTL =0X08;
     //设置T1CNTL清零定时器1计数寄存器
        T1CNTL =0X00;//随便一个数都可清零
    //设置LED灯工作模式等于按键次数
    if(keyCount==1)
    {
      LedState=1;
    }
    else if(keyCount==2)
    {
      LedState=2;
    }
    else if(keyCount==3)
    {
      LedState=3;
    } 
     /*.......答题区6结束...........*/
    keyCount = 0;//清零SW1按键按下次数
    counter = 0;
    memset(outputStr,'\0',100);//清空outputStr内容
    sprintf((char *)outputStr,"当前工作于mode%d模式\r\n",LedState);//构建串口发生信息内容
    UART0SendString(outputStr);//串口发送当前工作模式信息
  }
}
