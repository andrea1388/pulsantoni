#define pinPULSANTE 11
unsigned long TdaInizioVoto,TrxSync,Tvoto;
bool pulsantegiàpremuto;

void setup() {
  // put your setup code here, to run once:
  pinMode(pinPULSANTE, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println("Setup");
}
// algoritmo 1
void loop() {
  if(Serial.available()) ProcessaDatiSeriali();
  ElaboraPulsante();
  ElaboraRadio();
}

void ElaboraCmdInvioSync(byte * pkt) {
  TrxSync=micros();
  // estrae l'informazione dal pacchetto
  TdaInizioVoto=pkt[0];
  pulsantegiàpremuto=false;
  impostaled(1,1); //1 Hz Dc=10%
  Tvoto=0;
}

void ElaboraPulsante() {
  if(pin==0) {
    if (!pulsantegiàpremuto) {
      pulsantegiàpremuto=true;
      Tvoto=micros()-TrxSync+TdaInizioVoto;
    }
  }
}

void ElaboraRadio() {
  byte pkt[7];
  if(!datidisponibili) return;
  switch(pkt[0]) {
    case 's':
        ElaboraCmdInvioSync(pkt);
    case 'p':
        ElaboraPoll(pkt);
    case 'd':
        ElaboraCmdDiscovery(pkt);
    
        
  }
}

