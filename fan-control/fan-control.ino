#include <Arduino_APDS9960.h>
#include <Wire.h>
#include <ArduinoBLE.h>
#include <mbed.h>

#define SERIAL 0

#define FAN_DRIVER D0
#define FAN_ON HIGH
#define FAN_OFF LOW
#define TEMP_HIGH 30.0
#define TEMP_LOW 29.0

/**
 * NOTE: On my particular board LEDB and LEDG are swapped.
 */

#define COLOR(name, red, green, blue)   \
  void name()                           \
  {                                     \
    analogWrite(LEDR, 255 - red / 5);   \
    analogWrite(LEDB, 255 - green / 5); \
    analogWrite(LEDG, 255 - blue / 5);  \
  }

COLOR(red, 255, 0, 0)
COLOR(green, 0, 255, 0)
COLOR(blue, 0, 0, 255)
COLOR(yellow, 255, 255, 0)
COLOR(purple, 160, 32, 240)

typedef void (*color_t)();
static color_t colors[] = {red, green, yellow, blue, purple};
static const int num_colors = sizeof(colors) / sizeof(colors[0]);

enum Mode
{
  OFF = 0,
  ON = 1,
  AUTO = 2,
  AUTO_ON = 3,
  THROTTLE = 4,
};

float humidity = 0;
float temperature = 0;

volatile byte throttle = 0;

const char *serviceUuid = "4625E9D0-EA59-4E6C-A81D-282F46BC25A9";
const char *modeUuid = "7356A709-0235-4976-91AB-49F8AC825442";
const char *temperatureUuid = "9E762D6E-4034-4407-B4F3-94579F8658FE";
const char *humidityUuid = "B56F03B4-D496-48BF-8BFC-A54D19FC1D0D";
const char *throttleUuid = "9A76D379-96CD-4BC7-979E-15982AF7A1E9";

BLEService fanService(serviceUuid);
BLEByteCharacteristic modeCharacteristic(modeUuid, BLERead | BLEWrite | BLENotify);
BLEFloatCharacteristic temperatureCharacteristic(temperatureUuid, BLERead | BLENotify);
BLEFloatCharacteristic humidityCharacteristic(humidityUuid, BLERead | BLENotify);
BLEByteCharacteristic throttleCharacteristic(throttleUuid, BLERead | BLEWrite | BLENotify);

mbed::Ticker timer;

void setup()
{
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FAN_DRIVER, OUTPUT);

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

#if SERIAL
  Serial.begin(9600);
  while (!Serial)
    ;
#endif

  if (!APDS.begin() || !BLE.begin())
  {
    digitalWrite(LED_BUILTIN, HIGH);
    while (true)
    {
      delay(100);
    }
  }

  Wire1.begin();

  BLE.setLocalName("Fan Control");
  BLE.setAdvertisedService(fanService);
  fanService.addCharacteristic(modeCharacteristic);
  fanService.addCharacteristic(temperatureCharacteristic);
  fanService.addCharacteristic(humidityCharacteristic);
  fanService.addCharacteristic(throttleCharacteristic);
  BLE.addService(fanService);
  BLE.advertise();

  timer.attach_us(&run_throttle, 1000);
}

void loop()
{
  static Mode mode = OFF;
  static byte first = 1;
  if (first)
  {
    colors[mode]();
    first = 0;
  }

  if (APDS.gestureAvailable())
  {
    int gesture = APDS.readGesture();
#if SERIAL
    Serial.print("Gesture: ");
    Serial.println(gesture);
#endif
    if (gesture == GESTURE_UP)
    {
      mode = OFF;
    }
    else if (gesture == GESTURE_DOWN)
    {
      mode = ON;
    }
    else
    {
      mode = AUTO;
    }
    colors[mode]();
    modeCharacteristic.writeValue(mode);
  }

  static unsigned long last_hs3003 = 0;
  static bool hs3003_read = false;

  unsigned long now = millis();
  if (now - last_hs3003 > 1000)
  {
    last_hs3003 = now;
    wakeHS3003();
    hs3003_read = true;
  }
  else if ((now - last_hs3003 > 40) && hs3003_read)
  {
    readHS3003();
    hs3003_read = false;
  }

  static bool fan = false;
  if (temperature > TEMP_HIGH && !fan)
  {
    fan = true;
  }
  else if (temperature < TEMP_LOW && fan)
  {
    fan = false;
  }
  if (mode == AUTO && fan)
  {
    mode = AUTO_ON;
    colors[mode]();
    modeCharacteristic.writeValue(mode);
  }
  else if (mode == AUTO_ON && !fan)
  {
    mode = AUTO;
    colors[mode]();
    modeCharacteristic.writeValue(mode);
  }

  static BLEDevice central = BLE.central();
  if (!central)
  {
    central = BLE.central();
  }

  if (central)
  {
    central.poll();

    if (modeCharacteristic.written())
    {
#if SERIAL
      Serial.print("New mode: ");
      Serial.println(modeCharacteristic.value());
#endif
      mode = (Mode)modeCharacteristic.value();
      throttle = 0;
      throttleCharacteristic.writeValue(0);
      colors[mode]();
    }
    if (throttleCharacteristic.written())
    {
#if SERIAL
      Serial.print("New throttle: ");
      Serial.println(throttleCharacteristic.value());
#endif
      throttle = throttleCharacteristic.value();
      mode = throttle == 0 ? ON : THROTTLE;
      modeCharacteristic.writeValue(mode);
      colors[mode]();
    }
  }

  if (mode != THROTTLE)
  {
    digitalWrite(FAN_DRIVER, mode == ON || mode == AUTO_ON ? FAN_ON : FAN_OFF);
    throttle = 0;
  }
}

void wakeHS3003()
{
  Wire1.beginTransmission(0x44);
  Wire1.write(0x00);
  Wire1.endTransmission();
}

void readHS3003()
{
  Wire1.requestFrom(0x44, 4);
  if (!Wire1.available() == 4)
  {
#if SERIAL
    Serial.println("Error: HS3003 did not return 4 bytes");
#endif
    return;
  }
  uint16_t hum = (Wire1.read() << 8) | Wire1.read();
  uint16_t temp = (Wire1.read() << 8) | Wire1.read();

  if (hum & 0xc000)
  {
#if SERIAL
    Serial.println("Error: HS3003 returned stale data");
#endif
    return;
  }

  hum &= 0x3FFF;
  temp >>= 2;

  humidity = ((float)hum / 16383) * 100;
  temperature = ((float)temp / 16383) * 165 - 40;

  temperatureCharacteristic.writeValue(temperature);
  humidityCharacteristic.writeValue(humidity);

#if SERIAL
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Temperature: ");
  Serial.print(temperature);
  Serial.println("C");
#endif
}

// this is called every 1ms
void run_throttle()
{
  // a cycle begins by switching on the power supply; it takes 2ms for the buck converter to start
  // after 2+throttle ms switch off the power again and then wait at least 1ms (converter stops after 600µs)
  static byte cycle = 0;

  if (throttle == 0)
  {
    cycle = 0;
    return;
  }

  byte switch_off = throttle < 10 ? 4 : throttle - 6;
  byte restart = switch_off + 1;
  switch (throttle)
  {
  case 1:
    restart = 22;
    break;
  case 2:
    restart = 18;
    break;
  case 3:
    restart = 15;
    break;
  case 4:
    restart = 13;
    break;
  case 5:
    restart = 11;
    break;
  case 6:
    restart = 9;
    break;
  case 7:
    restart = 7;
    break;
  case 8:
    restart = 6;
    break;
  case 9:
    restart = 5;
    break;
  }

  if (cycle == 0)
    digitalWrite(FAN_DRIVER, FAN_ON);
  else if (cycle == switch_off)
    digitalWrite(FAN_DRIVER, FAN_OFF);

  cycle++;

  if (cycle == restart)
    cycle = 0;
}
