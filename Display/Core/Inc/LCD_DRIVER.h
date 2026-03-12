/*
 * LCD_DRIVER.h
 *
 *  Created on: 11 mar 2026
 *      Author: Esame
 */

#ifndef INC_LCD_DRIVER_H_
#define INC_LCD_DRIVER_H_
void LCD_Pulse_EN(void) ;
void LCD_Send_Nibble(uint8_t nibble) ;
void LCD_WaitBusy(void);
void LCD_Send_Byte(uint8_t byte, uint8_t isData) ;
void LCD_Init(void) ;
void LCD_Print(const char *str) ;
void LCD_Set_Cursor(uint8_t row, uint8_t col);

#endif /* INC_LCD_DRIVER_H_ */
