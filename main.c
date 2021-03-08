#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "tm_stm32f4_hd44780.h"
#include "stm32f10x_tim.h"
#include "delay.h"
#include "stdio.h"
//
	unsigned char s=255,d=200,p=50,i;
	unsigned int lux=0,data=0,adc=0;
	char write[9];

	//
	GPIO_InitTypeDef LED;
	GPIO_InitTypeDef PWM;
	GPIO_InitTypeDef LCD;
	TIM_TimeBaseInitTypeDef  TIME_Clock;
	//
	void ADC1_Initial(void);
	void UART1_Init(void);  
	//
		void TIM2_IRQHandler(){
					if( TIM_GetFlagStatus (TIM2,TIM_FLAG_Update) == 1)
				{
			if(lux>1000)
			{
				s++;
				if( s == 0) {
				GPIO_SetBits(GPIOC,GPIO_Pin_0);	
					}
				if( s == d) {
				GPIO_ResetBits(GPIOC,GPIO_Pin_0);
					}
				}
			else{
				s++;
				if( s == 0) {
				GPIO_SetBits(GPIOB,GPIO_Pin_0);	
					}
				if( s == p)	{
				GPIO_ResetBits(GPIOB,GPIO_Pin_0);
					}
				}
			}
				TIM_ClearFlag(TIM2,TIM_FLAG_Update);
				}
				
		//

	int main(){
		
		//
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
		//
		
		LCD.GPIO_Mode = GPIO_Mode_Out_PP;
		LCD.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
		LCD.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_Init(GPIOC,&LCD);

		PWM.GPIO_Mode = GPIO_Mode_Out_PP;
		PWM.GPIO_Pin = GPIO_Pin_0 ;
		PWM.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&PWM);

		LED.GPIO_Mode= GPIO_Mode_Out_PP;
		LED.GPIO_Pin= GPIO_Pin_1;
		LED.GPIO_Speed= GPIO_Speed_2MHz;
		GPIO_Init(GPIOB,&LED);		
		//
		TIME_Clock.TIM_Prescaler = 1125;
		TIME_Clock.TIM_Period = 64000;
		TIME_Clock.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit (TIM2,&TIME_Clock);
		
		TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
		NVIC_EnableIRQ(TIM2_IRQn);
		//
		TIM_Cmd(TIM2,ENABLE);
		//
		delay_intial();
		ADC1_Initial();
		UART1_Init();
		//
		TM_HD44780_Init( 16 , 2 );
		TM_HD44780_Clear();	
			//
			while(1)
			{
					ADC_SoftwareStartConvCmd(ADC1, ENABLE);
					while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==RESET);
					adc=ADC_GetConversionValue(ADC1);
						
					data=(adc*0.0008); //We dont need tenths
					lux=data*3030.3030;	//We dont need tenths
					//
					sprintf(write ,"lux:%00000d",lux); //We dont need good accuracy
					TM_HD44780_Puts(0,1,write);	
					delayMs(50);
					TM_HD44780_Clear();	
					//
					if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0) == 0)
					{
						for(i=0;i<9;i++)	{
					USART_SendData(USART1,write[i]);
					delayMs(1000);
						}
					}

					if(USART_ReceiveData(USART1) == 'M')	{ //Manual
					GPIO_SetBits(GPIOC,GPIO_Pin_1);
					GPIO_ResetBits(GPIOC,GPIO_Pin_0);						
					TIM_Cmd(TIM2,DISABLE);	
						}
					if(USART_ReceiveData(USART1) == 'A') { //Automatic
					GPIO_ResetBits(GPIOC,GPIO_Pin_1);	
					TIM_Cmd(TIM2,ENABLE);	
						}				
			}
		}
	void ADC1_Intial(void){	
			//
				ADC_InitTypeDef  ADC_Congig;
				GPIO_InitTypeDef GPIO_adc;
			//
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
				
				
				GPIO_adc.GPIO_Speed = GPIO_Speed_2MHz;
				GPIO_adc.GPIO_Mode = GPIO_Mode_AIN;
				GPIO_adc.GPIO_Pin = GPIO_Pin_0;
				GPIO_Init(GPIOA, &GPIO_adc);
			//
				RCC_ADCCLKConfig(RCC_PCLK2_Div6);  // 72Mhz / 6 = 12MHz 
			//
				ADC_Congig.ADC_Mode = ADC_Mode_Independent;
				ADC_Congig.ADC_ScanConvMode = DISABLE;
				ADC_Congig.ADC_ContinuousConvMode = ENABLE;
				ADC_Congig.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
				ADC_Congig.ADC_DataAlign = ADC_DataAlign_Right;
				ADC_Congig.ADC_NbrOfChannel = 1;
				ADC_Init(ADC1,&ADC_Congig);
			//
			ADC_Cmd(ADC1, ENABLE);
			//
			ADC_ResetCalibration(ADC1);
			while(ADC_GetResetCalibrationStatus(ADC1));
			ADC_StartCalibration(ADC1);
			while(ADC_GetCalibrationStatus(ADC1));	
			ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
			}
		void UART1_Init(void)
				{
					
				GPIO_InitTypeDef   GPIO_uart1 ;
				GPIO_InitTypeDef   GPIO_uart2 ;
				USART_InitTypeDef  UART_DATA;
					
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);		
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);		
					
				//GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);
					
				GPIO_uart1.GPIO_Mode= GPIO_Mode_AF_PP;
				GPIO_uart1.GPIO_Pin=  GPIO_Pin_9;
				GPIO_uart1.GPIO_Speed = GPIO_Speed_50MHz ;
				GPIO_Init(GPIOA,&GPIO_uart1);

				GPIO_uart2.GPIO_Mode= GPIO_Mode_IN_FLOATING;
				GPIO_uart2.GPIO_Pin=  GPIO_Pin_10;
				GPIO_uart2.GPIO_Speed = GPIO_Speed_50MHz ;
				GPIO_Init(GPIOA,&GPIO_uart2);

					
					
				UART_DATA.USART_WordLength=USART_WordLength_8b;
				UART_DATA.USART_BaudRate=9600;
				UART_DATA.USART_StopBits=USART_StopBits_1;
				UART_DATA.USART_Parity= USART_Parity_No;
				UART_DATA.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
				USART_Init(USART1,&UART_DATA);	
				USART_Cmd(USART1,ENABLE);	
				}
