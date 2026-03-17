# Project 2.1: Button Interrupt with Software Debounce

Interrupt-driven button handler on STM32F407 Discovery using EXTI, NVIC, and SysTick — direct register manipulation, no HAL or CMSIS.

## Overview

This project replaces polling-based input detection with hardware interrupts. Pressing the blue user button (PA0) triggers an EXTI interrupt that toggles the green LED (PD12) and reports press count over UART. A SysTick-based software debounce filter eliminates the multiple spurious triggers caused by mechanical button bounce.

The CPU main loop is empty — all behavior is driven by interrupt handlers. This is how production embedded systems work: the processor idles (or sleeps) until hardware events demand attention.

## Hardware

- **Board:** STM32F407G-DISC1 Discovery Board (MB997E revision)
- **MCU:** STM32F407VGT6 (ARM Cortex-M4, running at 16 MHz HSI)
- **Button:** Blue user button on PA0 (active-high, connects to VDD when pressed)
- **LED:** Green LED (LD4) on PD12
- **UART:** CP2102 USB-to-serial adapter on PA2 (TX) at 115200 baud

## Interrupt Architecture

```
Button Press (PA0 rising edge)
       │
       ▼
   ┌────────┐     ┌────────┐     ┌────────┐     ┌─────────────────┐
   │ SYSCFG │────▶│  EXTI  │────▶│  NVIC  │────▶│ EXTI0_IRQHandler│
   │  mux   │     │ line 0 │     │ IRQ #6 │     │   (in main.c)   │
   └────────┘     └────────┘     └────────┘     └─────────────────┘
   Maps PA0       Detects        Delivers           Clears pending,
   to EXTI0       rising edge    interrupt           debounce check,
                                 to CPU              toggles LED

SysTick (1ms tick)
       │
       ▼
   ┌─────────────────┐
   │ SysTick_Handler │──▶ Increments volatile `millis` counter
   └─────────────────┘    (provides timebase for debounce logic)
```

## Register-Level Configuration

### 1. Clock Enables (RCC)

| Register | Bit | Purpose |
|----------|-----|---------|
| `RCC_AHB1ENR` | 0 | GPIOA clock (button PA0 + UART PA2/PA3) |
| `RCC_AHB1ENR` | 3 | GPIOD clock (LED PD12) |
| `RCC_APB1ENR` | 17 | USART2 clock |
| `RCC_APB2ENR` | 14 | SYSCFG clock (required for EXTI routing) |

### 2. GPIO Configuration

| Pin | Register | Value | Function |
|-----|----------|-------|----------|
| PA0 | `GPIOA_MODER[1:0]` | `00` | Input mode |
| PA0 | `GPIOA_PUPDR[1:0]` | `10` | Internal pull-down resistor |
| PD12 | `GPIOD_MODER[25:24]` | `01` | General purpose output |
| PA2 | `GPIOA_MODER[5:4]` | `10` | Alternate function (USART2 TX) |
| PA2 | `GPIOA_AFRL[11:8]` | `0111` | AF7 (USART2) |

### 3. SYSCFG — EXTI Line Routing

`SYSCFG_EXTICR1[3:0]` = `0000` — routes port A pin 0 to EXTI line 0.

Each EXTI line (0–15) can connect to only one GPIO port at a time. SYSCFG is the multiplexer that selects which port. Forgetting to enable the SYSCFG clock in `RCC_APB2ENR` is one of the most common bare-metal interrupt bugs.

### 4. EXTI — External Interrupt Controller

| Register | Bit 0 | Purpose |
|----------|-------|---------|
| `EXTI_IMR` | `1` | Unmask interrupt on line 0 |
| `EXTI_RTSR` | `1` | Trigger on rising edge (button press) |
| `EXTI_FTSR` | `0` | No falling edge trigger (ignore release) |
| `EXTI_PR` | Write `1` | Clear pending bit in ISR (write-1-to-clear) |

### 5. NVIC — Nested Vectored Interrupt Controller

`NVIC_ISER0` bit 6 = `1` — enables IRQ #6 (EXTI0_IRQHandler).

The IRQ number comes from the vector table in the startup assembly file. NVIC registers live in the ARM Cortex-M4 core's System Control Space (`0xE000E100`), not in the STM32 peripheral address range.

### 6. SysTick — System Timer

| Register | Value | Purpose |
|----------|-------|---------|
| `STK_LOAD` | `15999` | Reload value for 1ms tick at 16 MHz HSI |
| `STK_VAL` | `0` | Clear current value to force reload |
| `STK_CTRL` | `0x07` | Enable counter + interrupt + processor clock |

SysTick is a 24-bit down-counter built into every Cortex-M core. It fires `SysTick_Handler` each time it reaches zero and auto-reloads. Reload calculation: 16,000,000 Hz / 1000 Hz - 1 = 15,999.

## Software Debounce Strategy

Mechanical buttons bounce — a single press can produce 5–20 electrical transitions within a few milliseconds. Without debouncing, each press would trigger multiple interrupts.

The debounce logic uses the SysTick millisecond counter:

1. `SysTick_Handler` increments a `volatile uint32_t millis` every 1ms
2. `EXTI0_IRQHandler` compares `millis` against `last_press` timestamp
3. If fewer than 50ms have elapsed, the interrupt is ignored (just clear pending bit)
4. If 50ms+ have elapsed, the press is valid — toggle LED, increment counter, update timestamp

The `volatile` keyword on shared globals (`millis`, `last_press`, `press_count`) is critical. Without it, the compiler may optimize away reads in `main()` because it doesn't see any normal code path modifying them.

## Design Decisions

**UART printf in main loop, not in ISR:** The interrupt handler sets a flag (increments `press_count`), and the main loop detects the change and prints. UART transmission is slow (blocking, character-by-character) and would block the ISR for too long. This ISR-sets-flag, main-loop-reacts pattern is fundamental to embedded design.

**Clear EXTI pending bit first:** The pending bit is cleared at the top of the ISR, before the debounce check. If cleared after processing, a bounce arriving during processing could be missed.

**Explicit register clears on defaults:** PA0's MODER defaults to input (`00`) after reset, but the code explicitly clears the bits anyway. This makes the configuration self-documenting and protects against running after a soft reset where registers may retain prior state.

## Build & Flash

```bash
make            # Compile — outputs build/btn-intrpt-dbnc.elf and .bin
make flash      # Flash to board via OpenOCD and ST-LINK
make clean      # Remove build artifacts
```

### UART Monitor

```bash
picocom -b 115200 /dev/ttyUSB0
```

Expected output on button press:
```
Button pressed - count: 1
Button pressed - count: 2
Button pressed - count: 3
```

## Debug (VSCode + Cortex-Debug)

1. Open this project folder in VSCode
2. Press F5 to launch a debug session (uses `.vscode/launch.json`)
3. Set breakpoints in `EXTI0_IRQHandler` to observe interrupt flow

The peripheral register viewer in Cortex-Debug can show live EXTI, NVIC, and GPIO register states if you load the STM32F407 SVD file.

## File Structure

```
project-2.1-button-interrupt-debounce/
├── Src/
│   ├── main.c              # Init, interrupt handlers, main loop
│   ├── uart2.c             # UART driver (self-contained, reusable)
│   └── syscalls.c          # Newlib stubs
├── Inc/
│   ├── stm32f407xx.h       # Register address definitions
│   └── uart2.h             # UART driver header
├── Startup/
│   └── startup_stm32f407vgtx.s  # Vector table + Reset_Handler
├── STM32F407VGTX_FLASH.ld  # Linker script
├── Makefile
├── .vscode/                # Debug configuration
└── README.md
```

## Reference Documents

- [RM0090](https://www.st.com/resource/en/reference_manual/dm00031020.pdf) — STM32F405/407 Reference Manual (§9: SYSCFG, §12: EXTI, §10: NVIC)
- [PM0214](https://www.st.com/resource/en/programming_manual/dm00046982.pdf) — Cortex-M4 Programming Manual (§4.3: NVIC registers, §4.5: SysTick)
- [DS8626](https://www.st.com/resource/en/datasheet/stm32f407vg.pdf) — STM32F407 Datasheet
- [UM1472](https://www.st.com/resource/en/user_manual/dm00039084.pdf) — STM32F4 Discovery Board User Manual

## What I Learned

- How EXTI, SYSCFG, and NVIC form a three-stage chain to route a GPIO pin change into an interrupt handler
- The difference between edge-triggered (fires once per transition) and level-triggered (fires continuously while active) interrupts
- Why `volatile` is mandatory for variables shared between interrupt context and main context
- Write-1-to-clear register semantics (EXTI_PR) — a common ARM peripheral pattern
- Software debouncing using a millisecond timebase from SysTick
- The ISR-sets-flag, main-loop-reacts pattern for keeping interrupt handlers fast
- Pull-down resistors hold floating inputs at a known logic level
- SysTick and NVIC live in the Cortex-M core's address space (`0xE000xxxx`), separate from ST's peripheral bus (`0x4000xxxx`)

## Part of the EdgeAIGuy 12-Month Roadmap

This is Project 2.1 in an 8-project progression from bare-metal fundamentals to TinyML deployment. See the [full roadmap context](https://github.com/edgeaiguy) for the complete project sequence.