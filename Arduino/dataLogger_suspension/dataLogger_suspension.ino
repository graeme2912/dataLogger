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

// address we will assign if dual sensor is present
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31

#define SHT_LOX1 7
#define SHT_LOX2 6

#define NUMBER_OF_SENSORS 2


unsigned int LOX_ADDRESSES[] = { 0x30, 0x31 };
//LOX_ADDRESSES = { 0x30, 0x31 };
//LOX_ADDRESSES[0] = 0x30;

unsigned int SHT_LOX[] = { 7, 6 };
// set the pins to shutdown




Adafruit_VL53L0X lox[NUMBER_OF_SENSORS - 1];

// objects for the vl53l0x
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
//Adafruit_VL53L0X lox3 = Adafruit_VL53L0X();
//Adafruit_VL53L0X lox4 = Adafruit_VL53L0X();

// this holds the measurement
VL53L0X_RangingMeasurementData_t measure[NUMBER_OF_SENSORS - 1];

//VL53L0X_RangingMeasurementData_t measure1;
//VL53L0X_RangingMeasurementData_t measure2;

/* Create an instance of HCRTC library */
HCRTC HCRTC;

/* Create an instance of the standard SD card library */
File DataFile;

/*
	Reset all sensors by setting all of their XSHUT pins low for delay(10), then set all XSHUT high to bring out of reset
	Keep sensor #1 awake by keeping XSHUT pin high
	Put all other sensors into shutdown by pulling XSHUT pins low
	Initialize sensor #1 with lox.begin(new_i2c_address) Pick any number but 0x29 and it must be under 0x7F. Going with 0x30 to 0x3F is probably OK.
	Keep sensor #1 awake, and now bring sensor #2 out of reset by setting its XSHUT pin high.
	Initialize sensor #2 with lox.begin(new_i2c_address) Pick any number but 0x29 and whatever you set the first sensor to
 */

void reset_all(bool do_reset) {
	for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
		if (do_reset) {
			digitalWrite(SHT_LOX[i], LOW);
		} else {
			digitalWrite(SHT_LOX[i], HIGH);
		}
	}
	delay(10);
}

void enable_sensor(bool enable, int num){
	
}

void setID() {

	reset_all(true);
	reset_all(false);

	// all reset
	//digitalWrite(SHT_LOX1, LOW);
	//digitalWrite(SHT_LOX2, LOW);
	//delay(10);

	// all unreset
	//digitalWrite(SHT_LOX1, HIGH);
	//digitalWrite(SHT_LOX2, HIGH);
	//delay(10);

	// activating LOX1 and reseting LOX2
	digitalWrite(SHT_LOX1, HIGH);
	digitalWrite(SHT_LOX2, LOW);

	

	// for (int i = 0; i < NUMBER_OF_SENSORS; i++) {
	// 	digitalWrite(SHT_LOX[1], HIGH);
	// 	if (!lox[i].begin(LOX_ADDRESSES[i])) {
	// 		Serial.println(F("Failed to boot VL53L0X "));
	// 		while (1);
	// 	}
	// }
	

	// initing LOX1
	if (!lox[0].begin(LOX_ADDRESSES[0])) {
		Serial.println(F("Failed to boot first VL53L0X"));
		while (1);
	}
	delay(10);

	// activating LOX2
	digitalWrite(SHT_LOX2, HIGH);
	delay(10);

	//initing LOX2
	if (!lox[1].begin(LOX_ADDRESSES[1])) {
		Serial.println(F("Failed to boot second VL53L0X"));
		while (1);
	}
}

void write_sensor_data() {
	bool debug = false;  // pass in 'true' to get debug data printout!
	for(int j = 0; j < NUMBER_OF_SENSORS; j++){
		lox[j].rangingTest(&measure[j], debug);
	}

	/* Read the current time from the RTC module */
	HCRTC.RTCRead(I2CDS1307Add);

	/* Lets output this data to the serial port */
	Serial.print(HCRTC.GetDateString());
	Serial.print(",");
	Serial.print(HCRTC.GetTimeString());
	Serial.print(",");

	/* Open the data.csv file to save our data to.
	   If the file already exists it will just tag our new data onto the end of it */
	DataFile = SD.open("datalog.csv", FILE_WRITE);

	if (DataFile)
	{
		DataFile.print(HCRTC.GetDateString());
		DataFile.print(F(","));
		DataFile.print(HCRTC.GetTimeString());
		DataFile.print(F(","));

		for(int i = 0; i < NUMBER_OF_SENSORS; i++){
			if (measure[i].RangeStatus != 4) {     // if not out of range
				DataFile.print(measure[i].RangeMilliMeter);
				Serial.print(measure[i].RangeMilliMeter);
			} else {
				DataFile.print(F("error: range"));
			}
			DataFile.print(",");
			Serial.print(",");
		}

		// print sensor one reading to file

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
	} else {
		Serial.println(F("SD Card OK"));
	}

	pinMode(SHT_LOX1, OUTPUT);
	pinMode(SHT_LOX2, OUTPUT);

	Serial.println(F("Shutdown pins inited..."));

	digitalWrite(SHT_LOX1, LOW);
	digitalWrite(SHT_LOX2, LOW);

	Serial.println(F("Both in reset mode...(pins are low)"));

	Serial.println(F("Starting..."));
delay(2000);
	setID();
}



/* Main Loop */
void loop()
{
	write_sensor_data();

	/* Wait half second before reading again */
	delay(500);
}
