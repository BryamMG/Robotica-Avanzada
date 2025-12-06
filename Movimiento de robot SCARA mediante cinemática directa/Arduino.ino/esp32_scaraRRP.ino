#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

// --- 1. CONFIGURACIÓN DE RED ---
const char* ssid = "Galaxy S20";
const char* password = "*****8";

WiFiUDP udp;
unsigned int localPort = 4210;       // Puerto donde escucha el ESP32
char packetBuffer[255];

IPAddress serverIP;                  
bool connectionEstablished = false;  

// --- 2. DEFINICIÓN DE OBJETOS SERVO ---
Servo servoBase;    
Servo servoCodo;    
Servo servoZ;        

// --- 3. PINES REALES ---
#define PIN_BASE 15
#define PIN_CODO 2
#define PIN_Z    5
#define PIN_GRIP 18 // Pin para el Relé/Electro-válvula

// --- 4. CALIBRACIÓN ---
// Offset de 90 grados para que el rango de control de -90 a +90 sea 0 a 180 en el servo.
int offset_base = 90; 
int offset_codo = 90; 

void moverRobot(float q1, float q2, float z, int grip);

void setup() {
  Serial.begin(115200);

  // --- Configuración de Servos (Q1, Q2, Z) ---
  // Usamos el rango estándar de pulso (500us a 2500us) para 0 a 180 grados.
  servoBase.setPeriodHertz(50); servoBase.attach(PIN_BASE, 500, 2500);
  servoCodo.setPeriodHertz(50); servoCodo.attach(PIN_CODO, 500, 2500);
  servoZ.setPeriodHertz(50);    servoZ.attach(PIN_Z, 500, 2500);
  
  // --- Configuración del Pin del Relé (PIN_GRIP) ---
  pinMode(PIN_GRIP, OUTPUT);
  // Estado inicial: Gripper desactivado. 
  digitalWrite(PIN_GRIP, LOW);

  // Posición Inicial Segura del Robot (Servos a Home)
  servoBase.write(offset_base); // 90 grados
  servoCodo.write(offset_codo); // 90 grados
  // Z inicial: 180 (Corresponde a Z=0 cm o la posición superior/retraída)
  servoZ.write(180);       

  // Conexión WiFi
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  WiFi.setSleep(false); 
  
  Serial.println("\n--- CONECTADO ---");
  Serial.print("IP del Robot: ");
  Serial.println(WiFi.localIP()); 
  
  udp.begin(localPort);
  Serial.println("Esperando primer comando para sincronizar IP del servidor...");
}

void loop() {
  // 1. RECEPCIÓN DE COMANDOS
  int packetSize = udp.parsePacket();
  
  if (packetSize) {
    if (!connectionEstablished) {
        serverIP = udp.remoteIP();
        connectionEstablished = true;
        Serial.print("✅ Servidor detectado en: ");
        Serial.println(serverIP);
    }
    // Si la IP del remitente es diferente, actualizamos la IP del servidor.
    if (udp.remoteIP() != serverIP) {
        serverIP = udp.remoteIP();
    }

    int len = udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;
    
    // --- DIAGNÓSTICO: Imprimir el paquete RAW recibido ---
    Serial.print("RAW Packet: ");
    Serial.println(packetBuffer);
    
    float q1, q2, z;
    int grip;
    
    // Formato esperado: "q1,q2,z,grip"
    int n = sscanf(packetBuffer, "%f,%f,%f,%d", &q1, &q2, &z, &grip);
    
    // --- DIAGNÓSTICO: Imprimir el resultado del parsing ---
    Serial.print("Parsed items (n): ");
    Serial.println(n);
    
    if (n == 4) {
        // --- DIAGNÓSTICO: Imprimir los valores PARSEADOS ---
        Serial.print("Parsed Q1, Q2, Z, Grip: ");
        Serial.print(q1); Serial.print(", ");
        Serial.print(q2); Serial.print(", ");
        Serial.print(z); Serial.print(", ");
        Serial.println(grip);
        
        moverRobot(q1, q2, z, grip);
        
        // --- DIAGNÓSTICO: Imprimir los ángulos finales del servo ---
        int angulo_base = constrain(offset_base + q1, 0, 180);
        int angulo_codo = constrain(offset_codo + q2, 0, 180);
        int angulo_z = map(z * 10, 0, 48, 180, 0); 
        angulo_z = constrain(angulo_z, 0, 180);

        Serial.print("Servo Writes: Base="); Serial.print(angulo_base);
        Serial.print(", Codo="); Serial.print(angulo_codo);
        Serial.print(", Z="); Serial.println(angulo_z);
        
    } else {
        Serial.println("ERROR: Fallo al parsear los 4 valores.");
    }
    Serial.println("---------------------------------");
  }

  // 2. HEARTBEAT (Se deja vacío para una respuesta rápida)
}

void moverRobot(float q1, float q2, float z, int grip) {
    
  // Q1 y Q2: Aplica el offset y restringe al rango 0-180.
  int angulo_base_servo = constrain(offset_base + q1, 0, 180);
  int angulo_codo_servo = constrain(offset_codo + q2, 0, 180);
  
  // Z: Mapea el desplazamiento de 0 a 4.8 cm a 180 a 0 grados, e.g.:
  //     z=0 (Arriba)   -> 180 grados
  //     z=4.8 (Abajo) -> 0 grados
  // Multiplicamos por 10 para usar solo enteros en el MAP (4.8 -> 48)
  int angulo_z_servo = map(z * 10, 0, 48, 180, 0); 
  angulo_z_servo = constrain(angulo_z_servo, 0, 180);

  servoBase.write(angulo_base_servo);
  servoCodo.write(angulo_codo_servo);
  servoZ.write(angulo_z_servo);
  
  // GRIPPER: HIGH activa (cierra/succiona), LOW desactiva (abre/libera).
  if (grip == 1) {
    digitalWrite(PIN_GRIP, HIGH); // Activar/Cerrar
  } else {
    digitalWrite(PIN_GRIP, LOW); // Desactivar/Abrir
  }
}
