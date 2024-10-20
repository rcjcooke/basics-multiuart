#include "A02YYUWviaUARTStream.hpp"

/************************
 * Constants
 ************************/
namespace A02YYUWviaUARTStreamNS {

  // Data packet header byte
  static const byte HEADER_BYTE = 0xFF;
  // Data packet size in bytes
  static const byte PACKET_SIZE = 4;
  // The minimum distance the sensor can detect reliably in millimeters 
  static const int LOWER_LIMIT_MM = 30;
  // The minimum time between data reads
  static const unsigned long READ_INTERVAL_MS = 100;

}

/*******************************
 * Constructors
 *******************************/
A02YYUWviaUARTStream::A02YYUWviaUARTStream(Stream* mUARTSerial, uint8_t modeSelectPin, bool processed) {
  mModeSelectPin = modeSelectPin;
  pinMode(mModeSelectPin, OUTPUT);

  // Set up the UART interface - The sensor supports a 9600 baud rate
  mSensorUART = mUARTSerial;

  /* 
  The distance sensor has two operating modes, "processed" or "real-time". 
  Essentially "processed" comes pre-filtered to reduce noise but changes less
  frequently (100-300ms), whereas the "real-time" option updates every 100ms.

  HIGH (or floating) = Processed
  LOW = real-time
  */
  // Define what mode to operate the sensor in
  setProcessed(processed);
}

/*******************************
 * Getters / Setters
 *******************************/
// Get the last measured distance / mm (numbers lower than 30 are incorrect - 30mm is the lower bound for readings)
float A02YYUWviaUARTStream::getDistance() {
  return mLastMeasuredDistance;
}

// Returns true if the sensor is returning processed data, otherwise its returning real-time data
bool A02YYUWviaUARTStream::isProcessed() {
  return mProcessed;
}

// Set processed = true to get the sensor do some pre-processing to reduce noise, otherwise the sensor will return real-time data
void A02YYUWviaUARTStream::setProcessed(bool processed) {
  mProcessed = processed;
  digitalWrite(mModeSelectPin, processed ? HIGH : LOW);
}

// For Debug purposes: The last time the sensor was asked to update the distance reading / ms since last reset
unsigned long A02YYUWviaUARTStream::getLastReadTime() {
  return mLastReadTime;
}

// For Debug purposes: The time the last sensor reading was successful / mm since reset (i.e. a full data packet was received)
unsigned long A02YYUWviaUARTStream::getLastReadSuccess() {
  return mLastReadSuccess;
}

// For Debug purposes: The result of the last read request: -1 if there's a checksum error, -2 if the frame wasn't read correctly, otherwise a distance in mm
int A02YYUWviaUARTStream::getLastReadResult() {
  return mLastReadResult;
}

// For Debug purposes: Get the underlying data stream for this sensor
Stream* A02YYUWviaUARTStream::getSensorUART() {
  return mSensorUART;
}

int A02YYUWviaUARTStream::getLastReadStatus() {
  return mLastReadStatus;
}

/*******************************
 * Actions
 *******************************/
// Reads the distance from the sensor (returns 0 if successful, -1 if there's a checksum error, -2 if the frame wasn't read correctly). If there wasn't enough data available, or this was called before the next read interval is due, then this returns 0;
int A02YYUWviaUARTStream::readDistance() {
  byte data[A02YYUWviaUARTStreamNS::PACKET_SIZE];
  unsigned long now = millis();
  // Note: There's a minimum 100ms between readings at best, so don't read if it's not been 100ms since the last correctly formatted reading
  if (now - mLastReadTime >= A02YYUWviaUARTStreamNS::READ_INTERVAL_MS) {
    mLastReadStatus = readSensorData(data);
    if (mLastReadStatus == 0) {
      mLastReadSuccess = now;
      int result = processData(data);
      // A negative result is an error code
      if (result > 0) {
        mLastMeasuredDistance = result;
        mLastReadResult = 0; // success
      } else {
        mLastReadResult = result;
      }
    }
    mLastReadTime = now;
  }
  return mLastReadResult;
}

int A02YYUWviaUARTStream::readSensorData(byte* data) {
  if (mSensorUART->available() < A02YYUWviaUARTStreamNS::PACKET_SIZE) return -1;

  // Read until we find the header byte or run out of data
  while (mSensorUART->available()) {
    byte firstByte = mSensorUART->read();
    if (firstByte == A02YYUWviaUARTStreamNS::HEADER_BYTE) {
      data[0] = firstByte;
      break;
    }
  }

  // If we didn't find the header byte, return false
  if (data[0] != A02YYUWviaUARTStreamNS::HEADER_BYTE) return -2;

  // Read the rest of the packet
  if (mSensorUART->available() >= A02YYUWviaUARTStreamNS::PACKET_SIZE - 1) {
    // Note: Pointer arithmetic below to ensure we're only filling the byte array _after_ the first byte.
    mSensorUART->readBytes(++data, A02YYUWviaUARTStreamNS::PACKET_SIZE - 1);
  } else {
    return -3; // Incomplete packet
  }

  return 0;
}

int A02YYUWviaUARTStream::processData(const byte* data) {
  if (data[0] != A02YYUWviaUARTStreamNS::HEADER_BYTE) {
    // Invalid data packet
    return -2;
  }

  byte checksum = (data[0] + data[1] + data[2]) & 0xFF;
  if (checksum != data[3]) {
    // Checksum error
    return -1;
  }

  int distance = (data[1] << 8) + data[2];
  if (distance < A02YYUWviaUARTStreamNS::LOWER_LIMIT_MM) {
    return A02YYUWviaUARTStreamNS::LOWER_LIMIT_MM;
  } else {
    return distance;
  }
}