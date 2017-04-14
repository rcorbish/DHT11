
#ifndef INCL_DHT11
#define INCL_DHT11

#include "Arduino.h"

class DHT11
{
  private:
    long dht11_next_read ;
    int dataPin ;

    volatile static int dht11_clk ;
    volatile static long prev_dht11_change ;
    volatile static long dht11_data[100] ;

    static void dht11DataChange() ;

  public:

    DHT11( int dataPin ) ;
    bool read( uint16_t &temp, uint16_t &humidity ) ;
} ;

#endif
