# Hardware Connection Guide

## EEE 158 Machine Problem 1: Colorful Lighting
### PIC32CM LS00 Curiosity Nano + Curiosity Nano Explorer Board

---

## Quick Setup Overview

1. **Insert** the PIC32CM LS00 Curiosity Nano board into the Curiosity Nano Explorer board's mikroBUS™ socket
2. **Connect** the RGB LED to the Explorer board terminals
3. **Power** via USB connection to the Curiosity Nano

---

## Pin Mapping Table

| MCU Pin | Function | Explorer Board Connection | Component | Notes |
|---------|----------|---------------------------|-----------|-------|
| **PA00** | SW1 Input | Pushbutton 1 on Explorer | SW1 Pushbutton | Brightness mode control |
| **PA01** | SW2 Input | Pushbutton 2 on Explorer | SW2 Pushbutton | Auto-cycle mode control |
| **PA03** | PWM Output (TCC3/WO[0]) | mikroBUS™ PWM Pin | Red LED Channel | PWM-A (Red channel) |
| **PA06** | PWM Output (TCC3/WO[1]) | mikroBUS™ AN Pin | Green LED Channel | PWM-B (Green channel) |
| **PB02** | ADC Input (AIN10) | Potentiometer on Explorer | 10kΩ Potentiometer | Analog input (0-3.3V) |
| **PB03** | PWM Output (TCC3/WO[2]) | mikroBUS™ RST Pin | Blue LED Channel | PWM-C (Blue channel) |

---

## RGB LED Connection Diagram

### Common Anode RGB LED Wiring

```
                    +3.3V (Common Anode)
                         |
                    [RGB LED]
                    /    |    \
                   /     |     \
                  R      G      B
                  |      |      |
              [Resistor] [Resistor] [Resistor]
              (220Ω)    (220Ω)    (220Ω)
                  |      |      |
                  |      |      |
                 PA03   PA06   PB03
              (PWM-A) (PWM-B) (PWM-C)
```

**Important:** The RGB LED is **Common Anode** type:
- Common pin connects to +3.3V
- Individual R, G, B cathodes connect through current-limiting resistors to MCU PWM pins
- **Active-Low PWM:** 0% duty cycle = Full brightness, 100% duty cycle = OFF

---

## Physical Connection Details

### 1. Curiosity Nano to Explorer Board

```
┌─────────────────────────────────────────────────┐
│     Curiosity Nano Explorer Board               │
│                                                  │
│  ┌────────────────────────────────┐             │
│  │  mikroBUS™ Socket               │             │
│  │  (Insert Curiosity Nano here)   │             │
│  │                                  │             │
│  │  [PIC32CM LS00 Curiosity Nano]  │             │
│  │         (16-pin header)          │             │
│  └────────────────────────────────┘             │
│                                                  │
│  [POT]  Potentiometer (10kΩ)                    │
│  [SW1]  Pushbutton 1                            │
│  [SW2]  Pushbutton 2                            │
│                                                  │
│  mikroBUS™ Pin Assignments:                     │
│  • AN (Analog)  → PA06 (Green PWM)              │
│  • RST          → PB03 (Blue PWM)               │
│  • PWM          → PA03 (Red PWM)                │
│                                                  │
└─────────────────────────────────────────────────┘
```

### 2. RGB LED Terminal Connections

| LED Pin | Connect To | Wire Color (Suggested) |
|---------|------------|------------------------|
| **Common Anode (+)** | +3.3V on Explorer | Red wire |
| **R (Red cathode)** | PA03 via 220Ω resistor | Orange wire |
| **G (Green cathode)** | PA06 via 220Ω resistor | Green wire |
| **B (Blue cathode)** | PB03 via 220Ω resistor | Blue wire |

---

## Component List

### Required Components

| Qty | Component | Specification | Purpose |
|-----|-----------|---------------|---------|
| 1 | PIC32CM LS00 Curiosity Nano | DM32CM5000 | Microcontroller board |
| 1 | Curiosity Nano Explorer | AC164161 | Development/expansion board |
| 1 | Common Anode RGB LED | 4-pin, 5mm | Color display |
| 3 | Current-limiting resistors | 220Ω, 1/4W | LED current protection |
| 1 | USB Cable | Micro-USB or USB-C | Power and programming |
| - | Jumper wires | 22-24 AWG | Connections |
| - | Breadboard (optional) | Half-size | For resistor/LED mounting |

### On-Board Components (Already on Explorer)

- 10kΩ Potentiometer (Connected to PB02/AIN10)
- 2× Pushbuttons (SW1 on PA00, SW2 on PA01)
- Power regulation circuitry
- mikroBUS™ socket

---

## Step-by-Step Assembly

### Step 1: Prepare the RGB LED Circuit

1. If using a breadboard:
   ```
   - Insert RGB LED into breadboard
   - Connect 220Ω resistor to each cathode (R, G, B)
   - Connect common anode to 3.3V rail
   ```

2. Alternative (direct wiring):
   ```
   - Solder 220Ω resistor to each LED cathode
   - Add wire leads for connection to Explorer board
   ```

### Step 2: Mount Curiosity Nano on Explorer

1. Align the PIC32CM LS00 Curiosity Nano's 16-pin header with the mikroBUS™ socket
2. Gently press down until fully seated
3. Ensure all pins are properly inserted

### Step 3: Connect RGB LED to Explorer Board

| Connection | From | To |
|------------|------|-----|
| Power | RGB LED Common Anode | +3.3V terminal on Explorer |
| Red PWM | Red cathode + resistor | PA03 (mikroBUS™ PWM pin) |
| Green PWM | Green cathode + resistor | PA06 (mikroBUS™ AN pin) |
| Blue PWM | Blue cathode + resistor | PB03 (mikroBUS™ RST pin) |
| Ground | Breadboard GND (if used) | GND terminal on Explorer |

### Step 4: Power Connection

1. Connect USB cable to PIC32CM LS00 Curiosity Nano's USB port
2. Connect other end to computer USB port
3. Board will power up (onboard LED may illuminate)

---

## mikroBUS™ Socket Pinout Reference

```
Left Side (Pins 1-8):          Right Side (Pins 9-16):
┌──────────────────┐          ┌──────────────────┐
│ 1  AN    → PA06  │          │ 16 PWM   → PA03  │  Red PWM
│ 2  RST   → PB03  │          │ 15 INT   → PA01  │  SW2 (auto-cycle)
│ 3  CS            │          │ 14 RX            │
│ 4  SCK           │          │ 13 TX            │
│ 5  MISO          │          │ 12 SCL           │
│ 6  MOSI          │          │ 11 SDA           │
│ 7  +3.3V         │          │ 10 +5V           │
│ 8  GND           │          │  9 GND           │
└──────────────────┘          └──────────────────┘

Green PWM ↑                              ↑ Blue PWM
```

**Key Pins Used:**
- Pin 16 (PWM): PA03 → Red LED
- Pin 1 (AN): PA06 → Green LED  
- Pin 2 (RST): PB03 → Blue LED
- Pin 7 (+3.3V): Power for RGB LED common anode

---

## Schematic Diagram

```
                    PIC32CM LS00                     Curiosity Nano Explorer
                                                            
    PA00 ●───────────────────────────────────────● SW1 Pushbutton ──┬── GND
                                                                     │
    PA01 ●───────────────────────────────────────● SW2 Pushbutton ──┴── GND
                                                            
                                                   ┌─────────────┐
    PB02 ●───────────────────────────────────────●─┤ Potentiometer│
                                                   │  (10kΩ)      │
                                                   └──┬────────┬──┘
                                                   +3.3V      GND
                                                            
           ┌───────────────────────────────────┐
    PA03 ●─┤ TCC3/WO[0] (Red PWM)              ├─● mikroBUS PWM (Pin 16)
           │                                    │         │
    PA06 ●─┤ TCC3/WO[1] (Green PWM)            ├─● mikroBUS AN  (Pin 1)
           │                                    │         │
    PB03 ●─┤ TCC3/WO[2] (Blue PWM)             ├─● mikroBUS RST (Pin 2)
           └───────────────────────────────────┘         │
                                                          │
                                                    ┌─────▼──────┐
                                                    │  RGB LED    │
                                                    │   Circuit   │
                                                    │             │
                                    +3.3V ──────────┤ Anode (+)   │
                                                    │             │
              ┌─────────────────────── Red ────────┤ R (-)       │
              │                                     │             │
              │     ┌─────────────── Green ────────┤ G (-)       │
              │     │                               │             │
              │     │     ┌───────────  Blue ──────┤ B (-)       │
              │     │     │                         └────────────┘
              │     │     │
            [220Ω] [220Ω] [220Ω]  Current-limiting resistors
              │     │     │
             PA03  PA06  PB03
```

---

## Electrical Specifications

| Parameter | Value | Notes |
|-----------|-------|-------|
| **Supply Voltage** | 3.3V | From Curiosity Nano regulator |
| **PWM Frequency** | 100 Hz | Configured via TCC3 |
| **PWM Period** | 1875 counts | At 48MHz / 256 prescaler |
| **LED Forward Voltage** | ~2.0-3.2V | Typical for RGB LED |
| **LED Current** | ~15mA per channel | With 220Ω resistors |
| **ADC Resolution** | 10-bit (0-1023) | Potentiometer reading |
| **ADC Reference** | AVDD (3.3V) | Internal reference |

---

## Testing Checklist

- [ ] Curiosity Nano properly seated in Explorer socket
- [ ] All RGB LED connections secure (R, G, B, common anode)
- [ ] Current-limiting resistors in series with each LED cathode
- [ ] USB cable connected and board powered
- [ ] SW1 button press changes to brightness control mode
- [ ] SW2 button press enables auto-cycling mode
- [ ] Potentiometer adjustment affects brightness (SW1 mode)
- [ ] Potentiometer adjustment affects cycle speed/direction (SW2 mode)
- [ ] Initial color displays as light yellow (#f7ff84) at 50% brightness

---

## Troubleshooting

| Problem | Possible Cause | Solution |
|---------|----------------|----------|
| No LED illumination | Loose connections | Check all wiring connections |
| | Wrong polarity | Verify common anode to +3.3V |
| | Missing resistors | Add 220Ω resistors in series |
| One color missing | Loose wire for that channel | Check specific PWM pin connection |
| | Damaged LED | Test LED with multimeter |
| Buttons not working | Poor Nano seating | Re-seat Curiosity Nano in socket |
| Potentiometer no effect | Wrong mode | Press SW1 or SW2 to enter correct mode |
| | ADC connection issue | Verify PB02 connected internally |

---

## Safety Notes

⚠️ **Important:**
- Do not exceed 3.3V on any MCU pin
- Always use current-limiting resistors with LEDs
- Verify polarity before applying power
- Disconnect power before making connection changes
- Maximum current per I/O pin: 7mA (typical), use resistors appropriately

---

## Additional Resources

- [PIC32CM LS00 Datasheet](https://www.microchip.com/en-us/product/PIC32CM5164LS00048)
- [Curiosity Nano Explorer User Guide](https://www.microchip.com/en-us/development-tool/AC164161)
- [mikroBUS™ Standard Specification](https://www.mikroe.com/mikrobus)

---

**Document Version:** 1.0  
**Last Updated:** 2026-01-06  
**Course:** EEE 158 - Electrical and Electronics Engineering Laboratory V
