/* ====================================================================
   PWRCNTRL - Power Controller for PSU_PDS317
   Arduino Nano

   ====================================================================
   PREMISSAS DE PINAGEM (ajuste os #define abaixo se estiver diferente
   do que voce tinha em mente - o resto do codigo nao muda):

   - O pedido original listava "3 saidas digitais para 3 reles" mas
     descrevia D5/D6/D7 como 3 COMPORTAMENTOS, nao como os 3 pinos de
     rele em si. Como cada rele e cada PWRG precisam de pino proprio
     e independente, montei assim:

       Reles (saida, aciona o rele):      D2, D3, D4
       Power Good (saida, PWRG1/2/3):     D8, D9, D10
       D5  -> ANY_RELAY_ON (indicador agregado: acende se pelo menos
              1 rele estiver ligado - nao pedido explicitamente, so
              um extra barato de manter; pode remover se nao quiser)
       D6  -> CPU_FEEDBACK_PIN (ENTRADA digital, ativo em LOW) - "receber
              feedback da CPU sobre o que fazer" - ver secao CPU FEEDBACK
       D7  -> nao usado como pino fisico. O LCD I2C usa A4(SDA)/A5(SCL)
              no Nano, nao precisa de pino digital dedicado.
       A0  -> sensor do rail de 12V (divisor 0-12V -> 0-5V no ADC)
       A1  -> sensor do rail de 5V  (0-5V direto, 1:1)
       A2  -> sensor do rail de -5V (front-end deve mapear -5V..0V do
              rail para 0..5V no ADC - ver CALIBRACAO abaixo)
    PWRCNTRLV7.INO
    Os pinos I2C no Arduino Pro Mini são:
    SDA (Data): Pino A4
    SCL (Clock): Pino A5
   ==================================================================== */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/* ---------------- Pinagem ---------------- */
/*ADC to mesure voltages*/
#define PIN_ADC_12V     A0
#define PIN_ADC_5V      A1
#define PIN_ADC_NEG5V   A3
/*GPIO to turn on/off relays with voltages to CPU*/
#define PIN_RELAY_12V   2
#define PIN_RELAY_5V    3
#define PIN_RELAY_NEG5V 4
/*Power goog information to CPU*/
#define PIN_PWRG_12V    8
#define PIN_PWRG_5V     9
#define PIN_PWRG_NEG5V  10
/*This sinalize any relay on*/
#define PIN_ANY_RELAY_ON     5   /* indicador agregado, ver nota acima */
/*CPU feedback for a powergood fail*/
#define PIN_CPU_FEEDBACK     6   /* entrada, ativo em LOW */
/*GPIO to turn on/off relays to turn on/off PSUs*/
#define PIN_RELAY_POWER_ON   7   /* SAIDA reles power on liga as fontes */
/*Switch to turn on/off relays to turn on/off PSUs*/
#define PIN_KEY_POWER_ON    11  /* ENTRADA chave liga/desliga power pino 7*/
/*GPIO The CPU sends a signal to shut down the power supply. */
#define PIN_CPU_POWERDOWN   13   /*Cpu ask for powerdown */

/*State of the power*/
#define PWRUP       0
#define PWRFAIL     1
#define PWRDOWN     2

/* ---------------- LCD I2C 16x4 ---------------- */
/* Endereco padrao mais comum e 0x27; alguns modulos PCF8574 vem em
   0x3F. Se o display nao responder, rode um I2C scanner pra confirmar. */
#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);



/* ---------------- Calibracao (AJUSTAR conforme o hardware real) ----------------
   Formula geral: V_real = (adc * 5.0 / 1023.0) * SCALE + OFFSET

   12V: divisor resistivo mapeando 0-12V -> 0-5V no ADC => SCALE = 12/5
   5V : 1:1, sem divisor                                 => SCALE = 1
   -5V: front-end (op-amp/divisor) mapeando -5V..0V do
        rail para 0..5V no ADC (0V ADC = -5V rail,
        5V ADC = 0V rail)                                => SCALE = 1, OFFSET = -5.0
*/
const float SCALE_12V   = 12.0 / 5.0;
const float OFFSET_12V  = 0.0;

const float SCALE_5V    = 1.0;
const float OFFSET_5V   = 0.0;

const float SCALE_NEG5V  = 1.0;
const float OFFSET_NEG5V = 0.0;

/* ---------------- Nominais e limiares de histerese ---------------- */
const float NOM_12V   = 12.0;
const float NOM_5V    = 5.0;
const float NOM_NEG5V = 5.0;   /* nominal negativo - tratamos por magnitude */

const float PCT_ON  = 0.95;  /* liga rele + PWRG ao atingir 95% do nominal */
const float PCT_OFF = 0.90;  /* desliga PWRG se cair abaixo de 90% do nominal */

/* Tempo minimo que a leitura precisa se manter acima/abaixo do limiar
   antes de agir - evita chaveamento por ruido/transiente (debounce) */
const unsigned long QUALIFY_MS = 100;

/* ---------------- hold-off de recuperacao pos power-fail ----------------
   Depois de um power-fail (PWRG caiu), a tensao pode ficar oscilando
   bem na fronteira do limiar de 95% por causa de ruido/transiente da
   propria fonte se recuperando. Sem um tempo minimo de estabilizacao
   CONTINUA acima de 95%, o PWRG fica piscando on/off a cada iteracao,
   e a CPU entra num loop de "liga -> powerfail -> pede desligar".

   RECOVERY_HOLD_MS exige que a tensao fique 95%+ SEM INTERRUPCAO por
   esse tempo antes de avisar a CPU que esta tudo bem de novo. E de
   propria intencao maior que o QUALIFY_MS da partida inicial, porque
   recuperacao de falha merece mais cautela que ligar pela primeira vez.
   Ajuste esse valor conforme o comportamento real da sua fonte. */
const unsigned long RECOVERY_HOLD_MS = 10000; /* 10s, ajustavel */

/* Intervalo de atualizacao do LCD (nao bloqueante) */
const unsigned long LCD_UPDATE_MS = 500;

volatile unsigned char powerOn = 0;


const unsigned long DEBOUNCE_MS = 100; /* ajuste conforme o botao/ruido */
bool stableState   = HIGH;  /* estado ja filtrado (HIGH = solto, com INPUT_PULLUP) */
bool lastRawState   = HIGH;
unsigned long lastChangeTime = 0;
bool outputState = false;   /* estado atual da saida controlada pelo toggle - true = sistema comandado a ligar */

/* ---------------- Debounce do PIN_CPU_FEEDBACK ----------------
   Mesmo principio do debounce do botao: uma leitura crua so vira
   "real" depois de ficar estavel por CPU_FEEDBACK_DEBOUNCE_MS. Isso
   evita que um glitch de ruido (ex: comutacao dos proprios reles bem
   proximos ao pino) dispare um corte de trilha por engano - o sinal
   precisa ficar em LOW de forma sustentada, nao so por 1 leitura. */
const unsigned long CPU_FEEDBACK_DEBOUNCE_MS = 30; /* ajustavel */
bool cpuFbStableState = HIGH; /* HIGH = inativo, com INPUT_PULLUP */
bool cpuFbLastRaw = HIGH;
unsigned long cpuFbLastChangeTime = 0;

const unsigned long CPU_POWERDOWN_DEBOUNCE_MS = 30; /* ajustavel */
bool cpuPDStableState = HIGH; /* HIGH = inativo, com INPUT_PULLUP */
bool cpuPDLastRaw = HIGH;
unsigned long cpuPDLastChangeTime = 0;


bool readCpuFeedbackFiltered() {
    bool raw = digitalRead(PIN_CPU_FEEDBACK);

    if (raw != cpuFbLastRaw) {
        cpuFbLastChangeTime = millis();
        cpuFbLastRaw = raw;
    }

    if ((millis() - cpuFbLastChangeTime) >= CPU_FEEDBACK_DEBOUNCE_MS) {
        cpuFbStableState = raw;
    }

    return cpuFbStableState;
}

bool readCpuPowerDownFiltered() {
    bool raw = digitalRead(PIN_CPU_POWERDOWN);

    if (raw != cpuPDLastRaw) {
        cpuPDLastChangeTime = millis();
        cpuPDLastRaw = raw;
    }

    if ((millis() - cpuPDLastChangeTime) >= CPU_POWERDOWN_DEBOUNCE_MS) {
        cpuPDStableState = raw;
    }

    return cpuPDStableState;
}


/* ---------------- Estado de cada trilho ---------------- */
struct Rail {
    const char *name;
    int   adcPin;
    float scale;
    float offset;
    float nominal;      /* pode ser negativo (-5V) */
    int   relayPin;
    int   pwrgPin;

    bool  relayOn;
    bool  pwrgOn;

    /* controle de debounce */
    bool  pendingOn;
    bool  pendingOff;
    unsigned long pendingSince;

    /* controle de hold-off na recuperacao pos power-fail */
    bool  pendingRecover;
    unsigned long pendingRecoverSince;

    /* marca que essa trilha foi cortada EM OPERACAO (nao por
       desligamento completo do botao) - ex: via CPU_FEEDBACK. Enquanto
       true, a proxima re-qualificacao usa RECOVERY_HOLD_MS em vez de
       QUALIFY_MS, mesmo entrando pelo bloco de "partida". */
    bool  hadFault;

    float lastVoltage;
    unsigned char status;
};

Rail rails[3] = {
    { "12V",  PIN_ADC_12V,   SCALE_12V,   OFFSET_12V,   NOM_12V,   PIN_RELAY_12V,   PIN_PWRG_12V,   false, false, false, false, 0, false, 0, false, 0.0,PWRDOWN },
    { "5V",   PIN_ADC_5V,    SCALE_5V,    OFFSET_5V,    NOM_5V,    PIN_RELAY_5V,    PIN_PWRG_5V,    false, false, false, false, 0, false, 0, false, 0.0,PWRDOWN },
    { "-5V",  PIN_ADC_NEG5V, SCALE_NEG5V, OFFSET_NEG5V, NOM_NEG5V, PIN_RELAY_NEG5V, PIN_PWRG_NEG5V, false, false, false, false, 0, false, 0, false, 0.0,PWRDOWN },
};

/* ---------------- Prototipos manuais (workaround da IDE Arduino) ----------------
   A IDE Arduino gera automaticamente prototipos de todas as funcoes do
   arquivo, mas os insere logo no topo do .ino - ANTES da struct Rail
   ser definida. Isso causa "'Rail' does not name a type" porque o
   compilador ve o tipo Rail sendo usado num prototipo antes de saber
   o que ele e. Declarando os prototipos aqui manualmente, DEPOIS da
   struct, a IDE reconhece que ja existe um prototipo e nao insere o
   dela (quebrado) no topo. E' um comportamento documentado da IDE,
   nao um erro no codigo em si. */
float readRailVoltage(const Rail &r);
void resetRailInternalState(Rail &r);
void updateRail(Rail &r);

unsigned long lastLcdUpdate = 0;

/* ---------------- Leitura de tensao ---------------- */
float readRailVoltage(const Rail &r) {
    const int N = 8;
    long sum = 0;
    for (int i = 0; i < N; i++) {
        sum += analogRead(r.adcPin);
    }
    float adcAvg = (float)sum / N;
    float vAdc = adcAvg * 5.0 / 1023.0;
    return vAdc * r.scale + r.offset;
}

/* ---------------- reseta o estado interno de uma trilho ----------------
   Usado quando o power geral e desligado pela chave (PIN_KEY_POWER_ON),
   pra manter o estado interno (relayOn/pwrgOn/pending*) coerente com
   os pinos fisicos, que ja iam pra LOW no handler do botao. */
void resetRailInternalState(Rail &r) {
    r.relayOn = false;
    r.pwrgOn = false;
    r.pendingOn = false;
    r.pendingOff = false;
    r.pendingRecover = false;
    r.hadFault = false; /* desligamento completo e ciclo limpo, nao falha */
    r.status = PWRDOWN;
    digitalWrite(r.relayPin, LOW);
    digitalWrite(r.pwrgPin, LOW);
}

/* ---------------- Logica de histerese por trilho ---------------- */
void updateRail(Rail &r) {
    float v = readRailVoltage(r);
    r.lastVoltage = v;
    //Serial.print("r: ");
    //Serial.print(r.name);
    //Serial.print(" tensao ");
    //Serial.println(v);

    /* CRITICO - CORRECAO DE SEGURANCA: se o sistema foi desligado pela
       chave (outputState==false), NAO reavalia liga/desliga de rele
       nenhum daqui pra frente - so registra a tensao (acima) pra
       continuar mostrando no LCD durante a descarga.

       Sem isso: apos o corte do rele de mains, os capacitores internos
       das fontes ainda mantem a tensao de saida acima de 95% por um
       instante (decaimento gradual, nao instantaneo). Como
       resetRailInternalState() ja tinha zerado hadFault, o bloco de
       "LIGAR" abaixo usava so QUALIFY_MS (100ms) - e em menos de
       100ms reconectava o rele da CPU bem no meio da tensao caindo,
       exatamente o cenario perigoso que motivou esse fix. Agora nada
       e reavaliado enquanto outputState for false; so volta a
       funcionar quando a chave ligar o sistema de novo de verdade. */
    if (!outputState) {
        return;
    }

    float frac = (r.nominal != 0.0) ? (v / r.nominal) : 0.0;

    unsigned long now = millis();

    /* ---- condicao de LIGAR (95% do nominal) ----
       Se essa trilha foi cortada EM OPERACAO (hadFault=true, por
       exemplo via CPU_FEEDBACK), exige o mesmo tempo de estabilidade
       da recuperacao normal (RECOVERY_HOLD_MS) em vez do QUALIFY_MS
       de partida a frio - senao um corte durante operacao vira uma
       reativacao rapida disfarcada de "partida nova". */
    if (!r.relayOn) {
        unsigned long requiredHold = r.hadFault ? RECOVERY_HOLD_MS : QUALIFY_MS;

        if (frac >= PCT_ON) {
            if (!r.pendingOn) {
                r.pendingOn = true;
                r.pendingSince = now;
            } else if (now - r.pendingSince >= requiredHold) {
                r.relayOn = true;
                r.pwrgOn  = true;
                digitalWrite(r.relayPin, HIGH);
                digitalWrite(r.pwrgPin, HIGH);
                r.pendingOn = false;
                r.hadFault = false; /* re-qualificada, limpa a marca de falha */
            }
        } else {
            r.pendingOn = false;
        }
    }

    /* ---- condicao de DESLIGAR o PWRG (queda de 10% do nominal) ----
       OBS: por pedido explicito, essa condicao so desliga o PWRG.
       O rele permanece fechado (nao abre sozinho) ate:
         a) a tensao voltar a >=95% por RECOVERY_HOLD_MS seguidos, ou
         b) o feedback da CPU (D6) pedir corte explicito - ver loop().
       Se voce preferir que o rele abra junto quando o PWRG cai,
       e so mover digitalWrite(r.relayPin, LOW) pra dentro deste bloco. */
    if (r.relayOn && r.pwrgOn) {
        if (frac < PCT_OFF) {
            if (!r.pendingOff) {
                r.pendingOff = true;
                r.pendingSince = now;
            } else if (now - r.pendingSince >= QUALIFY_MS) {
                r.pwrgOn = false;
                digitalWrite(r.pwrgPin, LOW);
                r.pendingOff = false;
            }
        } else {
            r.pendingOff = false;
        }
    }

    /* ---- recuperacao do PWRG apos power-fail, com hold-off ----
       Exige RECOVERY_HOLD_MS de estabilidade CONTINUA acima de 95%
       antes de avisar a CPU que esta tudo bem de novo. */
    if (r.relayOn && !r.pwrgOn) {
        if (frac >= PCT_ON) {
            if (!r.pendingRecover) {
                r.pendingRecover = true;
                r.pendingRecoverSince = now;
                Serial.print("[DEBUG] ");
                Serial.print(r.name);
                Serial.println(": iniciando hold-off de recuperacao");
            } else if (now - r.pendingRecoverSince >= RECOVERY_HOLD_MS) {
                r.pwrgOn = true;
                digitalWrite(r.pwrgPin, HIGH);
                r.pendingRecover = false;
                Serial.print("[DEBUG] ");
                Serial.print(r.name);
                Serial.println(": PWRG restaurado apos hold-off completo");
            } else {
                Serial.print("[DEBUG] ");
                Serial.print(r.name);
                Serial.print(": aguardando hold-off, faltam ");
                Serial.print(RECOVERY_HOLD_MS - (now - r.pendingRecoverSince));
                Serial.println(" ms");
            }
        } else {
            if (r.pendingRecover) {
                Serial.print("[DEBUG] ");
                Serial.print(r.name);
                Serial.println(": hold-off cancelado, tensao caiu de novo");
            }
            /* caiu de novo antes de completar o hold-off - reinicia
               a exigencia de estabilidade continua, nao acumula tempo */
            r.pendingRecover = false;
        }
    }
    //delay(1000); just for debug
    if(r.lastVoltage < 1.0){
        r.status = PWRDOWN;
    }else if(r.lastVoltage > 1.0 && r.lastVoltage < (r.nominal * 90/100)){
        r.status = PWRFAIL;
    }else if( r.lastVoltage > (r.nominal * 90/100)){    
        r.status = PWRUP;
    }else{
        lcd.setCursor(0, 3);
        lcd.print("PANIC: VOLTAGE VALUE");
    }


}

/* ---------------- Feedback da CPU (D6, entrada, ativo em LOW) ----------------
   Placeholder minimo: se a CPU assinalar esse pino enquanto QUALQUER
   PWRG estiver em falha, a interpretacao aqui e "corte a trilha com
   falha agora". Ajuste essa politica conforme o protocolo real que
   voce definir entre a CPU e o PWRCNTRL (hoje e so 1 bit, sem
   granularidade de qual trilho ou qual acao). */
bool cpuFeedbackWasActive = false; /* rastreia transicao pro log baseado em borda */

void handleCpuFeedback() {
    bool cpuRequestsCut = (readCpuFeedbackFiltered() == LOW);

    /* log so na transicao (inativo->ativo e ativo->inativo), nao a
       cada loop enquanto o sinal ficar sustentado - evita poluir o
       Serial Monitor sem perder a informacao de quando comecou/parou */
    if (cpuRequestsCut != cpuFeedbackWasActive) {
        cpuFeedbackWasActive = cpuRequestsCut;
        Serial.println(cpuRequestsCut
            ? "[DEBUG] PIN_CPU_FEEDBACK ficou ATIVO (LOW) - avaliando corte"
            : "[DEBUG] PIN_CPU_FEEDBACK voltou a INATIVO (HIGH)");
    }

    if (!cpuRequestsCut) return;

    for (int i = 0; i < 3; i++) {
        Rail &r = rails[i];
        if (r.relayOn && !r.pwrgOn) {
            r.status = PWRFAIL;
            r.relayOn = false;
            r.hadFault = true;  /* corte em operacao - proxima re-qualificacao usa RECOVERY_HOLD_MS */
            digitalWrite(r.relayPin, LOW);
            outputState = !outputState;
            Serial.print("[DEBUG] ");
            Serial.print(r.name);
            Serial.println(": relayOn FORCADO para false via CPU_FEEDBACK");
        }
    }
}
bool cpuRequestsPowerDownWasActive = false; /* rastreia transicao pro log baseado em borda */

void handleCpuPowerDown(){
    bool cpuRequestsPowerDown = (readCpuPowerDownFiltered() == LOW);
    if (cpuRequestsPowerDown != cpuRequestsPowerDownWasActive) {
        cpuRequestsPowerDownWasActive = cpuRequestsPowerDown;
        Serial.println(cpuRequestsPowerDown
            ? "[DEBUG] PIN_CPU_POWERDOWN ficou ATIVO (LOW) - avaliando corte"
            : "[DEBUG] PIN_CPU_POWERDOWN voltou a INATIVO (HIGH)");
    }

    if (!cpuRequestsPowerDown) return;

    for (int i = 0; i < 3; i++) {
        Rail &r = rails[i];
        if (r.relayOn && !r.pwrgOn) {
            r.status = PWRDOWN;
            r.relayOn = false;
            r.hadFault = true;  /* corte em operacao - proxima re-qualificacao usa RECOVERY_HOLD_MS */
            digitalWrite(r.relayPin, LOW);
            outputState = !outputState;
            Serial.print("[DEBUG] ");
            Serial.print(r.name);
            Serial.println(": relayOn FORCADO para false via CPU_POWERDOWN");
        }
    }


}
/* ---------------- Indicador agregado (D5) ---------------- */
void updateAnyRelayIndicator() {
    bool any = rails[0].relayOn || rails[1].relayOn || rails[2].relayOn;
    digitalWrite(PIN_ANY_RELAY_ON, any ? HIGH : LOW);
}

/* ---------------- LCD ---------------- */
void updateLcd() {

    lcd.setCursor(0, 0);
    lcd.print("12V:");
    lcd.print(rails[0].lastVoltage, 2);
    lcd.print("  ");
    if(rails[0].lastVoltage < 1.0)
        lcd.print(" PWR down");
    else
        lcd.print(rails[0].pwrgOn ? "OK      " : " FAIL    ");

    lcd.setCursor(0, 1);
    lcd.print(" 5V: ");
    lcd.print(rails[1].lastVoltage, 2);
    lcd.print("  ");
    if(rails[1].lastVoltage < 1.0)
        lcd.print("PWR down");
    else
        lcd.print(rails[1].pwrgOn ? "OK      " : "FAIL    ");

    lcd.setCursor(4, 2);
    lcd.print("-5V:");
    lcd.print((rails[2].lastVoltage * -1), 2);
    lcd.print("  ");
    lcd.setCursor(15, 2);
    if(rails[2].lastVoltage < 1.0)
        lcd.print("PWR down");
    else
        lcd.print(rails[2].pwrgOn ? "OK      " : "FAIL    ");


    lcd.setCursor(4, 3);
    lcd.print("CPU FB:");
    lcd.print(readCpuFeedbackFiltered() == LOW ? "ATIVO" : "-----");
    lcd.setCursor(17, 3);
    lcd.print(outputState ? "SYS: ON" : "SYS:OFF");            

}

/* ---------------- Debug serial (opcional, ajuda no bring-up) ---------------- */
void printSerialDebug() {
  return;
    for (int i = 0; i < 3; i++) {
        Serial.print(rails[i].name);
        Serial.print("=");
        Serial.print(rails[i].lastVoltage, 3);
        Serial.print("V relay=");
        Serial.print(rails[i].relayOn ? "ON " : "off");
        Serial.print(" pwrg=");
        Serial.print(rails[i].pwrgOn ? "ON " : "off");
        Serial.print("  ");
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);

    for (int i = 0; i < 3; i++) {
        pinMode(rails[i].relayPin, OUTPUT);
        pinMode(rails[i].pwrgPin, OUTPUT);
        digitalWrite(rails[i].relayPin, LOW);
        digitalWrite(rails[i].pwrgPin, LOW);
    }
    
    pinMode(PIN_KEY_POWER_ON, INPUT_PULLUP);
    pinMode(PIN_RELAY_POWER_ON, OUTPUT);
    digitalWrite(PIN_RELAY_POWER_ON, LOW);  /* Power off */
    pinMode(PIN_ANY_RELAY_ON, OUTPUT);
    digitalWrite(PIN_ANY_RELAY_ON, LOW);

    pinMode(PIN_CPU_FEEDBACK, INPUT_PULLUP);
    pinMode(PIN_CPU_POWERDOWN, INPUT_PULLUP);

    lcd.init();
    lcd.backlight();
    lcd.clear();
}

static void dbounce(bool raw){
    /* qualquer variacao na leitura crua reinicia o cronometro de debounce */
    if (raw != lastRawState) {
        lastChangeTime = millis();
        lastRawState = raw;
    }

    /* so aceita a leitura como "real" depois de ficar parada por DEBOUNCE_MS */
    if ((millis() - lastChangeTime) >= DEBOUNCE_MS) {
        if (raw != stableState) {
            bool previousStable = stableState;
            stableState = raw;

            /* dispara o toggle SO na borda de pressionar (HIGH -> LOW),
               nunca ao soltar (LOW -> HIGH) - e so uma vez por pressionada,
               porque so entra aqui quando o estado estavel realmente muda */
            if (previousStable == HIGH && stableState == LOW) {
                outputState = !outputState;
                if(! outputState ){
                    /* alem de zerar os pinos, reseta o estado
                       interno (relayOn/pwrgOn/pending*) de cada trilha.
                       Isso, combinado com o "if (!outputState) return;"
                       la em cima no updateRail(), garante que nada
                       reative os reles enquanto o sistema estiver
                       desligado, mesmo com tensao residual dos
                       capacitores das fontes. */
                    resetRailInternalState(rails[0]);
                    resetRailInternalState(rails[1]);
                    resetRailInternalState(rails[2]);
                    Serial.println("outputState == 0");
                    digitalWrite(PIN_RELAY_POWER_ON, LOW);
                }
                if( outputState && (rails[1].status == PWRDOWN) ){
                    Serial.println("1-status: PWRDOWN");
                    digitalWrite(PIN_RELAY_POWER_ON, HIGH);
                    rails[1].status = PWRUP;
                }
                if (rails[1].status == PWRUP){
                    Serial.println("status: PWRUP");
                }
                if (rails[1].status == PWRFAIL){
                    Serial.println("status: PWRFAIL");
                }
                if (rails[1].status == PWRDOWN){
                    Serial.println("status: PWRDOWN");
                }
                Serial.println(outputState ? "LIGADO" : "DESLIGADO");
                //lcd.setCursor(17, 3);
                //lcd.print(outputState ? "SYS: ON" : "SYS:OFF");            
            }
        }
    }
}
bool readPowerButtonFiltered() {
    bool raw = digitalRead(PIN_KEY_POWER_ON);
    dbounce(raw);
}

void loop() {
    readPowerButtonFiltered();

 
    for (int i = 0; i < 3; i++) {
        updateRail(rails[i]);
    }

    handleCpuFeedback();
    handleCpuPowerDown();
    updateAnyRelayIndicator();

    unsigned long now = millis();
    if (now - lastLcdUpdate >= LCD_UPDATE_MS) {
        lastLcdUpdate = now;
        updateLcd();
        printSerialDebug();
    }
}
