﻿using System;
using System.IO.Ports;
using System.Threading;

public class MasterClass 
{
	SerialPort com;
    public event EventHandler EventoStatoZero;
    public event EventHandler EventoInizioDiscovery;
    public event EventHandler<DiscoveryEventArgs> EventoNuovoClient;
    public event EventHandler<VotoEventArgs> EventoVotoAcquisito;
    public event EventHandler EventoInviaSync;
    public event EventHandler EventoInizioPoll;
    public event EventHandler<MsgEventArgs> EventoNuovoMessaggio;
    public event EventHandler<NumSlaveEventArgs> EventoRxNumMaxSlave;
    public String portname;
    public int BaudRate;
    Thread rdr;
	bool mustexit;
    String stato;

	public MasterClass()
	{
		com = new SerialPort();
		//com.ReadTimeout = 50;
	}
 	~MasterClass()
	{
		mustexit = true;
		com.Close();
		//com.Dispose();
		rdr.Abort();

	}
    public void Open()
    {
        com.PortName = portname;
        com.BaudRate = BaudRate;
        com.Parity = Parity.None;
        com.StopBits = StopBits.One;
        com.Open();
        rdr = new Thread(ReadSerial);
        mustexit = false;
        stato = "x";
        rdr.Start();
    }
    public void Close()
    {
        mustexit = true;
        com.Close();
        rdr.Abort();

    }
    public void StartDiscovery()
    {
        com.WriteLine("y");
    }
    /*
	protected virtual void OnThresholdReached(EventArgs e)
	{
		EventHandler handler = ThresholdReached;
		if (handler != null)
		{
			handler(this, e);
		}
	}
	*/
    private void ReadSerial()
	{
		//EventHandler handler = ThresholdReached;
		do
		{
			try
			{
				String comandoricevuto = com.ReadLine().Trim();
                String[] sottocomandi = comandoricevuto.Split(' ');
                switch (sottocomandi.Length)
                {
                    case 1:
                        stato = sottocomandi[0];
                        switch (sottocomandi[0])
                        {
                            case "s0":
                                if (EventoStatoZero != null) EventoStatoZero(this, new EventArgs());
                                break;
                            case "ds": // discovery
                                if (EventoInizioDiscovery != null) EventoInizioDiscovery(this, new EventArgs());
                                break;
                            case "is": // invia sync
                                if (EventoInviaSync != null) EventoInviaSync(this, new EventArgs());
                                break;
                            case "ip": // inizio poll
                                if (EventoInizioPoll != null) EventoInizioPoll(this, new EventArgs());
                                break;
                            default:
                                cmderr(comandoricevuto);
                                break;
                        }
                        break;
                    case 2:
                        switch (sottocomandi[0])
                        {
                            case "e": // messaggio dal master
                                MsgEventArgs mea=new MsgEventArgs();
                                mea.msg = sottocomandi[1];
                                if (EventoNuovoMessaggio != null) EventoNuovoMessaggio(this, mea);
                                break;
                            case "ns": // max numero slave
                                NumSlaveEventArgs nse = new NumSlaveEventArgs();
                                nse.numslave = int.Parse (sottocomandi[1]);
                                if (EventoRxNumMaxSlave != null) EventoRxNumMaxSlave(this, nse);
                                break;
                            default:
                                cmderr(comandoricevuto);
                                break;
                        }
                        break;
                    case 3:
                        switch (sottocomandi[0])
                        {
                            case "v": // nuovo voto acquisito
                                VotoEventArgs vea = new VotoEventArgs();
                                vea.indirizzo   =  int.Parse(sottocomandi[1]);
                                vea.oravoto = uint.Parse(sottocomandi[2]);
                                if (EventoVotoAcquisito != null) EventoVotoAcquisito(this, vea);
                                break;
                            default:
                                cmderr(comandoricevuto);
                                break;
                        }
                        break;
                    case 4:
                        switch (sottocomandi[0])
                        {
                            case "d": // trovato nuovo client
                                DiscoveryEventArgs dea = new DiscoveryEventArgs();
                                dea.indirizzo = int.Parse(sottocomandi[1]);
                                dea.batteria  = int.Parse(sottocomandi[2]);
                                dea.rssi = int.Parse(sottocomandi[3]);
                                if (EventoNuovoClient != null) EventoNuovoClient(this, dea);
                                break;
                            default:
                                cmderr(comandoricevuto);
                                break;
                        }
                        break;
                    default:
                        cmderr(comandoricevuto);
                        break;


                }
            }
			catch (TimeoutException te)
			{
			}
			catch (InvalidOperationException ee)
			{
                cmderr(ee.Message);
				return;
			}
            catch(System.IO.IOException ioe)
            {
                return;
            }
		} while (mustexit == false);
	}
    void cmderr(String cmd)
    {
        MsgEventArgs mea = new MsgEventArgs();
        mea.msg = "Bad command: " + cmd;
        if(EventoNuovoMessaggio != null) EventoNuovoMessaggio(this, mea);


    }
}


