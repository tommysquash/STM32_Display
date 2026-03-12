/*
 * LCD_DRIVER.c
 *
 *  Created on: 11 mar 2026
 *      Author: Esame
 */
#include <main.h>


void LCD_Pulse_EN(void) {
    // 1. Piccolissimo ritardo per far stabilizzare i dati prima di dare l'Enable (Setup Time)
    for(volatile int x=0; x<5; x++);

    // 2. Alza Enable
    LL_GPIO_SetOutputPin(LCD_EN_GPIO_Port, LCD_EN_Pin);
    for(volatile int x=0; x<20; x++); // Ritardo per fargli leggere il dato

    // 3. Abbassa Enable
    LL_GPIO_ResetOutputPin(LCD_EN_GPIO_Port, LCD_EN_Pin);
    for(volatile int x=0; x<20; x++); // Ritardo di Hold Time
}

void LCD_Send_Nibble(uint8_t nibble) {
    // DB4..DB7
    if (nibble & 0x01) LL_GPIO_SetOutputPin(LCD_DB4_GPIO_Port, LCD_DB4_Pin);
    else               LL_GPIO_ResetOutputPin(LCD_DB4_GPIO_Port, LCD_DB4_Pin);

    if (nibble & 0x02) LL_GPIO_SetOutputPin(LCD_DB5_GPIO_Port, LCD_DB5_Pin);
    else               LL_GPIO_ResetOutputPin(LCD_DB5_GPIO_Port, LCD_DB5_Pin);

    if (nibble & 0x04) LL_GPIO_SetOutputPin(LCD_DB6_GPIO_Port, LCD_DB6_Pin);
    else               LL_GPIO_ResetOutputPin(LCD_DB6_GPIO_Port, LCD_DB6_Pin);

    if (nibble & 0x08) LL_GPIO_SetOutputPin(LCD_DB7_GPIO_Port, LCD_DB7_Pin);
    else               LL_GPIO_ResetOutputPin(LCD_DB7_GPIO_Port, LCD_DB7_Pin);

    LCD_Pulse_EN();
}

// FUNZIONE CORRETTA: Legge il Busy Flag evitando cortocircuiti logici
void LCD_WaitBusy(void) {
    uint8_t isBusy = 1;
    uint32_t timeout = 50000;

    // FONDAMENTALE: Imposta TUTTI i pin dati come INPUT prima di leggere!
    LL_GPIO_SetPinMode(LCD_DB4_GPIO_Port, LCD_DB4_Pin, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(LCD_DB5_GPIO_Port, LCD_DB5_Pin, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(LCD_DB6_GPIO_Port, LCD_DB6_Pin, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(LCD_DB7_GPIO_Port, LCD_DB7_Pin, LL_GPIO_MODE_INPUT);

    // RS = 0 (Comando), RW = 1 (Lettura)
    LL_GPIO_ResetOutputPin(LCD_RS_GPIO_Port, LCD_RS_Pin);
    LL_GPIO_SetOutputPin(LCD_RW_GPIO_Port, LCD_RW_Pin);

    while (isBusy && timeout > 0) {
        // Leggi il primo nibble (dove si trova il Busy Flag su DB7)
        LL_GPIO_SetOutputPin(LCD_EN_GPIO_Port, LCD_EN_Pin);
        for(volatile int x=0; x<15; x++);
        isBusy = LL_GPIO_IsInputPinSet(LCD_DB7_GPIO_Port, LCD_DB7_Pin);
        LL_GPIO_ResetOutputPin(LCD_EN_GPIO_Port, LCD_EN_Pin);
        for(volatile int x=0; x<15; x++);

        // Leggi il secondo nibble (Obbligatorio a 4 bit per chiudere il ciclo!)
        LL_GPIO_SetOutputPin(LCD_EN_GPIO_Port, LCD_EN_Pin);
        for(volatile int x=0; x<15; x++);
        LL_GPIO_ResetOutputPin(LCD_EN_GPIO_Port, LCD_EN_Pin);
        for(volatile int x=0; x<15; x++);

        timeout--;
    }

    // Ripristina la scrittura (RW=0) e rimetti TUTTI i pin come OUTPUT
    LL_GPIO_ResetOutputPin(LCD_RW_GPIO_Port, LCD_RW_Pin);
    LL_GPIO_SetPinMode(LCD_DB4_GPIO_Port, LCD_DB4_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(LCD_DB5_GPIO_Port, LCD_DB5_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(LCD_DB6_GPIO_Port, LCD_DB6_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(LCD_DB7_GPIO_Port, LCD_DB7_Pin, LL_GPIO_MODE_OUTPUT);
}

void LCD_Send_Byte(uint8_t byte, uint8_t isData) {
    // 1. Aspetta la conferma dal display usando il Flag (velocissimo!)
    LCD_WaitBusy();

    // 2. Invia i dati
    LL_GPIO_ResetOutputPin(LCD_RW_GPIO_Port, LCD_RW_Pin);

    if (isData) LL_GPIO_SetOutputPin(LCD_RS_GPIO_Port, LCD_RS_Pin);
    else        LL_GPIO_ResetOutputPin(LCD_RS_GPIO_Port, LCD_RS_Pin);

    LCD_Send_Nibble(byte >> 4);   // nibble alto
    LCD_Send_Nibble(byte & 0x0F); // nibble basso
}

void LCD_Init(void) {
    HAL_Delay(50); // attesa power-on

    // Inizializzazione 4-bit forzata a mano (senza leggere il flag)
    LL_GPIO_ResetOutputPin(LCD_RS_GPIO_Port, LCD_RS_Pin);
    LL_GPIO_ResetOutputPin(LCD_RW_GPIO_Port, LCD_RW_Pin);

    LCD_Send_Nibble(0x03); HAL_Delay(5);
    LCD_Send_Nibble(0x03); HAL_Delay(1);
    LCD_Send_Nibble(0x03); HAL_Delay(1);
    LCD_Send_Nibble(0x02); HAL_Delay(1); // passa a 4-bit

    // Da qui in poi Send_Byte userà il Busy Flag per viaggiare al massimo!
    LCD_Send_Byte(0x28, 0); // 4-bit, 2 righe, 5x8
    LCD_Send_Byte(0x0C, 0); // display ON, cursore OFF
    LCD_Send_Byte(0x06, 0); // incremento automatico
    LCD_Send_Byte(0x01, 0); // clear display
}

void LCD_Print(const char *str) {
    while (*str) {
        LCD_Send_Byte((uint8_t)*str++, 1);
    }
}
void LCD_Set_Cursor(uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? 0x00 : 0x40;
    address += col;
    LCD_Send_Byte(0x80 | address, 0);
}

/* USER CODE END 0 */
