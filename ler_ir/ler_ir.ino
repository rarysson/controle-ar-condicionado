#include <IRremote.h>

int pin = 11;

IRrecv receptor_IR(pin);

decode_results resultados;

void setup()
{
  Serial.begin(9600);
  receptor_IR.enableIRIn();
  Serial.println("Receptor IR habilitado");
}

void loop() {
  if (receptor_IR.decode(&resultados)) {
    Serial.println(resultados.value);
    receptor_IR.resume();
  }
  
  delay(10);
}
