/* FILE:    Data_Logger_Shield_HCARDU0093_Example
   DATE:    24/02/15
   VERSION: 0.1

REVISIONS:

24/02/15 Created version 0.1

This is an example of how to use the Hobby Components data logger shield
(HCMODU0093). This shield contains a battery backed DS1307 real time clock
and an SD card interface. This allows any sensor data read by your Arduino
to be stored with an accurate time stamp to an SD card. In this example sketch
we will continually read the value of an LM35 temperature sensor and store the
result together with the current time and date to a CSV file. This file can then
be loaded into a spreadsheet such as Excel or Open Office.

To use this sketch you will require the HCTRC library which is available for
download in the software section of our support forum. You will also need to
connect an LM35 temperature sensor as follows:

LM35.....Arduino
Pin 1....+5V
Pin 2....Analogue pin A0
Pin 3....GND

You may copy, alter and reuse this code in any way you like, but please leave
reference to HobbyComponents.com in your comments if you redistribute this code.

This software may not be used directly for the purpose of promoting products that
directly compete with Hobby Components Ltd's own range of products.

THIS SOFTWARE IS PROVIDED "AS IS". HOBBY COMPONENTS MAKES NO WARRANTIES,
WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ACCURACY OR
LACK OF NEGLIGENCE. HOBBY COMPONENTS SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE
FOR ANY DAMAGES INCLUDING, BUT NOT LIMITED TO, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES FOR ANY REASON WHATSOEVER. */



/* Include the wire library */
//#include <Wire.h>
/* Include the Hobby Components RTC library */
#include <HCRTC.h>
/* Include the adafruit modified SD card library, for compatability with the arduino mega */
#include <Adafruit_SD.h>

/* The RTC has a fixed addresses of 0x68 */
#define I2CDS1307Add 0x68

/* DIO pin used to control the SD card CS pin */
//#define SD_CS_DIO 10 


#include "Adafruit_VL53L0X.h"
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h> // SparkFunMLX90614 Arduino library

// address we will assign if dual sensor is present

#define NUMBER_OF_SENSORS 4

#define LIS3DH_CS 9 //pin for accelerometer

unsigned int LOX_ADDRESSES[] = { 0x5A, 0x5B, 0x5C, 0x5D };

//enable extended serial output for debugging
bool verbose = false;



// objects for the vl53l0x
IRTherm therm[NUMBER_OF_SENSORS - 1]; // Create an IRTherm object to interact with throughout


/* Create an instance of HCRTC library */
HCRTC HCRTC;

/* Create an instance of the standard SD card library */
File DataFile;


void enable_sensors() {
	if(verbose) Serial.print("enabling sensors");
	for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
		therm[i].begin(); //enable sensors
		therm[i].setUnit(TEMP_C); //set unit to degrees celcius
		Serial.print("Sensor enabled: ");
		Serial.print(i);
	}

}


void write_sensor_data() {
	bool debug = false;  // pass in 'true' to get debug data printout!

	/* Read the current time from the RTC module */
	HCRTC.RTCRead(I2CDS1307Add);

	/* Lets output this data to the serial port */
	Serial.print(HCRTC.GetDateString());
	Serial.print(",");
	Serial.print(HCRTC.GetTimeString());
	Serial.print(",");

	/* Open the data.csv file to save our data to.
	   If the file already exists it will just tag our new data onto the end of it */
	DataFile = SD.open("datalog_brakeTemp.csv", FILE_WRITE);

	if (DataFile)
	{
		DataFile.print(HCRTC.GetDay()); DataFile.print("-"); DataFile.print(HCRTC.GetMonth());
		DataFile.print(",");
		DataFile.print(HCRTC.GetTimeString());
		DataFile.print(",");

		for (int i = 0; i < NUMBER_OF_SENSORS; i++) {


			if (therm[i].read()) { // On success, read() will return 1, on fail 0.
				DataFile.print(therm[i].object());
				DataFile.print(",");


				// Use the object() and ambient() functions to grab the object and ambient
				// temperatures.
				// They'll be floats, calculated out to the unit you set with setUnit().
				if (verbose) {
					Serial.print("Object: " + (therm[i].object(), 2));
					Serial.write('°'); // Degree Symbol
					Serial.println("F");
					Serial.print("Ambient: " + (therm[i].ambient(), 2));
					Serial.write('°'); // Degree Symbol
					Serial.println("F");
					Serial.println();
				}
			} else {
				Serial.print("Failed.");
			}
			DataFile.print(",");
			Serial.print(",");
		}

		DataFile.println(" ");
		Serial.println(" ");
		DataFile.close();
	} else {
		Serial.print("Error opening datafile");
	}
}


void setup()
{
	/* Use the RTCWrite library function to set the time and date.
		Parameters are: I2C address, year, month, date, hour, minute, second,
		day of week */
		//HCRTC.RTCWrite(I2CDS1307Add, 2019, 6, 26, 13, 29, 30, 3);

	int SD_CS_DIO = 10;
	int SD_MOSI = 11;
	int SD_MISO = 12;
	int SD_SCK = 13;

	/* Initialise the serial port */
	Serial.begin(9600);

	/* Set the SD card CS pin to an output */
	pinMode(SD_CS_DIO, OUTPUT);

	/* Use the RTCWrite library function to set the time and date.
	   Parameters are: I2C address, year, month, date, hour, minute, second,
	   day of week. You would normally only need to do this once */
	   //HCRTC.RTCWrite(I2CDS1307Add, 18, 10, 8, 15, 38, 0, 0);

	/* Initialise the SD card */
	if (!SD.begin(SD_CS_DIO, SD_MOSI, SD_MISO, SD_SCK))
	{
		/* If there was an error output this to the serial port and go no further */
		Serial.println(F("ERROR: SD card failed to initialise"));
		while (1);
	}
	else {
		Serial.println(F("SD Card OK"));
	}

	enable_sensors();


	Serial.println(F("Starting..."));
	delay(1000);
}

/* Main Loop */
void loop()
{
	write_sensor_data();
	/* Wait half second before reading again */
	delay(100);
}
