/*
 * ver 3
 * led acceso prima possibile
 * test send fail su discovery, poll e inviosync
 * ver 4
 * introdotto TLastPolll e sincronizzato
 * risponde r al poll se ha perso il sync e il master lo segnalerà come non funzionante
 */
#include <RFM69.h>
#include <EEPROM.h>

#define pinPULSANTE 3
#define PINBATTERIA 0 // per lettura tensione batteria 
#define TIMEOUTVOTO 10000000 // 10 sec
// parametri radio
#define NETWORKID 27
#define FREQUENCY 868000000
#define RFM69_CS 10
#define RFM69_IRQ 2
#define RFM69_IRQN 0 
#define RFM69_RST 9

#define MASTER 0

#define LEDPIN 4
// stati per elaborazione seriale
#define COMANDO 0
#define VALORE 1
// stati slave
#define ZERO 0
#define SINCRONIZZATO 1
#define VOTATO 2




unsigned long TdaInizioVoto,TrxSync,Tvoto, Tledoff, Tledon,TLastPoll;
bool pulsantegiapremuto;
int stato; // true dopo la sincronizzazione, false all'inizio, dopo il discovery e dopo il timeoutvoto
byte indirizzo;
RFM69 radio=RFM69(RFM69_CS, RFM69_IRQ, true, RFM69_IRQN);

void setup() {
  // accende subito il led
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  // pin pulsante col pullup
  pinMode(pinPULSANTE, INPUT_PULLUP);
  // legge indirizzo slave dal byte 0 della eeprom
  indirizzo = EEPROM.read(0);
  // info su seriale
  Serial.begin(250000);
  Serial.println(F("Slave - Firmware: 5"));
  Serial.print(F("Indirizzo: "));
  Serial.println(indirizzo);
  // imposta radio
  radioSetup();
  // stampa frequenza
  Serial.print(F("Frequenza: "));
  Serial.println(radio.getFrequency());
  //radio.readAllRegs();
  // imposta lampeggio led
  impostaled(100,2900);
  stato=ZERO;
  Serial.println(F("STATO0")); 
  TrxSync=0;
}

// algoritmo 1
void loop() {
  static unsigned long int tckradio=0;
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  ElaboraRadio();
  // dopo TIMEOUTVOTO reimposta un lampeggio lento per risparmiare batteria
  if((micros()-TLastPoll)>TIMEOUTVOTO) {
    if (stato!=ZERO) {
      stato=ZERO;
      impostaled(100,2900);
      Serial.println(F("STATO0")); 
    }
  }
  LampeggioLED();
  if((millis()-tckradio)>1000) {
      tckradio=millis();
      if(radio.readReg(0x01)==0) radioSetup();
  }
}

// algoritmo 2
void ElaboraRadio() {
  if(!radio.receiveDone()) return;
  switch(radio.DATA[0]) {
    case 's':
        ElaboraCmdInvioSync((byte *)radio.DATA); // cmd s
        break;
    case 'p':
        ElaboraPoll(); // cmd p
        break;
    case 'd':
        ElaboraCmdDiscovery(); // cmd d
        break;
  }
}

// algoritmo 3
void ElaboraPulsante() {
  if(stato==SINCRONIZZATO) {
    if(digitalRead(pinPULSANTE)==LOW) {
      stato=VOTATO;
      Tvoto=micros()-TrxSync+TdaInizioVoto;
      impostaled(1,1);
      Serial.print(F("VOTATO: tvoto="));      
      Serial.println(Tvoto,DEC);
    }
  }
}

// algoritmo 4
void ElaboraCmdInvioSync(byte * pkt) {
  unsigned long t;
  TrxSync=micros();
  TLastPoll=TrxSync;
  //stampapkt(pkt, 5);
  // estrae l'informazione dal pacchetto
  t=radio.DATA[1];
  t=t<<24;
  TdaInizioVoto=t;
  t=radio.DATA[2];
  t=t<<16;
  TdaInizioVoto+=t;
  t=radio.DATA[3];
  t=t<<8;
  TdaInizioVoto+=t;
  TdaInizioVoto+=radio.DATA[4];
  if(!radio.send(MASTER, "k", 1,false)) {
    radioSetup();
  }
  pulsantegiapremuto=false;
  impostaled(500,500); //1 Hz Dc=50%
  Tvoto=0;
  stato=SINCRONIZZATO;
  Serial.print(F("SINCRONIZZATO trxsync="));
  Serial.print(TrxSync);
  Serial.print(" tdainiziov=");
  Serial.println(TdaInizioVoto);
}

// algoritmo 5
void ElaboraCmdDiscovery() {
  byte pkt[4];
  pkt[0]='e';
  pkt[1]=(analogRead(PINBATTERIA)>>2);
  pkt[2]=radio.RSSI;
  if(!radio.send(MASTER, pkt, 3,false)) {
    radioSetup();
    return;
  }
  impostaled(30,70);
  Serial.print("ElaboraCmdDiscovery: VBatt=");
  Serial.print(pkt[1]);
  Serial.print(" RSSI=");
  Serial.println(pkt[2]);
  //stampapkt(pkt, 3);
    
}  
 

void ElaboraPoll() {
  byte pkt[5];
  byte pl;
  switch (stato)
  {
    case VOTATO:
      pkt[0]='q';
      pkt[1]=Tvoto >> 24;
      pkt[2]=(Tvoto >> 16) & 0xFF;
      pkt[3]=(Tvoto >> 8) & 0xFF;
      pkt[4]=(Tvoto) & 0xFF;
      pl=5;
      break;
    case ZERO:    
      pkt[0]='r';
      pl=1;
      break;
    case SINCRONIZZATO:    
      pkt[0]='t';
      pl=1;
      break;
  }
  if(!radio.send(MASTER, pkt, pl,false)) radioSetup();
  TLastPoll=micros();
  //stampapkt(pkt, 3);
}



void LampeggioLED() {
  //char s[50];
  static unsigned long tcambio=0,ison=false;
  unsigned long now=millis();
  unsigned long delta=now-tcambio;
  
  if(ison) {
    if(delta>=Tledon) {
      ison=false;
      tcambio=now;
      digitalWrite(LEDPIN, LOW); 
      //sprintf(s,"lampegon: now=%u",now);
      //Serial.println(s);
    }
    
  } else {
    if(delta>=Tledoff) {
      ison=true;
      tcambio=now;
      digitalWrite(LEDPIN, HIGH); 
      //sprintf(s,"lampegoff: now=%u",now);
      //Serial.println(s);
    }
    
  }
}

void impostaled(int Ton, int Toff) {
  Tledon=Ton;
  Tledoff=Toff;
}

//algoritmo 7
void ProcessaDatiSeriali() {
  static byte comando=0,prossimodato=0,k=0;
  static char valore[5];
  if(Serial.available()>0) {
    int c=Serial.read();
    /*
    Serial.print(F("datiser: char="));
    Serial.print(c,HEX);
    Serial.print(F(" cmd="));
    Serial.print(comando,HEX);
    Serial.print(F(" prox="));
    Serial.print(prossimodato,HEX);
    Serial.print(F(" k="));
    Serial.print(k,HEX);
    Serial.print(F(" valore="));
    Serial.println(valore);
    */
    if(c==' ') return;
    if(c=='\n') {
      // elabora il comando
      if(comando=='W') {
        // scrivi indirizzo slave sul byte 0 della eeprom
        int ind=atoi(valore);
        if(ind<1 || ind>255) {
          Serial.println(F("parametro errato"));  
        } else {
          EEPROM.write(0,ind);
          radio.setAddress(ind);
          Serial.print(F("indirizzo memorizzato: "));
          Serial.println(ind);
        }
      } else {
        Serial.println(F("comando errato"));  
        Serial.println(radio.readReg(0x01),HEX);
        Serial.println(radio.readReg(0x27),HEX);
        //radioSetup();
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
  

void radioSetup() {
  // Hard Reset the RFM module 
  Serial.println("radioSetup");
  pinMode(RFM69_RST, OUTPUT); 
  digitalWrite(RFM69_RST, HIGH); 
  delay(100);
  digitalWrite(RFM69_RST, LOW); 
  delay(100);
	radio.initialize(RF69_868MHZ,indirizzo,NETWORKID);
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

void stampapkt(byte *pkt,int len) {
  Serial.print("len:");
  Serial.print(len);
  Serial.print("pkt:");
  for (int i=0;i<len;i++) {
    Serial.print(pkt[i],HEX);
    Serial.print(":");
  }
  Serial.println();
    
}
