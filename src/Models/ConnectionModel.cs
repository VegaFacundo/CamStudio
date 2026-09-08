namespace CamStudio.Models;

public class ConnectionModel
{
    public bool IsConnected { get; set; }

    public string? ClientAddress { get; set; }

    public int Port { get; set; } = 5000;
}