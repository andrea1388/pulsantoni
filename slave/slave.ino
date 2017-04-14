#include <RFM69.h>
#include <EEPROM.h>

#define pinPULSANTE 3
#define PINBATTERIA 0 // per lettura tensione batteria 
#define TIMEOUTVOTO 1200000000 // 20 minuti
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


unsigned long TdaInizioVoto,TrxSync,Tvoto, Tledoff, Tledon;
bool pulsantegiapremuto;
RFM69 radio=RFM69(RFM69_CS, RFM69_IRQ, true, RFM69_IRQN);

void setup() {
  // put your setup code here, to run once:
  pinMode(pinPULSANTE, INPUT_PULLUP);
  byte indirizzo = EEPROM.read(0);
  Serial.begin(250000);
  Serial.println(F("Setup"));
  Serial.print(F("Indirizzo: "));
  Serial.println(indirizzo);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  impostaled(100,2900);
  radioSetup(indirizzo);
  radio.readAllRegs();
  TrxSync=0;
}

// algoritmo 1
void loop() {
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  ElaboraRadio();
  if(TrxSync!=0) {
    if((micros()-TrxSync)>TIMEOUTVOTO) {
      TrxSync=0;
      impostaled(100,2900);
    }
  }
  LampeggioLED();
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
  if(TrxSync!=0) {
    if(digitalRead(pinPULSANTE)==LOW) {
      if (!pulsantegiapremuto) {
        pulsantegiapremuto=true;
        Tvoto=micros()-TrxSync+TdaInizioVoto;
        impostaled(1000,1);
        Serial.print(F("elabpuls: tvoto="));      
        Serial.println(Tvoto,DEC);
      }
    }
  }
}

// algoritmo 4
void ElaboraCmdInvioSync(byte * pkt) {
  unsigned long t;
  TrxSync=micros();
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
  radio.send(MASTER, "k", 1,false);
  pulsantegiapremuto=false;
  impostaled(500,500); //1 Hz Dc=50%
  Tvoto=0;
  Serial.print("rxsync: trxsync=");
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
  radio.send(MASTER, pkt, 3,false);
  impostaled(30,70);
  //stampapkt(pkt, 3);
    
}  
 

void ElaboraPoll() {
  byte pkt[5];
  pkt[0]='q';
  pkt[1]=Tvoto >> 24;
  pkt[2]=(Tvoto >> 16) & 0xFF;
  pkt[3]=(Tvoto >> 8) & 0xFF;
  pkt[4]=(Tvoto) & 0xFF;
  radio.send(MASTER, pkt, 5,false);
  //delay(1);
  //Serial.print("elabpoll:");
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
  /*
  if ((now-tcambio) >= Tledciclo) {
    tcambio=now;
    //sprintf(s,(char *)F("lampeg: now=%u,tcambio=%u,ciclo=%u,on=%u"),now,tcambio,Tledciclo,Tledon);
    sprintf(s,"lampegon: now=%u",now);
    Serial.println(s);
    digitalWrite(LEDPIN, LOW); 
  } else {
    if ((now-tcambio) >= Tledon) {
      digitalWrite(LEDPIN, HIGH); 
      sprintf(s,"lampegoff: now=%u",now);
      Serial.println(s);
    }
    */
    


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
  

void radioSetup(byte indirizzo) {
  // Hard Reset the RFM module 
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
  radio.writeReg(0x37,radio.readReg(0x37) | 0b01010000); // data whitening
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
