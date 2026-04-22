# Project 2.2: ADC + PWM Timer-Driven System

Hardware-triggered ADC with interrupt-driven PWM on STM32F407 Discovery — potentiometer controls LED brightness through a fully hardware-driven pipeline. Direct register manipulation, no HAL or CMSIS.

## Overview

This project chains four peripherals into an autonomous pipeline: a timer (TIM2) triggers ADC conversions at a fixed 100 Hz interval, the ADC reads a potentiometer voltage on PA1, an interrupt handler scales the 12-bit result to a PWM duty cycle, and a second timer (TIM4) drives the green LED (PD12) at the corresponding brightness. The CPU main loop is uninvolved in the signal path — the entire analog-in to PWM-out flow runs on hardware triggers and interrupts.

This is the same architecture used in production data acquisition systems: timer-triggered sampling ensures precise, consistent intervals (critical for frequency-domain analysis in the flagship vibration sensor project), and interrupt-driven processing keeps CPU utilization minimal.

## Hardware

- **Board:** STM32F407G-DISC1 Discovery Board (MB997E revision)
- **MCU:** STM32F407VGT6 (ARM Cortex-M4, running at 16 MHz HSI)
- **Potentiometer:** Precision potentiometer on breadboard
- **LED:** Green LED (LD4) on PD12, driven by TIM4 PWM
- **UART:** CP2102 USB-to-serial adapter on PA2 (TX) at 115200 baud

### Wiring
![Wiring Diagram](docs/project-2.2-wiring-diagram.svg.svg)

```
STM32F407 Discovery          Breadboard
┌─────────────┐              ┌──────────────────────┐
│             │              │  (+) ────────────────── 3.3V rail
│     3V  ●───────────────────── Red (+) rail        │
│             │              │                        │
│    GND  ●───────────────────── Blue (−) rail       │
│             │              │  (−) ────────────────── GND rail
│             │              │                        │
│    PA1  ●───────────────────── Pot wiper (middle)  │
│             │              │                        │
│   PD12  ● (LED)            │    ┌───┐              │
│             │              │    │POT│              │
│  PA2/PA3 → CP2102 (UART)  │    └─┬─┘              │
└─────────────┘              │   L  M  R             │
                             │   │  │  │             │
                             │   +  PA1 −            │
                             └──────────────────────┘
L = 3.3V rail, M = PA1 (ADC input), R = GND rail
```

## System Architecture

```
┌──────────┐  TRGO   ┌──────────┐  EOC IRQ  ┌────────────────┐  CCR1   ┌──────────┐
│  TIM2    │────────▶│  ADC1    │──────────▶│ ADC_IRQHandler │───────▶│  TIM4    │──▶ PD12 LED
│  100 Hz  │ trigger │  ch1/PA1 │ interrupt │  scales 0-4095 │  write │  1 kHz   │   brightness
│  overflow│         │  12-bit  │           │  to 0-999 duty │        │  PWM     │
└──────────┘         └──────────┘           └────────────────┘        └──────────┘
     │                    │                         │
     │                    │                         │
  No CPU              No CPU                   ~10 CPU cycles
  involvement         involvement              per conversion
```

**Data flow:** TIM2 overflows every 10ms (100 Hz) → broadcasts TRGO signal → ADC1 automatically starts conversion on PA1 → conversion completes → EOC interrupt fires → ISR reads 12-bit result, scales to PWM range, writes to TIM4_CCR1 → LED brightness updates.

**Main loop role:** Periodic UART reporting only. The analog-to-PWM pipeline runs entirely without main loop involvement.

## Register-Level Configuration

### Clock Enables (RCC)

| Register | Bit | Peripheral | Bus |
|----------|-----|-----------|-----|
| `RCC_AHB1ENR` | 0 | GPIOA (PA1 ADC input, PA2/PA3 UART) | AHB1 |
| `RCC_AHB1ENR` | 3 | GPIOD (PD12 PWM LED) | AHB1 |
| `RCC_APB1ENR` | 0 | TIM2 (ADC trigger) | APB1 |
| `RCC_APB1ENR` | 2 | TIM4 (PWM generation) | APB1 |
| `RCC_APB2ENR` | 8 | ADC1 | APB2 |

### GPIO Configuration

| Pin | MODER | AF | Function |
|-----|-------|----|----------|
| PA1 | `11` (analog) | — | ADC1 channel 1 input (direct connection to ADC, no digital buffer) |
| PD12 | `10` (alternate function) | AF2 | TIM4_CH1 PWM output to green LED |
| PA2 | `10` (alternate function) | AF7 | USART2 TX |

### TIM4 — PWM Generation (1 kHz on PD12)

| Register | Value | Purpose |
|----------|-------|---------|
| `TIM4_PSC` | 15 | Prescaler: 16 MHz / (15+1) = 1 MHz timer tick |
| `TIM4_ARR` | 999 | Auto-reload: 1 MHz / (999+1) = 1 kHz PWM frequency |
| `TIM4_CCMR1` | `OC1M=110, OC1PE=1` | PWM Mode 1 with preload on channel 1 |
| `TIM4_CCER` | `CC1E=1` | Enable channel 1 output |
| `TIM4_CCR1` | 0–999 | Duty cycle threshold (written by ADC ISR) |
| `TIM4_CR1` | `CEN=1` | Start timer |

**PWM behavior:** Counter counts 0 → 999 continuously. Output is HIGH when counter < CCR1, LOW when counter ≥ CCR1. CCR1 = 500 means 50% duty cycle. Unlike PSC and ARR, CCR1 does not use the minus-1 pattern — it's a direct comparison value.

### TIM2 — ADC Trigger (100 Hz TRGO)

| Register | Value | Purpose |
|----------|-------|---------|
| `TIM2_PSC` | 15999 | Prescaler: 16 MHz / 16000 = 1 kHz timer tick |
| `TIM2_ARR` | 9 | Auto-reload: 1 kHz / 10 = 100 Hz overflow |
| `TIM2_CR2` | `MMS=010` | Master Mode: update event generates TRGO output |
| `TIM2_CR1` | `CEN=1` | Start timer |

TIM2 is purely internal — not connected to any GPIO pin. Its only job is to broadcast a TRGO pulse every 10ms that the ADC listens for.

### ADC1 — Analog-to-Digital Conversion

| Register | Value | Purpose |
|----------|-------|---------|
| `ADC1_SQR3` | `SQ1=00001` | Channel 1 (PA1) as first conversion in sequence |
| `ADC1_SQR1` | `L=0000` | Sequence length = 1 conversion |
| `ADC1_SMPR2` | `SMP1=101` | 84 cycles sample time for channel 1 |
| `ADC1_CR2` | `EXTSEL=0110` | External trigger: TIM2 TRGO (from RM0090 §13.13.2) |
| `ADC1_CR2` | `EXTEN=01` | Trigger on rising edge |
| `ADC1_CR1` | `EOCIE=1` | End-of-conversion interrupt enable |
| `ADC1_CR2` | `ADON=1` | Power on ADC (set last, after all configuration) |

**12-bit resolution:** ADC output range is 0–4095 mapping linearly to 0V–3.3V input. The potentiometer wiper sweeps this full range.

### NVIC

`NVIC_ISER0` bit 18 = `1` — enables the ADC IRQ (IRQ #18 from vector table).

## Interrupt Handler

```c
void ADC_IRQHandler(void) {
    uint32_t adc_val = ADC1_DR & 0xFFF;       // read 12-bit result (clears EOC flag)
    uint32_t duty = (adc_val * 999) / 4095;   // scale 0–4095 → 0–999
    TIM4_CCR1 = duty;                          // update PWM duty cycle
}
```

Reading `ADC1_DR` automatically clears the EOC flag — no manual flag clear needed (unlike EXTI_PR in Project 2.1). The ISR executes approximately 10 CPU instructions, keeping interrupt latency minimal.

## Development Progression

This project was built incrementally to isolate and verify each peripheral:

1. **Step 1–2:** TIM4 PWM on PD12 with hardcoded duty cycle — verified LED brightness control
2. **Step 3:** Manual software-triggered ADC (polling) — verified ADC reads potentiometer correctly
3. **Steps 4–5:** TIM2 hardware trigger + ADC interrupt — replaced polling with fully autonomous pipeline

## Debugging Notes

Two significant bugs were found and fixed during development:

**Copy-paste base address error:** All TIM2 register defines accidentally used `TIM4_BASE` instead of `TIM2_BASE`. Writes appeared to succeed (reading back from the same wrong address returned the written values), but the actual TIM2 hardware was never configured. Diagnosed by reading `TIM2_CNT` (the only correctly-defined register) and finding it stuck at 0. Lesson: when registers read back correctly but the peripheral doesn't work, verify the base address in the define.

**Double pointer dereference:** NVIC and SysTick defines had `(*(volatile unsigned int *)(*(volatile unsigned int *)(...)))` instead of `(*(volatile unsigned int *)(...))`. This read the register address, interpreted the contents as a pointer, and dereferenced that — writing NVIC configuration to a garbage address. The ADC converted successfully but the interrupt never fired because the NVIC enable bit was never set. Diagnosed by checking `ADC1_SR` (EOC flag was set, proving conversion happened) while `ADC_IRQHandler` never executed.

## UART Output

See docs/uart_output.txt for what the expected live UART output should look like when spinning the potentiometer.

## Demo

Live video of potentiometer effecting LED brightness output via ADC 2 PWM coming soon.

## Build & Flash

```bash
make            # Compile
make flash      # Flash to board via OpenOCD and ST-LINK
make clean      # Remove build artifacts
```

### UART Monitor

```bash
picocom -b 115200 /dev/ttyUSB0
```

## File Structure

```
project-2.2-adc-pwm-timer/
├── Src/
│   ├── main.c              # Init, ADC interrupt handler, main loop
│   ├── uart2.c             # UART driver (self-contained, reusable)
│   └── syscalls.c          # Newlib stubs
├── Inc/
│   ├── stm32f407xx.h       # Register address definitions (growing across projects)
│   └── uart2.h             # UART driver header
├── Startup/
│   └── startup_stm32f407vgtx.s  # Vector table + Reset_Handler
├── STM32F407VGTX_FLASH.ld  # Linker script
├── Makefile
├── .vscode/                # Debug configuration
└── README.md
```

## Reference Documents

- [RM0090](https://www.st.com/resource/en/reference_manual/dm00031020.pdf) — STM32F405/407 Reference Manual (§13: ADC, §18: General-purpose timers)
- [DS8626](https://www.st.com/resource/en/datasheet/stm32f407vg.pdf) — STM32F407 Datasheet (Table 9: pin-to-peripheral mapping)
- [UM1472](https://www.st.com/resource/en/user_manual/dm00039084.pdf) — STM32F4 Discovery Board User Manual

## What I Learned

- How to chain multiple peripherals into an autonomous hardware pipeline (timer → ADC → interrupt → PWM)
- Timer prescaler and auto-reload calculations for precise timing intervals
- The difference between PSC/ARR (subtract 1 for count range) and CCR (direct comparison value)
- PWM Mode 1: output HIGH when counter < CCR, LOW when counter ≥ CCR
- ADC external trigger configuration: EXTSEL selects the trigger source, EXTEN enables edge detection
- TRGO (Trigger Output) allows timers to signal other peripherals without CPU involvement
- Analog GPIO mode disconnects the digital buffer, connecting the pin directly to the ADC
- Pin-to-peripheral mapping is fixed in silicon — must check DS8626 Table 9
- AFRL covers GPIO pins 0–7, AFRH covers pins 8–15 (PD12 uses AFRH, not AFRL)
- Reading ADC_DR automatically clears the EOC flag (unlike EXTI_PR which needs explicit write-1-to-clear)
- Copy-paste errors in register defines are silent and brutal — the compiler can't catch a wrong base address
- Systematic debugging: dump register contents over UART to verify configuration matches intent

## Part of the EdgeAIGuy 12-Month Roadmap

This is Project 2.2 in an 8-project progression from bare-metal fundamentals to TinyML deployment. See the [full roadmap context](https://github.com/edgeaiguy) for the complete project sequence.
