// NOTE: this file is cheap to include. Include at will

/* Clock enable */
#define RCC_BASE  0x40023800UL // RCC base address. Note: end all hex address defines with UL (unsigned long)
#define RCC_AHB1ENR  (*(volatile unsigned int *)(RCC_BASE + 0x30)) // AHB1ENR is at offset 0x30. GPIOA/D lives here
#define RCC_APB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x40)) // APB1ENR is at offset 0x40. UART2 lives here.
#define RCC_APB2ENR (*(volatile unsigned int *)(RCC_BASE + 0x44)) // APB2ENR is at offset 0x44. SYSCFG lives here

/* GPIOA */
#define GPIOA_BASE 0x40020000UL // GPIOA base address. PA2 and PA3 (for UART comms) live here, also PA0 blue button
#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_BASE + 0x00)) //GPIOA_MODER is the first register in GPIOA
#define GPIOA_AFRL (*(volatile unsigned int *)(GPIOA_BASE + 0x20)) // also define the alternate function low register offset
#define GPIOA_PUPDR (*(volatile unsigned int *)(GPIOA_BASE + 0x0C)) // pull-up pull-down register (for button)
/* GPIOD */
#define GPIOD_BASE  0x40020C00UL // GPIOD base address (from memory map). LEDs live here. UL = unsigned long, compiler treats it as 32-bit value rather than signed integer
#define GPIOD_MODER (*(volatile unsigned int *)(GPIOD_BASE + 0x00))// GPIOD_MODER is the first register in GPIOD
#define GPIOD_ODR (*(volatile unsigned int *)(GPIOD_BASE + 0x14)) // GPIOD_ODR (output data register) is at offset 0x14
#define GPIOD_BSRR (*(volatile unsigned int *)(GPIOD_BASE + 0x18)) // GPIOD_BSRR (bit set/reset register) is at offset 0x18
#define GPIOD_AFRH (*(volatile unsigned int *)(GPIOD_BASE + 0x24)) // alternate function high (GPIOD pins 8-15) register offset

/* USART2 */
#define USART2_BASE 0x40004400UL // USART2 base address (from memory map)
#define USART2_BRR (*(volatile unsigned int *)(USART2_BASE + 0x08)) // BRR (baud rate register)
#define USART2_CR1 (*(volatile unsigned int *)(USART2_BASE + 0x0C)) // CR1 (control register 1)
#define USART2_SR (*(volatile unsigned int *)(USART2_BASE + 0x00)) // SR (status register)
#define USART2_DR (*(volatile unsigned int *)(USART2_BASE + 0x04)) // DR (data register)

/* SYSCFG */
#define SYSCFG_BASE 0x40013800UL // SYSCFG base address
#define SYSCFG_EXTICR1 (*(volatile unsigned int *)(SYSCFG_BASE + 0x08)) // EXTICR1 (external interrupt control register)
/* EXTI */
#define EXTI_BASE 0x40013C00UL // EXTI base address
#define EXTI_IMR (*(volatile unsigned int *)(EXTI_BASE + 0x00)) // interrupt mask register
#define EXTI_RTSR (*(volatile unsigned int *)(EXTI_BASE + 0x08)) // rising trigger selection register
#define EXTI_FTSR (*(volatile unsigned int *)(EXTI_BASE + 0x0C)) // falling trigger selection register
#define EXTI_PR (*(volatile unsigned int *)(EXTI_BASE + 0x14)) // pending register

/* NVIC & SysTick: found in programming manual */
#define NVIC_BASE 0xE000E100UL // NVIC base address --> in 'System Control Space', same for all Cortex-M4 chips
#define NVIC_ISER0 (*(volatile unsigned int *)(*(volatile unsigned int *)(NVIC_BASE + 0x00))) // interrupt set enable register
#define NVIC_IPR (*(volatile unsigned int *)(*(volatile unsigned int *)(NVIC_BASE + 0x300))) // interrupt priority register
#define SYSTICK_BASE 0xE000E010UL // SysTack base address
#define STK_CTRL (*(volatile unsigned int *)(*(volatile unsigned int *)(SYSTICK_BASE + 0x00))) // SysTick control and status register
#define STK_LOAD (*(volatile unsigned int *)(*(volatile unsigned int *)(SYSTICK_BASE + 0x04))) // SysTick reload value register
#define STK_VAL (*(volatile unsigned int *)(*(volatile unsigned int *)(SYSTICK_BASE + 0x08))) // SysTick current value register

/* TIMERS */
#define TIM2_BASE 0x40000000UL // timer 2
#define TIM4_BASE 0x40000800UL // timer 4
#define TIM4_PSC (*(volatile unsigned int *)(TIM4_BASE + 0x28)) // TIM4 prescaler register
#define TIM4_ARR (*(volatile unsigned int *)(TIM4_BASE + 0x2C)) // TIM4 auto-reload register
#define TIM4_CCR1 (*(volatile unsigned int *)(TIM4_BASE + 0x34)) // TIM4 capture/compare register 1
#define TIM4_CCMR1 (*(volatile unsigned int *)(TIM4_BASE + 0x18)) // TIM4 capture/compare mode register 1
#define TIM4_CCER (*(volatile unsigned int *)(TIM4_BASE + 0x20)) // TIM4 capture/compare enable register
#define TIM4_CR1 (*(volatile unsigned int *)(TIM4_BASE + 0x00)) // TIM4 control register 1

/* ADC */
#define ADC1_BASE 0x40012000UL // analog 2 digital converter 1
#define ADC1_SQR3 (*(volatile unsigned int *)(ADC1_BASE + 0x34)) // regular sequence register 3
#define ADC1_SQR1 (*(volatile unsigned int *)(ADC1_BASE + 0x2C)) // regular sequence register 1
#define ADC1_SMPR2 (*(volatile unsigned int *)(ADC1_BASE + 0x10)) // sample time register
#define ADC1_CR2 (*(volatile unsigned int *)(ADC1_BASE + 0x08)) // control register
#define ADC1_SR (*(volatile unsigned int *)(ADC1_BASE + 0x00)) // status register
#define ADC1_DR (*(volatile unsigned int *)(ADC1_BASE + 0x4C)) // data register