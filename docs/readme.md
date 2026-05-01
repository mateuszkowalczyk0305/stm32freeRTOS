# Stage 1 – UART Command Handling with FreeRTOS

## Overview

This stage implements a basic command-driven system using UART communication and FreeRTOS on the STM32 Nucleo-F303RE board.

The system receives text commands from a PC terminal and controls the onboard LED (LD2) accordingly.

---

## System Architecture

The communication flow is divided into independent layers to ensure clean design and scalability.

### 1. PC / Terminal

The user sends simple text commands via UART:

```
START LED\n
STOP LED\n
```

UART configuration:

* Baudrate: 115200
* Format: 8N1

---

### 2. UART Reception (Interrupt / DMA)

The STM32 receives incoming data using USART interrupt or DMA.

Responsibilities:

* Receive bytes (single or buffer)
* Do NOT interpret commands
* Do NOT control LED
* Push received data into a ring buffer

---

### 3. Ring Buffer

A circular buffer stores incoming UART data.

Purpose:

* Prevent data loss during high-speed reception
* Decouple interrupt context from application logic

---

### 4. UART Command Task (High Priority)

This FreeRTOS task processes incoming data.

Responsibilities:

1. Read bytes from the ring buffer
2. Build a command string (max 20 characters)
3. Detect end of command (`\n`)
4. Handle buffer overflow:

   * Send `"Syntax Error"`
   * Echo invalid buffer content
5. Pass complete commands to the parser

---

### 5. Command Parser

The parser converts text commands into internal command IDs.

Input:

```
"START LED\n"
"STOP LED\n"
```

Output:

```
CMD_ID_START_LED
CMD_ID_STOP_LED
CMD_ID_UNKNOWN
```

---

### 6. Command Handling (Switch Logic)

The command ID is processed in the UART task:

* Valid commands → sent to a FreeRTOS queue
* Unknown commands → `"Syntax Error"` sent via UART

---

### 7. FreeRTOS Queue (ledCommandQueue)

A queue is used to transfer commands between tasks.

Purpose:

* Decouple command processing from LED control
* Ensure thread-safe communication

Example values:

```
CMD_ID_START_LED
CMD_ID_STOP_LED
```

---

### 8. LED Task (Low Priority)

This task controls the onboard LED.

Responsibilities:

1. Wait for command ID from the queue
2. Execute command:

   * `START LED` → enable blinking
   * `STOP LED` → disable blinking
3. Send status messages via UART
4. Toggle LED periodically (e.g. every 500 ms)

---

### 9. Hardware Output

The LED (LD2) on the STM32 Nucleo board reflects the system state:

* Blinking → active
* Off → stopped

---

## Design Principles

* Interrupts are minimal and fast
* No logic inside ISR
* Clear separation of responsibilities
* Task-based architecture (FreeRTOS)
* Safe communication using queues
* Scalable for future extensions (e.g. WS2812, sensors)

---

## Next Steps

In the next stage, the system will be extended with:

* WS2812 LED control (DMA + timer)
* ADC input handling
* Extended command set
* More advanced task communication
