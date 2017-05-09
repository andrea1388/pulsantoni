class MotoreRtx {
	public:
		byte destinatario;
		Trasmetti(byte,char *,byte,char *, byte);
		ElaboraMessaggioRicevuto();
		MotoreRtx(RFM69*, byte);
		void (*callback)(byte)
}
MotoreRtx::Trasmetti(byte destinatario, char *dati, byte lunghezza, char *risposta, byte lenrisp) {
	byte tentativo=0;
	while(true) {
		if(tentativo<2) dest=destinatario; else dest=bestn[(tentativo-2) % MAXBESTNEIGHBOURS]->indirizzo;
		radio.send(dest, dati, m.lunghezza+INDICEINIZIODATI, false); // noack
		
		
		
		
	}
}
MotoreRtx::ElaboraMessaggioRicevuto() {
  CostruisciListaNodi();
  // se non è roba per me buttalo
  if(radio.TARGETID!=indirizzo) return;
  // ack indirizzato a me
  // mio pacchetto non ack
  if(radio.DATA[INDICEDESTINATARIOFINALE]==indirizzo) {
	  (*callback)(cb_arguments);
    if(radio.ACKRequested()) radio.sendACK(0,0); 
    //if(indirizzo_slave_corrente!=rxm.dati[INDICEMITTENTEINIZIALE]) {Serial.println("msg fuori tempo");};
    switch(rxm.dati[INDICEINIZIODATI]) {
      case 'e':
        // msg risposta a discovery
        for (int d=0;d<numero_slave;d++) if(slave[d]==rxm.dati[INDICEMITTENTEINIZIALE]) {Serial.println("già presente"); return;};
        slave[numero_slave]=new Slave(rxm.dati[INDICEMITTENTEINIZIALE]);
        slave[numero_slave]->oravoto=0;
        slave[numero_slave]->fallimenti=0;
        Serial.print("d ");
        Serial.print(slave[numero_slave]->indirizzo);
        Serial.print(" ");
        Serial.print(rxm.dati[INDICEINIZIODATI+1]);
        Serial.print(" ");
        Serial.print((int)rxm.dati[INDICEINIZIODATI+2]);
        tft.setTextColor(GREEN);
        tft.print(slave[numero_slave]->indirizzo);
        tft.print(" ");
        numero_slave++;
        indirizzo_slave_corrente++;
        break;
      case 'p':
          break;
      case 'd':
          break;
    }
    return;
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

