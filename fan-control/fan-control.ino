#include <Arduino_APDS9960.h>
#include <Wire.h>
#include <ArduinoBLE.h>

#define SERIAL 0

#define FAN_DRIVER D12
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
};

float humidity = 0;
float temperature = 0;

const char *serviceUuid = "4625E9D0-EA59-4E6C-A81D-282F46BC25A9";
const char *modeUuid = "7356A709-0235-4976-91AB-49F8AC825442";
const char *temperatureUuid = "9E762D6E-4034-4407-B4F3-94579F8658FE";
const char *humidityUuid = "B56F03B4-D496-48BF-8BFC-A54D19FC1D0D";

BLEService fanService(serviceUuid);
BLEByteCharacteristic modeCharacteristic(modeUuid, BLERead | BLEWrite);
BLEFloatCharacteristic temperatureCharacteristic(temperatureUuid, BLERead);
BLEFloatCharacteristic humidityCharacteristic(humidityUuid, BLERead);

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
  BLE.addService(fanService);
  BLE.advertise();
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

  BLEDevice central = BLE.central();
  if (central)
  {
    if (modeCharacteristic.written())
    {
#if SERIAL
      Serial.print("New mode: ");
      Serial.println(modeCharacteristic.value());
#endif
      mode = (Mode)modeCharacteristic.value();
      colors[mode]();
    }
  }

  digitalWrite(FAN_DRIVER, mode == ON || mode == AUTO_ON);
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
