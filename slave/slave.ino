#define pinPULSANTE 11


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
  ElaboraStato();
}

