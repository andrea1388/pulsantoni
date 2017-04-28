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
// stati slave
#define FUORISYNC 1
#define VOTATO 2
#define NONVOTATO 3

// parametri radio
#define NETWORKID 27
#define FREQUENCY 868000000
#define RFM69_CS 53
#define RFM69_IRQ 21
#define RFM69_IRQN 2 
#define RFM69_RST 49
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



#define pinPULSANTE 47
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
    bool funzionante;
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
      tft.setTextSize(2);
      tft.setTextColor(GREEN);  
      tft.println(F("Pronto"));
      break;
    case DISCOVERY:
	  ////disp.print(0,F("Ricerca dispositivi"));
	  //disp.print(1,F("Trovati: "));
      indirizzo_slave_discovery=1;
      numero_slave=0; // cancella la lista
      Serial.println(F("ds"));
      tft.fillScreen(BLACK);
      tft.setCursor(0, 0);
      tft.println(F("Discovering"));
      break;
    case INVIASYNC:
	  //disp.print(0,F("Invio sincronismo"));
      if(numero_slave==0) {
        Serial.println(F("slave=0"));
        tft.setCursor(0, 0);
        tft.fillScreen(BLACK);
        tft.println(F("Effettuare discovery"));
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
      tft.fillScreen(BLACK);
      tft.setCursor(0, 0);
      tft.println(F("Voto in corso"));
      break;
    default:
      stato=ZERO;
      Serial.println(F("e setstato errato"));
      return;
  }
  stato=newstato;
}

Stato stato;
RFM69 radio(RFM69_CS,RFM69_IRQ,true,RFM69_IRQN);
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
  tft.println();
  tft.print("numero slave: ");
  tft.println(numero_max_slave);

  delay(20);
  radioSetup();
  slave=(Slave **)malloc(sizeof(Slave)*numero_max_slave);
  //radio.readAllRegs();
  tft.print(F("Frequenza: "));
  tft.println(radio.getFrequency());
  tft.println();
  tft.println(F("Click lungo: Discovery"));
  tft.println(F("Click corto: Voto"));
  tft.println(F("Effettuare il Discovery"));
  tft.println();
  stato.setStato(ZERO);


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
            tft.println(F("Voto concluso"));
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
  static byte st[10],k=0;
  unsigned long now=millis();
  if(digitalRead(pinPULSANTE)==0) st[k++]=1; else st[k++]=0;
  if(k==10) k=0;
  int s=0;
  for(int w=0;w<10;w++) s+=st[w];
  
  if(s>8) {
    //Serial.print("puls: durata, inizio, now: ");
    if((now-inizio_blackout) > TBACKOUTPULSANTE) {
      if(first==true) {
        durata_pressione_pulsante=0;
        inizio_pressione_pulsante=now; 
        first=false;
      } else {
        durata_pressione_pulsante=now-inizio_pressione_pulsante;
      }
      /*
      Serial.print(durata_pressione_pulsante);
      Serial.print(" ");
      Serial.print(inizio_pressione_pulsante);
      Serial.print(" ");
      Serial.println(now);
      */
    }
  } else {
    if(durata_pressione_pulsante>0) {
      //Serial.println("puls rel");
      if(durata_pressione_pulsante>DURATACLICKLUNGO) PulsanteClickLungo(); else PulsanteClickCorto();
      first=true;
      inizio_blackout=now;
    }
    durata_pressione_pulsante=0;
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
    tft.setTextColor(GREEN);
    tft.print(indirizzo_slave_discovery);
    tft.print(" ");
  } else {
    Serial.print("dx ");
    Serial.println(indirizzo_slave_discovery);
    tft.setTextColor(RED);
    tft.print(indirizzo_slave_discovery);
    tft.print(" ");
  }
  if(indirizzo_slave_discovery==numero_max_slave) {
    tft.setTextColor(GREEN);
    tft.println();
    tft.print(F("Slave trovati: "));
    tft.print(numero_slave);
    tft.print(F(" su "));
    tft.println(numero_max_slave);
    stato.setStato(ZERO);
  }
  indirizzo_slave_discovery++;
  
}
//algoritmo 6
void Voto() {
  if(numero_votati==numero_slave) {
    tft.println(F("Voto concluso"));
    stato.setStato(ZERO);
  }
  else {
    interrogaTuttiGliSlave();
  }
}

//algoritmo 7
bool inviaSync() {
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.println(F("Invio sync"));
  t_inizio_voto=micros();
  byte retry;
  for(byte i=0;i<numero_slave;i++) {
    retry=0;
    while(true) {
      if(TrasmettiPacchettoSync(slave[i]->indirizzo,micros()-t_inizio_voto)) break;
      retry++;
      if(retry==10) {
        stato.setStato(0);
        Serial.print(F("e Slave non funzionante: "));
        Serial.println(slave[i]->indirizzo);
        tft.print(F("Slave non funzionante: "));
        tft.println(slave[i]->indirizzo);
        tft.println(F("Controllare o ripetere discovery"));
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
    tft.println(F("Voto concluso"));
    stato.setStato(ZERO);
    return;
  }
}

//algoritmo 10
void interrogaTuttiGliSlave() {
  unsigned long oravoto;
  byte statoslave;
  for(byte i=0;i<numero_slave;i++) {
    if(slave[i]->oravoto==0) {
      ElaboraPulsante();
      if(interrogaSlaveVoto(slave[i]->indirizzo,&oravoto,&statoslave)) {
        slave[i]->fallimenti=0;
        if(!slave[i]->funzionante) {
          slave[i]->funzionante=true;
          AggiornaDisplayKo();
        }
        if(statoslave==FUORISYNC) {
          Serial.print(F("e slave fuori sync:"));
          Serial.println(slave[i]->indirizzo);
          tft.print(F("Slave fuori sincronismo: "));
          tft.println(slave[i]->indirizzo);
          stato.setStato(ZERO);
          return;
        }
        if(statoslave==VOTATO) {
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
                for(int f=y;f<4;f++) {
                  if(best[f]!=0) best[f+1]=best[f];
                }
                //best[y]=slave[i];
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
        if(slave[i]->fallimenti>10) {
          slave[i]->fallimenti=11;
          if(slave[i]->funzionante) {
            slave[i]->funzionante=false;
            Serial.print(F("e Pulsante non risponde: "));
            Serial.println(slave[i]->indirizzo);
            AggiornaDisplayKo();
          }
        }
      }
    }
  }
}
void AggiornaDisplayKo() {
  int x=tft.getCursorX();
  int y=tft.getCursorY();
                Serial.println("h");

  tft.fillRect(0,200,320,40,BLACK);
  tft.setTextColor(RED);
  tft.setCursor(0, 205);
  for (int i=0;i<numero_slave;i++) {
    if(!slave[i]->funzionante) {
      tft.print(slave[i]->indirizzo);
      Serial.println("l");
      tft.print(" ");
    }
  }
  tft.setTextColor(GREEN);
  tft.setCursor(x, y);  
}

//algoritmo 15
bool interrogaSlaveDiscovery(byte indirizzo, byte *livbatt, byte *rssislave, byte *rssimaster) {
  byte pkt[1];
  pkt[0]='d';
  for(int i=0;i<10;i++) {
    //Serial.print("tento:");
    //Serial.println(indirizzo);
    //radio.send(indirizzo,pkt,1,false);
    if (!radio.send(indirizzo,pkt,1,false)) {
      radioSetup();
      Serial.println(F("e parametro errato")); 
      tft.println(F("radio send failed"));
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

bool interrogaSlaveVoto(byte indirizzo, unsigned long* oravoto, byte * statoslave) {
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
        *statoslave=VOTATO;
        return true;
      }
      // se lo slave per qualche motivo ha perso il sync risponde r
      // è da considerare non funzionante
      // caso raro
      if(radio.DATA[0]=='r') {
        *oravoto=0;
        *statoslave=FUORISYNC;
        return true;
      }
      // se lo slave per qualche motivo ha perso il sync risponde r
      if(radio.DATA[0]=='t') {
        *oravoto=0;
        *statoslave=NONVOTATO;
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
  radio.writeReg(0x37,radio.readReg(0x37) | 0b01010010); // data whitening e address filter
  radio.setFrequency(FREQUENCY);
  radio.setHighPower(); 
  radio.setPowerLevel(31);
}




//algoritmo 11
// chiamato ad ogni giro di poll
void MostraRisultatiVoto() {
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.setTextColor(GREEN);
  tft.println(F("Risultati voto:"));
  tft.println();
  for(int f=0;f<5;f++) {
    if(best[f]!=0) {
    
      if(f==0) {
          tft.setTextColor(RED);  
      }
      else {
          tft.setTextColor(CYAN);  
      }
      int ovi=best[f]->oravoto/1000000;
      int ovd=(best[f]->oravoto-ovi*1000000)/1000;
      char str[25];
      sprintf(str, " %3d %3d,%03d",best[f]->indirizzo,ovi,ovd);
      /*
      tft.print(f+1);
      tft.print(": ");
      tft.print(best[f]->indirizzo);
      tft.print(": ");
      tft.print(ovi);
      tft.print(",");
      */
      tft.println(str);
    }
  }
  tft.setTextSize(2);
  tft.setTextColor(GREEN);
  tft.println();
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

