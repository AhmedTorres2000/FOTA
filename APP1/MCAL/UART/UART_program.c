/********************************************************************************************/
/********************************************************************************************/
/**************************		Author:  Ahmed Mohamed Mohamed Hassanien        *************/
/**************************		Layer:   MCAL								   	*************/
/**************************		SWC:	 UART								*************/
/**************************		Version: 1.00 	             					*************/
/********************************************************************************************/
/********************************************************************************************/

/*******************************************************************************
*                        		Inclusions                                     *
*******************************************************************************/
#include "LIB/Error_type.h"
#include "LIB/BIT_MATH.h"
#include "LIB/stm32f103xx.h"

#include "MCAL/UART/UART_interface.h"
#include "MCAL/UART/UART_private.h"
#include "MCAL/UART/UART_config.h"

/*******************************************************************************
*                      Private Variables	                                   *
*******************************************************************************/
static uint8_t Global_UARTEND_STRING[3] = {'#','#','#'};

typedef void(*fptr_t)(void);
static fptr_t Global_pfArr[3] = {NULL,NULL,NULL};

UART_RegDef_t * UART_Astr[3] = {UART1,UART2,UART3};

/*******************************************************************************
*                      Functions Definitions                                   *
*******************************************************************************/

/*******************************************************************************
* Function Name:		UART_u8Init
********************************************************************************/
uint8_t UART_u8Init(uint8_t Copy_u8UartNumber, UART_configStruct_t * Copy_PstrUARTConfig)
{
	uint8_t Local_ErrorState = OK;
	uint16_t Local_Mantissa,Local_Fraction;
	uint32_t Baud = Copy_PstrUARTConfig->UART_u32_BAUD_RATE;

	if(Copy_PstrUARTConfig != NULL)
	{
		UART_Astr[Copy_u8UartNumber]->CR1 = 0;
		UART_Astr[Copy_u8UartNumber]->CR2 = 0;
		UART_Astr[Copy_u8UartNumber]->CR1 |= ((Copy_PstrUARTConfig->UART_u8_DATA_SIZE & 0b1) << 12);
		UART_Astr[Copy_u8UartNumber]->CR1 |= ((Copy_PstrUARTConfig->UART_u8_PARTIY & 0b11) << 9);
		/* First bit indicates the receive is interrupt or polling */
		UART_Astr[Copy_u8UartNumber]->CR1 |= ((Copy_PstrUARTConfig->UART_u8_RECEIVE_METHOD & 0b1) << 5);
		/* Second and third bit indicates the DMA Configuration */
		UART_Astr[Copy_u8UartNumber]->CR3 |= (((Copy_PstrUARTConfig->UART_u8_RECEIVE_METHOD>>1) & 0b11) << 6);
		UART_Astr[Copy_u8UartNumber]->CR2 |= ((Copy_PstrUARTConfig->UART_u8_STOP_BIT&0b11) << 12);
		Global_UARTEND_STRING[Copy_u8UartNumber] = Copy_PstrUARTConfig->UART_u8_END_STRING ;

		/* Baud Rate Calculations */
		Local_Mantissa = (F_CPU/(16*Baud));
		Local_Fraction = ((F_CPU%(16*Baud)) / Baud );
		float Reminder = (F_CPU - Local_Mantissa*16*Baud)/(float)Baud ;

		if( ((Reminder - (int)Reminder)*10)  >= 5 )
		{
			Local_Fraction ++;
			if(Local_Fraction > 15)
			{
				Local_Fraction = 0;
				Local_Mantissa ++;
			}
		}
		UART_Astr[Copy_u8UartNumber]->BRR = ((Local_Mantissa & 0xFFF) << 4) | (Local_Fraction & 0xF);

		SET_BIT(UART_Astr[Copy_u8UartNumber]->CR1,3); /* Transmit Enable */
		SET_BIT(UART_Astr[Copy_u8UartNumber]->CR1,2); /* Receive Enable */
		SET_BIT(UART_Astr[Copy_u8UartNumber]->CR1,13); /* Enable the UART */
	}
	else
	{
		Local_ErrorState = NOK;
	}

	return Local_ErrorState;
}

/*******************************************************************************
* Function Name:		UART_u8SendByte
********************************************************************************/
uint8_t UART_u8SendByte(uint8_t Copy_u8UartNumber, uint8_t Copy_u8DataByte)
{
	uint8_t Local_ErrorState = OK;

	CLR_BIT(UART_Astr[Copy_u8UartNumber]->SR,6);
	UART_Astr[Copy_u8UartNumber]->DR = Copy_u8DataByte; /* The flag is cleared when we write to the data register */
	while(GET_BIT(UART_Astr[Copy_u8UartNumber]->SR,6) == 0); /* Wait if the previous Transmission isn't completed */
	
	return Local_ErrorState;
}
/*******************************************************************************
* Function Name:		UART_u8SendString
********************************************************************************/
uint8_t UART_u8SendString(uint8_t Copy_u8UartNumber, uint8_t* Copy_Pu8DataString)
{
	uint8_t Local_ErrorState = OK;

	if(Copy_Pu8DataString != NULL)
	{
		uint16_t i ;
		for(i = 0 ; Copy_Pu8DataString[i] != '\0' ; i++)
		{
			UART_u8SendByte(Copy_u8UartNumber,Copy_Pu8DataString[i]);
		}
		UART_u8SendByte(Copy_u8UartNumber,'\0');
	}
	else
	{
		Local_ErrorState = NOK;
	}

	return Local_ErrorState;
}

/*******************************************************************************
* Function Name:		UART_u8SendStream
********************************************************************************/
uint8_t UART_u8SendStream(uint8_t Copy_u8UartNumber, uint8_t* Copy_pu8DataByte, uint16_t Copy_u8DataSize)
{
	uint8_t Local_ErrorState = OK;

	if(Copy_pu8DataByte != NULL)
	{
		while(Copy_u8DataSize --)
		{
			UART_u8SendByte(Copy_u8UartNumber,*Copy_pu8DataByte);
			Copy_pu8DataByte++;
		}
	}
	else
	{
		Local_ErrorState = NOK;
	}

	return Local_ErrorState;
}

/*******************************************************************************
* Function Name:		UART_u8ReceiveByte
********************************************************************************/
uint8_t UART_u8ReceiveByte(uint8_t Copy_u8UartNumber, uint8_t* Copy_pu8DataByte)
{
	uint8_t Local_ErrorState = OK;

	if(Copy_pu8DataByte != NULL)
	{
		while(GET_BIT(UART_Astr[Copy_u8UartNumber]->SR,5) == 0);
		*Copy_pu8DataByte = UART_Astr[Copy_u8UartNumber]->DR;
		CLR_BIT(UART_Astr[Copy_u8UartNumber]->SR,5); /* Clear the flag */
	}
	else
	{
		Local_ErrorState = NOK;
	}

	return Local_ErrorState;
}

/*******************************************************************************
* Function Name:		UART_u8ReceiveString
********************************************************************************/
uint8_t UART_u8ReceiveString(uint8_t Copy_u8UartNumber, uint8_t* Copy_pu8DataByte)
{
	uint8_t Local_ErrorState = OK;

	if(Copy_pu8DataByte != NULL)
	{
		uint16_t i = 0;
		UART_u8ReceiveByte(Copy_u8UartNumber,&Copy_pu8DataByte[i]);
		while(Copy_pu8DataByte[i] != Global_UARTEND_STRING[Copy_u8UartNumber] && Copy_pu8DataByte[i] != '\0')
		{
			i++;
			UART_u8ReceiveByte(Copy_u8UartNumber,&Copy_pu8DataByte[i]);
		}
		Copy_pu8DataByte[i] = '\0';
	}
	else
	{
		Local_ErrorState = NOK;
	}

	return Local_ErrorState;
}

/*******************************************************************************
* Function Name:		UART_u8ReceiveStream
********************************************************************************/
uint8_t UART_u8ReceiveStream(uint8_t Copy_u8UartNumber, uint8_t* Copy_pu8DataByte, uint16_t Copy_u8DataSize)
{
	uint8_t Local_ErrorState = OK;

	if(Copy_pu8DataByte != NULL)
	{
		while(Copy_u8DataSize--)
		{
			UART_u8ReceiveByte(Copy_u8UartNumber,Copy_pu8DataByte++);
		}
	}
	else
	{
		Local_ErrorState = NOK;
	}

	return Local_ErrorState;
}

/*******************************************************************************
* Function Name:		UART_u8SetCallBack
********************************************************************************/
uint8_t UART_u8SetCallBack(uint8_t Copy_u8UartNumber,  void(*Copy_pfCallback)(void))
{
	uint8_t Local_ErrorState = OK;

	if(Copy_pfCallback != NULL)
	{
		Global_pfArr[Copy_u8UartNumber] = Copy_pfCallback;
	}
	else
	{
		Local_ErrorState = NOK;
	}

	return Local_ErrorState;
}

/*******************************************************************************
*                       Interrupt Service Routines                            *
*******************************************************************************/
void USART1_IRQHandler(void)
{
	CLR_BIT(UART1->SR,5); /* Clear the flag */
	if(Global_pfArr[0] != NULL)
	{
		Global_pfArr[0]();
	}
}

void USART2_IRQHandler(void)
{
	CLR_BIT(UART2->SR,5); /* Clear the flag */
	if(Global_pfArr[1] != NULL)
	{
		Global_pfArr[1]();
	}
}

void USART3_IRQHandler(void)
{
	CLR_BIT(UART3->SR,5); /* Clear the flag */
	if(Global_pfArr[2] != NULL)
	{
		Global_pfArr[2]();
	}
}
