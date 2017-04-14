# DHT11
Arduino code to read DHT11 sensor

Notes this is interrupt driven, it requires a pin that can be triggered on a state change


# Copy this git to your libraries directory 

``` cd ~/Arduino/libraries```
``` git clone https://github.com/rcorbish/DHT11.git ```

# Try a sample sketch

```
#include "DHT11.h"

#define DHT11_DATA_PIN 2 

DHT11 *dht11 ;

void setup() {
    Serial.begin(115200);
    dht11 = new DHT11( DHT11_DATA_PIN ) ;
}


void loop() {
  
  uint16_t temp ;
  uint16_t humidity ;
  
  if( dht11->read( temp, humidity ) ) {
    Serial.print( temp ) ;
    Serial.print( "C " ) ;
    Serial.print( humidity ) ;
    Serial.println( "%" ) ;
  }
  
}

```
