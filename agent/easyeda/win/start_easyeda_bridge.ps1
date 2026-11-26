# On-demand (non-resident) EasyEDA Pro HTTP MCP + SSH Reverse Tunnel launcher
# Verbose step-by-step terminal logging; never kills the EasyEDA Pro GUI window.

$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$scriptDir  = Split-Path -Parent $MyInvocation.MyCommand.Path
$configPath = Join-Path $scriptDir "bridge.config.local.ps1"
$mcpProc    = $null
$MCP_PORT   = 3030
$stdoutLog  = $null
$stderrLog  = $null

function Write-Step {
    param([string]$Step, [string]$Message)
    $ts = Get-Date -Format "HH:mm:ss"
    Write-Host "[$ts] $Step $Message" -ForegroundColor Cyan
}

function Write-Ok {
    param([string]$Message)
    Write-Host "           [OK]   $Message" -ForegroundColor Green
}

function Write-WarnLine {
    param([string]$Message)
    Write-Host "           [WARN] $Message" -ForegroundColor Yellow
}

function Write-ErrLine {
    param([string]$Message)
    Write-Host "           [FAIL] $Message" -ForegroundColor Red
}

function Test-LocalTcpPort {
    param([int]$Port)
    $client = $null
    try {
        $client = New-Object System.Net.Sockets.TcpClient
        $iar = $client.BeginConnect("127.0.0.1", $Port, $null, $null)
        if ($iar.AsyncWaitHandle.WaitOne(250, $false)) {
            $client.EndConnect($iar)
            return $true
        }
        return $false
    } catch {
        return $false
    } finally {
        if ($client) { $client.Dispose() }
    }
}

function Stop-ProcessTree {
    param([int]$RootPid)
    try {
        $children = Get-CimInstance Win32_Process -Filter "ParentProcessId = $RootPid" -ErrorAction SilentlyContinue
        foreach ($child in $children) {
            Stop-ProcessTree -RootPid $child.ProcessId
        }
    } catch {}
    try {
        $p = Get-Process -Id $RootPid -ErrorAction SilentlyContinue
        if ($p -and -not $p.HasExited) {
            Stop-Process -Id $RootPid -Force -ErrorAction SilentlyContinue
        }
    } catch {}
}

function Show-LogIfAny {
    param([string]$Label, [string]$Path)
    if ($Path -and (Test-Path $Path)) {
        try {
            $content = Get-Content -Path $Path -Raw -Encoding UTF8 -ErrorAction SilentlyContinue
            if ($content -and $content.Trim() -ne "") {
                Write-Host "           --- $Label ---" -ForegroundColor DarkGray
                $content.TrimEnd() -split "`r?`n" | ForEach-Object {
                    Write-Host "           $_" -ForegroundColor Gray
                }
            }
        } catch {}
    }
}

function Invoke-LocalHttpProbe {
    param([int]$Port, [string]$Token, [string]$Path)
    $url = "http://127.0.0.1:${Port}${Path}"
    $body = '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-03-26","capabilities":{},"clientInfo":{"name":"win-self-test","version":"0.1.0"}}}'
    $headers = @{
        "Accept" = "application/json, text/event-stream"
    }
    if ($Token) {
        $headers["Authorization"] = "Bearer $Token"
    }
    try {
        $resp = Invoke-WebRequest -UseBasicParsing -Method Post -Uri $url `
            -Headers $headers -ContentType "application/json" -Body $body -TimeoutSec 5
        return @{ Status = [int]$resp.StatusCode; Body = $resp.Content; Error = $null }
    } catch {
        $status = 0
        $errBody = $_.Exception.Message
        if ($_.Exception.Response) {
            try { $status = [int]$_.Exception.Response.StatusCode } catch {}
            try {
                $reader = New-Object System.IO.StreamReader($_.Exception.Response.GetResponseStream())
                $errBody = $reader.ReadToEnd()
                $reader.Dispose()
            } catch {}
        }
        return @{ Status = $status; Body = $errBody; Error = $_.Exception.Message }
    }
}

try {
    # ------------------------------------------------------------------
    # [1/5] Config & command check
    # ------------------------------------------------------------------
    Write-Step "[1/5]" "Loading config and checking commands..."
    if (-not (Test-Path $configPath)) {
        $suggestedToken = ([guid]::NewGuid().ToString("N") + [guid]::NewGuid().ToString("N")).Substring(0, 32)
        Write-ErrLine "Config file not found: $configPath"
        Write-Host "           Copy 'bridge.config.example.ps1' to 'bridge.config.local.ps1' and edit it."
        Write-Host "           Suggested random token: $suggestedToken" -ForegroundColor Yellow
        exit 1
    }

    . $configPath
    if (-not $MCP_PORT) { $MCP_PORT = 3030 }

    if (-not $UBUNTU_HOST -or $UBUNTU_HOST.Trim() -eq "") {
        Write-ErrLine "UBUNTU_HOST is empty in bridge.config.local.ps1."
        exit 1
    }
    if (-not $MCP_TOKEN -or $MCP_TOKEN -eq "REPLACE_WITH_RANDOM_TOKEN" -or $MCP_TOKEN.Length -lt 16) {
        Write-ErrLine "MCP_TOKEN must be set (16+ chars) in bridge.config.local.ps1."
        exit 1
    }

    $maskedToken = $MCP_TOKEN.Substring(0, 4) + ("*" * [Math]::Min(8, $MCP_TOKEN.Length - 4)) + " (len=$($MCP_TOKEN.Length))"
    $target = if ($UBUNTU_USER -and $UBUNTU_USER.Trim() -ne "") { "${UBUNTU_USER}@${UBUNTU_HOST}" } else { "$UBUNTU_HOST" }
    Write-Ok "Config loaded: target=$target, port=$MCP_PORT, token=$maskedToken"

    $easyedaCmd = Get-Command "easyeda-pro" -ErrorAction SilentlyContinue
    if (-not $easyedaCmd) {
        Write-ErrLine "'easyeda-pro' not found in PATH."
        exit 1
    }
    Write-Ok "easyeda-pro: $($easyedaCmd.Source)"

    $sshCmd = Get-Command "ssh" -ErrorAction SilentlyContinue
    if (-not $sshCmd) {
        Write-ErrLine "'ssh' not found in PATH."
        exit 1
    }
    Write-Ok "ssh: $($sshCmd.Source)"

    # ------------------------------------------------------------------
    # [2/5] EasyEDA Pro GUI check (doctor)
    # ------------------------------------------------------------------
    Write-Step "[2/5]" "Checking EasyEDA Pro GUI connection (easyeda-pro doctor)..."
    try {
        $doctorRaw = (easyeda-pro doctor | Out-String).Trim()
        Write-Host "           raw: $doctorRaw" -ForegroundColor Gray
        $doctorJson = $doctorRaw | ConvertFrom-Json
        if ($doctorJson.ok -and $doctorJson.value.connected) {
            Write-Ok "EasyEDA Pro GUI connected (bridgeVersion=$($doctorJson.value.bridgeVersion))"
        } else {
            Write-WarnLine "EasyEDA Pro GUI is NOT connected yet (connected=false)."
            Write-WarnLine "Launch EasyEDA Pro Desktop GUI so the agent can attach to an editor."
        }
    } catch {
        Write-WarnLine "Failed to parse 'easyeda-pro doctor': $($_.Exception.Message)"
    }

    # ------------------------------------------------------------------
    # [3/5] Start EasyEDA Pro HTTP MCP server on 127.0.0.1
    # ------------------------------------------------------------------
    Write-Step "[3/5]" "Starting EasyEDA Pro HTTP MCP on 127.0.0.1:$MCP_PORT ..."
    if (Test-LocalTcpPort -Port $MCP_PORT) {
        Write-ErrLine "Port 127.0.0.1:$MCP_PORT is already in use. Stop the existing process or change MCP_PORT."
        exit 1
    }

    $stdoutLog = Join-Path $env:TEMP "easyeda_mcp_$MCP_PORT.stdout.log"
    $stderrLog = Join-Path $env:TEMP "easyeda_mcp_$MCP_PORT.stderr.log"
    Remove-Item $stdoutLog, $stderrLog -Force -ErrorAction SilentlyContinue

    $mcpArgs = @("--mcp", "http", "--host", "127.0.0.1", "--port", "$MCP_PORT", "--token", "$MCP_TOKEN")
    Write-Host "           cmd: $($easyedaCmd.Source) --mcp http --host 127.0.0.1 --port $MCP_PORT --token <hidden>" -ForegroundColor Gray

    $mcpProc = Start-Process -FilePath $easyedaCmd.Source -ArgumentList $mcpArgs `
        -PassThru -WindowStyle Hidden `
        -RedirectStandardOutput $stdoutLog `
        -RedirectStandardError  $stderrLog

    $ready = $false
    for ($i = 0; $i -lt 40; $i++) {
        if (Test-LocalTcpPort -Port $MCP_PORT) {
            $ready = $true
            break
        }
        if ($mcpProc.HasExited) {
            break
        }
        Start-Sleep -Milliseconds 250
    }

    Show-LogIfAny -Label "MCP stdout" -Path $stdoutLog
    Show-LogIfAny -Label "MCP stderr" -Path $stderrLog

    if (-not $ready) {
        $exitInfo = if ($mcpProc.HasExited) { "exited with code $($mcpProc.ExitCode)" } else { "still running (PID=$($mcpProc.Id)) but port did not open" }
        Write-ErrLine "MCP server failed to listen on 127.0.0.1:$MCP_PORT ($exitInfo)."
        exit 1
    }
    Write-Ok "MCP server is listening on 127.0.0.1:$MCP_PORT (PID=$($mcpProc.Id), HasExited=$($mcpProc.HasExited))"

    # ------------------------------------------------------------------
    # [4/5] Local HTTP self-test on Windows (before SSH)
    # ------------------------------------------------------------------
    Write-Step "[4/5]" "Running local MCP HTTP self-test on Windows (127.0.0.1:$MCP_PORT)..."
    $noAuth = Invoke-LocalHttpProbe -Port $MCP_PORT -Token "" -Path "/mcp"
    Write-Host "           No-token POST /mcp  -> HTTP $($noAuth.Status)" -ForegroundColor Gray

    $selfTestOk = $false
    foreach ($tryPath in @("/mcp", "/")) {
        $authRes = Invoke-LocalHttpProbe -Port $MCP_PORT -Token $MCP_TOKEN -Path $tryPath
        $preview = if ($authRes.Body) { $authRes.Body.Substring(0, [Math]::Min(200, $authRes.Body.Length)) } else { "" }
        Write-Host "           Auth POST $tryPath -> HTTP $($authRes.Status) | $preview" -ForegroundColor Gray
        if ($authRes.Status -eq 200) {
            Write-Ok "Windows local MCP self-test passed on path '$tryPath'!"
            $selfTestOk = $true
            break
        }
    }
    if (-not $selfTestOk) {
        Write-WarnLine "Windows local self-test did not return HTTP 200 (continuing to SSH so you can inspect)."
    }

    # ------------------------------------------------------------------
    # [5/5] Open SSH reverse tunnel (verbose so tunnel & traffic show up)
    # ------------------------------------------------------------------
    Write-Step "[5/5]" "Opening SSH reverse tunnel to $target (remote 127.0.0.1:$MCP_PORT -> local 127.0.0.1:$MCP_PORT)..."
    $sshArgs = @(
        "-N",
        "-o", "LogLevel=VERBOSE",
        "-o", "ExitOnForwardFailure=yes",
        "-o", "ServerAliveInterval=30",
        "-o", "ServerAliveCountMax=3",
        "-R", "127.0.0.1:${MCP_PORT}:127.0.0.1:${MCP_PORT}"
    )
    if ($SSH_PORT -and "$SSH_PORT".Trim() -ne "") {
        $sshArgs += @("-p", "$SSH_PORT")
    }
    if ($SSH_KEY -and $SSH_KEY.Trim() -ne "") {
        $sshArgs += @("-i", $SSH_KEY)
    }
    $sshArgs += $target

    Write-Host "           cmd: ssh $($sshArgs -join ' ')" -ForegroundColor Gray
    Write-Host "-----------------------------------------------------------------" -ForegroundColor DarkCyan
    Write-Host " Waiting for SSH verbose output ('remote forward success' = OK)." -ForegroundColor Green
    Write-Host " Press Ctrl+C in this window to stop the bridge cleanly." -ForegroundColor Yellow
    Write-Host "-----------------------------------------------------------------" -ForegroundColor DarkCyan

    & $sshCmd.Source @sshArgs
    $sshExit = $LASTEXITCODE
    Write-WarnLine "SSH exited with code $sshExit."
} catch {
    Write-ErrLine "Unhandled error: $($_.Exception.Message)"
    Write-Host $_.ScriptStackTrace -ForegroundColor DarkRed
    exit 1
} finally {
    Write-Host ""
    Write-Step "[CLEANUP]" "Stopping MCP server started by this launcher..."
    Show-LogIfAny -Label "MCP stdout (final)" -Path $stdoutLog
    Show-LogIfAny -Label "MCP stderr (final)" -Path $stderrLog

    if ($mcpProc) {
        Write-Host "           Stopping PID tree rooted at PID=$($mcpProc.Id)..." -ForegroundColor Gray
        Stop-ProcessTree -RootPid $mcpProc.Id
    }
    Start-Sleep -Milliseconds 300
    if (Test-LocalTcpPort -Port $MCP_PORT) {
        Write-ErrLine "Port 127.0.0.1:$MCP_PORT is STILL open! Check Task Manager for lingering MCP process."
    } else {
        Write-Ok "Port 127.0.0.1:$MCP_PORT is closed (non-resident verified)."
    }
}
