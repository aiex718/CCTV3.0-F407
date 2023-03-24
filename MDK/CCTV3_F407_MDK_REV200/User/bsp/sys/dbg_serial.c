#include "bsp/sys/dbg_serial.h"
#include "bsp/platform/periph_list.h"
#include "bsp/hal/usart.h"
#include "stdio.h"

int fputc(int ch, FILE *f)
{
	HAL_USART_WriteByte(Debug_Usart3, (uint8_t) ch);
	return (ch);
}

int fgetc(FILE *f)
{
	uint8_t ch;
	while (HAL_USART_ReadByte(Debug_Usart3, &ch)==false);
	return (int)ch;
}

// int fputc(int ch, FILE *f)
// {
// 	USART_SendData(USART3, (uint8_t) ch);
// 	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
// 	return (ch);
// }

// int fgetc(FILE *f)
// {
// 	while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);
// 	return (int)USART_ReceiveData(USART3);
// }

