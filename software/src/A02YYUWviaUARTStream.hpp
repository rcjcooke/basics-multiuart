#ifndef __A02YYUWVIAUARTSTREAM_H_INCLUDED__
#define __A02YYUWVIAUARTSTREAM_H_INCLUDED__

#include <Arduino.h>

class A02YYUWviaUARTStream {

public:

  /*******************************
   * Constructors
   *******************************/
  A02YYUWviaUARTStream(Stream* mUARTSerial, uint8_t modeSelectPin, bool processed);

  /*******************************
   * Getters / Setters
   *******************************/
  // Get the last measured distance / mm
  float getDistance();
  /* Returns true if the sensor is returning processed data, otherwise its
   * returning real-time data */
  bool isProcessed();
  /* Set processed = true to get the sensor do some pre-processing to reduce
   * noise, otherwise the sensor will return real-time data */
  void setProcessed(bool processed);
  /* For Debug purposes: The last time the sensor was asked to update the
   * distance reading / ms since last reset */
  unsigned long getLastReadTime();
  /* For Debug purposes: The time the last sensor reading was successful / mm
   * since reset (i.e. a full data packet was received) */
  unsigned long getLastReadSuccess();
  /* For Debug purposes: The result of the last read request: -1 if there's a
   * checksum error, -2 if the frame wasn't read correctly, otherwise a distance
   * in mm */
  int getLastReadResult();
  /* Status of the last attempt at retrieving a data packet from the sensor. 0 =
   * success, -1 if insufficient bytes available, -2 if the header byte couldn't
   * be found despite there being at least enough bytes for a packet of data, -3
   * if we couldn't retrieve a complete data packet. */
  int getLastReadStatus();
  // For Debug purposes: Get the underlying data stream for this sensor
  Stream* getSensorUART();

  /*******************************
   * Actions
   *******************************/
  /* Reads the distance from the sensor (returns 0 if successful, -1 if there's
   * a checksum error, -2 if the frame wasn't read correctly). If there wasn't
   * enough data available, or this was called before the next read interval is
   * due, then this returns 0; */
  int readDistance();

private:
  /*******************************
   * Member variables
   *******************************/
  /* The microcontroller pin that transmits to the Distance sensor's UART
   * interface */
  uint8_t mModeSelectPin;
  // The last measured distance / mm
  float mLastMeasuredDistance;
  /* If true the sensor is set to return processed data, otherwise its returning
   * real-time data */
  bool mProcessed;
  /* The last time the sensor was asked to update the distance reading / ms
   * since last reset */
  unsigned long mLastReadTime = 0;
  /* The time the last sensor reading was successful / mm since reset (i.e. a
   * full data packet was received) */
  unsigned long mLastReadSuccess = 0;
  /* Status of the last attempt at retrieving a data packet from the sensor. 0 =
   * success, -1 if insufficient bytes available, -2 if the header byte couldn't
   * be found despite there being at least enough bytes for a packet of data, -3
   * if we couldn't retrieve a complete data packet. */
  int mLastReadStatus = 0;
  /* The result of the last read request: (0 if successful, -1 if there's a
   * checksum error, -2 if the frame wasn't read correctly). If there wasn't
   * enough data available, or this was called before the next read interval is
   * due, then this returns 0; */
  int mLastReadResult = 0;
  // The UART interface to the distance sensor
  Stream* mSensorUART;

  /*******************************
   * Actions
   *******************************/
  /* Read data from the sensor into the data byte array supplied. 0 = success,
   * -1 if insufficient bytes available, -2 if the header byte couldn't be found
   * despite there being at least enough bytes for a packet of data, -3 if we
   * couldn't retrieve a complete data packet. */
  int readSensorData(byte *data);
  /* Process the data in the byte array supplied. Returns distance in mm or a
   * negative number if there's an error. */
  int processData(const byte *data);

};

#endif // __A02YYUWVIAUARTSTREAM_H_INCLUDED__