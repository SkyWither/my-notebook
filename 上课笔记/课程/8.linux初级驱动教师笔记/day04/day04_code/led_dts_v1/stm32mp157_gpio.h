#ifndef __STM32MP157_GPIO_H__
#define __STM32MP157_GPIO_H__

typedef struct {
	volatile unsigned int MODER;   // 0x00
	volatile unsigned int OTYPER;  // 0x04
	volatile unsigned int OSPEEDR; // 0x08
	volatile unsigned int PUPDR;   // 0x0C
	volatile unsigned int IDR;     // 0x10
	volatile unsigned int ODR;     // 0x14
	volatile unsigned int BSRR;    // 0x18
	volatile unsigned int LCKR;    // 0x1C
	volatile unsigned int AFRL;    // 0x20
	volatile unsigned int AFRH;    // 0x24
	volatile unsigned int BRR;     // 0x28
	//volatile unsigned int res;      
	volatile unsigned int SECCFGR; // 0x30

}gpio_t;

#define  GPIOA   0x50002000
#define  GPIOB   0x50003000
#define  GPIOC   0x50004000
#define  GPIOD   0x50005000
#define  GPIOE   0x50006000
#define  GPIOF   0x50007000
#define  GPIOG   0x50008000
#define  GPIOH   0x50009000
#define  GPIOI   0x5000A000
#define  GPIOJ   0x5000B000
#define  GPIOK   0x5000C000
#define  GPIOZ   0x54004000


#endif // __STM32MP157_GPIO_H__
