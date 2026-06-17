#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

Adafruit_ADS1115 ads;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =========================
// Parameter
// =========================
const int SAMPLE_COUNT = 32;

const float TDS_FACTOR = 0.5;
const float CALIBRATION_FACTOR = 1.0;

// =========================
// Kategori
// =========================
String getWaterCategory(float tds)
{
  if (tds < 300)
    return "Excellent";
  else if (tds < 600)
    return "Good";
  else if (tds < 900)
    return "Fair";
  else if (tds < 1200)
    return "Poor";
  else
    return "Unacceptable";
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Wire.begin(21, 22);

  Serial.println();
  Serial.println("========================");
  Serial.println(" HYDROSENSE DEBUG MODE ");
  Serial.println("========================");

  if (!ads.begin())
  {
    Serial.println("ADS1115 TIDAK TERDETEKSI!");
    while (1);
  }

  ads.setGain(GAIN_ONE);

  dht.begin();

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HydroSense");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);
}

void loop()
{
  // =========================
  // Suhu
  // =========================
  float temperature = dht.readTemperature();

  if (isnan(temperature))
  {
    temperature = 25.0;
  }

  // =========================
  // Sampling ADS1115
  // =========================
  long totalADC = 0;

  for (int i = 0; i < SAMPLE_COUNT; i++)
  {
    totalADC += ads.readADC_SingleEnded(0);
    delay(10);
  }

  float avgADC = totalADC / (float)SAMPLE_COUNT;

  // =========================
  // ADC -> Volt
  // =========================
  float voltage = ads.computeVolts((int16_t)avgADC);

  if (voltage < 0)
    voltage = 0;

  // =========================
  // Rumus EC
  // =========================
  float ec =
      (133.42 * voltage * voltage * voltage)
      - (255.86 * voltage * voltage)
      + (857.39 * voltage);

  // =========================
  // Kompensasi suhu
  // =========================
  float ec25 =
      ec /
      (1.0 + 0.02 * (temperature - 25.0));

  // =========================
  // TDS
  // =========================
  float tdsValue =
      ec25 *
      TDS_FACTOR *
      CALIBRATION_FACTOR;

  if (tdsValue < 0)
    tdsValue = 0;

  String category = getWaterCategory(tdsValue);

  // =========================
  // SERIAL DEBUG
  // =========================
  Serial.println();
  Serial.println("==================================");
  Serial.println("       HYDROSENSE DEBUG");
  Serial.println("==================================");

  Serial.print("ADC Average      : ");
  Serial.println(avgADC);

  Serial.print("Voltage (V)      : ");
  Serial.println(voltage, 6);

  Serial.print("Temperature (C)  : ");
  Serial.println(temperature, 2);

  Serial.print("EC Raw           : ");
  Serial.println(ec, 3);

  Serial.print("EC25             : ");
  Serial.println(ec25, 3);

  Serial.print("TDS (ppm)        : ");
  Serial.println(tdsValue, 1);

  Serial.print("Category         : ");
  Serial.println(category);

  Serial.println("==================================");

  // =========================
  // LCD HALAMAN 1
  // =========================
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("TDS:");
  lcd.print((int)tdsValue);
  lcd.print("ppm");

  lcd.setCursor(0, 1);
  lcd.print(category);

  delay(3000);

  // =========================
  // LCD HALAMAN 2
  // =========================
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(temperature, 1);
  lcd.print((char)223);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("V:");
  lcd.print(voltage, 3);

  delay(3000);
}
