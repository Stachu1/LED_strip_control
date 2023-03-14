#include <FastLED.h>
#include <EEPROM.h>


#define BUTTON_1_PIN 2
#define BUTTON_2_PIN 4
#define POTENTIOMETER_PIN A5

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


#define CEILING_LIGHT_MODES_COUNT 1 + ANIMATIONS_COUNT * PALETTES_COUNT

uint8_t glitter_fade_out_delay;
uint8_t dots_fade_out_delay;
uint8_t dots_fade_out_step;

bool desk_light_on;
bool ceiling_light_on;

uint8_t desk_light_brightness;
uint8_t ceiling_light_brightness;
uint8_t ceiling_light_brightness_gamma_corrected;

uint8_t ceiling_light_mode;

bool button_1_down = false;
bool button_2_down = false;

unsigned long button_1_down_time;
unsigned long button_2_down_time;


CRGB leds[NUM_LEDS];



DEFINE_GRADIENT_PALETTE (gp1)
{
    0, 255,   0,   0,
   64, 255, 255,   0,
  191, 255, 255, 255,
  255, 255,   0,   0
};

DEFINE_GRADIENT_PALETTE (gp2)
{
    0, 255, 165,  15,
   64, 255,  50, 160,
  191,  110,  0, 255,
  255, 255, 165,  15
};

DEFINE_GRADIENT_PALETTE (gp3)
{
    0,   0,   0, 255,
   64,   0, 255,   0,
  191,   0, 255, 255,
  255,   0,   0, 255
};

DEFINE_GRADIENT_PALETTE (gp4)
{
    0, 255,   0,   0,
   64,   0,   0, 255,
  191, 255,   0, 255,
  255, 255,   0,   0
};

DEFINE_GRADIENT_PALETTE (gp5)
{
    0, 255,   0,   0,
   64,   0, 255,   0,
  191,   0,   0, 255,
  255, 255,   0,   0
};

CRGBPalette16 PALETTES[] = { gp1, gp2, gp3, gp4, gp5 };




void setup()
{
  //TCCR2B = TCCR2B & B11111000 | B00000001;   // Set pwm frequency to 31372.55 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000010;   // Set pwm frequency to 3921.16 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000011;   // Set pwm frequency to 980.39 Hz
  

  Serial.begin(9600);
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(POTENTIOMETER_PIN, INPUT);

  pinMode(DESK_LIGHT_PIN, OUTPUT);
  pinMode(CEILING_LIGHT_PIN, OUTPUT);

  LoadSettingsFromEEPROM();
  UpdateBrightnessRelated();
  FastLED.addLeds<LED_TYPE, CEILING_LIGHT_PIN, GRB>(leds, NUM_LEDS);
}



void loop()
{
  UpdateButtons();
  UpdateDeskLight();
  UpdateCeilingLight();
}



void UpdateCeilingLight()
{
  static uint8_t mode = 255;
  if (ceiling_light_mode != mode)
  {
    mode = ceiling_light_mode;
    FastLED.clear();
  }

  if (ceiling_light_on)
  {
    if (mode == 0)
    {
      Rainbow();
    }
    else
    {
      uint8_t index = (mode-1) % PALETTES_COUNT;
      if (mode <= PALETTES_COUNT)
      {
        MovePalette(PALETTES[index]);
      }
      else if (mode <= PALETTES_COUNT * 2)
      {
        Glitter(PALETTES[index]);
      }
      else if (mode <= PALETTES_COUNT * 3)
      {
        Breath(PALETTES[index]);
      }
      else if (mode <= PALETTES_COUNT * 4)
      {
        DotsMove(PALETTES[index]);
      }
    }
  }
  else
  {
    FastLED.clear();
  }
  FastLED.show();
}



void DotsMove(CRGBPalette16 palette)
{
  static const uint8_t OFF_SET = NUM_LEDS / NUM_DOTS;
  static uint8_t color_index = 255;
  static uint16_t pos = 0;
  static bool reverse = false;
  EVERY_N_MILLISECONDS(1000/DOTS_PPS)
  {
    for (uint16_t i = 0; i < NUM_DOTS; i++)
    {
      uint16_t dot_pos = pos + i * OFF_SET;
      if (dot_pos > NUM_LEDS)
      {
        dot_pos -=  NUM_LEDS;
      }
      leds[dot_pos] = ColorFromPalette(palette, color_index, ceiling_light_brightness, LINEARBLEND);
    }

    if (reverse)
    {
      pos--;
      if (pos == 0)
      {
        reverse = false;
      }
    }
    else
    {
      pos++;
      if (pos == NUM_LEDS - 1)
      {
        reverse = true;
      }
    }
  }
  EVERY_N_MILLISECONDS(DOTS_COLOR_CHANGE_TIME/255)
  {
    color_index--;
  }
  EVERY_N_MILLISECONDS(dots_fade_out_delay)
  {
    fadeToBlackBy(leds, NUM_LEDS, dots_fade_out_step);
  }
}


void Breath(CRGBPalette16 palette)
{
  static uint8_t index = 255;
  EVERY_N_MILLISECONDS(1000/BREATH_FPS)
  {
    uint8_t sinBeat = beatsin8(BPM, 0, 255, 0, 0);
    uint8_t brightness = sinBeat * ceiling_light_brightness / 255;
    fill_solid(leds, NUM_LEDS, ColorFromPalette(palette, index, brightness, LINEARBLEND));
  }
  EVERY_N_MILLISECONDS(BREATH_COLOR_CHANGE_TIME/255)
  {
    index--;
  }
}



void Glitter(CRGBPalette16 palette)
{
  EVERY_N_MILLISECONDS(1000/GLITTER_PPS)
  {
    leds[random(0, NUM_LEDS - 1)] = ColorFromPalette(palette, random8(), ceiling_light_brightness, LINEARBLEND);
  }

  EVERY_N_MILLISECONDS(glitter_fade_out_delay)
  {
    fadeToBlackBy(leds, NUM_LEDS, 1);
  }
}



void MovePalette(CRGBPalette16 palette)
{
  static uint8_t index = 255;
  EVERY_N_MILLISECONDS(1000/PALETTE_MOVE_FPS)
  {
    fill_palette(leds, NUM_LEDS, index, PALETTE_COLOR_DELTA, palette, ceiling_light_brightness, LINEARBLEND);
    index--;
  }
}



void Rainbow()
{
  static uint8_t hue = 255;
  static const float hue_factor = 255.0f / (float)NUM_LEDS;
  EVERY_N_MILLISECONDS(1000/RAINBOW_FPS)
  {
    for (uint16_t i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(hue + uint8_t(i * hue_factor), 255, ceiling_light_brightness_gamma_corrected);
    }
    hue--;
  }
}


void UpdateBrightnessRelated()
{
  ceiling_light_brightness_gamma_corrected = GammaCorrection(ceiling_light_brightness);
  glitter_fade_out_delay = GLITTER_FADE_OUT_TIME / ceiling_light_brightness;
  dots_fade_out_step = 20 * ceiling_light_brightness / 255;
  dots_fade_out_delay = (1000 * DOTS_FADE_OUT_LENGTH) / (ceiling_light_brightness * DOTS_PPS);
}



uint8_t GammaCorrection(uint8_t brightness)
{
  static const double gamma = 1/(double)GAMMA;
  uint8_t corrected_brightness = pow((double)brightness/255, gamma) * 255;
  return corrected_brightness;
}



void UpdateDeskLight()
{
  if (DESK_LIGHT_PWM)
    {
    if (desk_light_on && desk_light_brightness != 0)
    {
      if (desk_light_brightness == 255)
      {
        digitalWrite(DESK_LIGHT_PIN, HIGH);
      }
      else
      {
        analogWrite(DESK_LIGHT_PIN, desk_light_brightness);
      }    
    }
    else
    {
      digitalWrite(DESK_LIGHT_PIN, LOW);
    }
  }
  else
  {
    if (desk_light_on)
    {
      digitalWrite(DESK_LIGHT_PIN, HIGH);
    }
    else
    {
      digitalWrite(DESK_LIGHT_PIN, LOW);
    }
  }
}



void UpdateButtons()
{
  uint8_t b1_state = digitalRead(BUTTON_1_PIN);
  uint8_t b2_state = digitalRead(BUTTON_2_PIN);
  bool b1_held = false;
  bool b2_held = false;
  
  if (b1_state == 0 && !button_1_down) // Button down   BUTTON 1
  {
    button_1_down = true;
    button_1_down_time = millis();
  }
  if(b1_state == 1 && button_1_down && millis() - button_1_down_time >= BUTTON_CLICK_TIME)  // Button up
  {
    button_1_down = false;
    if (millis() - button_1_down_time < BUTTON_HOLD_TIME) // Button's been clicked
    {
      if (!button_2_down)
      {
        desk_light_on = !desk_light_on;     // Change the desk light state
        EEPROM.update(EEPROM_OFFSET, desk_light_on);
      }
      else
      {
        if (ceiling_light_mode == 0)      // Change the ceiling light mode
        {
          ceiling_light_mode = CEILING_LIGHT_MODES_COUNT;
        }
        ceiling_light_mode--;
        EEPROM.update(EEPROM_OFFSET + 4, ceiling_light_mode);
        ceiling_light_on = true;
        EEPROM.update(EEPROM_OFFSET + 2, ceiling_light_on);
      }
    }
    else       // Button's been pressed
    {
      desk_light_brightness = GetBrightness();
      EEPROM.update(EEPROM_OFFSET + 1, desk_light_brightness);
    }
  }

  if (button_1_down && millis() - button_1_down_time >= BUTTON_HOLD_TIME)
  {
    b1_held = true;
    desk_light_brightness = GetBrightness();
  }


  if (b2_state == 0 && !button_2_down) // Button down   BUTTON 2
  {
    button_2_down = true;
    button_2_down_time = millis();
  }
  if(b2_state == 1 && button_2_down && millis() - button_2_down_time >= BUTTON_CLICK_TIME)  // Button up
  {
    button_2_down = false;
    if (millis() - button_2_down_time < BUTTON_HOLD_TIME) // Button's been clicked
    {
      if (!button_1_down)
      {
        ceiling_light_on = !ceiling_light_on;
        EEPROM.update(EEPROM_OFFSET + 2, ceiling_light_on);
      }
      else
      {
        if (ceiling_light_mode == CEILING_LIGHT_MODES_COUNT - 1)      // Change the ceiling light mode
        {
          ceiling_light_mode = 0;
        }
        else
        {
          ceiling_light_mode++;
        }
        EEPROM.update(EEPROM_OFFSET + 4, ceiling_light_mode);
        ceiling_light_on = true;
        EEPROM.update(EEPROM_OFFSET + 2, ceiling_light_on);
      }
    }
    else       // Button's been pressed
    {
      ceiling_light_brightness = GetBrightness();
      UpdateBrightnessRelated();
      EEPROM.update(EEPROM_OFFSET + 3, ceiling_light_brightness);
    }
  }
  if (button_2_down && millis() - button_2_down_time >= BUTTON_HOLD_TIME)
  {
    b2_held = true;
    ceiling_light_brightness = GetBrightness();
    UpdateBrightnessRelated();
  }

  if (b1_held && b2_held)
  {
    ceiling_light_mode = 0;
    EEPROM.update(EEPROM_OFFSET + 4, ceiling_light_mode);
    ceiling_light_on = true;
    EEPROM.update(EEPROM_OFFSET + 2, ceiling_light_on);
  }
}



uint8_t GetBrightness()
{
  uint16_t potentiometer_value = analogRead(POTENTIOMETER_PIN);
  uint8_t brightness = map(potentiometer_value, POTENTIOMETER_LOW, POTENTIOMETER_HIGH, 0, 255);
  if (POTENTIOMETER_REVERSE)
  {
    brightness = 255 - brightness;
  }
  return brightness;
}



void LoadSettingsFromEEPROM()
{
  desk_light_on = EEPROM.read(EEPROM_OFFSET);
  desk_light_brightness = EEPROM.read(EEPROM_OFFSET + 1);
  ceiling_light_on = EEPROM.read(EEPROM_OFFSET + 2);
  ceiling_light_brightness = EEPROM.read(EEPROM_OFFSET + 3);
  ceiling_light_mode = EEPROM.read(EEPROM_OFFSET + 4);
}