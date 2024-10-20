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
MULTIUART gMultiuart(53);
// Single stream abstraction for one of the UART devices attached to the MULTIUART board
MUARTSingleStream* gStream;
// The UART communicating sensor we're using to test this interface
A02YYUWviaUARTStream* gSensor;
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

// Setup for MULTIUART on its own
void simpleDirectHexReaderSetup() {
  // Initialise the UART baud rates
  // 0=1200, 1=2400, 2=4800, 3=9600, 4=19200, 5=38400, 6=57600, 7=115200
  gMultiuart.SetBaud(0, 3);		// UART0 = 9600 Baud

  // Set up user Serial comms
  Serial.begin(115200);
  while (!Serial) {};
}

// Setup for MULTIUART abstracted to a character Stream
void singleStreamReaderSetup() {
  gStream = new MUARTSingleStream(&gMultiuart, 0);
  gStream->begin(9600);

  // Set up user Serial comms
  Serial.begin(115200);
  while (!Serial) {};
}

void sensorSetup() {
  gStream = new MUARTSingleStream(&gMultiuart, 0);
  gStream->begin(9600);
  gSensor = new A02YYUWviaUARTStream(gStream, 8, true);

  // Set up debugger interface
  gDebugger = new SerialDebugger(115200);
}

void setup() {

  //SPI Prescaler Options
  //SPI_CLOCK_DIV4 / SPI_CLOCK_DIV16 / SPI_CLOCK_DIV64
  //SPI_CLOCK_DIV128 / SPI_CLOCK_DIV2 / SPI_CLOCK_DIV8 / SPI_CLOCK_DIV32
  // Set up the SPI and MultiUART Library
  gMultiuart.initialise(SPI_CLOCK_DIV64);
  
  // simpleDirectHexReaderSetup();
  // singleStreamReaderSetup();
  sensorSetup();

}

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

// Does the same thing as the MULTIUART library only loop for simplicity
void singleStreamReaderLoop() {

  uint8_t len = gStream->available();	//Check UART 0 for incoming data
  Serial.print(String(millis()) + ": UART 0: " + String(len) + " bytes");
  if (len > 0) {
    Serial.print(": ");
    for (uint8_t i = 0; i < len; i++) {
      uint8_t b = gStream->read();
      Serial.print(formatByteToIntelString(b));
      Serial.print(" ");
    }
  }
  Serial.println();

  delay(100);

}

void sensorLoop() {

  // Update the latest distance reading on the sensor (self-throttling)
  gSensor->readDistance();

  // Lets see what we've got
  gDebugger->updateValue("distance / mm", gSensor->getDistance());
  gDebugger->updateValue("last read time / ms since reset", gSensor->getLastReadTime());
  gDebugger->updateValue("last successful read time / ms since reset", gSensor->getLastReadSuccess());
  gDebugger->updateValue("last read status", gSensor->getLastReadStatus());
  gDebugger->updateValue("last read result", gSensor->getLastReadResult());
  gDebugger->updateValue("is pre-processed", gSensor->isProcessed());
  gDebugger->throttledPrintUpdate();

}

void loop() {

  // simpleDirectHexReaderLoop();
  // singleStreamReaderLoop();
  sensorLoop();

}
