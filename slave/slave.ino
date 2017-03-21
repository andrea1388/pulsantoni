#include <RFM69.h>
#include <EEPROM.h>

#define pinPULSANTE 11
#define PINBATTERIA 12 // per lettura tensione batteria 
#define TIMEOUTVOTO 1200000000 // 20 minuti
#define NETWORKID 27
#define MASTER 0
#define LEDPIN LED_BUILTIN
// stati per elaborazione seriale
#define COMANDO 0
#define VALORE 0

#define FREQUENCY 917000000

unsigned long TdaInizioVoto,TrxSync,Tvoto, Tledciclo, Tledon;
bool pulsantegiapremuto;
RFM69 radio;

void setup() {
  // put your setup code here, to run once:
  pinMode(pinPULSANTE, INPUT_PULLUP);
  byte indirizzo = EEPROM.read(0);
  radio.initialize(RF69_868MHZ,indirizzo,NETWORKID);
  radio.writeReg(0x03,0x0D); // 9k6
  radio.writeReg(0x04,0x05);
  /*
  radio.writeReg(0x03,0x00); // 153k6
  radio.writeReg(0x04,0xD0);
  */
  radio.setFrequency(FREQUENCY);
  radio.setHighPower(); 
  Serial.begin(115200);
  Serial.println(F("Setup"));
  Serial.print(F("Indirizzo: "));
  Serial.println(indirizzo);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  impostaled(3000,300);
  radio.readAllRegs();
}

// algoritmo 1
void loop() {
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  ElaboraRadio();
  if((micros-TrxSync)>TIMEOUTVOTO) impostaled(3000,300);
  LampeggioLED();
}

// algoritmo 2
void ElaboraRadio() {
  byte pkt[7];
  if(!radio.receiveDone()) return;
  switch(radio.DATA[0]) {
    case 's':
        ElaboraCmdInvioSync(pkt); // cmd s
    case 'p':
        ElaboraPoll(); // cmd p
    case 'd':
        ElaboraCmdDiscovery(); // cmd d
    
        
  }
}

// algoritmo 3
void ElaboraPulsante() {
  if(digitalRead(pinPULSANTE)==LOW) {
    if (!pulsantegiapremuto) {
      pulsantegiapremuto=true;
      Tvoto=micros()-TrxSync+TdaInizioVoto;
      impostaled(1000,1000);
    }
  }
}

// algoritmo 4
void ElaboraCmdInvioSync(byte * pkt) {
  TrxSync=micros();
  // estrae l'informazione dal pacchetto
  TdaInizioVoto=radio.DATA[1] << 24;
  TdaInizioVoto+=(radio.DATA[2] << 16);
  TdaInizioVoto+=(radio.DATA[3] << 8);
  TdaInizioVoto+=radio.DATA[4];
  radio.send(MASTER, "k", 1,false);
  pulsantegiapremuto=false;
  impostaled(1000,100); //1 Hz Dc=10%
  Tvoto=0;
}

// algoritmo 5
void ElaboraCmdDiscovery() {
  char pkt[4];
  pkt[0]='e';
  pkt[1]=(analogRead(PINBATTERIA)>>2);
  pkt[2]=radio.RSSI >> 8;
  pkt[3]=radio.RSSI & 0xFF;
  radio.send(MASTER, pkt, 4,false);
}

void ElaboraPoll() {
  char pkt[5];
  pkt[0]='q';
  pkt[1]=Tvoto >> 24;
  pkt[2]=(Tvoto >> 16) & 0xFF;
  pkt[3]=(Tvoto >> 8) & 0xFF;
  pkt[1]=(Tvoto) & 0xFF;
  radio.send(MASTER, pkt, 5,false);
}


//algoritmo 6
void LampeggioLED() {
  unsigned long now=millis();
  static unsigned long tcambio;
  if ((now-tcambio) >= Tledciclo) {
    digitalWrite(LEDPIN, HIGH); 
    tcambio=now;
  } else
    if ((now-tcambio) >= Tledon) digitalWrite(LEDPIN, LOW); 
}

void impostaled(int Tciclo, int TOn) {
  Tledciclo=Tciclo;
  Tledon=TOn;
}

//algoritmo 7
void ProcessaDatiSeriali() {
  static byte comando=0,prossimodato=0,k=0,valore[5];
  if(Serial.available()) {
    byte c=Serial.read();
    if(c=='\n' || c==' ') return;
    if(c=='\r') {
      // elabora il comando
      if(comando=='W') {
        // scrivi indirizzo slave sul byte 0 della eeprom
        int ind=atoi(valore);
        if(ind<1 || ind>255) {
          Serial.println(F("parametro errato"));  
        } else {
          EEPROM.write(0,ind);
          radio.setAddress(ind);
          Serial.println(F("indirizzo memorizzato"));
        }
      } else {
        Serial.println(F("comando errato"));  
      }
      k=0;
      prossimodato=COMANDO;     

    }
    if(prossimodato==COMANDO) {
      c=toupper(c);
      comando=c;
      prossimodato=VALORE;
      k=0;
    }
    if(prossimodato==VALORE) {
      valore[k++]=c;
      if(k>3) {
        k=0;
        prossimodato=COMANDO;     
      }
    }
    
  }
  
}

