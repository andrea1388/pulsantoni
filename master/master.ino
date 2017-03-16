#define DURATACLICKLUNGO 2000000 // tempo pressione pulsante per click lungo = 2 secondi
#define TBACKOUTPULSANTE 10000 // tempo blackout pulsante dopo un click
//stati
#define ZERO 0 // stato iniziale
#define DISCOVERY 1 // discovery: cerca gli slave presenti in rete
#define INVIASYNC 2 // pre voto. Invia l'ora del master a tutti gli slave svovati con discovery
#define VOTO 3 // fase di acquisizione dei voti e display risultati

#define pinPULSANTE 11

class Slave {
  public:
    unsigned long oravoto;
    byte tensionebatteria;
    byte rssi;
    bool funzionante;
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
      //aggiorna disply
      Serial.println(F("s 0"));
    case DISCOVERY:
      //aggiorna disply
      Serial.println(F("s 1"));
    case INVIASYNC:
      //aggiorna disply
      Serial.println(F("s 2"));
    case VOTO:
      //aggiorna disply
      Serial.println(F("s 0"));
    default:
      char s[50];
      sprintf(s,(char *)F("e setstato indefinito %d"),newstato);
      Serial.println(s);
      return;
  }
  stato=newstato;
}

Stato stato;
Slave* slave[255];
byte numero_slave;


void setup() {
  // put your setup code here, to run once:
  pinMode(pinPULSANTE, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println("Setup");
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
  byte risposta[10];
  if(interrogaSlave(indirizzo_slave_corrente,risposta)) {
    slave[numero_slave]=new Slave(indirizzo_slave_corrente);
    slave[numero_slave]->oravoto=0;
    slave[numero_slave]->tensionebatteria=risposta.tensionebatteria;
    slave[numero_slave]->rssi=risposta.rssi;
    slave[numero_slave]->funzionante=true;

    AggiornaDisplayDiscovery(numero_slave);
    byte msg[19];
    sprintf(msg,(char *)F("d %d %d %d"),indirizzo_slave_corrente,slave[numero_slave]->tensionebatteria, slave[numero_slave]->rssi);
    Serial.println(msg);
    numero_slave++;
  }
  if(indirizzo_slave_corrente=255) stato.setStato(ZERO);
  indirizzo_slave_corrente++;
}
//algoritmo 6
void Voto() {
  if(numero_votati==numero_slave) {
    stato.setStato(ZERO);
    Msg("Voto concluso");
  }
  else {
    interrogaTuttiGliSlave();
    if(numero_votati>0) {
      MostraRisultatiVoto();
      AggiornaSeriale();
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
      if(TrasmettiPacchettoSync(slave[i].indirizzo),micros-t_inizio_voto) break;
      retry++;
      if(retry==3) {
        stato.setStato(0);
        Msg(F("Slave non funzionante") + slave[i].indirizzo);
        return false;
      }
    }
  }
  stato.setStato(VOTO);
  return true;
  
}

//algoritmo 8
void PulsanteClickLungo() {
  if(getStato()==0) {
    stato.setStato(DISCOVERY);
    indirizzo_slave_discovery=1;    
  } else if(stato.getStato()==DISCOVERY) stato.setStato(ZERO);
}

//algoritmo 9
void PulsanteClickCorto() {
  byte stato=getStato();
  if(stato==DISCOVERY) return;
  if(stato==ZERO) {
    stato.setStato(INVIASYNC);
    indice_slave_corrente=0;
  }
  if(stato==VOTO) {
    stato.setStato(ZERO);
  }
}

//algoritmo 10
void interrogaTuttiGliSlave() {
  byte risposta[10];
  for(byte i=0;i<numero_slave;i++) {
    if(interrogaSlave(slave[i].indirizzo,risposta)) {
      slave[i].oravoto=risposta.oravoto;
      slave[i]->tensionebatteria=risposta.tensionebatteria;
      slave[i]->rssi=risposta.rssi;
      slave[i]->funzionante=true;
    } else {
      slave[i]->funzionante=false;
      msg(F("Pulsante non risponde:") + i);
    }
  }
}

//algoritmo 15
bool interrogaSlave(byte a, byte *r) {
  return true;
}

//algoritmo 18
bool TrasmettiPacchettoSync(byte indirizzo, unsigned long t_da_iniziovoto) {
  
}





