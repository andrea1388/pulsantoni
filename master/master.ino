/*
 * Master
 * Sistema polling con slave repeaters
 * versione iniziale master.psr.1
 */
 #include <RFM69.h>
#include <EEPROM.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library

#define INDICEMITTENTEINIZIALE 0
#define INDICEDESTINATARIOFINALE 1
#define INDICEH1 2
#define INDICEH2 3
#define INDICEH3 4
#define INDICET3 5
#define INDICET2 6
#define INDICET1 7
#define INDICET0 8
#define INDICEINIZIODATI 9

#define TEMPOTRATENTATIVI 20 // in ms
#define TIMEOUTTRANSAZIONE 1000

#define MAXBESTNEIGHBOURS 5


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


#define LEDVERDE 41
#define LEDROSSO 43
#define LEDBLU   45
#define pinPULSANTE 47
#define MASTER 0
/*
class Display {
	public:
		Display(rs, enable, d4, d5, d6, d7);
		Print(char *);
		
}
Display::Print(char* s) {
	
}
*/
class Messaggio {
  public:
    unsigned long tstart,tultimotentativo;
    byte destinatario,mittente;
    byte lunghezza;
    byte dati[19];
    byte tentativo;
    bool txincorso;
    bool isack;
    PrendiDaRadio(RFM69*);
    PrendiDaPkt(Messaggio);
    //Messaggio();
    unsigned long GetTime();
    void SetTime(unsigned long);
    void print();
    
    
};

class Nodo {
  public:
    byte indirizzo;
    int16_t segnale;
    Nodo();
};

byte numero_votati,indirizzo_slave_corrente;
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
      indirizzo_slave_corrente=0;
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
      indirizzo_slave_corrente=1;
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
unsigned long t_inizio_voto, inizio_tx_slave_corrente;
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);





Messaggio m;
Nodo* bestn[MAXBESTNEIGHBOURS];
const byte indirizzo=0;

void setup() {
  pinMode(pinPULSANTE, INPUT_PULLUP);
  pinMode(LEDVERDE, OUTPUT);
  pinMode(LEDROSSO, OUTPUT);
  pinMode(LEDBLU, OUTPUT);
  digitalWrite(LEDVERDE, LOW);
  digitalWrite(LEDROSSO, HIGH);
  digitalWrite(LEDBLU, LOW);
  
  Serial.begin(250000);
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
  for (int i=0;i<MAXBESTNEIGHBOURS;i++) bestn[i]=new Nodo();
  stato.setStato(ZERO);


}
// algoritmo 1
void loop() {
  unsigned long ts=0;
  if(Serial.available()) ProcessaDatiSeriali();
      digitalWrite(LEDVERDE, LOW);
      digitalWrite(LEDROSSO, HIGH);
      digitalWrite(LEDBLU, LOW);
  ElaboraPulsante();
  ElaboraStato();
  if(m.txincorso) tx2();  
  if(radio.receiveDone()) ElaboraDatiRadio();
  if (stato.getStato()==DISCOVERY && ((millis()-inizio_tx_slave_corrente)>TIMEOUTTRANSAZIONE)) {indirizzo_slave_corrente++; m.txincorso=false;}

}



void ElaboraDatiRadio() {
  Messaggio rxm;
  //=new Messaggio();
  rxm.PrendiDaRadio(&radio);
  Serial.print(F("dati radio: "));
  rxm.print();
  
  /*
  Serial.print(radio.SENDERID);
  Serial.print(" ");
  Serial.print(radio.TARGETID);
  Serial.print(" ");
  Serial.print(radio.DATA[INDICEMITTENTEINIZIALE]);
  Serial.print(" ");
  Serial.print(radio.DATA[INDICEDESTINATARIOFINALE]);
  Serial.print(" ");
  Serial.print(radio.DATA[INDICEH1]);
  Serial.print(" ");
  Serial.print(radio.DATA[INDICEH2]);
  Serial.print(" ");
  Serial.print(radio.DATA[INDICEH3]);
  Serial.print(" ");
  Serial.println(radio.RSSI);
  */
  CostruisciListaNodi();

  // se non è roba per me buttalo
  if(radio.TARGETID!=indirizzo) return;
  
  // ack indirizzato a me
  if(m.txincorso && radio.ACK_RECEIVED) {
    m.txincorso=false;
    Serial.println("tx completed");
    return;
  }
  // mio pacchetto non ack
  if(radio.DATA[INDICEDESTINATARIOFINALE]==indirizzo) {
    if(radio.ACKRequested()) radio.sendACK(0,0); 
    //if(indirizzo_slave_corrente!=rxm.dati[INDICEMITTENTEINIZIALE]) {Serial.println("msg fuori tempo");};
    switch(rxm.dati[INDICEINIZIODATI]) {
      case 'e':
        // msg risposta a discovery
        for (int d=0;d<numero_slave;d++) if(slave[d]==rxm.dati[INDICEMITTENTEINIZIALE]) {Serial.println("già presente"); return;};
        slave[numero_slave]=new Slave(rxm.dati[INDICEMITTENTEINIZIALE]);
        slave[numero_slave]->oravoto=0;
        slave[numero_slave]->fallimenti=0;
        Serial.print("d ");
        Serial.print(slave[numero_slave]->indirizzo);
        Serial.print(" ");
        Serial.print(rxm.dati[INDICEINIZIODATI+1]);
        Serial.print(" ");
        Serial.print((int)rxm.dati[INDICEINIZIODATI+2]);
        tft.setTextColor(GREEN);
        tft.print(slave[numero_slave]->indirizzo);
        tft.print(" ");
        numero_slave++;
        indirizzo_slave_corrente++;
        break;
      case 'p':
          break;
      case 'd':
          break;
    }
    return;
  }
    


  // pkt indirizzato a me ma con destinatario finale diverso = da ritrasmettere
  if (radio.DATA[INDICEDESTINATARIOFINALE]!=indirizzo) {
    // è un pacchetto da ritrasmettere
    // se ho già un okt in corso di trasmissione non do l'ack e lo butto
    if(!m.txincorso) {
      if(radio.ACKRequested()) radio.sendACK(0,0);
      Serial.println(F("pkt da ritrasm"));
      m.PrendiDaPkt(rxm);
      m.txincorso=true;
      m.mittente=indirizzo;
      for(byte h=0;h<3;h++) if(m.dati[INDICEH1+h]==255) {m.dati[INDICEH1+h]=indirizzo; break;}
      m.print();

    }
  }
}

void CostruisciListaNodi() {
  if(radio.SENDERID!=MASTER) {
    // se il nodo ricevuto è più forte del più debole lo sostituisco con questo
    bool giainlista=false;
    for (int i=0;i<MAXBESTNEIGHBOURS;i++) if(bestn[i]->indirizzo==radio.SENDERID) {bestn[i]->segnale=radio.RSSI; giainlista=true;}
    if(!giainlista) {
      int minimo=bestn[0]->segnale;
      byte indicemin=0;
      for (int i=0;i<MAXBESTNEIGHBOURS;i++) if(bestn[i]->segnale<minimo) {minimo=bestn[i]->segnale; indicemin=i;};
      if(radio.RSSI>minimo) {bestn[indicemin]->segnale=radio.RSSI; bestn[indicemin]->indirizzo=radio.SENDERID;}
    }
      
    Serial.print(F("bestn: "));
    for (int i=0;i<MAXBESTNEIGHBOURS;i++) {
      Serial.print(bestn[i]->indirizzo);
      Serial.print(" ");
      Serial.println(bestn[i]->segnale);
      
    }
  }
  
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
          nums=atoi((const char *)valore);
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
      digitalWrite(LEDVERDE, LOW);
      digitalWrite(LEDROSSO, LOW);
      digitalWrite(LEDBLU, HIGH);
      Discovery();
      break;
    case VOTO:
      digitalWrite(LEDVERDE, HIGH);
      digitalWrite(LEDROSSO, LOW);
      digitalWrite(LEDBLU, LOW);      
      Voto();
      break;
    case INVIASYNC:
      if(inviaSync()) stato.setStato(VOTO); else stato.setStato(ZERO);
      break;
  }
}

//algoritmo 5
void Discovery() {
  static byte last=0;
  if(m.txincorso) return;
  if(indirizzo_slave_corrente==last) return;
  last=indirizzo_slave_corrente;
  if(indirizzo_slave_corrente==numero_max_slave) {
    tft.setTextColor(GREEN);
    tft.println();
    tft.print(F("Slave trovati: "));
    tft.print(numero_slave);
    tft.print(F(" su "));
    tft.println(numero_max_slave);
    stato.setStato(ZERO);     
    return;  
  }
  byte pkt[1];
  pkt[0]='d';
  inizio_tx_slave_corrente=millis();
  tx(indirizzo_slave_corrente,pkt,1);
}
//algoritmo 6
void Voto() {
  if(numero_votati==numero_slave) {
    tft.println(F("Voto concluso"));
      digitalWrite(LEDVERDE, LOW);
      digitalWrite(LEDROSSO, HIGH);
      digitalWrite(LEDBLU, LOW);
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
          /*
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
          */
          for(int y=0;y<5;y++) {
            if(best[y]==0) {
              best[y]=slave[i];
              break;
            }
            else {
              if(oravoto<best[y]->oravoto) {
                for(int f=y;f<4;f++) {
                  if(best[f]!=0)
                     best[f+1]=best[f];
                     break;
                }
                best[y]=slave[i];
                break;
              }
            
            }
          }
          // aggiorna display
          MostraRisultatiVoto();
        }
  
      } else {
        slave[i]->fallimenti++;
        if(slave[i]->fallimenti>100) {
          if(slave[i]->fallimenti>250) slave[i]->fallimenti=250;
          /*
          Serial.print(slave[i]->indirizzo);
          Serial.print(" - ");
          Serial.println(slave[i]->fallimenti);
          */
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
  tft.fillRect(0,200,320,40,BLACK);
  tft.setTextColor(RED);
  tft.setCursor(0, 205);
  for (int i=0;i<numero_slave;i++) {
    if(!slave[i]->funzionante) {
      tft.print(slave[i]->indirizzo);
      tft.print(" ");
    }
  }
  tft.setTextColor(GREEN);
  tft.setCursor(x, y);  
}


bool interrogaSlaveVoto(byte indirizzo, unsigned long* oravoto, byte * statoslave) {
  byte pkt[1];
  pkt[0]='p';
  if (!radio.send(indirizzo,pkt,1,false)) {
    radioSetup();
    return false;
  }
  Serial.print(indirizzo);
  Serial.print(" : ");
  unsigned long sentTime = millis();
  while (millis() - sentTime < 10) {
    if(radio.receiveDone()) {


        if(indirizzo==10) {
          Serial.println(radio.RSSI);
        }


      
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
      if(radio.DATA[0]=='t') {
        *oravoto=0;
        *statoslave=NONVOTATO;
        return true;
      }
    }
  }
          if(indirizzo==10) {
          Serial.println("ko");
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
	if (tx(indirizzo, pkt, 5)) ;
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

// motore rtx
bool tx(byte destinatario, char *dati, byte lunghezza) {
  if(m.txincorso) {
    Serial.println(F("tx busy"));
    return false;
  }
  Serial.print(F("tx: dest: "));
  Serial.println(destinatario);
  m.mittente=indirizzo;
  m.dati[INDICEDESTINATARIOFINALE]=destinatario;
  m.dati[INDICEMITTENTEINIZIALE]=indirizzo;
  m.dati[INDICEH1]=255;
  m.dati[INDICEH2]=255;
  m.dati[INDICEH3]=255;
  m.txincorso=true;
  m.tstart=micros();
  m.SetTime(0);
  m.tentativo=0;
  m.lunghezza=lunghezza;
  for(byte i=0;i<lunghezza;i++) m.dati[INDICEINIZIODATI+i]=dati[i];
  return true;
}


// richiamata continuamente finché c'è una trasmissione in corso
void tx2() {
  // dal secondo tentativo aspetta il timeout
  if( (m.tentativo>0) && ((millis() - m.tultimotentativo) < TEMPOTRATENTATIVI)) return;
  if(m.tentativo++==10) {
    m.txincorso=false; 
    if (stato.getStato()==DISCOVERY) indirizzo_slave_corrente++;
    Serial.println(F("trasmissione fallita")); 
    return;
  }  
  if (m.tentativo<3) m.destinatario=m.dati[INDICEDESTINATARIOFINALE]; 
  else m.destinatario=bestn[(m.tentativo-3) % MAXBESTNEIGHBOURS]->indirizzo;
  if(m.destinatario==m.dati[INDICEMITTENTEINIZIALE]) return;
  if(m.destinatario==m.dati[INDICEH1]) return;
  if(m.destinatario==m.dati[INDICEH2]) return;
  if(m.destinatario==m.dati[INDICEH3]) return;
  // se i 3 salti sono completi è possibile mandare solo al destinatario finale
  if(m.destinatario!=m.dati[INDICEDESTINATARIOFINALE] && m.dati[INDICEH3]!=255) return;
  // nodo fittizzio
  if(m.destinatario==255) return;
  Serial.print(F("tx2: tentativo/dest "));
  Serial.print(m.tentativo);
  Serial.print(" ");
  Serial.println(m.destinatario);
  txf();
}

// effettua la trasmissione del frame se il canale è libero
void txf() {
  Serial.print(F("txf: dest/len/dt "));
  unsigned long dt=(micros()-m.tstart);
  m.SetTime(m.GetTime()+dt);
  m.tultimotentativo=millis();
  m.print();

  radio.send(m.destinatario, m.dati, m.lunghezza+INDICEINIZIODATI, true);
}


void Messaggio::print() {
  if(isack) {
     Serial.print(F("msgACK: mitt/dest "));
     Serial.print(mittente);
     Serial.print("/");
     Serial.print(destinatario);
     return;   
  }
  Serial.print(F("msg: mitt/dest/orig/finale/h1/h2/h3/t/len/dati "));
  Serial.print(mittente);
  Serial.print("/");
  Serial.print(destinatario);
  Serial.print("/");
  Serial.print(dati[INDICEMITTENTEINIZIALE]);
  Serial.print("/");
  Serial.print(dati[INDICEDESTINATARIOFINALE]);
  Serial.print("/");
  for(byte k=0;k<3;k++) {Serial.print(dati[INDICEH1+k]); Serial.print("/");}
  Serial.print(GetTime());
  Serial.print("/");
  Serial.print(lunghezza);
  Serial.print("/");
  for(byte k=0;k<lunghezza;k++) {Serial.print(dati[INDICEINIZIODATI+k],HEX);}
  Serial.println("");
}
unsigned long Messaggio::GetTime() {
  unsigned long tt,tmp;
  tmp=dati[INDICET3];
  tmp=tmp<<24;
  tt=tmp;
  tmp=dati[INDICET2];
  tmp=tmp<<16;
  tt+=tmp;
  tmp=dati[INDICET1];
  tmp=tmp<<8;
  tt+=tmp;
  tmp=dati[INDICET0];
  tt+=tmp;
  return tt;
}

void Messaggio::SetTime(unsigned long dt) {
  dati[INDICET3]=dt >> 24;
  dati[INDICET2]=(dt >> 16) & 0xFF;
  dati[INDICET1]=(dt >> 8) & 0xFF;
  dati[INDICET0]=(dt) & 0xFF;
}

Messaggio::PrendiDaRadio(RFM69 *r) {
  destinatario=r->TARGETID;
  mittente=r->SENDERID;
  isack=r->ACK_RECEIVED;
  for(byte k=0;k<r->DATALEN;k++) dati[k]=r->DATA[k];
  txincorso=true;
  tstart=micros();
  tentativo=0;
  lunghezza=r->DATALEN-INDICEINIZIODATI;
}
Messaggio::PrendiDaPkt(Messaggio m) {
  isack=m.isack;
  destinatario=m.destinatario;
  mittente=m.mittente;
  for(byte k=0;k<m.lunghezza+INDICEINIZIODATI;k++) dati[k]=m.dati[k];
  txincorso=true;
  tstart=micros();
  tentativo=0;
  lunghezza=m.lunghezza;
}

Nodo::Nodo() { indirizzo=255; segnale=-200;}

