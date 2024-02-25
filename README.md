# Addressable LEDs controller
Meant to be run on an Arduino-like board.
Simple hardware requires only two buttons and a potentiometer to control the brightness.
Uses EEPROM for storing the last settiing and the [FastLED](https://fastled.io/) library for controlling the LEDs.

## Download
```bash
git clone https://github.com/Stachu1/LED_strip_control.git
```

## Configuration
```C
#define DESK_LIGHT_PIN 3       // Pin to whitch MOSFET is connected
#define CEILING_LIGHT_PIN 10   // Pin to whitch LED strip is connected
#define NUM_LEDS 408           // Number of LEDs
#define LED_TYPE WS2812        // Type of LEDs
#define EEPROM_OFFSET 10       // EEPROM offset defines where in the EEPROM data should be saved
#define BUTTON_HOLD_TIME 500  // Time button has to be held in order to be recognized as held (milliseconds)
#define BUTTON_CLICK_TIME 50   // Time button has to be held in oreder to be recoginzed as clicked (milliseconds)
#define DESK_LIGHT_PWM false   // Enable of disable brihtness controll of the desk light
#define POTENTIOMETER_LOW 0    // Potentiometer range
#define POTENTIOMETER_HIGH 1023
#define POTENTIOMETER_REVERSE true    // Reverse potentiometer direction
#define ANIMATIONS_COUNT 4
#define PALETTES_COUNT 5
#define GAMMA 2.2   // Gamma correction


// Animations settings
#define RAINBOW_FPS 30  // Updates per second (256 updates == one cycle)

#define PALETTE_MOVE_FPS 100   // Updates per second
#define PALETTE_COLOR_DELTA 8   // How much the palette should change between LEDs

#define GLITTER_PPS 30    // New points per second
#define GLITTER_FADE_OUT_TIME 3000    // How long it should take for a pixel to fade out (milliseconds)

#define BREATH_FPS 100   // Update per second
#define BPM 20      // Breaths per minute
#define BREATH_COLOR_CHANGE_TIME 10000    //Time it takes to go through a whole palette (milliseconds)

#define DOTS_PPS 100    // Updates/speed per second (408 == one cycle in one second)
#define NUM_DOTS 6      // Number of dots
#define DOTS_FADE_OUT_LENGTH 25    // Lenght of the fade out effect (pixels)
#define DOTS_COLOR_CHANGE_TIME 4000   //Time it takes to go through a whole palette (milliseconds)
```

## TIPS
1. If you have a long LED stirp remember to add power injections so the color is consistant
2. It's a good idea to add a decoupling capacitor
3. Make sure that the power supply can handle all the LEDs at max brightness

## DEMO
<img width="1024" alt="image" src="https://github.com/Stachu1/LED_strip_control/assets/77758413/cf3c0962-e5d7-46c9-974e-bff0d8291310">
