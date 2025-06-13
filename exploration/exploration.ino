#include "mbed.h"

void setup()
{
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  Serial.begin(115200UL);
  while (!Serial)
    ;
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, LOW);
  delay(100);
  Serial.println("hello");

  int x = 0;
  Serial.print("x = ");
  Serial.print(sizeof(x));
  Serial.print(" bytes, address = ");
  Serial.println((int)&x, HEX);
  Serial.println((int)setup, HEX);
  Serial.println((int)(new int[3]), HEX);

  mbed::FlashIAP flash;

  flash.init();
  Serial.print("flash size = ");
  Serial.println(flash.get_flash_size());
  Serial.print("page size = ");
  Serial.println(flash.get_page_size());
  Serial.print("sector size = ");
  Serial.println(flash.get_sector_size(100000));
  Serial.print("start address = ");
  Serial.println(flash.get_flash_start(), HEX);
}

void loop() {}
