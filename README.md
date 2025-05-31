# Basic-Home-Alarm-System

> Firmware for a simple home alarm with Disarmed, Armed, and Report modes.

---

## Overview

This project implements the logic for a home alarm system on a microcontroller. The system has two primary modes:

- **Disarmed**: Default state; sensors inactive  
- **Armed**: Sensors active; triggers alarm on breach  
- **Report**: Sub-state after multiple failed code attempts

Users authenticate via a 4-digit PIN to arm or disarm the system. Visual and audible feedback is provided via an LCD display, buzzer, and LED.

### Flowchart

![image](https://github.com/user-attachments/assets/e67a7e7e-4bf6-4f3a-a50c-14207301e46c)

### Hardware Design

![image](https://github.com/user-attachments/assets/8fa199d2-aa06-4cc8-899b-587f2f35457d)

![image](https://github.com/user-attachments/assets/76277aa3-c387-4f2a-99fa-6e2435dcb794)

---

## System Flow

### Default Stage (Disarmed State)

1. **Startup**  
   - System initializes in **Disarmed** mode.  
   - LCD displays `DISARMED`.  

2. **Sensor Activation**  
   - Simulated by button press.  
   - On trigger, `disarmed_state()` sets button state to `1` and displays one or both messages on the LCD:
     - **Button 1** → “Check Windows”  
     - **Button 2** → “Check Sensors”  
     - **Both** → Both messages  

---

### User Code Insertion

When a sensor is activated:

1. **Prompt**  
   - LCD shows `Code:____`.  
   - Each digit pressed replaces an underscore with `*`.  

2. **Input Handling**  
   - Valid digits appended to `enteredCode[]`.  
   - Invalid keys (e.g., letters) erase the last entry and restore `_`.  

3. **Arming/Disarming**  
   - After 4 digits, user presses **A** (arm) or **D** (disarm).  
   - Invalid state key → LCD shows “Wrong Key. Retry.”  

---

### Armed State

1. **Transition**  
   - Correct code + **A** → LCD changes to `ARMED`.  
   - Buzzer and LED activate for 30 s as confirmation.  

2. **Monitoring**  
   - Entering `armed_state()`, system watches buttons:  
     - Button 1 or 2 sets `b_val3 = 1`, triggering buzzer & blinking LED.  
   - Alarm continues until correct code + **D** is entered.

---

### Incorrect Key & Code Handling

- **Wrong Key (not A/D)**  
  - LCD: “Wrong Key. Retry.”  
  - Returns to Disarmed, Armed, or Report as appropriate.

- **Wrong Code**  
  - LCD: “Incorrect Code” for 5 s.  
  - Increment `incAttempts`.  
  - After 3 failures:
    - **Disarmed**: 30 s alarm + keypad lock  
    - **Armed**: Switch to **Report** mode, alarm continues  
    - **Report**: Remains until correct code

---

### Code Verification & Reset

- Stored PIN: `2918` in `correctCode[]`  
- `codeverif()` compares `enteredCode[]` to `correctCode[]`  
  - **Match** → Transition per key (A/D)  
  - **Disarm** from Armed:
    - Turn off buzzer & LED  
    - Return to Disarmed state

---

## Functions

| Function                   | Description                                          |
|----------------------------|------------------------------------------------------|
| `disarmed_state()`         | Handle button triggers in Disarmed mode              |
| `insertingcode_disarmed()` | Prompt & process code entry in Disarmed mode         |
| `armed_state()`            | Monitor sensors & alarm in Armed mode                |
| `insertingcode_armed()`    | Prompt & process code entry in Armed/Report modes    |
| `codeverif()`              | Compare entered code with preset code                |



