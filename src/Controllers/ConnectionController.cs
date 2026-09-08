using System.Buffers.Binary;
using System.Net.Sockets;
using CamStudio.Models;
using System.IO;

namespace CamStudio.Controllers;

public class ConnectionController
{
    private readonly ConnectionModel _model;

    private TcpClient? _client;
    private NetworkStream? _stream;

    public event Action? ConnectionChanged;

    public event Action<byte[], long, int>? VideoFrameReceived;

    public ConnectionController(ConnectionModel model)
    {
        _model = model;
    }

    public async Task ConnectAsync(string ip, int port)
    {
        try
        {
            _client = new TcpClient();

            await _client.ConnectAsync(ip, port);

            _stream = _client.GetStream();

            _model.IsConnected = true;
            _model.ClientAddress = $"{ip}:{port}";

            ConnectionChanged?.Invoke();

            _ = ReceiveLoopAsync();
        }
        catch
        {
            _model.IsConnected = false;
            _model.ClientAddress = null;

            ConnectionChanged?.Invoke();
        }
    }

    private async Task ReceiveLoopAsync()
    {
        try
        {
            while (_client?.Connected == true &&
                   _stream != null)
            {
                byte[] sizeBuffer = new byte[4];
                byte[] ptsBuffer = new byte[8];
                byte[] flagsBuffer = new byte[4];

                await ReadExactlyAsync(_stream, sizeBuffer);
                await ReadExactlyAsync(_stream, ptsBuffer);
                await ReadExactlyAsync(_stream, flagsBuffer);

                int size =
                    BinaryPrimitives.ReadInt32BigEndian(sizeBuffer);

                long pts =
                    BinaryPrimitives.ReadInt64BigEndian(ptsBuffer);

                int flags =
                    BinaryPrimitives.ReadInt32BigEndian(flagsBuffer);

                byte[] frame = new byte[size];

                await ReadExactlyAsync(_stream, frame);

                VideoFrameReceived?.Invoke(
                    frame,
                    pts,
                    flags
                );
            }
        }
        catch
        {
            Disconnect();
        }
    }

    private static async Task ReadExactlyAsync(
        NetworkStream stream,
        byte[] buffer)
    {
        int totalRead = 0;

        while (totalRead < buffer.Length)
        {
            int read = await stream.ReadAsync(
                buffer.AsMemory(totalRead)
            );

            if (read == 0)
            {
                throw new IOException(
                    "Remote connection closed."
                );
            }

            totalRead += read;
        }
    }

    public void Disconnect()
    {
        try
        {
            _stream?.Close();
            _stream?.Dispose();

            _client?.Close();
            _client?.Dispose();
        }
        finally
        {
            _stream = null;
            _client = null;

            _model.IsConnected = false;
            _model.ClientAddress = null;

            ConnectionChanged?.Invoke();
        }
    }
}