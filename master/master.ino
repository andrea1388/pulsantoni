#include <RFM69.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#define DURATACLICKLUNGO 2000 // tempo pressione pulsante per click lungo = 2 secondi
#define TBACKOUTPULSANTE 100 // tempo blackout pulsante dopo un click
//stati
#define ZERO 0 // stato iniziale
#define DISCOVERY 1 // discovery: cerca gli slave presenti in rete
#define INVIASYNC 2 // pre voto. Invia l'ora del master a tutti gli slave svovati con discovery
#define VOTO 3 // fase di acquisizione dei voti e display risultati
// parametri radio
#define NETWORKID 27
#define FREQUENCY 868000000
#define RFM69_CS 10
#define RFM69_IRQ 2
#define RFM69_IRQN 0 
#define RFM69_RST 9
// stati per elaborazione seriale
#define COMANDO 0
#define VALORE 1
// dati display
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);



#define pinPULSANTE 3
/*
class Display {
	public:
		Display(rs, enable, d4, d5, d6, d7);
		Print(char *);
		
}
Display::Print(char* s) {
	
}
*/

byte numero_votati,indirizzo_slave_discovery;
byte numero_slave;


class Slave {
  public:
    unsigned long oravoto;
    byte fallimenti;
    //byte tensionebatteria;
    //byte rssi;
    //bool funzionante;
    byte indirizzo;
    Slave(byte);
};
Slave::Slave(byte ind) {
  indirizzo=ind;
}

Slave** slave;
Slave* best[5];


class Stato {
  public:
    Stato();
    byte getStato();
    void setStato(byte);
  private:
    byte stato;
};
Stato::Stato() {stato=0;};
//algoritmo 10
byte Stato::getStato() {return stato;};
void Stato::setStato(byte newstato) {
  stato=newstato;
  switch(newstato) {
    case ZERO:
	  //disp.print(0,F("Pronto"));
      numero_votati=0;  
      indirizzo_slave_discovery=0;
      // azzera i 5 migliori
      for(int f=0;f<5;f++) best[f]=0;
      Serial.println(F("s0"));
      break;
    case DISCOVERY:
	  ////disp.print(0,F("Ricerca dispositivi"));
	  //disp.print(1,F("Trovati: "));
      indirizzo_slave_discovery=1;
      numero_slave=0; // cancella la lista
      Serial.println(F("ds"));
      break;
    case INVIASYNC:
	  //disp.print(0,F("Invio sincronismo"));
      if(numero_slave==0) {
        Serial.println(F("slave=0"));
        stato=0;
        return;
      }
      Serial.println(F("is"));
      break;
    case VOTO:
	  //disp.print(0,F("Pronto per gara"));
      for(byte i=0;i<numero_slave;i++)
        slave[i]->oravoto=0;
      Serial.println(F("ip"));
      break;
    default:
      stato=ZERO;
      Serial.println(F("e setstato errato"));
      return;
  }
  stato=newstato;
}

Stato stato;
RFM69 radio;
byte numero_max_slave;
unsigned long t_inizio_voto;
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


void setup() {
  pinMode(pinPULSANTE, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.print(F("ns "));
  //lcd.begin(16, 2);
  numero_max_slave=EEPROM.read(1);
  Serial.println(numero_max_slave);

  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());
  tft.reset();
  uint16_t identifier = tft.readID();
  identifier=0x9341;
  tft.begin(identifier);
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(RED);  
  tft.setTextSize(3);
  tft.setRotation(1);
  tft.println("Pulsantoni");
  tft.setTextSize(2);
  tft.setTextColor(GREEN);  
  tft.print("numero slave: ");
  tft.println(numero_max_slave);

  delay(20);
  radioSetup();
  slave=(Slave **)malloc(sizeof(Slave)*numero_max_slave);
  stato.setStato(ZERO);
  //radio.readAllRegs();

}
// algoritmo 1
void loop() {
  unsigned long ts=0;
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  ElaboraStato();
}

void ProcessaDatiSeriali() {
  static byte comando=0,prossimodato=0,k=0,valore[5];
  int nums;
  if(Serial.available()) {
    char c=Serial.read();
    /*
    Serial.print(F("datiser: char="));
    Serial.print(c,HEX);
    Serial.print(F(" cmd="));
    Serial.print(comando,HEX);
    Serial.print(F(" prox="));
    Serial.print(prossimodato,HEX);
    Serial.print(F(" k="));
    Serial.println(k,HEX);
    */
    if(c==' ') return;
    if(c=='\n') {
      // elabora il comando
      switch(comando) {
        case 'Y':
          // discovery
          if(stato.getStato()==0) {
            stato.setStato(DISCOVERY);
          }
          break;
        case 'X':
          // termina discovery
          if(stato.getStato()==DISCOVERY) {
            stato.setStato(ZERO);
          }
          break;
        case 'Z':
          // inizia voto
          if(stato.getStato()==ZERO) {
            stato.setStato(INVIASYNC);
          }
          break;
        case 'Q':
          // termina voto
          if(stato.getStato()==VOTO) {
            stato.setStato(ZERO);
          }
          // invia discovery a 1
          break;
        case 'S':
          // scrivi indirizzo numero max slave sul byte 1 della eeprom
          nums=atoi(valore);
          if(nums<1 || nums>254) {
            Serial.println(F("e parametro errato"));  
          } else {
            EEPROM.write(1,nums);
            numero_max_slave=nums;
            slave=(Slave **)realloc(slave,sizeof(Slave)*nums);
            Serial.print(F("e numero memorizzato: "));
            Serial.println(nums);
          }
          break;
        case 'N':
          // stampa il num max di client
          Serial.print(F("ns "));
          Serial.println(numero_max_slave);
          break;
      }
      k=0;
      prossimodato=COMANDO;   
      return;  

    }
    if(prossimodato==COMANDO) {
      c=toupper(c);
      comando=c;
      prossimodato=VALORE;
      k=0;
      return;
    }
    if(prossimodato==VALORE) {
      valore[k++]=c;
      valore[k] = 0;
      if(k>3) {
        k=0;
        prossimodato=COMANDO;  
        return;   
      }
    }
    
  }

}

//algoritmo 3
void ElaboraPulsante() {
  static unsigned long inizio_blackout=0, durata_pressione_pulsante=0,inizio_pressione_pulsante=0;
  static bool first=true;
  unsigned long now=millis();
  if(digitalRead(pinPULSANTE)==0) {
    //Serial.print("puls press: durata=");

    if((now-inizio_blackout) > TBACKOUTPULSANTE) {
      durata_pressione_pulsante=(now-inizio_pressione_pulsante);
      if(first) {inizio_pressione_pulsante=now; first=false;} else durata_pressione_pulsante=(now-inizio_pressione_pulsante);
      //Serial.println(durata_pressione_pulsante);
    }
    //Serial.print("puls press: durata=");
  } else {

    if(durata_pressione_pulsante>0) {
      //Serial.println("puls rel");
      if(durata_pressione_pulsante>DURATACLICKLUNGO) PulsanteClickLungo(); else PulsanteClickCorto();
      durata_pressione_pulsante=0;
      first=true;
      inizio_blackout=now;
    }
  }
}

//algoritmo 4
void ElaboraStato() {
  switch(stato.getStato()) {
    case DISCOVERY:
      Discovery();
      break;
    case VOTO:
      Voto();
      break;
    case INVIASYNC:
      if(inviaSync()) stato.setStato(VOTO); else stato.setStato(ZERO);
      break;
  }
}

//algoritmo 5
void Discovery() {
  /*
  Serial.println("Discovery");
  Serial.print("disc:corrente: ");
  Serial.println(indirizzo_slave_discovery);
  Serial.print(" numero_slave: ");
  Serial.println(numero_slave);
  */
  byte livbatt;
  byte rssislave;
  byte rssimaster;
  if(interrogaSlaveDiscovery(indirizzo_slave_discovery,&livbatt,&rssislave,&rssimaster)) {
    slave[numero_slave]=new Slave(indirizzo_slave_discovery);
    slave[numero_slave]->oravoto=0;
    slave[numero_slave]->fallimenti=0;
    //slave[numero_slave]->tensionebatteria=livbatt;
    //slave[numero_slave]->rssi=rssi;
    //slave[numero_slave]->funzionante=true;
    numero_slave++;
    // Aggiorna display
	  //disp.print(1,10,numero_slave);
    // Aggiorna seriale
    Serial.print("d ");
    Serial.print(indirizzo_slave_discovery);
    Serial.print(" ");
    Serial.print(livbatt);
    Serial.print(" ");
    Serial.print(rssislave);
    Serial.print(" ");
    Serial.println(rssimaster);
  } else {
    Serial.print("dx ");
    Serial.println(indirizzo_slave_discovery);
  }
  if(indirizzo_slave_discovery==numero_max_slave) stato.setStato(ZERO);
  indirizzo_slave_discovery++;
  
}
//algoritmo 6
void Voto() {
  if(numero_votati==numero_slave) {
    stato.setStato(ZERO);
  }
  else {
    interrogaTuttiGliSlave();
  }
}

//algoritmo 7
bool inviaSync() {
  t_inizio_voto=micros();
  byte retry;
  for(byte i=0;i<numero_slave;i++) {
    retry=0;
    while(true) {
      if(TrasmettiPacchettoSync(slave[i]->indirizzo,micros()-t_inizio_voto)) break;
      retry++;
      if(retry==3) {
        stato.setStato(0);
        Serial.print(F("e Slave non funzionante: "));
        Serial.println(slave[i]->indirizzo);
	      return false;
      }
    }
  }
  delay(30);
  return true;
}

//algoritmo 8
void PulsanteClickLungo() {
  //Serial.println("PulsanteClickLungo");
  if(stato.getStato()==0) {
    stato.setStato(DISCOVERY);
  } else if(stato.getStato()==DISCOVERY) stato.setStato(ZERO);
}

//algoritmo 9
void PulsanteClickCorto() {
  //Serial.println("PulsanteClickCorto");
  byte s=stato.getStato();
  if(s==DISCOVERY) return;
  if(s==ZERO) {
    stato.setStato(INVIASYNC);
    return;
  }
  if(s==VOTO) {
    stato.setStato(ZERO);
    return;
  }
}

//algoritmo 10
void interrogaTuttiGliSlave() {
  unsigned long oravoto;
  for(byte i=0;i<numero_slave;i++) {
    if(slave[i]->oravoto==0) {
      if(interrogaSlaveVoto(slave[i]->indirizzo,&oravoto)) {
        slave[i]->fallimenti=0;
        if(oravoto>0) {
          slave[i]->oravoto=oravoto;
          numero_votati++;
          Serial.print("v ");
          Serial.print(slave[i]->indirizzo);
          Serial.print(" ");
          Serial.println(oravoto);
          
          // ordina i 5 migliori
          for(int y=0;y<5;y++) {
            if(best[y]!=0) {
              if(oravoto<best[y]->oravoto) {
                for(int f=y;f<4;f++) best[f+1]=best[f];
                break;
              };
            } else {
              best[y]=slave[i];
              break;
            }
          }
          // aggiorna display
          MostraRisultatiVoto();
        }
  
      } else {
        slave[i]->fallimenti++;
        if(slave[i]->fallimenti>3) {
          Serial.print(F("e Pulsante non risponde: "));
          Serial.println(slave[i]->indirizzo);
        }
      }
    }
  }
}

//algoritmo 15
bool interrogaSlaveDiscovery(byte indirizzo, byte *livbatt, byte *rssislave, byte *rssimaster) {
  byte pkt[1];
  pkt[0]='d';
  for(int i=0;i<3;i++) {
    //Serial.print("tento:");
    //Serial.println(indirizzo);
    //radio.send(indirizzo,pkt,1,false);
    if (!radio.send(indirizzo,pkt,1,false)) {
      radioSetup();
      return false;
    }

    unsigned long sentTime = millis();
    while (millis() - sentTime < 20) {
      if(radio.receiveDone()) {
        //stampapkt(radio.DATA,radio.PAYLOADLEN);
        if(radio.DATA[0]=='e') {
          *livbatt=radio.DATA[1];
          *rssislave=radio.DATA[2];
          *rssimaster=radio.RSSI;
          return true;
        }
      }
    }
  }
  return false;
}

bool interrogaSlaveVoto(byte indirizzo, unsigned long* oravoto) {
  byte pkt[1];
  pkt[0]='p';
  if (!radio.send(indirizzo,pkt,1,false)) {
    radioSetup();
    return false;
  }
  unsigned long sentTime = millis();
  while (millis() - sentTime < 10) {
    if(radio.receiveDone()) {
      //stampapkt(radio.DATA,radio.PAYLOADLEN);
      if(radio.DATA[0]=='q') {
        unsigned long t;
        t=radio.DATA[1];
        t=t<<24;
        *oravoto=t;
        t=radio.DATA[2];
        t=t<<16;
        *oravoto+=t;
        t=radio.DATA[3];
        t=t<<8;
        *oravoto+=t;
        *oravoto+= radio.DATA[4];
        return true;
      }
    }
  }
  return false;
}

//algoritmo 18
bool TrasmettiPacchettoSync(byte indirizzo, unsigned long t_da_iniziovoto) {
	char pkt[5];
	pkt[0]='s';
	pkt[1]=t_da_iniziovoto >> 24;
	pkt[2]=(t_da_iniziovoto >> 16) & 0xFF;
	pkt[3]=(t_da_iniziovoto >> 8) & 0xFF;
	pkt[4]=(t_da_iniziovoto) & 0xFF;
	radio.send(indirizzo, pkt, 5);
  unsigned long sentTime = millis();
  while (millis() - sentTime < 20) {
    if(radio.receiveDone()) {
      if(radio.DATA[0]=='k') {
        return true;
      }
    }
  }
  return false;  
}

void radioSetup() {
  // Hard Reset the RFM module 
  Serial.println("radiosetup");
  pinMode(RFM69_RST, OUTPUT); 
  digitalWrite(RFM69_RST, HIGH); 
  delay(100);
  digitalWrite(RFM69_RST, LOW); 
  delay(100);
  radio.initialize(RF69_868MHZ,0,NETWORKID);
  /*
  radio.writeReg(0x03,0x0D); // 9k6
  radio.writeReg(0x04,0x05);
  */
  radio.writeReg(0x03,0x00); // 153k6
  radio.writeReg(0x04,0xD0);
  radio.writeReg(0x37,radio.readReg(0x37) | 0b01010000); // data whitening
  radio.setFrequency(FREQUENCY);
  radio.setHighPower(); 
  radio.setPowerLevel(31);
}




//algoritmo 11
// chiamato ad ogni giro di poll
void MostraRisultatiVoto() {
  /*
  Serial.print("e best: ");
  for(int f=0;f<5;f++) {
    if(best[f]!=0) {
      Serial.print(best[f]->indirizzo);
      Serial.print(":");
      Serial.print(best[f]->oravoto);
      Serial.print(" ");
    }
  }
  Serial.println("");
  */
  
}

void stampapkt(byte *pkt,int len) {
  Serial.print("millis:");
  Serial.print(millis());
  Serial.print("len:");
  Serial.print(len);
  Serial.print("pkt:");
  for (int i=0;i<len;i++) {
    Serial.print(pkt[i],HEX);
    Serial.print(":");
  }
  Serial.println();
    
}

