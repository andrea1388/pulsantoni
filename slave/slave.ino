#include <RFM69.h>
#include <EEPROM.h>

#define pinPULSANTE 11
#define TIMEOUTVOTO 1200000000 // 20 minuti
#define NETWORKID 27
#define MASTER 0

unsigned long TdaInizioVoto,TrxSync,Tvoto;
bool pulsantegiapremuto;
RFM69 radio;

void setup() {
  // put your setup code here, to run once:
  pinMode(pinPULSANTE, INPUT_PULLUP);
  byte indirizzo = EEPROM.read(0);
  radio.initialize(RF69_868MHZ,indirizzo,NETWORKID);
  radio.setHighPower(); 
  Serial.begin(115200);
  Serial.println(F("Setup"));
  Serial.print(F("Indirizzo: "));
  Serial.println(indirizzo);
}

// algoritmo 1
void loop() {
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  ElaboraRadio();
  if((micros-TrxSync)>TIMEOUTVOTO) impostaled(0.3,0.3);
}

// algoritmo 2
void ElaboraRadio() {
  byte pkt[7];
  if(!radio.receiveDone()) return;
  switch(radio.DATA[0]) {
    case 's':
        ElaboraCmdInvioSync(pkt);
    case 'p':
        ElaboraPoll(pkt);
    case 'd':
        ElaboraCmdDiscovery(pkt);
    
        
  }
}

// algoritmo 3
void ElaboraPulsante() {
  if(pin==0) {
    if (!pulsantegiapremuto) {
      pulsantegiapremuto=true;
      Tvoto=micros()-TrxSync+TdaInizioVoto;
    }
  }
}

// algoritmo 4
void ElaboraCmdInvioSync(byte * pkt) {
  TrxSync=micros();
  // estrae l'informazione dal pacchetto
  TdaInizioVoto=radio.DATA[1] << 24;
  TdaInizioVoto+=radio.DATA[2] << 16;
  TdaInizioVoto+=radio.DATA[3] << 8;
  TdaInizioVoto+=radio.DATA[4];
  radio.send(MASTER, "k", 1,false);
  pulsantegiapremuto=false;
  impostaled(1,1); //1 Hz Dc=10%
  Tvoto=0;
}

// algoritmo 5
void ElaboraCmdDiscovery(byte * pkt) {
  char pkt[3];
  pkt[0]=batteria;
  pkt[1]=radio.RSSI >> 8;
  pkt[2]=radio.RSSI & 0xFF;
  radio.send(MASTER, pkt, 3,false);
}

void ProcessaDatiSeriali() {
	static byte comando=0;prossimodato=0,k=0,valore[5];
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
					Serial.println(F("indirizzo memorizzato"));
				}
			} else {
				Serial.println(F("comando errato"));	
			}
			k=0;
			prossimodato=COMANDO;			

		}
		if(prossimodato==COMANDO) {
			c=topper(c);
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


