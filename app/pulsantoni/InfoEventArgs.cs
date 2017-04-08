using System;
public class MsgEventArgs : EventArgs
{
	public String msg { get; set; }
}

public class DiscoveryEventArgs : EventArgs
{
    public int indirizzo { get; set; }
    public int batteria { get; set; }
    public int rssi { get; set; }

}

public class VotoEventArgs : EventArgs
{
    public int indirizzo { get; set; }
    public uint oravoto { get; set; }
}

public class NumSlaveEventArgs : EventArgs
{
    public int numslave { get; set; }
}

