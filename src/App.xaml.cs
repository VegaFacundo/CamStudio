using System.Configuration;
using System.Data;
using System.Windows;
using System;
using System.Linq;
using CamStudio.src.Services.CameraRegistration.Windows;

namespace CamStudio;

public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        if (e.Args.Any(arg =>
            string.Equals(
                arg,
                "--register-windows11-camera",
                StringComparison.OrdinalIgnoreCase)))
        {
            bool success =
                Windows11CameraRegistration.RegisterForElevatedProcess();

            Environment.Exit(success ? 0 : 1);
            return;
        }

        base.OnStartup(e);

        var window = new MainWindow();
        window.Show();
    }
}

