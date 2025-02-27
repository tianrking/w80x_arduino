#include "Arduino.h"
#include "wm_hal.h"
//#include "pins_arduino.h"

// array of timers
TIM_HandleTypeDef htim0;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

const PIN_MAP pin_Map[] =
{
    { GPIOA, GPIO_PIN_0,    PA0,    MUX_PA0},
    { GPIOA, GPIO_PIN_1,    PA1,    MUX_PA1},
    { GPIOA, GPIO_PIN_2,    PA2,    MUX_PA2},
    { GPIOA, GPIO_PIN_3,    PA3,    MUX_PA3},
    { GPIOA, GPIO_PIN_4,    PA4,    MUX_PA4},
    { GPIOA, GPIO_PIN_5,    PA5,    MUX_PA5},
    { GPIOA, GPIO_PIN_6,    PA6,    MUX_PA6},
    { GPIOA, GPIO_PIN_7,    PA7,    MUX_PA7},
    { GPIOA, GPIO_PIN_8,    PA8,    MUX_PA8},
    { GPIOA, GPIO_PIN_9,    PA9,    MUX_PA9},
    { GPIOA, GPIO_PIN_10,   PA10,   MUX_PA10},
    { GPIOA, GPIO_PIN_11,   PA11,   MUX_PA11},
    { GPIOA, GPIO_PIN_12,   PA12,   MUX_PA12},
    { GPIOA, GPIO_PIN_13,   PA13,   MUX_PA13},
    { GPIOA, GPIO_PIN_14,   PA14,   MUX_PA14},
    { GPIOA, GPIO_PIN_15,   PA15,   MUX_PA15},

    { GPIOB, GPIO_PIN_0,    PB0,    MUX_PB0},
    { GPIOB, GPIO_PIN_1,    PB1,    MUX_PB1},
    { GPIOB, GPIO_PIN_2,    PB2,    MUX_PB2},
    { GPIOB, GPIO_PIN_3,    PB3,    MUX_PB3},
    { GPIOB, GPIO_PIN_4,    PB4,    MUX_PB4},
    { GPIOB, GPIO_PIN_5,    PB5,    MUX_PB5},
    { GPIOB, GPIO_PIN_6,    PB6,    MUX_PB6},
    { GPIOB, GPIO_PIN_7,    PB7,    MUX_PB7},
    { GPIOB, GPIO_PIN_8,    PB8,    MUX_PB8},
    { GPIOB, GPIO_PIN_9,    PB9,    MUX_PB9},
    { GPIOB, GPIO_PIN_10,   PB10,   MUX_PB10},
    { GPIOB, GPIO_PIN_11,   PB11,   MUX_PB11},
    { GPIOB, GPIO_PIN_12,   PB12,   MUX_PB12},
    { GPIOB, GPIO_PIN_13,   PB13,   MUX_PB13},
    { GPIOB, GPIO_PIN_14,   PB14,   MUX_PB14},
    { GPIOB, GPIO_PIN_15,   PB15,   MUX_PB15},
    { GPIOB, GPIO_PIN_16,   PB16,   MUX_PB16},
    { GPIOB, GPIO_PIN_17,   PB17,   MUX_PB17},
    { GPIOB, GPIO_PIN_18,   PB18,   MUX_PB18},
    { GPIOB, GPIO_PIN_19,   PB19,   MUX_PB19},
    { GPIOB, GPIO_PIN_20,   PB20,   MUX_PB20},
    { GPIOB, GPIO_PIN_21,   PB21,   MUX_PB21}, 
    { GPIOB, GPIO_PIN_22,   PB22,   MUX_PB22},
    { GPIOB, GPIO_PIN_23,   PB23,   MUX_PB23},
    { GPIOB, GPIO_PIN_24,   PB24,   MUX_PB24},
    { GPIOB, GPIO_PIN_25,   PB25,   MUX_PB25}, 
    { GPIOB, GPIO_PIN_26,   PB26,   MUX_PB26},
    { GPIOB, GPIO_PIN_27,   PB27,   MUX_PB27}
};


I2S_HandleTypeDef hi2s;



// Массив для фиксации занятых каналов PWM
typedef struct {
    uint8_t channel;
    uint8_t pin;
    PWM_HandleTypeDef hpwm;
	bool isPresent;
	uint8_t lastDuty;
} pwmPinStateTD;
pwmPinStateTD pwmPinState[PWM_COUNT] = { 0 };
//PWM_HandleTypeDef pwm[PWM_COUNT] = { 0 };

// Массив для фиксации занятых каналов ADC
typedef struct
{
    uint32_t channel;
    uint8_t pin;
    ADC_HandleTypeDef hadc;
	bool isPresent;
} adcPinStateTD;
adcPinStateTD adcPinState[ADC_COUNT] = {0};
//ADC_HandleTypeDef adc[ADC_COUNT] = { 0 };

// Прототипы функций

void ADC_Init(ADC_HandleTypeDef* hadc, uint32_t channel);
bool check_pin(void);

// Задержка в микросекундах
void delayMicroseconds(uint32_t us) {
    HAL_Delay(us);
}
// Задержка в миллисекундах
void delay(uint32_t ms) {
	delayMicroseconds(ms*1000);
}
// Время upTime в микросекундах
uint32_t micros() {
	return HAL_GetTick();
}
// Время upTime в миллисекундах
uint32_t millis() {
	return HAL_GetTick() / 1000;
}

// Установка режимов ввода/вывода
void pinMode(uint8_t pin, uint8_t mode)
{
    uint8_t pwm_channel = 0;
	uint32_t adc_channel = 0;
	uint8_t adc_index = 0;
	
    //if(!is_loop()) {
		if(pin_Map[pin].ulPinAttribute != NONE) {
			if (check_pin()) {
				if(pin_Map[pin].ulPinAttribute & PIN_DIO_Msk) {
					if ((mode == INPUT) || (mode == OUTPUT) || (mode == INPUT_PULLUP) || (mode == INPUT_PULLDOWN))
					{
						GPIO_InitTypeDef GPIO_InitStruct = {0};
						GPIO_InitStruct.Pin = pin_Map[pin].halPin;
						switch (mode)
						{
						case INPUT:
							GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
							GPIO_InitStruct.Pull = GPIO_NOPULL;
							break;
						case INPUT_PULLUP:
							GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
							GPIO_InitStruct.Pull = GPIO_PULLUP;
							break;
						case INPUT_PULLDOWN:
							GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
							GPIO_InitStruct.Pull = GPIO_PULLDOWN;
							break;
						case OUTPUT:
							GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT;
							GPIO_InitStruct.Pull = GPIO_NOPULL;
							break;
						default:
							break;
						}
						if (!is_gpio_clk_en)
						{
							__HAL_RCC_GPIO_CLK_ENABLE();
							is_gpio_clk_en = true;
						}
						HAL_GPIO_Init(pin_Map[pin].pPort, &GPIO_InitStruct);
					}
				}
				if(mode == ANALOG_INPUT) {
					if(pin_Map[pin].ulPinAttribute & PIN_ADC_Msk) {
						switch(pin_Map[pin].ulPinAttribute & PIN_ADC_Msk) {
							case ADC1:
								adc_channel = ADC_CHANNEL_0;
								adc_index = 0;
								__HAL_AFIO_REMAP_ADC(GPIOA, GPIO_PIN_1);
								break;
							case ADC2:
								adc_channel = ADC_CHANNEL_1;
								adc_index = 1;
								__HAL_AFIO_REMAP_ADC(GPIOA, GPIO_PIN_4);
								break;
							case ADC3:
								adc_channel = ADC_CHANNEL_2;
								adc_index = 2;
								__HAL_AFIO_REMAP_ADC(GPIOA, GPIO_PIN_3);
								break;
							case ADC4:
								adc_channel = ADC_CHANNEL_3;
								adc_index = 3;
								__HAL_AFIO_REMAP_ADC(GPIOA, GPIO_PIN_2);
								break;
							default:
								break;
						}
						if(!is_adc_clk_en) {
							__HAL_RCC_ADC_CLK_ENABLE();
							is_adc_clk_en = true;
						}
						adcPinState[adc_index].channel = adc_channel;
						adcPinState[adc_index].pin = pin;
						//adcPinState[adc_index].hadc = adc[adc_index];
						adcPinState[adc_index].isPresent = true;
					} else {
					printf("Вывод %d не работает в АЦП режиме! \n\r", pin);}
				}
				if(mode == PWM_OUT) {
					if(pin_Map[pin].ulPinAttribute & PIN_PWM_Msk) {
						switch(pin_Map[pin].ulPinAttribute & PIN_PWM_Msk) {
							case PWM0:
								pwm_channel = PWM_CHANNEL_0;
								//__HAL_AFIO_REMAP_PWM0(pin_Map[pin].pPort, pin_Map[pin].halPin);
								break;
							case PWM1:
								pwm_channel = PWM_CHANNEL_1;
								//__HAL_AFIO_REMAP_PWM1(pin_Map[pin].pPort, pin_Map[pin].halPin);
								break;
							case PWM2:
								pwm_channel = PWM_CHANNEL_2;
								//__HAL_AFIO_REMAP_PWM2(pin_Map[pin].pPort, pin_Map[pin].halPin);
								break;
							case PWM3:
								pwm_channel = PWM_CHANNEL_3;
								//__HAL_AFIO_REMAP_PWM3(pin_Map[pin].pPort, pin_Map[pin].halPin);
								break;
							case PWM4:
								pwm_channel = PWM_CHANNEL_4;
								//__HAL_AFIO_REMAP_PWM4(pin_Map[pin].pPort, pin_Map[pin].halPin);
								break;
							
							default:
								break;
						}
						if(!is_pwm_clk_en) {
							__HAL_RCC_PWM_CLK_ENABLE();
							is_pwm_clk_en = true;
						}
						// Setup the pin as GPIO OUTPUT and set it LOW
						// to set zero duty at PWM init.
						// We need it because PWM always produce pulses even with Duty = 0
						pinMode(pin, OUTPUT);
						digitalWrite(pin, LOW);
						pwmPinState[pwm_channel].channel = pwm_channel;
						pwmPinState[pwm_channel].pin = pin;
						//pwmPinState[pwm_channel].hpwm = pwm[pwm_channel];
						if (pwmPinState[pwm_channel].isPresent) {
							HAL_PWM_Stop(&(pwmPinState[pwm_channel].hpwm));
							HAL_PWM_DeInit(&(pwmPinState[pwm_channel].hpwm));
                            }
						pwmPinState[pwm_channel].isPresent = true;
						pwmPinState[pwm_channel].lastDuty = 0;
						PWM_Init(&(pwmPinState[pwm_channel].hpwm), pwm_channel);
					} else {
						printf("Вывод %d не работает в ШИМ режиме! \n\r", pin);
					}
				}
			} else {
				printf("Повторное использование вывода %d не допустимо! \n\r", pin);
			}
		} else {
			printf("Вывод %d не конфигурируется! \n\r", pin);
		}	
    /*} else {
		printf("Конфигурирование выводов в Loop запрещено! \n\r");
    }*/
}

// Запись/чтение цифрового выхода/входа
void digitalWrite(uint8_t pin, uint8_t val) {
	
    HAL_GPIO_WritePin(pin_Map[pin].pPort, pin_Map[pin].halPin, (GPIO_PinState)val);
}

uint8_t digitalRead(uint8_t pin) {
	
    return HAL_GPIO_ReadPin(pin_Map[pin].pPort, pin_Map[pin].halPin);
}

void digitalToggle(uint8_t pin) {
	
    HAL_GPIO_TogglePin(pin_Map[pin].pPort, pin_Map[pin].halPin);
}


PWM_HandleTypeDef* getPWMHandle(uint8_t pin) {
	for(uint8_t i = 0; i < PWM_COUNT; i++) {
		if(pwmPinState[i].pin == pin) {
			return(&(pwmPinState[i].hpwm));
		}
	}
	return NULL;
}
// PWM Duty setup
void analogWrite(uint8_t pin, uint8_t val)
{

	for (uint8_t i = 0; i < PWM_COUNT; i++)
	{
		if (pwmPinState[i].pin == pin)
		{
			if (val == pwmPinState[i].lastDuty) {   
				return;    // value not changed, nothing to do
			}
			
			if (val && (pwmPinState[i].lastDuty == 0))
			{
					switch (pwmPinState[i].channel)
					{
					case PWM_CHANNEL_0:
						__HAL_AFIO_REMAP_PWM0(pin_Map[pin].pPort, pin_Map[pin].halPin);
						break;
					case PWM_CHANNEL_1:
						__HAL_AFIO_REMAP_PWM1(pin_Map[pin].pPort, pin_Map[pin].halPin);
						break;
					case PWM_CHANNEL_2:
						__HAL_AFIO_REMAP_PWM2(pin_Map[pin].pPort, pin_Map[pin].halPin);
						break;
					case PWM_CHANNEL_3:
						__HAL_AFIO_REMAP_PWM3(pin_Map[pin].pPort, pin_Map[pin].halPin);
						break;
					case PWM_CHANNEL_4:
						__HAL_AFIO_REMAP_PWM4(pin_Map[pin].pPort, pin_Map[pin].halPin);
						break;
					}
					
			}
			else if ((val == 0) && (pwmPinState[i].lastDuty != 0))
			{
					// Setup the pin as GPIO OUTPUT and set it LOW
					// to set zero duty at PWM channel.
					// We need it because PWM always produce pulses even with Duty = 0
					pinMode(pin, OUTPUT);
					digitalWrite(pin, LOW);
					
			}
			pwmPinState[i].lastDuty = val;
			HAL_PWM_Duty_Set(&(pwmPinState[i].hpwm), val);
			pwmPinState[i].hpwm.Init.Pulse = val;
		}
	}
}

// Инициализация АЦП
/*void setup_adc() {
	for(uint8_t i = 0; i < ADC_COUNT; i++) {
		if (adcPinState[i].isPresent == true) {
			ADC_Init(&(adcPinState[i].hadc), adcPinState[i].channel);
		}
	}
}*/
// Чтение данных АЦП
double analogRead(uint8_t pin) {
	double value = 0.0;
    for(uint8_t i = 0; i < ADC_COUNT; i++) {
		if(adcPinState[i].pin == pin) {
			ADC_Init(&(adcPinState[i].hadc), adcPinState[i].channel);
			value = HAL_ADC_GET_INPUT_VOLTAGE(&(adcPinState[i].hadc));
		}
    }
	return value;
}

void ADC_Init(ADC_HandleTypeDef* hadc, uint32_t channel) {
    hadc->Instance = ADC;
    hadc->Init.channel = channel;
    hadc->Init.freq = 1000;
    //HAL_ADC_Init(hadc);
}

//Встроенный криптомодуль ядра
void trngInit(void) {
	RCC->CLK_EN |= 1<<17;            //Тактирование 1-Вкл, 0-Выкл
    GPSEC->KEY0 = 31415926ul;         //Загрузка какого либо числа
    GPSEC->TRNG_CR =   (4<<3) | (0<<2) | (1<<1) | (1<<0) | 0x00;	//Магия! Все манипуляции с регистрами взяты из wm-w80x-sdk
}
void prngInit(void) {
    RCC->CLK_EN |= 1<<17;            
    GPSEC->KEY0 = 0x31415926ul;  
    GPSEC->CFG =   (1<<30) | (1<<29) |  (1<<28) |  0x00;
    GPSEC->TRNG_CR =   (4<<3) | (0<<2) |  (0<<1) |  (1<<0) | 0x00;
}

// Добавлено для совместимости wiring shiftOut/shiftIn
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
	uint8_t value = 0;
	uint8_t i;

	for (i = 0; i < 8; ++i) {
		digitalWrite(clockPin, HIGH);
		if (bitOrder == LSBFIRST)
			value |= digitalRead(dataPin) << i;
		else
			value |= digitalRead(dataPin) << (7 - i);
		digitalWrite(clockPin, LOW);
	}
	return value;
}
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
	uint8_t i;

	for (i = 0; i < 8; i++)  {
		if (bitOrder == LSBFIRST) {
			digitalWrite(dataPin, val & 1);
			val >>= 1;
		} else {	
			digitalWrite(dataPin, (val & 128) != 0);
			val <<= 1;
		}
			
		digitalWrite(clockPin, HIGH);
		digitalWrite(clockPin, LOW);		
	}
}

// ToDo: Контроль коллизий в использовании выводов платы
bool check_pin() {
	return true;
}

// added for compatibility
__attribute__((weak)) void yield() {}

