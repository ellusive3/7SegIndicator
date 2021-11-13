/**
* @short
*	Вспомогательная библиотека для работы с семисегментными индикаторами с точкой.  Возможность управлять множеством разрядов индикатора, если сдвиговые регистры соединены
*	в последовательную цепь. Возможность подключения к МК STM32 и AVR (Arduino). Библиотека умеет выводить на индикаторы целые числа, вещественные числа с автоматическим определением 
*	размера дробной части, отрицательные и неотрицательные значения, буквы латинского алфавита.
* @author
*	Zyukov A.V.
* @editor
*	Zyukov A.V.
* @date
*	31-October-2021
*/

#define USE_STM32F4XX
#if defined USE_STM32F4XX
#define __STM32F4XX__
#undef __ARDUINO__
#elif defined USE_ARDUINO
#undef __STM32F4XX__
#define __ARDUNO__
#endif
#ifdef __STM32F4XX__
	#include "stm32f4xx_hal.h"
#endif
#define __USE_C99_MATH

#include <stdbool.h>

/**
 * Состояние светодиодов:
 * 1 - не горит
 * 0 - горит
*/
#define ON 0
#define OFF 1

#define I0_N ON << 0 | ON << 1 | ON << 2 | ON << 3 | ON << 4 | ON << 5 | OFF << 6 | OFF << 7  // OxC0
#define I0D_N ON << 0 | ON << 1 | ON << 2 | ON << 3 | ON << 4 | ON << 5 | OFF << 6 | ON << 7  // Ox40
#define I1_N OFF << 0 | ON << 1 | ON << 2 | OFF << 3 | OFF << 4 | OFF << 5 | OFF << 6 | OFF << 7 // 0xF9
#define I1D_N OFF << 0 | ON << 1 | ON << 2 | OFF << 3 | OFF << 4 | OFF << 5 | OFF << 6 | ON << 7 // 0x79
#define I2_N ON << 0 | ON << 1 | OFF << 2 | ON << 3 | ON << 4 | OFF << 5 | ON << 6 | OFF << 7 // 0xA4
#define I2D_N ON << 0 | ON << 1 | OFF << 2 | ON << 3 | ON << 4 | OFF << 5 | ON << 6 | ON << 7 // 0x24
#define I3_N ON << 0 | ON << 1 | ON << 2 | ON << 3 | OFF << 4 | OFF << 5 | ON << 6 | OFF << 7 // 0xB0
#define I3D_N ON << 0 | ON << 1 | ON << 2 | ON << 3 | OFF << 4 | OFF << 5 | ON << 6 | ON << 7 // 0x30
#define I4_N OFF << 0 | ON << 1 | ON << 2 | OFF << 3 | OFF << 4 | ON << 5 | ON << 6 | OFF << 7 // 0x99
#define I4D_N OFF << 0 | ON << 1 | ON << 2 | OFF << 3 | OFF << 4 | ON << 5 | ON << 6 | ON << 7 // 0x19
#define I5_N ON << 0 | OFF << 1 | ON << 2 | ON << 3 | OFF << 4 | ON << 5 | ON << 6 | OFF << 7 // 0x92
#define I5D_N ON << 0 | OFF << 1 | ON << 2 | ON << 3 | OFF << 4 | ON << 5 | ON << 6 | ON << 7 // 0x12
#define I6_N ON << 0 | OFF << 1 | ON << 2 | ON << 3 | ON << 4 | ON << 6 | ON << 7 | OFF << 7 // 0x82
#define I6D_N ON << 0 | OFF << 1 | ON << 2 | ON << 3 | ON << 4 | ON << 6 | ON << 7 | ON << 7 // 0x2
#define I7_N ON << 0 | ON << 1 | ON << 2 | OFF << 3 | OFF << 4 | OFF << 6 | OFF << 7 | OFF << 7 // 0xD8
#define I8_N ON << 0 | ON << 1 | ON << 2 | ON << 3 | ON << 4 | ON << 6 | ON << 7 | OFF << 7 // 0x80
#define I8D_N ON << 0 | ON << 1 | ON << 2 | ON << 3 | ON << 4 | ON << 6 | ON << 7 | ON << 7 // 0x0
#define I9_N ON << 0 | ON << 1 | ON << 2 | ON << 3 | OFF << 4 | ON << 6 | ON << 7 | OFF << 7 // 0x90
#define I9D_N ON << 0 | ON << 1 | ON << 2 | ON << 3 | OFF << 4 | ON << 6 | ON << 7 | ON << 7 // 0x16
#define IMINUS OFF << 0 | OFF << 1 | OFF << 2 | OFF << 3 | OFF << 4 | OFF << 6 | ON << 7 | OFF << 7 // 0x90
#define IEMPTY_N OFF << 0 | OFF << 1 | OFF << 2 | OFF << 3 | OFF << 4 | OFF << 5 | OFF << 6 | OFF << 7  // OxFF
#define IF_SYMBOL ON << 0 | OFF << 1 | OFF << 2 | OFF << 3 | ON << 4 | ON << 5 | ON << 6 | OFF << 7  // Ox8E
#define IO_SYMBOL OFF << 0 | OFF << 1 | ON << 2 | ON << 3 | ON << 4 | OFF << 5 | ON << 6 | OFF << 7  // OxA3

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
#ifdef __STM32F4XX__
	GPIO_TypeDef * PinPort;
#endif
	uint16_t PinNumber;
} Pin;

typedef struct {
  Pin * pins[2];
  uint32_t maxAllowedValue;
  uint8_t countOfDigits;
} Indicator;

/**
* @short
*	Инициализация структуры индикатора
* @param[in] _indicator - указатель на инициализируемую структуру индикатора
* @param[in] _dataPin - указатель на структуру вывода, по которому передаются данные на сдвиговый регистр
* @param[in] _clockPin - указатель на структуру вывода, по которому осуществляется тактирование индикатора
* @param[in] _countOfDigits - количество соединенных вместе индикаторов
*/
void InitIndicatorStruct(Indicator * _indicator, Pin * _dataPin,  Pin * _clockPin, uint8_t _countOfDigits);

/**
* @short
*	Вывод целого числа на индикаторе
* @detail
*	Функция вывода целого числа на индикатор. Число не должно превышать максимальное значение, которое
*	может поместиться на всех рязрядах индикатора. В случае переполнения на индикаторе отобразится надпись "Of"
* @param[in] _indicator - указатель на структуру индикатора
* @param[in] _digit - число для вывода на индикатор
*/
void IND_WriteDigit(Indicator * _indicator,  int _digit);

/**
* @short
*	Вывод вещественного числа на индикаторе
* @detail
*	Функция вывода вещественного числа на индикатор. Число не должно превышать максимальное значение, которое
*	может поместиться на всех рязрядах индикатора. В случае переполнения на индикаторе отобразится надпись "Of"
* @param[in] _indicator - указатель на структуру индикатора
* @param[in] _value - число для вывода на индикатор
*/
void IND_WriteFloat(Indicator * _indicator,  float _value);

/**
* @short
*	Функция очистки значения на индикаторе
* @param[in] _indicator - указатель на структуру индикатора
*/
void IND_FlushIndication(Indicator * _indicator);
#ifdef __cplusplus
}
#endif

void IND_WriteBit(Indicator * _indicator, bool _writtingBit);

void IND_WriteNum(Indicator * _indicator, uint8_t _writtingNumber);

void IND_WriteByte(Indicator * _indicator, uint8_t _writtingByte);
