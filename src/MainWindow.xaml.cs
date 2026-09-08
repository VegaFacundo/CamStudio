using System.Windows;
using CamStudio.Controllers;
using CamStudio.Models;
using CamStudio.Services;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace CamStudio;

public partial class MainWindow : Window
{
    private readonly ConnectionModel _model;
    private readonly ConnectionController _connectionController;
    private readonly H264Decoder _decoder;

    public MainWindow()
    {
        InitializeComponent();

        FFmpegService.Initialize();

        _model = new ConnectionModel();

        _connectionController =
            new ConnectionController(_model);

        _connectionController.ConnectionChanged +=
            OnConnectionChanged;

        _decoder = new H264Decoder();

        _connectionController.VideoFrameReceived +=
            OnVideoFrameReceived;
    }

    private async void ConnectButton_Click(
    object sender,
    RoutedEventArgs e)
    {
        if (_model.IsConnected)
        {
            _connectionController.Disconnect();
            return;
        }

        string ip = IpTextBox.Text.Trim();

        if (!int.TryParse(
                PortTextBox.Text,
                out int port))
        {
            StatusText.Text = "🔴 Puerto inválido";
            return;
        }

        StatusText.Text = "🟡 Conectando...";
        ConnectButton.IsEnabled = false;

        await _connectionController.ConnectAsync(
            ip,
            port
        );

        ConnectButton.IsEnabled = true;
    }

    private void OnConnectionChanged()
    {
        Dispatcher.Invoke(() =>
        {
            if (_model.IsConnected)
            {
                StatusText.Text = "🟢 Conectado";

                ClientText.Text =
                    $"Celular: {_model.ClientAddress}";

                ConnectButton.Content = "Desconectar";
                try
                {
                    bool initialized = Native.Initialize(1280, 720, 30);

                 
                }
                catch (Exception ex)
                {
                    MessageBox.Show(
                        ex.ToString(),
                        "ERROR Native.Initialize"
                    );
                }
            }
            else
            {
                StatusText.Text = "🔴 Desconectado";
                ClientText.Text = "Celular: -";

                ConnectButton.Content = "Conectar";
            }
        });
    }
    private long _frameCounter;

    private void OnVideoFrameReceived(
            byte[] frame,
            long pts,
            int flags
        )
    {
        _decoder.Decode(
            frame,
            pts,
            flags,
            bgrFrame =>
            {
                Dispatcher.Invoke(() =>
                {
                    ShowFrame(
                        bgrFrame,
                        _decoder.Width,
                        _decoder.Height
                    );
                });

                Native.WriteFrame(bgrFrame);
            }
        );
    }

    private void ShowFrame(
            byte[] bgr,
            int width,
            int height
        )
    {
        int stride = width * 3;

        BitmapSource bitmap =
            BitmapSource.Create(
                width,
                height,
                96,
                96,
                PixelFormats.Bgr24,
                null,
                bgr,
                stride
            );

        VideoPreview.Source = bitmap;
    }
}