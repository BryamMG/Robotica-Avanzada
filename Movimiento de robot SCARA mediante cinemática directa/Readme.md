# 🤖 SCARA Robot 

Este repositorio contiene el sistema completo para controlar un **robot SCARA**, incluyendo:

- 🔌 **Firmware en Arduino** — Control directo de servos y actuadores.  
- 🐍 **Servidor Python (Flask + Socket.IO)** — Comunicación en tiempo real con interfaces web.
- 🧠 **Módulo de Cinemática (`scara_kinematics.py`)** — Cálculo de cinemática directa e inversa.
- 🖥️ **Panel 3D (Digital Twin)** — Visualización del robot en un entorno tridimensional.
- 🎮 **Teach Pendant Web** — Control manual tipo panel industrial desde el navegador.

Todo el sistema permite operar, simular y visualizar el robot tanto en *hardware real* como en *simulación*.

---

## 📁 Estructura del Repositorio


---

## ⚙️ 1. Arduino – Control del Robot

El programa en `arduino/scara_robot.ino` se encarga de:

- Controlar servomotores para **Q1**, **Q2** y el actuador **Z**.
- Recibir comandos desde Python vía serial/UDP.
- Ejecutar posiciones articulares calculadas por el servidor.
- Control del *gripper* (abrir/cerrar).
- Protección de límites de movimiento.

---

## 🐍 2. Python – Lógica del Servidor

El servidor implementa:

### ✔️ **`app.py`**
- Servidor **Flask** para entregar páginas HTML.
- **Socket.IO** para comunicación en tiempo real.
- Puente entre:
  - Panel 3D  
  - Teach Pendant  
  - Arduino  
  - Módulo de cinemática  

Permite enviar:
- Movimientos articulares  
- Movimientos cartesianos  
- Interpolación lineal  
- Estados del robot en tiempo real  

---

## 🧮 3. `scara_kinematics.py` – Cinemática Directa e Inversa

Este módulo implementa toda la lógica matemática del robot:

- Longitudes reales (CAD) L1, L2 y límites articulares.
- **Cinemática directa** → dada `Q1`, `Q2` y `Z` calcula `X, Y, Z`.
- **Cinemática inversa** → dado `X, Y, Z` calcula los ángulos correctos.
- Validación de límites físicos y seguridad.
- Manejo de errores numéricos (EPSILON).

Es el núcleo del Digital Twin y del movimiento del robot real.

---

## 🖥️ 4. Panel 3D – Digital Twin (`panel.html`)

Incluye:

- Visualización 3D del SCARA usando **Three.js**.
- Estado actual del robot (Q1, Q2, Z).
- Posición cartesiana calculada.
- Matriz homogénea en tiempo real.
- Control cartesiano mediante:
  - Interpolación lineal (LIC)
  - Interpolación en espacio articular (JSI)

---

## 🎮 5. Teach Pendant Web (`pendant.html`)

Simula un panel industrial real:

- Jog de movimientos:  
  - J1+/J1−  
  - J2+/J2−  
  - Z+/Z−  
- Control de velocidad (Override).
- HOME, RESET, GRIP, SAVE POINT, AUTORUN.
- Pantalla LCD con datos del robot.
- Indicadores de conexión, errores y estado WiFi.

---

## 🚀 Instalación del Servidor Python

### **1. Crear entorno virtual**
```bash
python -m venv venv
source venv/bin/activate   # Linux/Mac
venv\Scripts\activate      # Windows

