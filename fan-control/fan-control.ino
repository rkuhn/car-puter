#include <Arduino_APDS9960.h>
#include <Arduino_HS300x.h>

void red()
{
  analogWrite(LEDR, 220);
  analogWrite(LEDG, 255);
  analogWrite(LEDB, 255);
}

void green()
{
  analogWrite(LEDR, 255);
  analogWrite(LEDG, 255);
  analogWrite(LEDB, 220);
}

void yellow()
{
  analogWrite(LEDR, 230);
  analogWrite(LEDG, 255);
  analogWrite(LEDB, 230);
}

void flash()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

void setup()
{
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  Serial.begin(9600);

  if (!APDS.begin() || !HS300x.begin())
  {
    digitalWrite(LED_BUILTIN, HIGH);
    while (true)
    {
      delay(100);
    }
  }
}

typedef void (*color_t)();
static color_t colors[] = {red, green, yellow};
static const int num_colors = sizeof(colors) / sizeof(colors[0]);

void loop()
{
  static byte color = 0;
  if (APDS.gestureAvailable())
  {
    int gesture = APDS.readGesture();
    Serial.println(gesture);
    for (int i = 0; i <= gesture; i++)
    {
      flash();
    }
    if (gesture == GESTURE_UP)
    {
      color = (color + 1) % num_colors;
      colors[color]();
    }
    else if (gesture == GESTURE_DOWN)
    {
      color = (color + num_colors - 1) % num_colors;
      colors[color]();
    }
  }

  static float temp = 0;
  float new_temp = HS300x.readTemperature(CELSIUS);
  if (new_temp != temp)
  {
    temp = new_temp;
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println("°C");
  }
}
