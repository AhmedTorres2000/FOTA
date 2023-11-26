/*=================================================================================*/
/*  File        : FPEC program.c       	                                           */
/*  Description : This program file includes FPEC Program                          */
/*  Author      : Ahmed Mohamed Mohamed Hassanien . Embedded SW Engineer       	   */
/*  Linkedin    : https://www.linkedin.com/in/ahmed-mohamed-a23651294/             */
/*  Git account : https://github.com/AhmedTorres2000				               */
/*  mail        : Ahmedtorres2000@gmil.com                                         */
/*=================================================================================*/

#include "../../LIB/STD_TYPES.h"
#include "../../LIB/BIT_MATH.h"
#include "../../LIB/stm32f103xx.h"

#include "FPEC_config.h"
#include "FPEC_private.h"
#include "FPEC_interface.h"

void FPEC_voidEraseAppArea(void)
{
	u8 i ;
	for (i=20;i<64;i++)
	{
		FPEC_voidFlashPageErase(i);
	}
}

void FPEC_voidFlashPageErase(u8 Copy_u8PageNumber)
{
	/* Wait Busy Flag */
	while (GET_BIT( FPEC-> SR,0) == 1);

	/* Check if FPEC is locked or not */
	if ( GET_BIT(FPEC -> CR , 7) == 1)
	{
		FPEC -> KEYR = 0x45670123;
		FPEC -> KEYR = 0xCDEF89AB;
	}

	/* Page Erase Operation */
	SET_BIT(FPEC -> CR , 1);

	/* Write Page address */
	FPEC -> AR = (u32)(Copy_u8PageNumber * 1024) + 0x08000000u ;

	/* Start operation */
	SET_BIT(FPEC -> CR , 6);

	/* Wait Busy Flag */
	while (GET_BIT(FPEC -> SR , 0) == 1);

	/* EOP */
	SET_BIT(FPEC -> SR , 5);
	CLR_BIT(FPEC -> CR , 1);
}

void FPEC_voidFlashWrite(u32 Copy_u32Address, u16* Copy_u16Data, u8 Copy_u8Length)
{
	u8 i;
	volatile u16 Temp ;

	while (GET_BIT(FPEC -> SR , 0) == 1);

	/* Check if FPEC is locked or not */
	if ( GET_BIT(FPEC -> CR , 7) == 1)
	{
		FPEC -> KEYR = 0x45670123;
		FPEC -> KEYR = 0xCDEF89AB;
	}


	for (i = 0 ; i< Copy_u8Length ; i++)
	{
		/* Write Flash Programming */
		SET_BIT(FPEC -> CR , 0);

		/* Half word operation */

		Temp = Copy_u16Data[i];
		*((volatile u16*)Copy_u32Address) = Copy_u16Data[i];
		Copy_u32Address += 2 ;

		/* Wait Busy Flag */
		while (GET_BIT(FPEC -> SR , 0) == 1);

		/* EOP */
		SET_BIT(FPEC -> SR , 5);
		CLR_BIT(FPEC -> CR , 0);
	}

}



