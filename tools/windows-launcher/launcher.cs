/* Bloodborne PC launcher (exe twin of start_lazy.bat, made for adding to Steam).
 * Sets the same environment and starts scripts/run_windows.py with the portable Python. */
using System;
using System.Diagnostics;
using System.IO;

class BloodbornePCLauncher {
    static int Main(string[] args) {
        string root = AppDomain.CurrentDomain.BaseDirectory;
        string python = Path.Combine(root, "python", "python.exe");
        string game = Path.Combine(root, "Games");
        string runtime = Path.Combine(root, "runtime");

        if (!File.Exists(python)) { Wait("ERROR: portable Python not found:\n  " + python); return 1; }
        if (!File.Exists(Path.Combine(game, "eboot.bin"))) { Wait("ERROR: eboot.bin not found:\n  " + Path.Combine(game, "eboot.bin")); return 1; }
        if (!File.Exists(Path.Combine(runtime, "SDL3.dll"))) { Wait("ERROR: runtime DLLs not found:\n  " + runtime); return 1; }

        Environment.SetEnvironmentVariable("PYTHONUTF8", "1");
        Environment.SetEnvironmentVariable("PYTHONIOENCODING", "utf-8");
        Environment.SetEnvironmentVariable("BB_PREBUILT", "1");
        Environment.SetEnvironmentVariable("BB_DATA_DIR", root);
        Environment.SetEnvironmentVariable("BB_GAME_DIR", game);
        Environment.SetEnvironmentVariable("BB_PYTHON", python);
        Environment.SetEnvironmentVariable("BB_RUNTIME", runtime);
        Environment.SetEnvironmentVariable("PATH", runtime + ";" + Environment.GetEnvironmentVariable("PATH"));

        var psi = new ProcessStartInfo {
            FileName = python,
            Arguments = Quote(Path.Combine(root, "scripts", "run_game.py")) + " --game-dir " + Quote(game)
                        + (args.Length > 0 ? " " + string.Join(" ", Array.ConvertAll(args, Quote)) : ""),
            WorkingDirectory = root,
            UseShellExecute = false
        };
        Console.WriteLine("Bloodborne PC - exe launcher");
        Console.WriteLine("Root:    " + root);
        Console.WriteLine("Game:    " + game);
        Console.WriteLine("Python:  " + python);
        Console.WriteLine("Build:   PREBUILT");
        Console.WriteLine("Starting Bloodborne...");
        using (var p = Process.Start(psi)) {
            p.WaitForExit();
            int code = p.ExitCode;
            Console.WriteLine();
            Console.WriteLine("Bloodborne PC exited with code " + code);
            if (code != 0) Wait("");
            return code;
        }
    }

    static string Quote(string s) { return "\"" + s + "\""; }
    static void Wait(string msg) {
        if (msg.Length > 0) Console.WriteLine(msg);
        Console.WriteLine("Press Enter to close...");
        try { Console.ReadLine(); } catch (IOException) { }
    }
}
