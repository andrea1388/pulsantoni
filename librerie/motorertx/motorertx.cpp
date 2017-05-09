class MotoreRtx {
	public:
		byte destinatario;
		Trasmetti(byte,char *,byte,char *, byte);
		ElaboraMessaggioRicevuto();
		MotoreRtx(RFM69*, byte);
		void (*callback)(byte)
}
MotoreRtx::Trasmetti(byte destinatario, char *dati, byte lunghezza, char *risposta, byte *lenrisp,byte waitchar) {
	byte tentativo=0;
	while(true) {
		if(tentativo<2) dest=destinatario; else dest=bestn[(tentativo-2) % MAXBESTNEIGHBOURS]->indirizzo;
		radio.send(dest, dati, m.lunghezza+INDICEINIZIODATI, false); // noack
		if(radio.receiveDone()) ElaboraDatiRadio();
		
		
		
		
	}
}
MotoreRtx::ElaboraMessaggioRicevuto() {
  CostruisciListaNodi();
  // se non è roba per me buttalo
  if(radio.TARGETID!=indirizzo) return;
  // ack indirizzato a me
  // mio pacchetto
  if(radio.DATA[INDICEDESTINATARIOFINALE]==indirizzo) {
	  (*callback)(cb_arguments);
    	return;
  }

  // pkt indirizzato a me ma con destinatario finale diverso = da ritrasmettere
  if (radio.DATA[INDICEDESTINATARIOFINALE]!=indirizzo) {
    // è un pacchetto da ritrasmettere
    // se ho già un okt in corso di trasmissione non do l'ack e lo butto
	  Trasmetti(radio.DATA[INDICEDESTINATARIOFINALE],radio.DATA,radio.PAYLOADLEN,0,0,0);
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

