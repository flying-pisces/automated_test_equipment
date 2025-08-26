# Keysight Power Meter Control PowerShell Script
# Advanced Windows management utility for device reboot and monitoring

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("health", "reset", "recover", "monitor", "help")]
    [string]$Action = "menu",
    
    [Parameter(Mandatory=$false)]
    [switch]$Verbose,
    
    [Parameter(Mandatory=$false)]
    [int]$MonitorInterval = 30
)

# Configuration
$PowerMeterIP = "100.65.16.193"
$VISAAddress = "TCPIP0::100.65.16.193::inst0::INSTR"
$WebURL = "http://$PowerMeterIP/pm/index.html"
$ScriptPath = Join-Path $PSScriptRoot "keysight_manager.py"

# Logging function
function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $color = switch ($Level) {
        "ERROR" { "Red" }
        "WARNING" { "Yellow" }
        "SUCCESS" { "Green" }
        default { "White" }
    }
    Write-Host "[$timestamp] $Message" -ForegroundColor $color
}

# Test network connectivity
function Test-NetworkConnectivity {
    Write-Log "Testing network connectivity to $PowerMeterIP..."
    
    try {
        $ping = Test-Connection -ComputerName $PowerMeterIP -Count 1 -Quiet -ErrorAction Stop
        if ($ping) {
            Write-Log "✓ Network ping successful" "SUCCESS"
            return $true
        } else {
            Write-Log "✗ Network ping failed" "ERROR"
            return $false
        }
    } catch {
        Write-Log "✗ Network test error: $($_.Exception.Message)" "ERROR"
        return $false
    }
}

# Test web interface
function Test-WebInterface {
    Write-Log "Testing web interface at $WebURL..."
    
    try {
        $response = Invoke-WebRequest -Uri $WebURL -TimeoutSec 5 -ErrorAction Stop
        if ($response.StatusCode -eq 200) {
            Write-Log "✓ Web interface responding" "SUCCESS"
            return $true
        } else {
            Write-Log "✗ Web interface returned status: $($response.StatusCode)" "ERROR"
            return $false
        }
    } catch {
        Write-Log "✗ Web interface test failed: $($_.Exception.Message)" "ERROR"
        return $false
    }
}

# Run Python health check
function Invoke-HealthCheck {
    Write-Log "Running comprehensive health check..."
    
    if (-not (Test-Path $ScriptPath)) {
        Write-Log "✗ Python script not found: $ScriptPath" "ERROR"
        return $false
    }
    
    try {
        $result = & python $ScriptPath health
        return $LASTEXITCODE -eq 0
    } catch {
        Write-Log "✗ Health check failed: $($_.Exception.Message)" "ERROR"
        return $false
    }
}

# Run Python reset
function Invoke-DeviceReset {
    Write-Log "Performing device reset..."
    
    if (-not (Test-Path $ScriptPath)) {
        Write-Log "✗ Python script not found: $ScriptPath" "ERROR"
        return $false
    }
    
    try {
        $result = & python $ScriptPath reset
        if ($LASTEXITCODE -eq 0) {
            Write-Log "✓ Device reset successful" "SUCCESS"
            return $true
        } else {
            Write-Log "✗ Device reset failed" "ERROR"
            return $false
        }
    } catch {
        Write-Log "✗ Reset command failed: $($_.Exception.Message)" "ERROR"
        return $false
    }
}

# Run Python recovery
function Invoke-DeviceRecovery {
    Write-Log "Starting device recovery process..."
    
    if (-not (Test-Path $ScriptPath)) {
        Write-Log "✗ Python script not found: $ScriptPath" "ERROR"
        return $false
    }
    
    try {
        $result = & python $ScriptPath recover
        if ($LASTEXITCODE -eq 0) {
            Write-Log "✓ Device recovery successful" "SUCCESS"
            return $true
        } else {
            Write-Log "✗ Device recovery failed" "ERROR"
            Write-Log "Manual intervention may be required:" "WARNING"
            Write-Log "  1. Physical power cycle of the device" "WARNING"
            Write-Log "  2. Check network connections" "WARNING"
            Write-Log "  3. Restart network infrastructure if needed" "WARNING"
            return $false
        }
    } catch {
        Write-Log "✗ Recovery command failed: $($_.Exception.Message)" "ERROR"
        return $false
    }
}

# Continuous monitoring
function Start-DeviceMonitoring {
    param([int]$IntervalSeconds = 30)
    
    Write-Log "Starting continuous monitoring (interval: $IntervalSeconds seconds)"
    Write-Log "Press Ctrl+C to stop monitoring"
    
    $consecutiveFailures = 0
    $maxFailures = 3
    
    try {
        while ($true) {
            $health = Invoke-HealthCheck
            
            if ($health) {
                $consecutiveFailures = 0
                Write-Log "✓ Device healthy" "SUCCESS"
            } else {
                $consecutiveFailures++
                Write-Log "✗ Device unhealthy (failure #$consecutiveFailures)" "ERROR"
                
                if ($consecutiveFailures -ge $maxFailures) {
                    Write-Log "Multiple consecutive failures detected. Attempting recovery..." "WARNING"
                    
                    if (Invoke-DeviceRecovery) {
                        $consecutiveFailures = 0
                        Write-Log "✓ Automatic recovery successful" "SUCCESS"
                    } else {
                        Write-Log "✗ Automatic recovery failed. Continuing monitoring..." "ERROR"
                    }
                }
            }
            
            Start-Sleep -Seconds $IntervalSeconds
        }
    } catch [System.Management.Automation.ParameterBindingException] {
        # User pressed Ctrl+C
        Write-Log "Monitoring stopped by user"
    }
}

# Interactive menu
function Show-InteractiveMenu {
    do {
        Clear-Host
        Write-Host "============================================" -ForegroundColor Cyan
        Write-Host "   Keysight Power Meter Control Utility" -ForegroundColor Cyan
        Write-Host "============================================" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "Device: $PowerMeterIP" -ForegroundColor Gray
        Write-Host "Web Interface: $WebURL" -ForegroundColor Gray
        Write-Host ""
        Write-Host "1. Health Check      - Check device status"
        Write-Host "2. Quick Reset       - Soft reset via VISA"
        Write-Host "3. Full Recovery     - Comprehensive recovery"
        Write-Host "4. Start Monitoring  - Continuous health monitoring"
        Write-Host "5. Network Test      - Test network connectivity only"
        Write-Host "6. Web Test          - Test web interface only"
        Write-Host "7. Help              - Show detailed usage"
        Write-Host "8. Exit"
        Write-Host ""
        
        $choice = Read-Host "Enter your choice (1-8)"
        
        switch ($choice) {
            "1" { 
                Clear-Host
                $result = Invoke-HealthCheck
                Write-Host ""
                Read-Host "Press Enter to continue"
            }
            "2" { 
                Clear-Host
                $result = Invoke-DeviceReset
                Write-Host ""
                Read-Host "Press Enter to continue"
            }
            "3" { 
                Clear-Host
                $result = Invoke-DeviceRecovery
                Write-Host ""
                Read-Host "Press Enter to continue"
            }
            "4" { 
                Clear-Host
                $interval = Read-Host "Enter monitoring interval in seconds (default: 30)"
                if ([string]::IsNullOrEmpty($interval)) { $interval = 30 }
                Start-DeviceMonitoring -IntervalSeconds [int]$interval
                Read-Host "Press Enter to continue"
            }
            "5" { 
                Clear-Host
                $result = Test-NetworkConnectivity
                Write-Host ""
                Read-Host "Press Enter to continue"
            }
            "6" { 
                Clear-Host
                $result = Test-WebInterface
                Write-Host ""
                Read-Host "Press Enter to continue"
            }
            "7" { 
                Clear-Host
                Show-Help
                Read-Host "Press Enter to continue"
            }
            "8" { 
                Write-Log "Exiting..."
                return
            }
            default { 
                Write-Host "Invalid choice. Please try again." -ForegroundColor Red
                Start-Sleep 2
            }
        }
    } while ($true)
}

# Help information
function Show-Help {
    Write-Host @"
KEYSIGHT POWER METER CONTROL UTILITY
====================================

USAGE:
    .\keysight_control.ps1 [action] [options]

ACTIONS:
    health      Check device health status
    reset       Perform soft reset via VISA
    recover     Attempt full recovery process
    monitor     Start continuous monitoring
    help        Show this help message
    (no action) Show interactive menu

OPTIONS:
    -Verbose            Show detailed output
    -MonitorInterval    Monitoring interval in seconds (default: 30)

EXAMPLES:
    .\keysight_control.ps1                    (Interactive menu)
    .\keysight_control.ps1 health             (Quick health check)
    .\keysight_control.ps1 reset -Verbose     (Reset with verbose output)
    .\keysight_control.ps1 monitor -MonitorInterval 60  (Monitor every 60 seconds)

DEVICE INFORMATION:
    IP Address:     $PowerMeterIP
    VISA Address:   $VISAAddress
    Web Interface:  $WebURL

TROUBLESHOOTING:
    If all automatic recovery methods fail:
    1. Physical power cycle of the device
    2. Check network cable connections
    3. Verify network switch/router status
    4. Check Windows firewall settings
    5. Contact system administrator

"@ -ForegroundColor White
}

# Main execution
switch ($Action.ToLower()) {
    "health" {
        $result = Invoke-HealthCheck
        exit $(if ($result) { 0 } else { 1 })
    }
    "reset" {
        $result = Invoke-DeviceReset
        exit $(if ($result) { 0 } else { 1 })
    }
    "recover" {
        $result = Invoke-DeviceRecovery
        exit $(if ($result) { 0 } else { 1 })
    }
    "monitor" {
        Start-DeviceMonitoring -IntervalSeconds $MonitorInterval
    }
    "help" {
        Show-Help
    }
    default {
        Show-InteractiveMenu
    }
}