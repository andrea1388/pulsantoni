#include <RFM69.h>
//#include <LiquidCrystal.h>
#define DURATACLICKLUNGO 2000000 // tempo pressione pulsante per click lungo = 2 secondi
#define TBACKOUTPULSANTE 10000 // tempo blackout pulsante dopo un click
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


#define pinPULSANTE 11
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
unsigned long best[5];

class Slave {
  public:
    unsigned long oravoto;
    //byte tensionebatteria;
    //byte rssi;
    //bool funzionante;
    byte indirizzo;
    Slave(byte);
};
Slave::Slave(byte ind) {
  indirizzo=ind;
}

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
  switch(newstato) {
    case ZERO:
	  //disp.print(0,F("Pronto"));
      numero_votati=0;  
      indirizzo_slave_discovery=0;
      // azzera i 5 migliori
      for(int f=0;f<5;f++) best[f]=0xffffffff;
      Serial.println(F("s0"));
    case DISCOVERY:
	  ////disp.print(0,F("Ricerca dispositivi"));
	  //disp.print(1,F("Trovati: "));
      Serial.println(F("ds"));
    case INVIASYNC:
	  //disp.print(0,F("Invio sincronismo"));
      Serial.println(F("is"));
    case VOTO:
	  //disp.print(0,F("Pronto per gara"));
      Serial.println(F("ip"));
    default:
      Serial.println(F("e setstato indefinito"));
      return;
  }
  stato=newstato;
}

Stato stato;
Slave* slave[255];
byte numero_slave;
RFM69 radio;
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


void setup() {
  pinMode(pinPULSANTE, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println("Setup");
  //lcd.begin(16, 2);
  radioSetup(0);
  stato.setStato(ZERO);
}
// algoritmo 1
void loop() {
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  ElaboraStato();
}

// algoritmo 2
void ProcessaDatiSeriali() {
  
}

//algoritmo 3
void ElaboraPulsante() {
  static unsigned long inizio_blackout=0, durata_pressione_pulsante=0,inizio_pressione_pulsante=0;
  unsigned long now=millis();
  if(digitalRead(pinPULSANTE)) {
    if((now-inizio_blackout) > TBACKOUTPULSANTE) 
      if(durata_pressione_pulsante==0) inizio_pressione_pulsante=now; else durata_pressione_pulsante=(now-inizio_pressione_pulsante);
  } else {
    if(durata_pressione_pulsante>0) {
      if(durata_pressione_pulsante>DURATACLICKLUNGO) PulsanteClickLungo(); else PulsanteClickCorto();
      durata_pressione_pulsante=0;
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
    case INVIASYNC:
      if(inviaSync()) stato.setStato(VOTO); else stato.setStato(ZERO);
      break;
  }
}

//algoritmo 5
void Discovery() {
  static byte indirizzo_slave_corrente=0;
  if(indirizzo_slave_corrente==0) numero_slave=0; // cancella la lista
  byte livbatt;
  short rssi;
  if(interrogaSlaveDiscovery(indirizzo_slave_corrente,&livbatt,&rssi)) {
    slave[numero_slave]=new Slave(indirizzo_slave_corrente);
    slave[numero_slave]->oravoto=0;
    //slave[numero_slave]->tensionebatteria=livbatt;
    //slave[numero_slave]->rssi=rssi;
    //slave[numero_slave]->funzionante=true;
    numero_slave++;
    // Aggiorna display
	  //disp.print(1,10,numero_slave);
    // Aggiorna seriale
    Serial.print("d ");
    Serial.print(indirizzo_slave_corrente);
    Serial.print(" ");
    Serial.print(livbatt);
    Serial.print(" ");
    Serial.println(rssi);
  }
  if(indirizzo_slave_corrente=255) stato.setStato(ZERO);
  indirizzo_slave_corrente++;
}
//algoritmo 6
void Voto() {
  if(numero_votati==numero_slave) {
    stato.setStato(ZERO);
  }
  else {
    interrogaTuttiGliSlave();
    if(numero_votati>0) {
      MostraRisultatiVoto();
    }
    
   }
}

//algoritmo 7
bool inviaSync() {
  unsigned long t_inizio_voto=micros();
  byte retry;
  for(byte i=0;i<numero_slave;i++) {
    retry=0;
    while(true) {
      if(TrasmettiPacchettoSync(slave[i]->indirizzo,micros()-t_inizio_voto)) break;
      retry++;
      if(retry==3) {
        stato.setStato(0);
        Serial.print(F("e Slave non funzionante"));
        Serial.println(slave[i]->indirizzo);
	      return false;
      }
    }
  }
  stato.setStato(VOTO);
  return true;
  
}

//algoritmo 8
void PulsanteClickLungo() {
  if(stato.getStato()==0) {
    stato.setStato(DISCOVERY);
    indirizzo_slave_discovery=1;    
  } else if(stato.getStato()==DISCOVERY) stato.setStato(ZERO);
}

//algoritmo 9
void PulsanteClickCorto() {
  byte s=stato.getStato();
  if(s==DISCOVERY) return;
  if(s==ZERO) {
    stato.setStato(INVIASYNC);
  }
  if(s==VOTO) {
    stato.setStato(ZERO);
  }
}

//algoritmo 10
void interrogaTuttiGliSlave() {
  byte risposta[10];
  unsigned long oravoto;
  for(byte i=0;i<numero_slave;i++) {
    if(slave[i]->oravoto==0) {
      if(interrogaSlaveVoto(slave[i]->indirizzo,&oravoto)) {
        if(oravoto>0) {
          slave[i]->oravoto=oravoto;
          //slave[i]->funzionante=true;
          numero_votati++;
          Serial.print("v ");
          Serial.print(slave[i]->indirizzo);
          Serial.print(" ");
          Serial.println(oravoto);
          
          // ordina i 5 migliori
          for(int y=0;y<5;y++) {
            if(oravoto<best[y]) {
              for(int f=y;f<4;f++) best[f+1]=best[f];
              best[y]=oravoto;
            }
          }
          // aggiorna display
        }
  
      } else {
        //slave[i]->funzionante=false;
        Serial.print(F("e Pulsante non risponde: "));
        Serial.println(i);
      }
    }
  }
}

//algoritmo 15
bool interrogaSlaveDiscovery(byte indirizzo, byte *livbatt, short *rssi) {
  byte pkt[1];
  pkt[0]='d';
  radio.send(indirizzo,pkt,1,false);
  unsigned long sentTime = millis();
  while (millis() - sentTime < 20) {
    if(radio.receiveDone()) {
      if(radio.DATA[0]=='e') {
        livbatt=radio.DATA[1];
        rssi=radio.DATA[2] << 8 + radio.DATA[3];
        return true;
      }
    }
  }
  return false;
}

bool interrogaSlaveVoto(byte indirizzo, unsigned long* oravoto) {
  byte pkt[1];
  pkt[0]='p';
  radio.send(indirizzo,pkt,1,false);
  unsigned long sentTime = millis();
  while (millis() - sentTime < 20) {
    if(radio.receiveDone()) {
      if(radio.DATA[0]=='q') {
        oravoto=radio.DATA[1] << 24 + radio.DATA[2] << 16 + radio.DATA[3] << 8 + radio.DATA[4];
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

void radioSetup(byte indirizzo) {
  // Hard Reset the RFM module 
  pinMode(RFM69_RST, OUTPUT); 
  digitalWrite(RFM69_RST, HIGH); 
  delay(100);
  digitalWrite(RFM69_RST, LOW); 
  delay(100);
  radio.initialize(RF69_868MHZ,indirizzo,NETWORKID);
  radio.writeReg(0x03,0x0D); // 9k6
  radio.writeReg(0x04,0x05);
  /*
  radio.writeReg(0x03,0x00); // 153k6
  radio.writeReg(0x04,0xD0);
  */
  radio.setFrequency(FREQUENCY);
  radio.setHighPower(); 
  radio.setPowerLevel(31);
}




//algoritmo 11
// chiamato ad ogni giro di poll
void MostraRisultatiVoto() {
  
}


