#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

const char* ssid = "INFINITUM86C2";
const char* password = "ELd4TfVbHW";

WiFiUDP udp;
unsigned int localPort = 4210;
char packetBuffer[255];

// Definición de Servos
Servo servo1; // Base
Servo servo2; // Codo
Servo servo3; // Z
Servo servo4; // Gripper

// PINES (Ajusta según tu conexión en la Super Mini)
#define PIN_S1 0 
#define PIN_S2 1
#define PIN_S3 2
#define PIN_S4 3

void setup() {
  Serial.begin(115200);
  
  // Configurar Servos
  servo1.attach(PIN_S1);
  servo2.attach(PIN_S2);
  servo3.attach(PIN_S3);
  servo4.attach(PIN_S4);

  // Conexión WiFi
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP:");
  Serial.println(WiFi.localIP()); // <--- ESTA IP VA EN EL CODIGO PYTHON

  udp.begin(localPort);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;
    
    String data = String(packetBuffer);
    // Formato recibido: "q1,q2,z,grip"
    // Ejemplo simple de parsing:
    float q1 = getValue(data, ',', 0).toFloat();
    float q2 = getValue(data, ',', 1).toFloat();
    float z = getValue(data, ',', 2).toFloat();
    int grip = getValue(data, ',', 3).toInt();

    moverRobot(q1, q2, z, grip);
  }
}

void moverRobot(float q1, float q2, float z, int grip) {
  // Mapeo de ángulos matemáticos (-90 a 90) a servos (0 a 180)
  // AJUSTA LOS OFFSET SEGÚN TU CALIBRACIÓN MECÁNICA
  int s1_angle = map(q1, -90, 90, 0, 180); 
  int s2_angle = map(q2, -90, 90, 0, 180);
  
  // Eje Z (asumiendo servo)
  int s3_angle = map(z, 0, 5.5, 0, 180); 

  servo1.write(s1_angle);
  servo2.write(s2_angle);
  servo3.write(s3_angle);
  if(grip == 1) servo4.write(100); else servo4.write(0);
}

// Función auxiliar para separar strings
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}