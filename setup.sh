#!/bin/bash
set -e

# ============================================================
# Smart Fish Vending Machine — One-Click Setup Script
# For Ubuntu / Debian-based Linux laptops
# ============================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_step() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

log_ok() {
    echo -e "${GREEN}✓ $1${NC}"
}

log_warn() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

log_err() {
    echo -e "${RED}✗ $1${NC}"
}

# ---- Preflight checks ----
if [ "$EUID" -eq 0 ]; then
    log_err "Please do NOT run this script as root. Run it as a normal user with sudo privileges."
    exit 1
fi

if [ ! -f /etc/os-release ]; then
    log_err "Cannot detect OS. This script supports Ubuntu/Debian-based systems."
    exit 1
fi

. /etc/os-release
if [[ "$ID" != "ubuntu" && "$ID" != "debian" && "$ID_LIKE" != *"ubuntu"* && "$ID_LIKE" != *"debian"* ]]; then
    log_warn "OS detected: $ID. This script is designed for Ubuntu/Debian. Proceeding anyway..."
fi

# ---- Detect machine IP ----
detect_ip() {
    ip route get 1.1.1.1 2>/dev/null | awk '{print $7}' | head -1
}
MACHINE_IP=$(detect_ip)
if [ -z "$MACHINE_IP" ]; then
    MACHINE_IP=$(hostname -I 2>/dev/null | awk '{print $1}')
fi
if [ -z "$MACHINE_IP" ]; then
    MACHINE_IP="127.0.0.1"
fi

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Smart Fish Vending Machine Setup${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "Detected machine IP: ${YELLOW}$MACHINE_IP${NC}"

# ---- Step 1: System dependencies ----
log_step "[1/7] Installing system dependencies"
sudo apt update -y
sudo apt install -y python3 python3-pip python3-venv git curl build-essential

# Install Node.js 18.x if missing or too old
if ! command -v node &> /dev/null; then
    log_warn "Node.js not found. Installing Node.js 18.x..."
    curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
    sudo apt install -y nodejs
else
    NODE_MAJOR=$(node -v | cut -d'.' -f1 | tr -d 'v')
    if [ "$NODE_MAJOR" -lt 16 ]; then
        log_warn "Node.js version $(node -v) is too old. Upgrading to 18.x..."
        curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
        sudo apt install -y nodejs
    else
        log_ok "Node.js $(node -v) already installed"
    fi
fi

# Install Arduino CLI (optional, for ESP32)
if ! command -v arduino-cli &> /dev/null; then
    log_warn "Installing Arduino CLI (for ESP32 programming)..."
    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh
    export PATH="$HOME/.local/bin:$PATH"
    arduino-cli core update-index || true
    arduino-cli core install esp32:esp32 || true
    log_ok "Arduino CLI installed"
else
    log_ok "Arduino CLI already installed"
fi

# ---- Step 2: Get project code ----
log_step "[2/7] Fetching project code"
REPO_DIR="fish_vending_machine"

if [ -d "$REPO_DIR/.git" ]; then
    log_warn "Repository already exists. Pulling latest changes..."
    cd "$REPO_DIR"
    git pull || true
    cd ..
else
    REPO_URL="${GITHUB_REPO_URL:-https://github.com/YOUR_USERNAME/fish_vending_machine.git}"
    if [ -d "$REPO_DIR" ] && [ ! -d "$REPO_DIR/.git" ]; then
        log_warn "Directory exists but is not a git repo. Removing and re-cloning..."
        rm -rf "$REPO_DIR"
    fi
    echo -e "Cloning from: ${YELLOW}$REPO_URL${NC}"
    git clone "$REPO_URL" "$REPO_DIR"
fi
log_ok "Project code ready at ./$REPO_DIR"
cd "$REPO_DIR"

# ---- Step 3: Backend setup ----
log_step "[3/7] Setting up Python backend"
cd backend

if [ ! -d "venv" ]; then
    python3 -m venv venv
fi

source venv/bin/activate
pip install --upgrade pip --quiet
pip install -r requirements.txt --quiet
log_ok "Python dependencies installed"

cd ..

# Create .env if missing
if [ ! -f ".env" ]; then
    cp .env.example .env
    log_warn "Created .env from .env.example"
fi

# Auto-detect and set IPs in .env
if grep -q "FLASK_SERVER_IP=" .env; then
    sed -i "s|^FLASK_SERVER_IP=.*|FLASK_SERVER_IP=$MACHINE_IP|" .env
fi
if grep -q "^ESP32_IP=" .env && [ "$ESP32_IP" != "192.168.1.100" ]; then
    :
else
    if grep -q "^ESP32_IP=" .env; then
        sed -i "s|^ESP32_IP=.*|ESP32_IP=192.168.1.100|" .env
    fi
fi
log_ok ".env configured with detected IP: $MACHINE_IP"

# Initialize database
python database/init_db.py
log_ok "Database initialized"
deactivate

# ---- Step 4: Frontend setup ----
log_step "[4/7] Setting up React frontend"
cd frontend
npm install
log_ok "Node modules installed"
cd ..

# ---- Step 5: Firewall / Ports ----
log_step "[5/7] Configuring firewall"
if command -v ufw &> /dev/null && sudo ufw status | grep -q "active"; then
    sudo ufw allow 5050/tcp >/dev/null 2>&1 || true
    sudo ufw allow 3000/tcp >/dev/null 2>&1 || true
    log_ok "Firewall rules added (ports 5050, 3000)"
else
    log_warn "UFW not active. Skipping firewall configuration."
fi

# ---- Step 6: Create run script ----
log_step "[6/7] Creating run script"
cat > run.sh << 'RUNEOF'
#!/bin/bash
cd "$(dirname "$0")"

detect_ip() {
    ip route get 1.1.1.1 2>/dev/null | awk '{print $7}' | head -1
}
MACHINE_IP=$(detect_ip)
if [ -z "$MACHINE_IP" ]; then
    MACHINE_IP=$(hostname -I 2>/dev/null | awk '{print $1}')
fi
if [ -z "$MACHINE_IP" ]; then
    MACHINE_IP="127.0.0.1"
fi

BACKEND_PID=""
FRONTEND_PID=""

cleanup() {
    echo ""
    echo "Stopping servers..."
    [ -n "$BACKEND_PID" ] && kill "$BACKEND_PID" 2>/dev/null || true
    [ -n "$FRONTEND_PID" ] && kill "$FRONTEND_PID" 2>/dev/null || true
    exit 0
}
trap cleanup SIGINT SIGTERM EXIT

echo "========================================"
echo "  Smart Fish Vending Machine"
echo "========================================"
echo "Starting backend (Flask) on http://$MACHINE_IP:5050 ..."
cd backend
source venv/bin/activate
python app.py > ../backend.log 2>&1 &
BACKEND_PID=$!
cd ..

sleep 3

echo "Starting frontend (Vite) on http://$MACHINE_IP:3000 ..."
cd frontend
npm run dev -- --host > ../frontend.log 2>&1 &
FRONTEND_PID=$!
cd ..

sleep 2

echo ""
echo "Servers are running!"
echo "  Customer UI : http://$MACHINE_IP:3000"
echo "  Admin panel : http://$MACHINE_IP:3000 (navigate to Admin)"
echo "  API         : http://$MACHINE_IP:5050"
echo ""
echo "Logs: backend.log | frontend.log"
echo "Press Ctrl+C to stop."
echo ""

wait
RUNEOF

chmod +x run.sh
log_ok "run.sh created"

# ---- Step 7: ESP32 helper ----
log_step "[7/7] ESP32 setup helper"
mkdir -p esp32-setup
cat > esp32-setup/upload-instructions.md << 'EOF'
# ESP32 Upload Instructions

## Using Arduino IDE
1. Open Arduino IDE
2. Go to File → Open and select `esp32-code/esp32_fish_vending/esp32_fish_vending.ino`
3. Update WiFi SSID and Password in the code
4. Update Flask server IP (BACKEND_SERVER_IP) to match your machine's IP
5. Select Board: ESP32 Dev Module
6. Click Upload

## Using Arduino CLI
arduino-cli compile --fqbn esp32:esp32:esp32dev esp32-code/esp32_fish_vending/
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32dev esp32-code/esp32_fish_vending/

## Using PlatformIO
cd esp32-firmware
pio run --target upload

## Verify Connection
curl http://<ESP32_IP>/status
EOF

log_ok "ESP32 instructions saved to esp32-setup/upload-instructions.md"

# ---- Final summary ----
echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Setup Complete!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "To start the application:"
echo -e "  ${YELLOW}cd $REPO_DIR && ./run.sh${NC}"
echo ""
echo -e "Access URLs:"
echo -e "  Customer UI : ${YELLOW}http://$MACHINE_IP:3000${NC}"
echo -e "  Admin panel : ${YELLOW}http://$MACHINE_IP:3000${NC} (navigate to Admin)"
echo -e "  API         : ${YELLOW}http://$MACHINE_IP:5050${NC}"
echo ""
echo -e "${YELLOW}Before first run:${NC}"
echo "  1. Edit .env with your ESP32 IP and MQTT credentials"
echo "  2. Ensure ESP32 is connected to the same network"
echo "  3. Upload ESP32 code from esp32-code/ folder"
echo ""
echo -e "${YELLOW}Quick commands:${NC}"
echo "  Find IP      : hostname -I"
echo "  Check ports  : sudo ss -tlnp | grep -E '5050|3000'"
echo "  View logs    : tail -f backend.log frontend.log"
echo "  Stop servers : pkill -f 'python app.py' && pkill -f 'npm run dev'"
echo ""
echo -e "${GREEN}Happy vending! 🐟${NC}"
