#!/bin/bash
# Keysight Power Meter Control Script
# Cross-platform bash script for device management

# Configuration
POWER_METER_IP="100.65.16.193"
VISA_ADDRESS="TCPIP0::100.65.16.193::inst0::INSTR"
WEB_URL="http://$POWER_METER_IP/pm/index.html"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PYTHON_SCRIPT="$SCRIPT_DIR/keysight_manager.py"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging function
log() {
    local level="$1"
    local message="$2"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    
    case "$level" in
        "ERROR")   echo -e "[$timestamp] ${RED}✗ $message${NC}" ;;
        "SUCCESS") echo -e "[$timestamp] ${GREEN}✓ $message${NC}" ;;
        "WARNING") echo -e "[$timestamp] ${YELLOW}⚠ $message${NC}" ;;
        "INFO")    echo -e "[$timestamp] $message" ;;
        *)         echo -e "[$timestamp] $message" ;;
    esac
}

# Check if Python script exists
check_python_script() {
    if [[ ! -f "$PYTHON_SCRIPT" ]]; then
        log "ERROR" "Python script not found: $PYTHON_SCRIPT"
        return 1
    fi
    return 0
}

# Test network connectivity
test_network() {
    log "INFO" "Testing network connectivity to $POWER_METER_IP..."
    
    if ping -c 1 -W 5 "$POWER_METER_IP" >/dev/null 2>&1; then
        log "SUCCESS" "Network ping successful"
        return 0
    else
        log "ERROR" "Network ping failed"
        return 1
    fi
}

# Test web interface
test_web() {
    log "INFO" "Testing web interface at $WEB_URL..."
    
    if command -v curl >/dev/null 2>&1; then
        if curl -s --connect-timeout 5 "$WEB_URL" >/dev/null 2>&1; then
            log "SUCCESS" "Web interface responding"
            return 0
        else
            log "ERROR" "Web interface not responding"
            return 1
        fi
    elif command -v wget >/dev/null 2>&1; then
        if wget -q --timeout=5 --tries=1 --spider "$WEB_URL" >/dev/null 2>&1; then
            log "SUCCESS" "Web interface responding"
            return 0
        else
            log "ERROR" "Web interface not responding"
            return 1
        fi
    else
        log "WARNING" "Neither curl nor wget available - cannot test web interface"
        return 1
    fi
}

# Health check
health_check() {
    log "INFO" "=== Keysight Power Meter Health Check ==="
    
    if ! check_python_script; then
        return 1
    fi
    
    python3 "$PYTHON_SCRIPT" health
    return $?
}

# Device reset
device_reset() {
    log "INFO" "Performing device reset..."
    
    if ! check_python_script; then
        return 1
    fi
    
    python3 "$PYTHON_SCRIPT" reset
    local result=$?
    
    if [[ $result -eq 0 ]]; then
        log "SUCCESS" "Device reset completed"
    else
        log "ERROR" "Device reset failed"
    fi
    
    return $result
}

# Device recovery
device_recovery() {
    log "INFO" "Starting device recovery process..."
    
    if ! check_python_script; then
        return 1
    fi
    
    python3 "$PYTHON_SCRIPT" recover
    local result=$?
    
    if [[ $result -eq 0 ]]; then
        log "SUCCESS" "Device recovery completed"
    else
        log "ERROR" "Device recovery failed"
        log "WARNING" "Manual intervention may be required:"
        log "WARNING" "  1. Physical power cycle of the device"
        log "WARNING" "  2. Check network connections"
        log "WARNING" "  3. Restart network infrastructure if needed"
    fi
    
    return $result
}

# Continuous monitoring
monitor_device() {
    local interval=${1:-30}
    
    log "INFO" "Starting continuous monitoring (interval: ${interval}s)"
    log "INFO" "Press Ctrl+C to stop monitoring"
    
    local consecutive_failures=0
    local max_failures=3
    
    trap 'log "INFO" "Monitoring stopped by user"; exit 0' INT
    
    while true; do
        if health_check >/dev/null 2>&1; then
            consecutive_failures=0
            log "SUCCESS" "Device healthy"
        else
            ((consecutive_failures++))
            log "ERROR" "Device unhealthy (failure #$consecutive_failures)"
            
            if [[ $consecutive_failures -ge $max_failures ]]; then
                log "WARNING" "Multiple consecutive failures detected. Attempting recovery..."
                
                if device_recovery >/dev/null 2>&1; then
                    consecutive_failures=0
                    log "SUCCESS" "Automatic recovery successful"
                else
                    log "ERROR" "Automatic recovery failed. Continuing monitoring..."
                fi
            fi
        fi
        
        sleep "$interval"
    done
}

# Show help
show_help() {
    cat << EOF
KEYSIGHT POWER METER CONTROL UTILITY
====================================

USAGE:
    $0 [action] [options]

ACTIONS:
    health      Check device health status
    reset       Perform soft reset via VISA
    recover     Attempt full recovery process
    monitor     Start continuous monitoring
    network     Test network connectivity only
    web         Test web interface only
    help        Show this help message
    (no action) Show interactive menu

OPTIONS:
    For monitor action:
    [interval]  Monitoring interval in seconds (default: 30)

EXAMPLES:
    $0                    (Interactive menu)
    $0 health             (Quick health check)
    $0 reset              (Reset device)
    $0 monitor 60         (Monitor every 60 seconds)

DEVICE INFORMATION:
    IP Address:     $POWER_METER_IP
    VISA Address:   $VISA_ADDRESS
    Web Interface:  $WEB_URL

EOF
}

# Interactive menu
show_menu() {
    while true; do
        clear
        echo -e "${BLUE}============================================${NC}"
        echo -e "${BLUE}   Keysight Power Meter Control Utility${NC}"
        echo -e "${BLUE}============================================${NC}"
        echo ""
        echo -e "Device: ${YELLOW}$POWER_METER_IP${NC}"
        echo -e "Web Interface: ${YELLOW}$WEB_URL${NC}"
        echo ""
        echo "1. Health Check      - Check device status"
        echo "2. Quick Reset       - Soft reset via VISA"
        echo "3. Full Recovery     - Comprehensive recovery"
        echo "4. Start Monitoring  - Continuous health monitoring"
        echo "5. Network Test      - Test network connectivity"
        echo "6. Web Test          - Test web interface"
        echo "7. Help              - Show detailed usage"
        echo "8. Exit"
        echo ""
        
        read -p "Enter your choice (1-8): " choice
        
        case "$choice" in
            1)
                clear
                health_check
                read -p "Press Enter to continue..."
                ;;
            2)
                clear
                device_reset
                read -p "Press Enter to continue..."
                ;;
            3)
                clear
                device_recovery
                read -p "Press Enter to continue..."
                ;;
            4)
                clear
                read -p "Enter monitoring interval in seconds (default: 30): " interval
                interval=${interval:-30}
                monitor_device "$interval"
                ;;
            5)
                clear
                test_network
                read -p "Press Enter to continue..."
                ;;
            6)
                clear
                test_web
                read -p "Press Enter to continue..."
                ;;
            7)
                clear
                show_help
                read -p "Press Enter to continue..."
                ;;
            8)
                log "INFO" "Exiting..."
                exit 0
                ;;
            *)
                echo -e "${RED}Invalid choice. Please try again.${NC}"
                sleep 2
                ;;
        esac
    done
}

# Main execution
case "${1:-menu}" in
    "health")
        health_check
        exit $?
        ;;
    "reset")
        device_reset
        exit $?
        ;;
    "recover")
        device_recovery
        exit $?
        ;;
    "monitor")
        monitor_device "${2:-30}"
        ;;
    "network")
        test_network
        exit $?
        ;;
    "web")
        test_web
        exit $?
        ;;
    "help")
        show_help
        ;;
    "menu"|*)
        show_menu
        ;;
esac