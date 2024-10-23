#include <Arduino.h>
#include <SPI.h>

#include "MULTIUART.hpp"
#include "MUARTSingleStream.hpp"
#include "A02YYUWviaUARTStream.hpp"
#include "SerialDebugger.hpp"

/* Arduino Mega relevant pins from pinout diagram:
 *
 * CIPO = MISO = 50 = blue
 * COPI = MOSI = 51 = green
 * SCK = SCK = 52 = yellow
 * CS = SS = 53 = orange
 */

// Multiuart board interface instance - Constructor argument is Chip Select pin number
MULTIUART gMultiuart = MULTIUART(53);
// Single stream abstraction for one of the UART devices attached to the MULTIUART board - sensor 1
MUARTSingleStream* gStream1;
// Single stream abstraction for one of the UART devices attached to the MULTIUART board - sensor 2
MUARTSingleStream* gStream2;
// The UART communicating sensor we're using to test this interface
A02YYUW::A02YYUWviaUARTStream* gSensor1;
// The second UART communicating sensor we're using to test this interface
A02YYUW::A02YYUWviaUARTStream* gSensor2;
// Debugger for output
SerialDebugger* gDebugger;

/*******************
 * Utility functions
 *******************/
// Formats a single byte to a Intel format HEX annotation, e.g. 0xF2
String formatByteToIntelString(uint8_t byte) {
  String result = "0x";
  if (byte < 16) result = result + "0";
  result = result + String((int) byte, HEX);
  return result;
}

/*******************
 * Setup functions
 *******************/
void setupSerial() {
  // Set up user Serial comms
  Serial.begin(115200);
  while (!Serial);
}

// A02YYUWviaUARTStream* setupSensor(int index, uint8_t modeSelectPin) {
//   MUARTSingleStream* stream = new MUARTSingleStream(&gMultiuart, index);
//   stream->begin(9600);
//   return new A02YYUWviaUARTStream(stream, modeSelectPin, true);
// }

void setupDebugger() {
  // Set up debugger interface
  gDebugger = new SerialDebugger(115200);
}

// Setup for MULTIUART on its own
void simpleDirectHexReaderSetup() {
  // Initialise the UART baud rates
  // 0=1200, 1=2400, 2=4800, 3=9600, 4=19200, 5=38400, 6=57600, 7=115200
  gMultiuart.SetBaud(0, 3);		// UART0 = 9600 Baud

  setupSerial();
}

// Setup for MULTIUART abstracted to a character Stream
void singleStreamReaderSetup() {
  gStream1 = new MUARTSingleStream(&gMultiuart, 0);
  gStream1->begin(9600);

  setupSerial();
}

// Set up for 1 sensor test
void sensor1Setup() {
  gStream1 = new MUARTSingleStream(&gMultiuart, 0);
  gStream1->begin(9600);
  gSensor1 = new A02YYUW::A02YYUWviaUARTStream(gStream1, 8, true);

  setupDebugger();
}

// Set up for 2 sensors test
void sensorsSetup() {
  gStream1 = new MUARTSingleStream(&gMultiuart, 0);
  gStream1->begin(9600);
  gSensor1 = new A02YYUW::A02YYUWviaUARTStream(gStream1, 8, true);
  
  gStream2 = new MUARTSingleStream(&gMultiuart, 1);
  gStream2->begin(9600);
  gSensor2 = new A02YYUW::A02YYUWviaUARTStream(gStream2, 9, true);

  setupDebugger();
}

void setup() {

  // gMultiuart = new MULTIUART(53);
  //SPI Prescaler Options
  //SPI_CLOCK_DIV4 / SPI_CLOCK_DIV16 / SPI_CLOCK_DIV64
  //SPI_CLOCK_DIV128 / SPI_CLOCK_DIV2 / SPI_CLOCK_DIV8 / SPI_CLOCK_DIV32
  // Set up the SPI and MultiUART Library
  gMultiuart.initialise(SPI_CLOCK_DIV64);
  
  // simpleDirectHexReaderSetup();
  // singleStreamReaderSetup();
  // sensor1Setup();
  sensorsSetup();

}

/*******************
 * Loop functions
 *******************/
// Loop for MULTIUART on its own
void simpleDirectHexReaderLoop() {

  uint8_t len = gMultiuart.checkRx(0);	//Check UART 0 for incoming data
  Serial.print(String(millis()) + ": UART 0: " + String(len) + " bytes");
  if (len > 0) {
    Serial.print(": ");
    for (uint8_t i = 0; i < len; i++) {
      uint8_t b = gMultiuart.ReceiveByte(0);
      Serial.print(formatByteToIntelString(b));
      Serial.print(" ");
    }
  }
  Serial.println();

  delay(100);

}

// Does the same thing (almost) as the MULTIUART library only loop for simplicity
void singleStreamReaderLoop() {

  uint8_t len = gStream1->available();	//Check UART 0 for incoming data
  Serial.print(String(millis()) + ": UART 0: " + String(len) + " bytes");
  if (len > 0) {
    Serial.print(": ");
    for (uint8_t i = 0; i < len; i++) {
      uint8_t b = gStream1->read();
      Serial.print(formatByteToIntelString(b));
      Serial.print(" ");
    }
  }
  Serial.println();

  delay(100);

}

// Loop for a single sensor
void sensor1Loop() {

  // Update the latest distance reading on the sensors (self-throttling)
  gSensor1->readDistance();

  // Lets see what we've got
  gDebugger->updateValue("distance / mm", gSensor1->getDistance());
  gDebugger->updateValue("last read time / ms since reset", gSensor1->getLastReadTime());
  gDebugger->updateValue("last successful read time / ms since reset", gSensor1->getLastReadSuccess());
  gDebugger->updateValue("last read status", gSensor1->getLastReadStatus());
  gDebugger->updateValue("last read result", gSensor1->getLastReadResult());
  gDebugger->updateValue("is pre-processed", gSensor1->isProcessed());
  gDebugger->throttledPrintUpdate();

}

// Loop for 2 sensors
void sensorsLoop() {

  // Update the latest distance reading on the sensors (self-throttling)
  gSensor1->readDistance();
  gSensor2->readDistance();

  // Lets see what we've got
  gDebugger->updateValue("distance (1) / mm", gSensor1->getDistance());
  gDebugger->updateValue("last successful read time (1) / ms since reset", gSensor1->getLastReadSuccess());
  gDebugger->updateValue("distance (2) / mm", gSensor2->getDistance());
  gDebugger->updateValue("last successful read time (2) / ms since reset", gSensor2->getLastReadSuccess());
  gDebugger->throttledPrintUpdate();

}

void loop() {

  // simpleDirectHexReaderLoop();
  // singleStreamReaderLoop();
  // sensor1Loop();
  sensorsLoop();

}
