/* Base Addresses */
#define RCC_BASE  0x40023800UL // RCC base address (from the memory map chapter). Note: end all hex address defines with UL (unsigned long)
#define GPIOA_BASE 0x40020000UL // GPIOA base address. PA2 and PA3 (for UART comms) live here, also PA0 blue button
#define GPIOD_BASE  0x40020C00UL // GPIOD base address (from memory map). LEDs live here. UL = unsigned long, compiler treats it as 32-bit value rather than signed integer
#define USART2_BASE 0x40004400UL // USART2 base address (from memory map)
#define SYSCFG_BASE 0x40013800UL // SYSCFG base address
#define EXTI_BASE 0x40013C00UL // EXTI base address
#define NVIC_BASE 0xE000E100UL // NVIC base address --> in 'System Control Space', same for all Cortex-M4 chips
#define SysTick_Base 0xE000E010 // SysTack base address

/* Clock enable */
#define RCC_AHB1ENR  (*(volatile unsigned int *)(RCC_BASE + 0x30)) // AHB1ENR is at offset 0x30. GPIOA/D lives here
#define RCC_APB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x40)) // APB1ENR is at offset 0x40. UART2 lives here.
#define RCC_APB2ENR (*(volatile unsigned int *)(RCC_BASE + 0x44)) // APB2ENR is at offset 0x44. SYSCFG lives here

/* GPIOA */
#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_BASE + 0x00)) //GPIOA_MODER is the first register in GPIOA
#define GPIOA_AFRL (*(volatile unsigned int *)(GPIOA_BASE + 0x20)) // also define the alternate function low register offset
#define GPIOA_PUPDR (*(volatile unsigned int *)(GPIOA_BASE + 0x0C)) // pull-up pull-down register (for button)
/* GPIOD */
#define GPIOD_MODER (*(volatile unsigned int *)(GPIOD_BASE + 0x00))// GPIOD_MODER is the first register in GPIOD
#define GPIOD_ODR (*(volatile unsigned int *)(GPIOD_BASE + 0x14)) // GPIOD_ODR (output data register) is at offset 0x14
#define GPIOD_BSRR (*(volatile unsigned int *)(GPIOD_BASE + 0x18)) // GPIOD_BSRR (bit set/reset register) is at offset 0x18

/* USART2 */
#define USART2_BRR (*(volatile unsigned int *)(USART2_BASE + 0x08)) // BRR (baud rate register)
#define USART2_CR1 (*(volatile unsigned int *)(USART2_BASE + 0x0C)) // CR1 (control register 1)
#define USART2_SR (*(volatile unsigned int *)(USART2_BASE + 0x00)) // SR (status register)
#define USART2_DR (*(volatile unsigned int *)(USART2_BASE + 0x04)) // DR (data register)

/* SYSCFG & EXTI */
#define SYSCFG_EXTICR1 (*(volatile unsigned int *)(SYSCFG_BASE + 0x08)) // EXTICR1 (external interrupt control register)
#define EXTI_IMR (*(volatile unsigned int *)(EXTI_BASE + 0x00)) // interrupt mask register
#define EXTI_RTSR (*(volatile unsigned int *)(EXTI_BASE + 0x08)) // rising trigger selection register
#define EXTI_FTSR (*(volatile unsigned int *)(EXTI_BASE + 0x0C)) // falling trigger selection register
#define EXTI_PR (*(volatile unsigned int *)(EXTI_BASE + 0x14)) // pending register

/* NVIC & SysTick: found in programming manual */
#define NVIC_ISER0 (*(volatile unsigned int *)(0xE000E100)) // interrupt set enable register
#define NVIC_IPR (*(volatile unsigned int *)(0xE000E400)) // interrupt priority register
#define STK_CTRL (*(volatile unsigned int *)(0xE000E010)) // SysTick control and status register
#define STK_LOAD (*(volatile unsigned int *)(0xE000E014)) // SysTick reload value register
#define STK_VAL (*(volatile unsigned int *)(0xE000E018)) // SysTick current value register