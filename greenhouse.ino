#include <iocontrol.h>
#include <BH1750.h>
#include <WiFi.h>
#include <FastLED.h>  // конфигурация матрицы // LED matrix configuration
//#include <FastLED_GFX.h>
//#include <FastLEDMatrix.h>
#include <MCP3221.h>
#include <Adafruit_MCP9808.h>
#define NUM_LEDS 64      // количество светодиодов в матрице // number of LEDs
CRGB leds[NUM_LEDS];     // определяем матрицу (FastLED библиотека) // defining the matrix (fastLED library)
#define LED_PIN 18       // пин к которому подключена матрица // matrix pin
#define COLOR_ORDER GRB  // порядок цветов матрицы // color order
#define CHIPSET WS2812   // тип светодиодов // LED type

int red[64] = { 1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                1, 1, 1, 1, 1, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 1
              };

int purple[64] = { 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0,
                   1, 1, 1, 1, 1, 1, 1, 1,
                   1, 1, 1, 1, 1, 1, 1, 1,
                   1, 1, 1, 1, 1, 1, 1, 1,
                   1, 1, 1, 1, 1, 1, 1, 1,
                   0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0
                 };

// Название панели на сайте iocontrol.ru
const char* myPanelName = "teplica228";
int status;
#include <Wire.h>
BH1750 lightMeter;
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;
#include <ESP32Servo.h>
// Название переменных как на сайте iocontrol.ru
const char* VarName_temp = "temp";
const char* VarName_hud = "hud";
const char* VarName_pre = "pre";
const char* VarName_wind = "wind";
const char* VarName_pump = "pump";
const char* VarName_osvet = "osvet";
const char* VarName_hud_d = "hud_d";
const char* VarName_window = "window";
const char* VarName_temp_d = "temp_d";
const char* VarName_svetred = "svetred";
const char* VarName_svetyellow = "svetyellow";

const char* ssid = "myphone";
const char* password = "19681313";
Servo myservo;
int pos = 1;        // начальная позиция сервомотора // servo start position
int prevangle = 1;  // предыдущий угол сервомотора // previous angle of servo
// Создаём объект клиента
WiFiClient client;
MCP3221 mcp3221_5(0x4E); // Адрес может отличаться(также попробуйте просканировать адрес: https://github.com/MAKblC/Codes/tree/master/I2C%20scanner)
int a = 2248; // масштабирование значений
int b = 1165;
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
#define pump 17  // пин насоса // pump pin
#define wind 16  // пин вентилятора // cooler pin
// Создаём объект iocontrol, передавая в конструктор название панели и клиента
iocontrol mypanel(myPanelName, client);
#define MGB_D1015 1
void setup()
{
  FastLED.addLeds<WS2812, LED_PIN>(leds, NUM_LEDS);  // настройки матрицы
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  bool bme_status = bme280.begin();
  if (!bme_status) {
    Serial.println("Не найден по адресу 0х77, пробую другой...");
    bme_status = bme280.begin(0x76);
    if (!bme_status)
      Serial.println("Датчик не найден, проверьте соединение");
  }
  // Ждём подключения
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  lightMeter.begin();
  mcp3221_5.setAlpha(DEFAULT_ALPHA);
  mcp3221_5.setNumSamples(DEFAULT_NUM_SAMPLES);
  mcp3221_5.setSmoothing(ROLLING_AVG);

  if (!tempsensor.begin(0x18))
  {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }
  tempsensor.setResolution(3);
  // Вызываем функцию первого запроса к сервису
  mypanel.begin();
  myservo.attach(19);
  pinMode(pump, OUTPUT);
  pinMode(wind, OUTPUT);    // настройка пинов насоса и вентилятора на выход // pump and cooler pins configured on output mode
  digitalWrite(pump, LOW);  // устанавливаем насос и вентилятор изначально выключенными // turn cooler and pump off
  digitalWrite(wind, LOW);
}


void loop()
{

  // ************************ ЧТЕНИЕ ************************

  // Чтение значений переменных из сервиса
  status = mypanel.readUpdate();
  // Если статус равен константе OK...
  if (status == OK) {
    // Выводим текст в последовательный порт
    Serial.println("------- Read OK -------");
    // Записываем считанный из сервиса значения в переменные
    long io_window = mypanel.readInt(VarName_window);   // целочисленная переменна
    Serial.println((String)"io_window = " + io_window);
    long io_wind = mypanel.readInt(VarName_wind);   // целочисленная переменна
    Serial.println((String)"io_wind = " + io_wind);
    long io_pump = mypanel.readInt(VarName_pump);   // целочисленная переменна
    Serial.println((String)"io_pump = " + io_pump);
    long io_svetred = mypanel.readInt(VarName_svetred);   // целочисленная переменна
    Serial.println((String)"io_svetred = " + io_svetred);
    long io_svetyellow = mypanel.readInt(VarName_svetyellow);   // целочисленная переменна
    Serial.println((String)"io_svetyellow = " + io_svetyellow);
    if (io_window == 0) {
      myservo.write(105);
    }
    if (io_window == 1) {
      myservo.write(5);
    }
    if (io_wind == 1) {
      digitalWrite(wind, HIGH);
    }
    if (io_wind == 0) {
      digitalWrite(wind, LOW);
    }
    if (io_pump == 1) {
      digitalWrite(pump, HIGH);
    }
    if (io_pump == 0) {
      digitalWrite(pump, LOW);
    }
    if (io_svetred == 1) {
      for (int i = 0; i < 64; i++) {
        if (red[i] == 1) leds[63 - i] = CRGB(0, 255, 0);
      }
      FastLED.show();
    }
    if (io_svetred == 0) {
      for (int i = 0; i < 64; i++) {
        if (red[i] == 1) leds[63 - i] = CRGB(0, 0, 0);
      }
      FastLED.show();
    }
    if (io_svetyellow == 1) {
      for (int i = 0; i < 64; i++) {
        if (purple[i] == 1) leds[63 - i] = CRGB(165, 255, 0);
      }
      FastLED.show();
    }
    if (io_svetyellow == 0) {
      for (int i = 0; i < 64; i++) {
        if (purple[i] == 1) leds[63 - i] = CRGB(0, 0, 0);
      }
      FastLED.show();
    }
  }

  // ************************ ЗАПИСЬ ************************
  // Измерение
  float t = bme280.readTemperature();
  float h = bme280.readHumidity();
  float p = bme280.readPressure() / 100.0F;
  float lux = lightMeter.readLightLevel(); // считывание освещенности
  tempsensor.wake();
  float t1 = tempsensor.readTempC();
  int hj = mcp3221_5.getData();
  hj = map(hj, a, b, 0, 100);
  // Записываем  значение в переменную для отпраки в сервис
  mypanel.write(VarName_hud, h);   // Записали значение 1.2345
  mypanel.write(VarName_temp, t);   // Записали значение 1.2345
  mypanel.write(VarName_osvet, lux);
  mypanel.write(VarName_hud_d, hj);    // Записали значение 12345
  mypanel.write(VarName_temp_d, t1);   // Записали значение 1.2345

  // Отправляем переменные из контроллера в сервис
  status = mypanel.writeUpdate();
  // Если статус равен константе OK...
  if (status == OK) {
    Serial.println("------- Write OK -------");
  }
}
