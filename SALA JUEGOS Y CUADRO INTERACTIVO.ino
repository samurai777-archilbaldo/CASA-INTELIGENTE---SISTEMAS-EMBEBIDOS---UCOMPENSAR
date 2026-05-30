/*
 ╔══════════════════════════════════════════════════════════════╗
 ║        SISTEMA MULTITAREA ARDUINO UNO  —  v2.0              ║
 ║        LCD 16x2 I2C + OLED SSD1306 + 1 Botón                ║
 ╠══════════════════════════════════════════════════════════════╣
 ║  BUGS CORREGIDOS (v1.0 → v2.0)                              ║
 ║  [BUG-01] Salto Dino roto (1 frame) → timer real 500ms      ║
 ║  [BUG-02] Bala destruye enemigo sin verificar fila Y → FIX  ║
 ║  [BUG-03] Nave muere con enemigo en fila distinta → FIX     ║
 ║  [BUG-04] delay(2000) congela OLED en GameOver → timer      ║
 ║  [BUG-05] Botón flotante sin debounce → PULLUP + 50ms       ║
 ║                                                              ║
 ║  MEJORAS                                                     ║
 ║  [M-01] Sin lcd.clear() en gameplay → cero flickering       ║
 ║  [M-02] OLED solo refresca cuando cambia animacion          ║
 ║  [M-03] Strings en Flash con F()                            ║
 ║  [M-04] Custom chars en PROGMEM                             ║
 ║  [M-05] Animacion walking Dino (2 frames)                   ║
 ║  [M-06] Superman: escudo relleno + letra S centrada         ║
 ║  [M-07] Ghost mejorado: ojos con brillo, boca, pies         ║
 ║  [M-08] I2C a 400kHz (Fast Mode)                            ║
 ╠══════════════════════════════════════════════════════════════╣
 ║  CONEXIONES                                                  ║
 ║  OLED SSD1306  SDA=A4, SCL=A5, VCC=5V, GND                 ║
 ║  LCD I2C       SDA=A4, SCL=A5, VCC=5V, GND                 ║
 ║  BOTON         Pin 2 a GND  (INPUT_PULLUP activo LOW)       ║
 ║                                                              ║
 ║  Si tu esquema usa R1 pull-down (boton a 5V):               ║
 ║  Cambiar INPUT_PULLUP -> INPUT                               ║
 ║  Cambiar "== LOW" -> "== HIGH" en handleBtn()               ║
 ╚══════════════════════════════════════════════════════════════╝
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>

// ──────────────────────────────────────────────────────────────
// PINES Y HARDWARE
// ──────────────────────────────────────────────────────────────
#define BTN_PIN     2
#define OLED_W    128
#define OLED_H     64
#define OLED_ADDR 0x3C
#define LCD_ADDR  0x27

// ──────────────────────────────────────────────────────────────
// TIMINGS en milisegundos
// ──────────────────────────────────────────────────────────────
#define T_DEBOUNCE     50UL
#define T_LONGPRESS  5000UL
#define T_OLED_SWAP  5000UL
#define T_GHOST_MOVE   65UL
#define T_GHOST_BLINK 600UL
#define T_DINO_JUMP   500UL    // [BUG-01 FIX] tiempo en el aire
#define T_DINO_WALK   180UL    // intervalo animacion walking
#define T_DINO_INIT   230UL    // velocidad inicial obstaculo
#define T_DINO_MIN     50UL    // velocidad maxima
#define T_DINO_ACCEL    3U     // aceleracion por punto
#define T_SPACE       150UL    // velocidad juego space
#define T_GAMEOVER   2500UL    // pantalla gameover (no-bloqueante)

// ──────────────────────────────────────────────────────────────
// POSICIONES EN LCD
// ──────────────────────────────────────────────────────────────
#define DINO_COL    1
#define OBS_START  15
#define SHIP_COL    1
#define SHIP_ROW    1
#define ENE_START  15
#define SCORE_COL  10

// ──────────────────────────────────────────────────────────────
// INDICES CUSTOM CHARS  (HD44780 CGRAM, max 8 chars: 0-7)
// ──────────────────────────────────────────────────────────────
#define CC_DINO_A  0
#define CC_DINO_B  1
#define CC_CACTUS  2
#define CC_SHIP    3
#define CC_ENEMY   4
#define CC_BULLET  5

// Custom chars almacenados en Flash con PROGMEM
// Se copian a la CGRAM del LCD en setup(), no ocupan RAM.
static const uint8_t CC_DATA[][8] PROGMEM = {
  // CC_DINO_A: Dino frame A  patas abiertas
  { 0x04, 0x04, 0x0E, 0x15, 0x1F, 0x0E, 0x0A, 0x11 },
  // CC_DINO_B: Dino frame B  patas juntas (mid-stride)
  { 0x04, 0x04, 0x0E, 0x15, 0x1F, 0x0E, 0x06, 0x0C },
  // CC_CACTUS
  { 0x04, 0x04, 0x15, 0x15, 0x1F, 0x04, 0x04, 0x04 },
  // CC_SHIP
  { 0x04, 0x0E, 0x1F, 0x15, 0x04, 0x0E, 0x04, 0x00 },
  // CC_ENEMY
  { 0x1F, 0x15, 0x1F, 0x0E, 0x1F, 0x15, 0x04, 0x00 },
  // CC_BULLET
  { 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00 }
};

// ──────────────────────────────────────────────────────────────
// OBJETOS
// ──────────────────────────────────────────────────────────────
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
Adafruit_SSD1306  oled(OLED_W, OLED_H, &Wire, -1);

// ──────────────────────────────────────────────────────────────
// MAQUINA DE ESTADOS
// ──────────────────────────────────────────────────────────────
enum SysState : uint8_t { ST_MENU, ST_DINO, ST_SPACE, ST_GAMEOVER };
SysState sysState = ST_MENU;

// ──────────────────────────────────────────────────────────────
// BOTON
// ──────────────────────────────────────────────────────────────
bool          btnRaw      = false;
bool          btnFiltered = false;
bool          btnPrev     = false;
bool          longFired   = false;
unsigned long btnDebT     = 0;
unsigned long btnDownT    = 0;

// ──────────────────────────────────────────────────────────────
// OLED
// ──────────────────────────────────────────────────────────────
bool          showGhost   = true;
bool          oledDirty   = true;
unsigned long oledSwapT   = 0;
unsigned long ghostMoveT  = 0;
unsigned long ghostBlinkT = 0;
int16_t       ghostX      = 10;
int8_t        ghostDir    = 1;
bool          eyesOpen    = true;

// ──────────────────────────────────────────────────────────────
// MENU
// ──────────────────────────────────────────────────────────────
uint8_t menuSel = 0;

// ──────────────────────────────────────────────────────────────
// DINO
// ──────────────────────────────────────────────────────────────
uint8_t       dinoRow     = 1;
bool          dinoJumping = false;
uint8_t       dinoFrame   = 0;
unsigned long dinoJumpT   = 0;
unsigned long dinoMoveT   = 0;
unsigned long dinoWalkT   = 0;
int8_t        obsCol      = OBS_START;
uint16_t      dinoScore   = 0;
uint16_t      dinoSpeed   = T_DINO_INIT;
// Smart-redraw (255 / -99 = invalido, fuerza primer draw)
uint8_t       pDinoRow    = 255;
uint8_t       pDinoFrame  = 255;
int8_t        pObsCol     = -99;
uint16_t      pDinoScore  = 0xFFFF;

// ──────────────────────────────────────────────────────────────
// SPACE
// ──────────────────────────────────────────────────────────────
int8_t        eneCol      = ENE_START;
uint8_t       eneRow      = 0;
bool          bulOn       = false;
int8_t        bulCol      = 0;
uint16_t      spaceScore  = 0;
unsigned long spaceMoveT  = 0;
int8_t        pEneCol     = -99;
uint8_t       pEneRow     = 255;
int8_t        pBulCol     = -99;
bool          pBulOn      = false;
uint16_t      pSpaceScore = 0xFFFF;

// ──────────────────────────────────────────────────────────────
// GAMEOVER
// ──────────────────────────────────────────────────────────────
unsigned long goTimer = 0;
uint16_t      goScore = 0;


// ════════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════════
void setup() {

  // LCD
  lcd.init();
  lcd.backlight();
  {
    uint8_t buf[8];
    for (uint8_t i = 0; i < 6; i++) {
      memcpy_P(buf, CC_DATA[i], 8);
      lcd.createChar(i, buf);
    }
  }

  // OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    // Sin OLED: blink LED builtin para diagnostico
    pinMode(LED_BUILTIN, OUTPUT);
    for (;;) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(250);
    }
  }

  // I2C Fast Mode 400kHz
  Wire.setClock(400000);

  oled.setTextColor(SSD1306_WHITE);
  oled.clearDisplay();
  oled.display();

  // Boton INPUT_PULLUP: activo en LOW, no requiere resistencia externa
  pinMode(BTN_PIN, INPUT_PULLUP);

  drawMenu();
}


// ════════════════════════════════════════════════════════════════
//  LOOP PRINCIPAL
// ════════════════════════════════════════════════════════════════
void loop() {
  handleBtn();
  updateOLED();
  switch (sysState) {
    case ST_MENU:                     break;
    case ST_DINO:     runDino();      break;
    case ST_SPACE:    runSpace();     break;
    case ST_GAMEOVER: runGameOver();  break;
  }
}


// ════════════════════════════════════════════════════════════════
//  BOTON  Debounce + Short/Long press
// ════════════════════════════════════════════════════════════════
void handleBtn() {
  bool raw = (digitalRead(BTN_PIN) == LOW); // LOW = presionado (PULLUP)

  // Debounce: ignorar cambios mas rapidos que T_DEBOUNCE
  if (raw != btnRaw) { btnDebT = millis(); btnRaw = raw; }
  if (millis() - btnDebT < T_DEBOUNCE) return;

  // Flancos de subida/bajada
  btnPrev     = btnFiltered;
  btnFiltered = raw;

  if (btnFiltered && !btnPrev) {          // flanco subida
    btnDownT  = millis();
    longFired = false;
  }

  if (btnFiltered && !longFired &&        // long press
      (millis() - btnDownT >= T_LONGPRESS)) {
    longFired = true;
    onLongPress();
  }

  if (!btnFiltered && btnPrev && !longFired) {  // short press
    onShortPress();
  }
}

void onShortPress() {
  switch (sysState) {
    case ST_MENU:
      if (menuSel == 0) startDino(); else startSpace();
      break;
    case ST_DINO:
      // [BUG-01 FIX] Salto real: permanece T_DINO_JUMP ms arriba
      if (!dinoJumping) {
        dinoJumping = true;
        dinoRow     = 0;
        dinoJumpT   = millis();
      }
      break;
    case ST_SPACE:
      if (!bulOn) {
        bulOn  = true;
        bulCol = SHIP_COL + 1;
      }
      break;
    default: break;
  }
}

void onLongPress() {
  if (sysState == ST_MENU) {
    menuSel = (menuSel + 1) % 2;
    drawMenu();
  } else {
    returnToMenu();
  }
}


// ════════════════════════════════════════════════════════════════
//  MENU
// ════════════════════════════════════════════════════════════════
void drawMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(menuSel == 0 ? F("> DINO RUNNER ") : F("  DINO RUNNER "));
  lcd.setCursor(0, 1);
  lcd.print(menuSel == 1 ? F(">SPACE SHOOTER ") : F(" SPACE SHOOTER "));
}

void returnToMenu() {
  sysState = ST_MENU;
  drawMenu();
}


// ════════════════════════════════════════════════════════════════
//  OLED  Actualizacion inteligente
// ════════════════════════════════════════════════════════════════
void updateOLED() {
  unsigned long now = millis();
  bool redraw = oledDirty;

  if (now - oledSwapT >= T_OLED_SWAP) {
    oledSwapT = now;
    showGhost = !showGhost;
    redraw    = true;
  }

  if (showGhost) {
    if (now - ghostMoveT >= T_GHOST_MOVE) {
      ghostMoveT = now;
      ghostX    += ghostDir;
      if (ghostX >= 88) ghostDir = -1;
      if (ghostX <=  3) ghostDir =  1;
      redraw = true;
    }
    if (now - ghostBlinkT >= T_GHOST_BLINK) {
      ghostBlinkT = now;
      eyesOpen    = !eyesOpen;
      redraw      = true;
    }
  }

  if (!redraw) return;
  oledDirty = false;

  oled.clearDisplay();
  if (showGhost) drawGhostScreen();
  else           drawSupermanScreen();
  oled.display();
}

// ──────────────────────────────────────────────────────────────
// GHOST
// ──────────────────────────────────────────────────────────────
void drawGhostScreen() {
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(16, 0);
  oled.print(F("~~~ GHOST MODE ~~~"));
  drawGhost(ghostX, 12);
}

void drawGhost(int16_t x, int16_t y) {
  // Cuerpo: cabeza circular + tronco rectangular
  oled.fillCircle(x + 12, y + 12, 12, SSD1306_WHITE);
  oled.fillRect  (x,      y + 12, 25, 15, SSD1306_WHITE);

  // Pies ondulados: 3 bultos con huecos concavos
  oled.fillCircle(x +  4, y + 27, 5, SSD1306_WHITE);
  oled.fillCircle(x + 12, y + 27, 5, SSD1306_WHITE);
  oled.fillCircle(x + 20, y + 27, 5, SSD1306_WHITE);
  oled.fillCircle(x +  8, y + 28, 4, SSD1306_BLACK);
  oled.fillCircle(x + 16, y + 28, 4, SSD1306_BLACK);

  // Ojos
  if (eyesOpen) {
    oled.fillCircle(x +  8, y + 10, 4, SSD1306_BLACK);
    oled.fillCircle(x + 17, y + 10, 4, SSD1306_BLACK);
    oled.fillCircle(x +  7, y +  9, 1, SSD1306_WHITE); // brillo izq
    oled.fillCircle(x + 16, y +  9, 1, SSD1306_WHITE); // brillo der
  } else {
    // Ojos cerrados: lineas
    oled.drawFastHLine(x +  5, y +  9, 7, SSD1306_BLACK);
    oled.drawFastHLine(x +  5, y + 10, 7, SSD1306_BLACK);
    oled.drawFastHLine(x + 13, y +  9, 7, SSD1306_BLACK);
    oled.drawFastHLine(x + 13, y + 10, 7, SSD1306_BLACK);
  }

  // Boca (sonrisa)
  oled.drawPixel    (x +  8, y + 19, SSD1306_BLACK);
  oled.drawFastHLine(x +  9, y + 20, 7, SSD1306_BLACK);
  oled.drawPixel    (x + 16, y + 19, SSD1306_BLACK);
}


// ──────────────────────────────────────────────────────────────
// SUPERMAN  Escudo relleno + Letra S centrada
// ──────────────────────────────────────────────────────────────
void drawSupermanScreen() {
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(22, 0);
  oled.print(F("[ SUPERMAN ]"));
  drawSupermanShield(14, 7);
}

/*
  Escudo de 100x54 px, centrado en X=64 de la pantalla (ox=14).

  Forma:
       (ox+10, oy) ─────────── (ox+90, oy)      top
           \    escudo blanco      /
     (ox, oy+33) ──────── (ox+100, oy+33)        waist
              \               /
           (ox+50, oy+54)                         punta

  La S (textSize 3 = 15x21 px) queda centrada en X=64.
  cursor_x = (64) - (15/2) = ox+50-7 = ox+43
  cursor_y = oy + 15  (margen desde el top del escudo)
*/
void drawSupermanShield(int16_t ox, int16_t oy) {

  // ESCUDO BLANCO (trapecio superior + triangulo inferior)
  oled.fillTriangle(ox+10, oy,    ox+90, oy,
                    ox,    oy+33,               SSD1306_WHITE);
  oled.fillTriangle(ox+90, oy,    ox,    oy+33,
                    ox+100,oy+33,               SSD1306_WHITE);
  oled.fillTriangle(ox,    oy+33, ox+100,oy+33,
                    ox+50, oy+54,               SSD1306_WHITE);

  // BORDE NEGRO del escudo (2 pasadas = 2px de grosor)
  for (int8_t d = 0; d < 2; d++) {
    oled.drawLine(ox+10+d, oy+d,     ox+90-d, oy+d,     SSD1306_BLACK);
    oled.drawLine(ox+10-d, oy+d,     ox-d,    oy+33,    SSD1306_BLACK);
    oled.drawLine(ox+90+d, oy+d,     ox+100+d,oy+33,    SSD1306_BLACK);
    oled.drawLine(ox-d,    oy+33,    ox+50,   oy+54+d,  SSD1306_BLACK);
    oled.drawLine(ox+100+d,oy+33,    ox+50,   oy+54+d,  SSD1306_BLACK);
  }

  // LETRA S grande y centrada (negro sobre fondo blanco del escudo)
  oled.setTextSize(3);
  oled.setTextColor(SSD1306_BLACK);
  oled.setCursor(ox + 43, oy + 14);
  oled.print(F("S"));

  // Restaurar para el resto del programa
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
}


// ════════════════════════════════════════════════════════════════
//  DINO RUNNER
// ════════════════════════════════════════════════════════════════
void startDino() {
  sysState    = ST_DINO;
  dinoRow     = 1;
  dinoJumping = false;
  dinoFrame   = 0;
  obsCol      = OBS_START;
  dinoScore   = 0;
  dinoSpeed   = T_DINO_INIT;
  dinoMoveT   = millis();
  dinoWalkT   = millis();

  lcd.clear();
  pDinoRow   = 255;
  pDinoFrame = 255;
  pObsCol    = -99;
  pDinoScore = 0xFFFF;

  dinoDraw(true);
}

void runDino() {
  unsigned long now = millis();

  // [BUG-01 FIX] Gravedad: bajar al suelo despues de T_DINO_JUMP ms
  if (dinoJumping && (now - dinoJumpT >= T_DINO_JUMP)) {
    dinoJumping = false;
    dinoRow     = 1;
  }

  // Animacion walking (solo en el suelo)
  if (!dinoJumping && (now - dinoWalkT >= T_DINO_WALK)) {
    dinoWalkT = now;
    dinoFrame = 1 - dinoFrame;
  }

  // Mover obstaculo
  if (now - dinoMoveT >= dinoSpeed) {
    dinoMoveT = now;
    obsCol--;
    if (obsCol < 0) {
      obsCol = OBS_START;
      dinoScore++;
      if (dinoSpeed > T_DINO_MIN) dinoSpeed -= T_DINO_ACCEL;
    }
  }

  // Colision: cactus llega a col del dino Y dino esta en el suelo
  if (obsCol == DINO_COL && dinoRow == 1) {
    gameOver(dinoScore);
    return;
  }

  dinoDraw(false);
}

void dinoDraw(bool force) {
  // Sprite activo: salto siempre usa frame A
  uint8_t cc = (!dinoJumping && dinoFrame == 1) ? CC_DINO_B : CC_DINO_A;

  // Dino: redibujar si cambio de fila o de frame
  bool rowChanged   = (force || dinoRow  != pDinoRow);
  bool frameChanged = (!dinoJumping && dinoFrame != pDinoFrame);

  if (rowChanged || frameChanged) {
    if (rowChanged && !force && pDinoRow < 2) {
      lcd.setCursor(DINO_COL, pDinoRow);
      lcd.print(' ');
    }
    lcd.setCursor(DINO_COL, dinoRow);
    lcd.write(byte(cc));
    pDinoRow   = dinoRow;
    pDinoFrame = dinoFrame;
  }

  // Obstaculo
  if (force || obsCol != pObsCol) {
    if (!force && pObsCol >= 0 && pObsCol < 16) {
      lcd.setCursor(pObsCol, 1);
      lcd.print(' ');
    }
    if (obsCol >= 0 && obsCol < 16) {
      lcd.setCursor(obsCol, 1);
      lcd.write(byte(CC_CACTUS));
    }
    pObsCol = obsCol;
  }

  // Score
  if (force || dinoScore != pDinoScore) {
    lcd.setCursor(SCORE_COL, 0);
    lcd.print(F("S:"));
    if      (dinoScore <    10) lcd.print(F("   "));
    else if (dinoScore <   100) lcd.print(F("  "));
    else if (dinoScore <  1000) lcd.print(' ');
    lcd.print(dinoScore);
    pDinoScore = dinoScore;
  }
}


// ════════════════════════════════════════════════════════════════
//  SPACE SHOOTER
// ════════════════════════════════════════════════════════════════
void startSpace() {
  sysState    = ST_SPACE;
  eneCol      = ENE_START;
  eneRow      = 0;
  bulOn       = false;
  bulCol      = 0;
  spaceScore  = 0;
  spaceMoveT  = millis();

  lcd.clear();
  pEneCol     = -99;
  pEneRow     = 255;
  pBulCol     = -99;
  pBulOn      = false;
  pSpaceScore = 0xFFFF;

  // Nave fija: se dibuja una vez
  lcd.setCursor(SHIP_COL, SHIP_ROW);
  lcd.write(byte(CC_SHIP));

  spaceDraw(true);
}

void runSpace() {
  unsigned long now = millis();

  if (now - spaceMoveT < T_SPACE) return;
  spaceMoveT = now;

  // Mover enemigo
  eneCol--;
  if (eneCol < 0) {
    eneCol = ENE_START;
    eneRow = (uint8_t)random(0, 2);
  }

  // Mover bala
  if (bulOn) {
    bulCol++;
    if (bulCol >= 16) { bulOn = false; bulCol = 0; }
  }

  // [BUG-02 FIX] Colision bala-enemigo: verifica fila Y
  // La bala viaja en SHIP_ROW (fila 1).
  // Solo impacta si el enemigo esta en la MISMA fila.
  if (bulOn && bulCol == eneCol && eneRow == SHIP_ROW) {
    bulOn  = false;
    eneCol = ENE_START;
    eneRow = (uint8_t)random(0, 2);
    spaceScore++;
  }

  // [BUG-03 FIX] Colision enemigo-nave: verifica fila Y
  // En v1.0: si eneCol==SHIP_COL sin importar fila -> GameOver
  // Ahora: GameOver solo si el enemigo choca en la fila de la nave
  if (eneCol == SHIP_COL && eneRow == SHIP_ROW) {
    gameOver(spaceScore);
    return;
  }

  spaceDraw(false);
}

void spaceDraw(bool force) {

  // Enemigo
  bool eChanged = (force || eneCol != pEneCol || eneRow != pEneRow);
  if (eChanged) {
    if (!force && pEneCol >= 0 && pEneCol < 16 && pEneRow < 2) {
      lcd.setCursor(pEneCol, pEneRow);
      lcd.print(' ');
    }
    if (eneCol >= 0 && eneCol < 16) {
      lcd.setCursor(eneCol, eneRow);
      lcd.write(byte(CC_ENEMY));
    }
    pEneCol = eneCol;
    pEneRow = eneRow;
  }

  // Bala
  bool bShow    = (bulOn && bulCol > 0 && bulCol < 16);
  bool bChanged = (force || bulOn != pBulOn || bulCol != pBulCol);

  if (bChanged) {
    // Borrar bala previa (nunca en col de nave pq bulCol >= SHIP_COL+1)
    if (!force && pBulOn && pBulCol > 0 && pBulCol < 16) {
      lcd.setCursor(pBulCol, SHIP_ROW);
      lcd.print(' ');
    }
    if (bShow) {
      lcd.setCursor(bulCol, SHIP_ROW);
      lcd.write(byte(CC_BULLET));
    }
    pBulCol = bulCol;
    pBulOn  = bulOn;

    // Asegurar nave visible en primer draw
    if (force) {
      lcd.setCursor(SHIP_COL, SHIP_ROW);
      lcd.write(byte(CC_SHIP));
    }
  }

  // Score
  if (force || spaceScore != pSpaceScore) {
    lcd.setCursor(SCORE_COL, 0);
    lcd.print(F("S:"));
    if      (spaceScore <    10) lcd.print(F("   "));
    else if (spaceScore <   100) lcd.print(F("  "));
    else if (spaceScore <  1000) lcd.print(' ');
    lcd.print(spaceScore);
    pSpaceScore = spaceScore;
  }
}


// ════════════════════════════════════════════════════════════════
//  GAME OVER  No-bloqueante [BUG-04 FIX]
//  En v1.0: delay(2000) congelaba la OLED.
//  En v2.0: timer millis(), OLED sigue animando.
// ════════════════════════════════════════════════════════════════
void gameOver(uint16_t score) {
  sysState = ST_GAMEOVER;
  goScore  = score;
  goTimer  = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("  ** GAME OVER **"));
  lcd.setCursor(0, 1);
  lcd.print(F("   Score: "));
  lcd.print(goScore);
}

void runGameOver() {
  if (millis() - goTimer >= T_GAMEOVER) returnToMenu();
}
