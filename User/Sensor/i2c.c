/*
�� ����������� ���������� I2C ��� ����������������� STM32F030
���������� �� ������������
SPL ������������ ������ ��� ������ �������������� �������
��� ������� ����� ��������� AF ������� � �������� SPL ������
--------------------------
����� Zlodey
masya-chel@mail.ru
masya-chel@yandex.ru
--------------------------
���� 2015�.
*/

#include "i2c.h"


/*
��������� ��������� ��������� ����.
����������� �����, �������������� �������.
����������� �������� ��� ������ ����.
*/
void I2C_Initialization (void)
{
	// ������������ GPIO �� �������, �.�. ��� ��� �������� � ������ ���������
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;		// ������� ������������ I2C
	// ��������� ��� PB6, PB7
	GPIOB->MODER |= GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;			// ����� �������������� �������
	GPIOB->OTYPER |= GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7;				// �������� ���������
	GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR6 | GPIO_OSPEEDR_OSPEEDR7;	// ������������ ��������
	// ����� �������������� �������
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_1);	// I2C1_SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_1);	// I2C1_SDA
	
	I2C_BUS->CR1 &= ~I2C_CR1_PE;			// �������� I2C
	while (I2C_BUS->CR1 & I2C_CR1_PE) {};	// ��� ���� ����������
	
	// ������� ������������ ������ I2C = 48 ���
	// ������� ���� I2C = 100 kHz
	// ���������� ��������
	I2C_BUS->TIMINGR |= \
	((uint32_t)0xB  << I2C_OFFSET_TIMINGR_PRESC)|\
	((uint32_t)0x13 << I2C_OFFSET_TIMINGR_SCLL)|\
	((uint32_t)0xF  << I2C_OFFSET_TIMINGR_SCLH)|\
	((uint32_t)0x4  << I2C_OFFSET_TIMINGR_SCLDEL)|\
	((uint32_t)0x2  << I2C_OFFSET_TIMINGR_SDADEL);
	
	I2C_BUS->CR1 |= I2C_CR1_PE;					// ������� I2C
	while ((I2C_BUS->CR1 & I2C_CR1_PE)==0) {};	// ��� ���� ���������
}


/*
��� ��������� �������, ������������ � �� �����.
������������� ����������� ������ - ���� ��� ��������.
����� ����� ������������ ������.
����� ����� �������� ����������.
����� ����� �� ����.
���������:
Direction - ����������� (0-��������, 1-����)
Adress - ����� �������� ����������
Size - ������ ������ (�� 1 �� 255 ����)
*/
void I2C_Start_Direction_Adress_Size (I2C_Direction Direction, uint8_t Adress, uint8_t Size)
{
	//I2C_BUS->CR2 &= ~I2C_CR2_AUTOEND;				// �������� ���� �������
	//I2C_BUS->CR2 &= ~I2C_CR2_RELOAD;				// �� ������������ ����� ������������
	if (Direction) I2C_BUS->CR2 |= I2C_CR2_RD_WRN;	// ����� �����
	else I2C_BUS->CR2 &= ~I2C_CR2_RD_WRN;			// ����� ��������
	I2C_BUS->CR2 &= ~I2C_CR2_NBYTES;				// �������� ������ ������
	I2C_BUS->CR2 |= Size<<I2C_OFFSET_CR2_NBYTES;	// ���������� ������ ������
	I2C_BUS->CR2 &= ~I2C_CR2_SADD;	// �������� ����� �������� ����������
	I2C_BUS->CR2 |= Adress;			// ���������� ����� �������� ����������
	I2C_BUS->CR2 |= I2C_CR2_START;					// ������ ����� �� ����
	while ((I2C_BUS->ISR & I2C_ISR_BUSY)==0) {};	// ������� ������ ������
}


/*
��� ��������� �������, ������������ � �� �����.
����� ���� �� ����.
������� �����.
��������� ������� ������, ������� ����� ������.
*/
void I2C_Stop (void)
{
	I2C_BUS->CR2 |= I2C_CR2_STOP;				// ������ ���� �� ����
	while (I2C_BUS->ISR & I2C_ISR_BUSY) {};		// ������� ������ �����
	// ������ ����� - ���������� ��� ���������� ������ ����
	I2C_BUS->ICR |= I2C_ICR_STOPCF;		// STOP ����
	I2C_BUS->ICR |= I2C_ICR_NACKCF;		// NACK ����
	// ���� ���� ������ �� ���� - ������ �����
	if (I2C_BUS->ISR & (I2C_ISR_ARLO | I2C_ISR_BERR))
	{
		I2C_BUS->ICR |= I2C_ICR_ARLOCF;
		I2C_BUS->ICR |= I2C_ICR_BERRCF;
	}
}


/*
��������� ���������� ������ Size ���� � ������� Register �� ������ Adress.
���������:
Adress - ����� �������� ����������
Register - �������, � ������� ����� �������� ������
Data - ��������� ������ ����� ������ ��� ��������
Size - ������� ���� ����� �������� (�� 1 �� 254)
����������:
1 - ���� ������ ������� ��������
0 - ���� ��������� ������
*/
uint8_t I2C_Write_Transaction (uint8_t Adress, uint8_t Register, uint8_t *Data, uint8_t Size)
{
	uint8_t Count=0;	// ������� ������� ���������� ����
	// �����
	I2C_Start_Direction_Adress_Size (I2C_Transmitter, Adress, 1+Size);
	// ������ ���� I2C �������� ������ ���� ��� ��������,
	// ���� ������� NACK-����, ��������� � ���, ��� ���������� �� ��������.
	// ���� ������� NACK-����, �������� ����������.
	while ((((I2C_BUS->ISR & I2C_ISR_TXIS)==0) && ((I2C_BUS->ISR & I2C_ISR_NACKF)==0)) && (I2C_BUS->ISR & I2C_ISR_BUSY)) {};
	if (I2C_BUS->ISR & I2C_ISR_TXIS) I2C_BUS->TXDR=Register;	// ��������� ����� ��������
	
	// ���������� ����� �� ��� ���, ���� �� ������� TC-����.
	// ���� ������� NACK-����, �������� ����������.
	while ((((I2C_BUS->ISR & I2C_ISR_TC)==0) && ((I2C_BUS->ISR & I2C_ISR_NACKF)==0)) && (I2C_BUS->ISR & I2C_ISR_BUSY))
	{
		if (I2C_BUS->ISR & I2C_ISR_TXIS) I2C_BUS->TXDR=*(Data+Count++);	// ��������� ������
	}
	I2C_Stop();
	if (Count == Size) return 1; return 0;
}


/*
��������� ���������� ������ Size ���� �� �������� Register �� ������ Adress.
���������:
Adress - ����� �������� ����������
Register - �������, �� �������� ����� ������� ������
Data - ��������� ���� ���������� �������� ������
Size - ������� ���� ����� ������� (�� 1 �� 255)
����������:
1 - ���� ������ ������� �������
0 - ���� ��������� ������
*/
uint8_t I2C_Read_Transaction (uint8_t Adress, uint8_t Register, uint8_t *Data, uint8_t Size)
{
	uint8_t Count=0;	// ������� ������� �������� ����
	// �����
	I2C_Start_Direction_Adress_Size (I2C_Transmitter, Adress, 1);
	// ������ ���� I2C �������� ������ ���� ��� ��������,
	// ���� ������� NACK-����, ��������� � ���, ��� ���������� �� ��������.
	// ���� ������� NACK-����, �������� ����������.
	while ((((I2C_BUS->ISR & I2C_ISR_TC)==0) && ((I2C_BUS->ISR & I2C_ISR_NACKF)==0)) && (I2C_BUS->ISR & I2C_ISR_BUSY))
	{
		if (I2C_BUS->ISR & I2C_ISR_TXIS) I2C_BUS->TXDR = Register;	// ��������� ����� ��������
	}
	// ��������� �����
	I2C_Start_Direction_Adress_Size (I2C_Receiver, Adress, Size);
	// ��������� ����� �� ��� ���, ���� �� ������� TC-����.
	// ���� ������� NACK-����, ���� ����������.
	while ((((I2C_BUS->ISR & I2C_ISR_TC)==0) && ((I2C_BUS->ISR & I2C_ISR_NACKF)==0)) && (I2C_BUS->ISR & I2C_ISR_BUSY))
	{
		if (I2C_BUS->ISR & I2C_ISR_RXNE) *(Data+Count++) = I2C_BUS->RXDR;	// �������� ������
	}
	I2C_Stop();
	if (Count == Size) return 1; return 0;
}

