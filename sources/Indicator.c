#include "Indicator.h"

void InitIndicatorStruct(Indicator * _indicator, Pin * _dataPin,  Pin * _clockPin, uint8_t _countOfDigits) {
	_indicator->maxAllowedValue = 0;
	uint32_t multiplier = 1;
	for (uint8_t i = 0; i < _countOfDigits; i++) {
		_indicator->maxAllowedValue += (9 * multiplier);
		multiplier *=  10;
	}
	_indicator->countOfDigits = _countOfDigits;
	_indicator->pins[0] = _dataPin;
	_indicator->pins[1] = _clockPin;
#if defined __STM32F4XX__
	// Настраиваем пины тактирования и данных на выход
	GPIO_InitTypeDef GPIO_InitStructData = {0};
	GPIO_InitStructData.Pin = _indicator->pins[0]->PinNumber; //Пин данных
	GPIO_InitStructData.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructData.Pull = GPIO_NOPULL;
	GPIO_InitStructData.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(_indicator->pins[0]->PinPort, &GPIO_InitStructData);

	GPIO_InitTypeDef GPIO_InitStructClock = {0};
	GPIO_InitStructClock.Pin = _indicator->pins[1]->PinNumber; //Пин тактирования
	GPIO_InitStructClock.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructClock.Pull = GPIO_NOPULL;
	GPIO_InitStructClock.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(_indicator->pins[1]->PinPort, &GPIO_InitStructClock);

	HAL_GPIO_WritePin(_indicator->pins[0]->PinPort, _indicator->pins[0]->PinNumber, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(_indicator->pins[1]->PinPort, _indicator->pins[1]->PinNumber, GPIO_PIN_RESET);
#elif defined __ARDUINO__
	writeDigit(_indicator->pins[0]->PinNumber, LOW);
	writeDigit(_indicator->pins[1]->PinNumber, LOW);
#endif
	IND_FlushIndication(_indicator);
}

void IND_WriteDigit(Indicator * _indicator,  int _digit) {
	if (_digit < 0) {
		if (-_digit > (_indicator->maxAllowedValue / 10)) {
			IND_WriteByte(_indicator, IO_SYMBOL);
			IND_WriteByte(_indicator, IF_SYMBOL);
		}
		else {
			IND_WriteByte(_indicator, IMINUS);
			_digit = -_digit;
			uint32_t multiplier = 1;
			uint8_t currentNumber = 0;
			for (uint8_t i = 0; i < _indicator->countOfDigits - 2; i++) {
				multiplier *= 10;
			}
			currentNumber = _digit / multiplier;
			if (currentNumber > 0)
				IND_WriteNum(_indicator, currentNumber);
			multiplier /= 10;
			while (multiplier >= 10)  {
				currentNumber = _digit / multiplier % 10;
				if (currentNumber > 0)
					IND_WriteNum(_indicator, currentNumber);
				multiplier /= 10;
			}
			currentNumber = _digit % 10;
			IND_WriteNum(_indicator, currentNumber);
		}
	}
	else {
			// На вход передают целое десятичное неотрицательное число. Необходимо преобразовать его в битовую маску для каждого светодиода в соответствии с таблицей.
		if (_digit > _indicator->maxAllowedValue) { // Если передали число, больше чем мы можем вывести, то пишем "Overflow"
			IND_WriteByte(_indicator, IO_SYMBOL);
			IND_WriteByte(_indicator, IF_SYMBOL);
		}
		else {
			// Укладываемся в доступные разряды. Парсим переданное число поразрядно и каждый разряд,
			// начиная со старшего пишем последовательно на цепочку индикаторов
			uint32_t multiplier = 1;
			uint8_t currentNumber = 0;
			for (uint8_t i = 0; i < _indicator->countOfDigits - 1; i++) {
				multiplier *= 10;
			}
			currentNumber = _digit / multiplier;
			if (currentNumber > 0)
				IND_WriteNum(_indicator, currentNumber);
			multiplier /= 10;
			while (multiplier >= 10)  {
				currentNumber = _digit / multiplier % 10;
				if (currentNumber > 0)
					IND_WriteNum(_indicator, currentNumber);
				multiplier /= 10;
			}
			currentNumber = _digit % 10;
			IND_WriteNum(_indicator, currentNumber);
		}
	}
}

void IND_WriteFloat(Indicator * _indicator,  float _value) {
	if (_value < 0) {
		if (-(int)_value > (_indicator->maxAllowedValue / 10)) {
			IND_WriteByte(_indicator, IO_SYMBOL);
			IND_WriteByte(_indicator, IF_SYMBOL);
		}
	}
	else {
		if ((int)_value > _indicator->maxAllowedValue) {
			IND_WriteByte(_indicator, IO_SYMBOL);
			IND_WriteByte(_indicator, IF_SYMBOL);
		}
		else {
		}
	}
}

void IND_FlushIndication(Indicator * _indicator) {
	IND_WriteByte(_indicator, IEMPTY_N);
}

void IND_WriteBit(Indicator * _indicator, bool _writtingBit) {
	if (_writtingBit)
#if defined __STM32F4XX__
		HAL_GPIO_WritePin(_indicator->pins[0]->PinPort, _indicator->pins[0]->PinNumber, GPIO_PIN_SET);
#elif defined __ARDUINO__
		WriteDigit(_indicator->pins[0]->PinNumber, HIGH);
#endif
	else
#if defined __STM32F4XX__
		HAL_GPIO_WritePin(_indicator->pins[0]->PinPort, _indicator->pins[0]->PinNumber, GPIO_PIN_RESET);
#elif defined __ARDUINO__
		WriteDigit(_indicator->pins[0]->PinNumber, LOW);
#endif
#if defined __STM32F4XX__
	HAL_GPIO_WritePin(_indicator->pins[1]->PinPort, _indicator->pins[1]->PinNumber, GPIO_PIN_SET);
    HAL_GPIO_WritePin(_indicator->pins[1]->PinPort, _indicator->pins[1]->PinNumber, GPIO_PIN_RESET);
#elif defined __ARDUINO__
    WriteDigit(_indicator->pins[1]->PinNumber, HIGH);
    WriteDigit(_indicator->pins[1]->PinNumber, LOW);
#endif
}

void IND_WriteNum(Indicator * _indicator, uint8_t _writtingNumber) {
    switch (_writtingNumber) {
      case 0: {IND_WriteByte(_indicator, I0_N); break;}
      case 1: {IND_WriteByte(_indicator, I1_N); break;}
      case 2: {IND_WriteByte(_indicator, I2_N); break;}
      case 3: {IND_WriteByte(_indicator, I3_N); break;}
      case 4: {IND_WriteByte(_indicator, I4_N); break;}
      case 5: {IND_WriteByte(_indicator, I5_N); break;}
      case 6: {IND_WriteByte(_indicator, I6_N); break;}
      case 7: {IND_WriteByte(_indicator, I7_N); break;}
      case 8: {IND_WriteByte(_indicator, I8_N); break;}
      case 9: {IND_WriteByte(_indicator, I9_N); break;}
      default: {IND_WriteByte(_indicator, I0_N); break;}
    }
  }

void IND_WriteByte(Indicator * _indicator, uint8_t _writtingByte) {
    for (int i = 0; i < 8; i++) {
    	IND_WriteBit(_indicator, (_writtingByte >> i) & 1u);
    }
  }
