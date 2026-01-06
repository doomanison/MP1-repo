# MP1-repo - EEE 158 Machine Problem 1

## Colorful Lighting

This project implements RGB LED control using PWM on the **PIC32CM LS00 Curiosity Nano+ Touch Evaluation Kit** with the **Curiosity Nano Explorer** board.

### Features
- Display 5 different colors in a specified sequence
- Brightness control via potentiometer (SW1 mode)
- Automatic color cycling with variable speed and direction (SW2 mode)
- Hardware debouncing on switches
- ADC input filtering for potentiometer

### Hardware
- **MCU:** PIC32CM LS00
- **Board:** Curiosity Nano Explorer
- **RGB LED:** Common Anode

### Pin Mappings
| PIC32 Pin | Function |
|-----------|----------|
| PA00 | SW1 Pushbutton |
| PA01 | SW2 Pushbutton |
| PA03 | PWM Red Channel |
| PA06 | PWM Green Channel |
| PB02 | Potentiometer |
| PB03 | PWM Blue Channel |

### Color Sequence
1. #f7ff84 (Light Yellow)
2. #ee65ff (Pink/Magenta)
3. #9726ff (Purple)
4. #3542ff (Blue)
5. #ff216c (Red/Pink)

---
EEE 158: Electrical and Electronics Engineering Laboratory V
