/*
 * formato pacchetto
 * ([bufsize+3][toaddress][fromaddress][ctlbyte])[mittente iniziale][destinatariofinale][h1][h2][h3][t3][t2][t1][t0][dn]..[d0]
 * ver 3
 * led acceso prima possibile
 * test send fail su discovery, poll e inviosync
 * ver 4
 * introdotto TLastPolll e sincronizzato
 * risponde r al poll se ha perso il sync e il master lo segnalerà come non funzionante
 */
#include <RFM69.h>
#include <EEPROM.h>

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

#define TEMPOTRATENTATIVI 50 // in ms

#define MAXBESTNEIGHBOURS 5

#define REG_PACKETCONFIG2 0x3D
#define RF_PACKET2_RXRESTART                  0x04

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

class Messaggio {
  public:
    unsigned long tstart,t,tultimotentativo;
    byte destinatario;
    byte lunghezza;
    byte *dati[19];
    byte tentativo;
    bool txincorso;
    //Messaggio(byte);
};

class Nodo {
  public:
    byte indirizzo;
    int16_t segnale;
    Nodo();
};
Nodo::Nodo() { indirizzo=0; segnale=-200;}

Messaggio m;
unsigned long TdaInizioVoto,TrxSync,Tvoto, Tledoff, Tledon,TLastPoll;
Nodo* bestn[MAXBESTNEIGHBOURS];
bool pulsantegiapremuto=false,votato=false,vototrasmesso;
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
  Serial.println(F("Slave - Firmware: 8"));
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
  for (int i=0;i<MAXBESTNEIGHBOURS;i++) bestn[i]=new Nodo();
  // void tx(byte destinatariofinale, byte mittenteoriginale, byte h1, byte h2, byte h3, unsigned long t, char *dati, byte lunghezza) {

  tx(1,indirizzo,0,0,0,0,"",0);
  
}

// algoritmo 1
void loop() {
  static unsigned long int tckradio=0;
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  if(radio.receiveDone()) ElaboraDatiRadio();
  LampeggioLED();
  if(votato && !vototrasmesso && !m.txincorso) {tx(1,indirizzo,0,0,0,0,"v",1); vototrasmesso=true;}
  if(m.txincorso) tx2();  
  if((millis()-tckradio)>1000) {
      tckradio=millis();
      if(radio.readReg(0x01)==0) radioSetup();
  }
}

void tx(byte destinatariofinale, byte mittenteoriginale, byte h1, byte h2, byte h3, unsigned long t, char *dati, byte lunghezza) {
  Serial.print(F("tx: dest/t/len "));
  Serial.print(destinatariofinale);
  Serial.print(" ");
  Serial.print(t);
  Serial.print(" ");
  Serial.println(lunghezza);
  
  m.txincorso=true;
  m.dati[INDICEDESTINATARIOFINALE]=destinatariofinale;
  m.dati[INDICEMITTENTEINIZIALE]=mittenteoriginale;
  m.dati[INDICEH1]=h1;
  m.dati[INDICEH2]=h2;
  m.dati[INDICEH3]=h3;
  m.tstart=micros();
  m.t=t;
  m.tentativo=0;
  m.lunghezza=lunghezza;
  memcpy(dati,m.dati+INDICEINIZIODATI,lunghezza);
}

// richiamata continuamente finché c'è una trasmissione in corso
void tx2() {
  // dal secondo tentativo aspetta il timeout
  if( (m.tentativo>0) && ((millis() - m.tultimotentativo) < TEMPOTRATENTATIVI)) return;
  if (m.tentativo<2) m.destinatario=m.dati[INDICEDESTINATARIOFINALE]; 
  else m.destinatario=bestn[m.tentativo % MAXBESTNEIGHBOURS]->indirizzo;
  if(m.tentativo++==10) {m.txincorso=false; return;}  
  if(m.destinatario==m.dati[INDICEMITTENTEINIZIALE]) return;
  if(m.destinatario==m.dati[INDICEH1]) return;
  if(m.destinatario==m.dati[INDICEH2]) return;
  if(m.destinatario==m.dati[INDICEH3]) return;
  // se i 3 salti sono completi è possibile mandare solo al destinatario finale
  if(m.destinatario!=m.dati[INDICEDESTINATARIOFINALE] && m.dati[INDICEH3]!=0) return;
  if(m.destinatario==0) return;
  Serial.print(F("tx2: tentativo/dest "));
  Serial.print(m.tentativo);
  Serial.print(" ");
  Serial.println(m.destinatario);
  txf();
}

// effettua la trasmissione del frame se il canale è libero
void txf() {
  Serial.print(F("txf: dest/len "));
  Serial.print(m.destinatario);
  Serial.print(" ");
  Serial.println(m.lunghezza);
  unsigned long dt=(micros()-m.tstart) + m.t;
  m.dati[INDICET3]=dt >> 24;
  m.dati[INDICET2]=(dt >> 16) & 0xFF;
  m.dati[INDICET1]=(dt >> 8) & 0xFF;
  m.dati[INDICET0]=(dt) & 0xFF;
  m.tultimotentativo=millis();
  radio.send(m.destinatario, m.dati, m.lunghezza+INDICEINIZIODATI, true);
}

void ElaboraDatiRadio() {
  Serial.print(F("dati radio: sender/target/iniziale/finale/h1/h2/h3/rssi "));
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
  
  if(radio.SENDERID!=1) {
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
  // pacchetto indirizzato a me
  if(m.txincorso && radio.ACK_RECEIVED && radio.TARGETID==indirizzo) {
    m.txincorso=false;
    Serial.println("tx completed");
  }
  if(radio.TARGETID==indirizzo && radio.DATA[INDICEDESTINATARIOFINALE]==indirizzo) {
    if(radio.ACKRequested()) {
      radio.sendACK(0,0);
      ElaboraMioPkt();
    }
    
  }
  if (radio.TARGETID==indirizzo && radio.DATA[INDICEDESTINATARIOFINALE]!=indirizzo) {
    // è un pacchetto da ritrasmettere
    if(!m.txincorso) {
      Serial.println(F("pkt da ritrasm"));
      if(radio.ACKRequested()) radio.sendACK(0,0);
      // aggiunge l'indirizzo di questo nodo come ultimo salto
      for(byte h=0;h<3;h++) if(radio.DATA[INDICEH1+h]==0) radio.DATA[INDICEH1+h]=indirizzo;
      unsigned long t,tmp;
      tmp=radio.DATA[INDICET3];
      tmp=tmp<<24;
      t=tmp;
      tmp=radio.DATA[INDICET2];
      tmp=tmp<<16;
      t+=tmp;
      tmp=radio.DATA[INDICET1];
      tmp=tmp<<8;
      t+=tmp;
      tmp=radio.DATA[INDICET0];
      t+=tmp;
      // void tx(byte destinatariofinale, byte mittenteoriginale, byte h1, byte h2, byte h3, unsigned long t, char *dati, byte lunghezza) {
      tx(radio.DATA[INDICEDESTINATARIOFINALE],radio.DATA[INDICEMITTENTEINIZIALE],radio.DATA[INDICEH1],radio.DATA[INDICEH2],radio.DATA[INDICEH3],t,radio.DATA+INDICEINIZIODATI,radio.PAYLOADLEN-INDICEINIZIODATI);
    }
  }
}

void ElaboraMioPkt() {
  Serial.println(F("ElaboraMioPkt: "));
}
void ElaboraPulsante() {
	if(digitalRead(pinPULSANTE)==LOW) {
		impostaled(1,1);
		votato=true;
    vototrasmesso=false;
		Serial.println(F("Votato"));      
	}
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
  radio.writeReg(0x37,radio.readReg(0x37) | 0b01010000); // data whitening e no address filter
  radio.setFrequency(FREQUENCY);
	radio.setHighPower(); 
  radio.setPowerLevel(31);
  radio.promiscuous(true);
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
