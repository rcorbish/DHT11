
#include "DHT11.h"


volatile int DHT11::dht11_clk ;
volatile long DHT11::prev_dht11_change ;
volatile long DHT11::dht11_data[100] ;


void DHT11::dht11DataChange() {
      if( dht11_clk < 95 ) {
        long m = micros() ;
        dht11_data[dht11_clk] = m-prev_dht11_change ;
        prev_dht11_change = m ;
        ++dht11_clk ;
      }
}

DHT11::DHT11( int _dataPin ) : dataPin( _dataPin ) {
      pinMode(dataPin, OUTPUT);
      dht11_next_read = millis() + 2000 ;	// first read wait a bit extra ...
}


bool DHT11::read( uint16_t &temp, uint16_t &humidity ) {
      bool rc = false ;
      // Max. read time is 1Hz
      if( millis() > dht11_next_read ) {
        // send request for data - set the DATA low
        digitalWrite( dataPin, LOW ) ;
        delay( 18 ) ;            // min 18mS of delay
                                 // clear out the output bytes to prepare for DATA from DHT11
        memset( (void*)dht11_data, 0, sizeof( dht11_data ) ) ;
                                 // prepare DATA to come from DHT11
        pinMode( dataPin, INPUT_PULLUP ) ;

        dht11_clk = 0 ;          // the transition count of the DATA pin
                                 // we measure pulse length - not absolute timings
        prev_dht11_change = micros() ;

                                 // remember which pin to attach to interrupt handler
        int pin = digitalPinToInterrupt(dataPin) ;
                                 // call interrupt on any state change of DATA ( i.e. rising and falling edges )
        attachInterrupt( pin, dht11DataChange, CHANGE);

        delay( 6 ) ;             // Max = (70+50) * 40 + 50 + 80 + 80   ( see data sheet ) ~5mS

        detachInterrupt( pin );  // remove the interrupts

                                 // take back the DATA line
        pinMode( dataPin, OUTPUT ) ;
                                 // put DHT11 to idle mode
        digitalWrite( dataPin, HIGH ) ;

        // Received 40 bit data will be stored in 5 bytes
        uint16_t data[5] ;      // 40 digits = 5x8 bits   ( humidity x 2    temp x 2   checksum )
        data[0] = 0x00 ;	
        data[1] = 0x00 ;
        data[2] = 0x00 ;
        data[3] = 0x00 ;
        data[4] = 0x00 ;	

        // --------------------------------------
        // NOW WE PARSE THE DATA PIN CHANGES ....
        //
        // STEP 1: look for 2 x 80uS pulses from DHT - ignoring noise at the front of pulse train
        int start = 1 ;
        int num80us = 0 ;
        for( int i=0 ; i<dht11_clk ; i++, start++ ) {
          if( dht11_data[i]>70 ) num80us++ ;
          if( num80us > 1 ) break ;
        }
	
	if( dht11_clk < 80 ) { // didn't read 40 bits - must fail
	  return false ;
	}

        // STEP2 look for 40 data bits. Each bit is a 50uS pulse then a short( ~27uS == 0 ) or long ( ~70uS == 1 ) pulse
        int dataIndex = 0 ;
        uint16_t mask = 0x80 ;
        for( int i=start ; i<dht11_clk ; i+=2 ) {
          if( dht11_data[i]>60 || dht11_data[i]<40 ) {
            return false ;       // must have 50uS low between pulses
          }
          data[dataIndex] += dht11_data[i+1]>50 ? mask : 0 ;
          mask >>= 1 ;
          if( ((i-start)%16) == 14 ) {
            mask = 0x80 ;
            dataIndex++ ;
          }
        }

	/*
	Serial.print( "Read data " ) ;
	Serial.println( dht11_clk ) ;
	Serial.print( data[0], HEX ) ;
	Serial.print( " " ) ;
	Serial.print( data[1], HEX ) ;
	Serial.print( " " ) ;
	Serial.print( data[2], HEX ) ;
	Serial.print( " " ) ;
	Serial.print( data[3], HEX ) ;
	Serial.print( " " ) ;
	Serial.print( data[4], HEX ) ;
	Serial.println( " " ) ;
	*/

        // STEP3: There should be an extra low clock of 50uS at the end of all this - but we don't care !!!!
        // -------------------------

        // DHT11 doesn't do fractions
        humidity = data[0] ;
        temp = data[2] ;

        // Checksum should be the same as total of 1st 4 bytes
        int checksum = data[4] ;
                                 // assuming no fractional data. False return indicates data invalid
        rc = checksum == humidity+temp ;

                                 // can't read the DHT11 for another 1 second
        dht11_next_read = millis() + 1000 ;
      }

      return rc ;                // true = data OK, false = bad data or reading too soon ( faster than 1sec )
    }


