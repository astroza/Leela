#include <Servo.h>

#define BAUDS 9600
#define SERVOS_COUNT 12
#define MESSAGE_TIME 5
/* 9600 bauds, por cada byte se transmiten 10 bits (8 y 2 de control), (9600/10) 
 * 960 bytes por segundo, transmitir un byte toma 1.0/960 (0.001042) segundos
  * que en milisegundos son 1.042, transmitir 4 bytes toma 4.2 milisegundos. Lo dejo en 5.
  */                                     
#define MESSAGE_LENGTH 4

Servo servos[SERVOS_COUNT];

struct response {
  char status;
  int data;
} resp;

void setup()
{    
  Serial.begin(9600);
}

void process_message(char *msg)
{
  int angle;
  int sel = abs(msg[1]);
  Servo *servo;
  
  resp.data = 0;
  resp.status = 'K'; /* OK */
  
  if(msg[0] == 'S' && sel < SERVOS_COUNT) {
    angle = msg[3] << 8 | msg[2];
    servo = &servos[sel];
    if(!servo->attached()) {
      servo->attach(sel+2);
    }
    servo->write(angle);
  } else if(msg[0] == 'D' && sel < SERVOS_COUNT) {
    servo = &servos[sel];
    servo->detach();
  } else if(msg[0] == 'A' && sel < 8) {
    resp.data = analogRead(sel);
    Serial.write((uint8_t *)&resp, 3);
  }
}
/* Quiero evitar errores de mensajes incompletos:
 * + Si pasan mas de N milisegundos luego de la llegada del primer byte, se responde con un error
 * + Se se completa el mensaje dentro de los N milisegundos, el mensaje se analiza y se responde positivamente o con error si su contenido no tiene sentido.
 */
bool first_byte = false;
long start_time;
void loop()
{
  int i;
  int available = Serial.available();
  char msg[MESSAGE_LENGTH];
  if(available == 0)
    return;
    
  if(available < MESSAGE_LENGTH) {
    if(!first_byte) {
      start_time = millis();
      first_byte = true;
    }
    if(millis() - start_time > MESSAGE_TIME) {
      first_byte = false;
      // Flush incoming buffer
      while(available--)
        Serial.read();
      Serial.write('I'); /* Incomplete */
    }
  } else {
    for(i = 0; i < MESSAGE_LENGTH; i++)
      msg[i] = Serial.read(); 
      
    process_message(msg);
    first_byte = false;
  }
}

