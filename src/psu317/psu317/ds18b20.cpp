#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);	

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

OneWire ds(2);
byte addrS1[8];
byte addrS2[8];
byte busStatus;
byte data[9];
float temp; 
void setupSearch();

void setupds18b20(void)
{
  sensors.begin();	// Start up the library
  Serial.begin(115200);

  ds.reset();
  ds.search(addrS1); //getting the address of Sesnor-1

  setupSearch(); 
}

void loopds18b20(void)
{ 
  // Send the command to get temperatures
  sensors.requestTemperatures(); 

  //print the temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(sensors.getTempCByIndex(0));

  Serial.print("°C  |  ");
 
  //print the temperature in Fahrenheit
  Serial.print((sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0);
 
  Serial.println("°F");
  
  delay(500);
}
