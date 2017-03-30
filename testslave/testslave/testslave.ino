#include <RFM69.h>
#include <EEPROM.h>
#define TBACKOUTPULSANTE 100 // tempo blackout pulsante dopo un click
#define DURATACLICKLUNGO 2000 // tempo pressione pulsante per click lungo = 2 secondi

#define pinPULSANTE 3
#define PINBATTERIA 12 // per lettura tensione batteria 
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
unsigned long tiniziovoto;
void stampapkt(byte *pkt,int len);

unsigned long TdaInizioVoto,TrxSync,Tvoto, Tledoff, Tledon;
bool pulsantegiapremuto;
RFM69 radio=RFM69(RFM69_CS, RFM69_IRQ, true, RFM69_IRQN);

void setup() {
  // put your setup code here, to run once:
  pinMode(pinPULSANTE, INPUT_PULLUP);
  byte indirizzo = EEPROM.read(0);
  Serial.begin(9600);
  Serial.println(F("testslave"));
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  impostaled(10,400);
  radioSetup(0);
  radio.readAllRegs();
}

// algoritmo 1
void loop() {
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  //LampeggioLED();
}

void ElaboraPulsante() {
  static unsigned long inizio_blackout=0, durata_pressione_pulsante=0,inizio_pressione_pulsante=0;
  static bool first=true;
  unsigned long now=millis();
  if(digitalRead(pinPULSANTE)==0) {
    if((now-inizio_blackout) > TBACKOUTPULSANTE) {
      durata_pressione_pulsante=(now-inizio_pressione_pulsante);
      if(first) {inizio_pressione_pulsante=now; first=false;} else durata_pressione_pulsante=(now-inizio_pressione_pulsante);
    }
    //Serial.print("puls press: durata=");
  } else {
    if(durata_pressione_pulsante>0) {
      if(durata_pressione_pulsante>DURATACLICKLUNGO) PulsanteClickLungo(); else PulsanteClickCorto();
      durata_pressione_pulsante=0;
      first=true;
      inizio_blackout=now;
    }
  }
}
void PulsanteClickLungo() {
}

//algoritmo 9
void PulsanteClickCorto() {
  Discovery();
  Voto();
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
  static byte comando=0,prossimodato=0,k=0,valore[5];
  if(Serial.available()) {
    char c=Serial.read();
    Serial.print(F("datiser: char="));
    Serial.print(c,HEX);
    Serial.print(F(" cmd="));
    Serial.print(comando,HEX);
    Serial.print(F(" prox="));
    Serial.print(prossimodato,HEX);
    Serial.print(F(" k="));
    Serial.println(k,HEX);
    if(c==' ') return;
    if(c=='\n') {
      // elabora il comando
      Serial.println("elab");
      switch(comando) {
        case 'D':
          Discovery();
          // invia discovery a 1
          break;
        case 'S':
          Sync();
          // invia discovery a 1
          break;
        case 'P':
          Voto();
          // invia discovery a 1
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

void Sync() {
  tiniziovoto=micros();
  Serial.print("Sync micros:");
  Serial.println(tiniziovoto);
  if(TrasmettiPacchettoSync(1, tiniziovoto)) {
    Serial.print("ok");
  } else Serial.print("nooo!");
  
}

void Discovery() {
  byte livbatt;
  byte rssi;
  Serial.println("discovery");

  digitalWrite(LEDPIN, LOW); 
  if(interrogaSlaveDiscovery(1,&livbatt,&rssi)) {
    Serial.print("d ");
    Serial.print(1);
    Serial.print(" ");
    Serial.print(livbatt*(3.3/255));
    Serial.print(" ");
    Serial.println(rssi);
    digitalWrite(LEDPIN, HIGH); 
    delay(100);
    digitalWrite(LEDPIN, LOW); 
  } else {
    digitalWrite(LEDPIN, HIGH); 
    delay(1000);
    digitalWrite(LEDPIN, LOW); 
    
  }
}

void Voto() {
  unsigned long oravoto;
    Serial.println("Voto");
    Serial.print("micros:");
  Serial.println(micros());
  digitalWrite(LEDPIN, LOW); 

  if(interrogaSlaveVoto(1,&oravoto)) {
    Serial.print("oravoto: ");
    Serial.print(oravoto-tiniziovoto,DEC);
    Serial.print(" ");
    Serial.println((oravoto-tiniziovoto)/1000,DEC);
    digitalWrite(LEDPIN, HIGH); 
    delay(100);
    digitalWrite(LEDPIN, LOW); 
  } else {
    digitalWrite(LEDPIN, HIGH); 
    delay(1000);
    digitalWrite(LEDPIN, LOW); 
    
  }
  
}
bool interrogaSlaveDiscovery(byte indirizzo, byte *livbatt, byte *rssi) {
  byte pkt[1];
  pkt[0]='d';
  
  radio.send(indirizzo,pkt,1,false);
  unsigned long sentTime = millis();
  while (millis() - sentTime < 20) {
    if(radio.receiveDone()) {
      stampapkt(radio.DATA,radio.PAYLOADLEN);
      if(radio.DATA[0]=='e') {
        *livbatt=radio.DATA[1];
        *rssi=radio.DATA[2];
        return true;
      }
    }
  }
  return false;
}

bool interrogaSlaveVoto(byte indirizzo, unsigned long* oravoto) {
  unsigned long t;
  byte pkt[1];
  pkt[0]='p';
  radio.send(indirizzo,pkt,1,false);
  unsigned long sentTime = millis();
  while (millis() - sentTime < 20) {
    if(radio.receiveDone()) {
      stampapkt(radio.DATA,radio.PAYLOADLEN);
      if(radio.DATA[0]=='q') {
        *oravoto=0;
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

bool TrasmettiPacchettoSync(byte indirizzo, unsigned long t_da_iniziovoto) {
  char pkt[5];
  pkt[0]='s';
  pkt[1]=t_da_iniziovoto >> 24;
  pkt[2]=(t_da_iniziovoto >> 16) & 0xFF;
  pkt[3]=(t_da_iniziovoto >> 8) & 0xFF;
  pkt[4]=(t_da_iniziovoto) & 0xFF;
  stampapkt(pkt,5);
  radio.send(indirizzo, pkt, 5);
  unsigned long sentTime = millis();
  while (millis() - sentTime < 20) {
    if(radio.receiveDone()) {
      stampapkt(radio.DATA,radio.PAYLOADLEN);
      if(radio.DATA[0]=='k') {
        return true;
      }
    }
  }
  return false;  
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



