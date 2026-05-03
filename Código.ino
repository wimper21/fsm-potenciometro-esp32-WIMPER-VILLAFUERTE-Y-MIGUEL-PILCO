// Pines
const int pinLED = 5;
const int pinBTN = 4;
const int pinPOT = 34;

// Máquina de estados
volatile int estado = 0;

// Timer (NUEVA API)
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Variables ISR
volatile bool ledState = false;
volatile int contadorParpadeos = 0;
volatile bool ejecutarParpadeo = false;

// Botón
int lastBtnState = HIGH;
int btnState;

// ================= ISR =================
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);

  if (estado == 2) {
    ledState = !ledState;
    ejecutarParpadeo = true;
    contadorParpadeos++;

    if (contadorParpadeos >= 10) { // 5 parpadeos
      estado = 0;
      contadorParpadeos = 0;
      ledState = false;
    }
  }

  portEXIT_CRITICAL_ISR(&timerMux);
}

// ================= SETUP =================
void setup() {
  pinMode(pinBTN, INPUT_PULLUP);

  // PWM (LEDC)
  ledcAttach(pinLED, 5000, 8); // NUEVO (reemplaza ledcSetup + ledcAttachPin)

  // Timer (NUEVA API)
  timer = timerBegin(1000000); // 1 MHz
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0); // 1 segundo
}

// ================= LOOP =================
void loop() {

  // -------- BOTÓN --------
  btnState = digitalRead(pinBTN);

  if (btnState == LOW && lastBtnState == HIGH) {
    estado++;
    if (estado > 2) estado = 0;
  }

  lastBtnState = btnState;

  // -------- ESTADOS --------
  switch (estado) {

    case 0: // Standby
      ledcWrite(pinLED, 0);
      break;

    case 1: { // Potenciómetro
      int lectura = analogRead(pinPOT);
      int pwm = map(lectura, 0, 4095, 0, 255);
      ledcWrite(pinLED, pwm);
      break;
    }

    case 2: // Parpadeo
      if (ejecutarParpadeo) {
        ejecutarParpadeo = false;

        if (ledState) {
          ledcWrite(pinLED, 255);
        } else {
          ledcWrite(pinLED, 0);
        }
      }
      break;
  }
}