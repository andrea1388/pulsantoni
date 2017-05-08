/*
 * Slave
 * Sistema polling con slave repeaters
 * versione iniziale psr1
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

#define pinPULSANTE 3
#define PINBATTERIA 0 // per lettura tensione batteria 
#define TIMEOUTVOTO 360000000 // 360 sec = 6 min
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
    unsigned long tstart,tultimotentativo;
    byte destinatario,mittente;
    byte lunghezza;
    byte dati[19];
    byte tentativo;
    bool txincorso;
    PrendiDaRadio(RFM69*);
    PrendiDaPkt(Messaggio);
    //Messaggio();
    unsigned long GetTime();
    void SetTime(unsigned long);
    void print();
    
    
};
void Messaggio::print() {
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
  for(byte k=0;k<r->DATALEN;k++) dati[k]=r->DATA[k];
  txincorso=true;
  tstart=micros();
  tentativo=0;
  lunghezza=r->DATALEN-INDICEINIZIODATI;
}
Messaggio::PrendiDaPkt(Messaggio m) {
  destinatario=m.destinatario;
  mittente=m.mittente;
  for(byte k=0;k<m.lunghezza+INDICEINIZIODATI;k++) dati[k]=m.dati[k];
  txincorso=true;
  tstart=micros();
  tentativo=0;
  lunghezza=m.lunghezza;
}
class Nodo {
  public:
    byte indirizzo;
    int16_t segnale;
    Nodo();
};
Nodo::Nodo() { indirizzo=255; segnale=-200;}

Messaggio m;
Nodo* bestn[MAXBESTNEIGHBOURS];


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
  Serial.println(F("Slave - Firmware: psr1"));
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
  for (int i=0;i<MAXBESTNEIGHBOURS;i++) bestn[i]=new Nodo();
}

// algoritmo 1
void loop() {
  static unsigned long int tckradio=0;
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  if(radio.receiveDone()) ElaboraDatiRadio();
  // motore trasmissione 
  if(m.txincorso) tx2();  
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

bool tx(byte destinatario, char *dati, byte lunghezza) {
  if(m.txincorso) {
    Serial.println(F("tx busy"));
    return false;
  }
  Serial.println(F("tx"));
  m.mittente=indirizzo;
  m.dati[INDICEDESTINATARIOFINALE]=destinatario;
  m.dati[INDICEMITTENTEINIZIALE]=indirizzo;
  m.dati[INDICEH1]=0;
  m.dati[INDICEH2]=0;
  m.dati[INDICEH3]=0;
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
  if (m.tentativo<2) m.destinatario=m.dati[INDICEDESTINATARIOFINALE]; 
  else m.destinatario=bestn[m.tentativo % MAXBESTNEIGHBOURS]->indirizzo;
  if(m.tentativo++==10) {m.txincorso=false; return;}  
  if(m.destinatario==m.dati[INDICEMITTENTEINIZIALE]) return;
  if(m.destinatario==m.dati[INDICEH1]) return;
  if(m.destinatario==m.dati[INDICEH2]) return;
  if(m.destinatario==m.dati[INDICEH3]) return;
  // se i 3 salti sono completi è possibile mandare solo al destinatario finale
  if(m.destinatario!=m.dati[INDICEDESTINATARIOFINALE] && m.dati[INDICEH3]!=0) return;
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
    if(radio.ACKRequested()) {
      radio.sendACK(0,0);
      switch(rxm.dati[INDICEINIZIODATI]) {
        case 's':
            ElaboraCmdInvioSync(rxm.dati[INDICEINIZIODATI+1],rxm.dati[INDICEINIZIODATI+2],rxm.dati[INDICEINIZIODATI+3],rxm.dati[INDICEINIZIODATI+4]); // cmd s
            break;
        case 'p':
            ElaboraPoll(); // cmd p
            break;
        case 'd':
            ElaboraCmdDiscovery(); // cmd d
            break;
      }
      return;
    }
    
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
void ElaboraCmdInvioSync(byte b3,byte b2,byte b1,byte b0) {
  unsigned long t;
  TrxSync=micros();
  TLastPoll=TrxSync;
  //stampapkt(pkt, 5);
  // estrae l'informazione dal pacchetto
  t=b3;
  t=t<<24;
  TdaInizioVoto=t;
  t=b2;
  t=t<<16;
  TdaInizioVoto+=t;
  t=b1;
  t=t<<8;
  TdaInizioVoto+=t;
  TdaInizioVoto+=b0;
  if(tx(MASTER, "k", 1)) {
    pulsantegiapremuto=false;
    impostaled(500,500); //1 Hz Dc=50%
    Tvoto=0;
    stato=SINCRONIZZATO;
    Serial.print(F("SINCRONIZZATO trxsync="));
    Serial.print(TrxSync);
    Serial.print(" tdainiziov=");
    Serial.println(TdaInizioVoto);
 }
}

// algoritmo 5
void ElaboraCmdDiscovery() {
  byte pkt[4];
  pkt[0]='e';
  pkt[1]=(analogRead(PINBATTERIA)>>2);
  pkt[2]=radio.RSSI;
  if(tx(MASTER, pkt, 3)) {
    impostaled(30,70);
    Serial.print("ElaboraCmdDiscovery: VBatt=");
    Serial.print(pkt[1]);
    Serial.print(" RSSI=");
    Serial.println(pkt[2]);
  }    
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
      // fuori sync   
      pkt[0]='r';
      pl=1;
      break;
    case SINCRONIZZATO:    
      // sincronizzato ok ma non ancora votato
      pkt[0]='t';
      pl=1;
      break;
  }
  tx(MASTER, pkt, pl);
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
