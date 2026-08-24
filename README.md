# Smart Fish Vending Machine

A complete web application for a smart fish vending machine system with customer-facing web interface, admin dashboard, and ESP32 integration.

## 🏗️ Project Structure

```
fish_vending_machine/
├── backend/                    # Python Flask Backend
│   ├── app.py                  # Main Flask application
│   ├── models.py               # SQLAlchemy database models
│   ├── routes.py               # REST API routes
│   ├── mqtt_handler.py         # MQTT integration for Adafruit IO
│   ├── requirements.txt        # Python dependencies
│   └── templates/
│       └── index.html          # Admin & customer SPA template
├── database/
│   └── init_db.py              # Database initialization script
├── frontend/                   # React + Vite Frontend
│   ├── src/
│   │   ├── components/         # Reusable UI components
│   │   ├── pages/              # Page components
│   │   ├── services/
│   │   ├── App.jsx
│   │   ├── index.css
│   │   └── main.jsx
│   ├── package.json
│   ├── vite.config.js
│   └── index.html
├── esp32-code/                 # Arduino IDE sketch (.ino)
├── esp32-firmware/             # PlatformIO firmware
├── esp32-setup/                # ESP32 upload helpers
├── setup.sh                    # One-click setup script
├── run.sh                      # Start both servers
├── .env.example                # Environment template
└── README.md                   # This file
```

## 🚀 Quick Start (Recommended)

For student laptops, use the **one-click setup script**:

```bash
# 1. Clone the repository (or copy from disk)
git clone https://github.com/YOUR_USERNAME/fish_vending_machine.git
cd fish_vending_machine

# 2. Run setup (installs everything)
./setup.sh

# 3. Edit .env with your ESP32 IP and MQTT credentials
nano .env

# 4. Start the app
./run.sh
```

Then open:
- **Customer UI**: `http://localhost:3000`
- **Admin panel**: Navigate to Admin section from the navbar
- **API**: `http://localhost:5050`

## 📋 System Requirements

- **OS**: Ubuntu 20.04+ / Debian 11+ (or compatible Linux)
- **RAM**: 2 GB minimum, 4 GB recommended
- **Disk**: 1 GB free space
- **Network**: Same local network as ESP32 device

## 🐧 Linux Quick Reference

### Find Your Machine's IP Address

```bash
# Method 1: hostname (simplest)
hostname -I

# Method 2: ip command (most reliable)
ip addr show | grep "inet " | grep -v 127.0.0.1 | awk '{print $2}' | cut -d/ -f1

# Method 3: specific interface (e.g., wlan0 or eth0)
ip -4 addr show wlan0 | grep -oP '(?<=inet\s)\d+(\.\d+){3}'
```

### Check if Ports Are In Use

```bash
# Check if backend (5050) or frontend (3000) are already running
sudo ss -tlnp | grep -E '5050|3000'

# Or using netstat
sudo netstat -tlnp | grep -E '5050|3000'

# Kill a process on a specific port
sudo kill -9 $(sudo lsof -t -i:5050)
sudo kill -9 $(sudo lsof -t -i:3000)
```

### Test Network Connectivity

```bash
# Ping the ESP32 (replace with actual IP)
ping 192.168.1.100

# Test backend API
curl http://localhost:5050/api/fish

# Test ESP32 dispense endpoint
curl -X POST http://<ESP32_IP>/dispense -H "Content-Type: application/json" -d '{"orderId":"TEST","fishType":"Tilapia","qty":1}'
```

### View Logs

```bash
# If using run.sh (logs are saved to files)
tail -f backend.log
tail -f frontend.log

# If running manually in separate terminals, logs appear in the terminal
```

### Firewall Rules

```bash
# Allow ports if using ufw
sudo ufw allow 5050/tcp
sudo ufw allow 3000/tcp
sudo ufw allow 80/tcp   # For ESP32 web server
sudo ufw status
```

## 🛠️ Manual Installation

### Prerequisites

- Python 3.8+
- Node.js 16+
- npm or yarn

### Backend Setup

1. Navigate to the backend directory:
```bash
cd backend
```

2. Create a virtual environment:
```bash
python3 -m venv venv
source venv/bin/activate
```

3. Install dependencies:
```bash
pip install -r requirements.txt
```

4. Create a `.env` file (copy from `.env.example`):
```env
SECRET_KEY=dev-secret-key-change-in-production-12345
FLASK_ENV=development
PORT=5050
DATABASE_URL=sqlite:///fish_vending.db
MQTT_BROKER=io.adafruit.com
MQTT_PORT=1883
MQTT_USERNAME=your-adafruit-username
MQTT_PASSWORD=your-adafruit-io-key
ESP32_IP=192.168.1.100
ESP32_PORT=80
ESP32_TIMEOUT=30
ESP32_RETRIES=3
ESP32_RETRY_DELAY=5
FLASK_SERVER_IP=127.0.0.1
FLASK_SERVER_PORT=5050
CORS_ORIGIN=*
```

> **Important**: Set `FLASK_SERVER_IP` to your machine's actual IP address so the ESP32 can reach the backend.

5. Initialize the database:
```bash
python database/init_db.py
```

6. Run the Flask server:
```bash
python app.py
```

The backend will run on `http://localhost:5050`

### Frontend Setup

1. Navigate to the frontend directory:
```bash
cd frontend
```

2. Install dependencies:
```bash
npm install
```

3. Start the development server:
```bash
npm run dev -- --host
```

The frontend will run on `http://localhost:3000`

## 🔌 API Endpoints

### Fish/Inventory
- `GET /api/fish` - Get all fish types
- `GET /api/fish/:id` - Get specific fish
- `POST /api/fish` - Add new fish type
- `PUT /api/fish/:id` - Update fish details (name, price, quantity, image_url, compartment)
- `DELETE /api/fish/:id` - Remove fish type

### Cart/Checkout
- `POST /api/checkout` - Process checkout
- `POST /api/payment/process` - Process payment

### Orders
- `GET /api/orders` - Get all orders
- `GET /api/order/:id` - Get specific order
- `POST /api/order/:id/complete` - Complete order after dispensing
- `DELETE /api/order/:id` - Delete order

### Dashboard
- `GET /api/dashboard/stats` - Get overall statistics
- `GET /api/dashboard/revenue` - Get revenue data (last 7 days)
- `GET /api/dashboard/fish-stats` - Get fish statistics

### Admin
- `POST /api/admin/reset-database` - Reset database to default values (deletes all orders and revenue, resets inventory)
- `DELETE /api/fish/:id` - Delete a fish type from inventory

### ESP32
- `POST /dispense` - Trigger fish dispensing
- `POST /esp32/status` - Update machine status
- `POST /api/esp32/sensor-data` - Receive sensor data from ESP32

## 🔧 ESP32 Integration

The ESP32 sends POST requests to the `/dispense` endpoint:

```json
{
  "orderId": "ORD1001",
  "fishType": "Tilapia",
  "qty": 1
}
```

Expected response:
```json
{
  "success": true,
  "message": "Dispensing 1x Tilapia",
  "orderId": "ORD1001",
  "fishType": "Tilapia",
  "quantity": 1
}
```

### ESP32 Code Upload

See `esp32-setup/upload-instructions.md` for detailed steps.

**Quick Arduino CLI commands:**
```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32dev esp32-code/esp32_fish_vending/

# Upload
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32dev esp32-code/esp32_fish_vending/

# PlatformIO
cd esp32-firmware && pio run --target upload
```

## 📊 Database Schema

### Fish Table
- `id`: Primary key
- `name`: Fish name
- `price`: Price per unit
- `quantity`: Available stock
- `image_url`: Product image URL
- `description`: Fish description
- `compartment`: Compartment number (1-4)

### Orders Table
- `id`: Order ID (format: ORDXXXX)
- `fish_type`: Type of fish
- `fish_name`: Name of fish at time of order
- `quantity`: Number of units
- `total_price`: Order total
- `status`: Order status (pending/processing/dispensing/completed)
- `payment_status`: Payment status
- `customer_name`: Customer name
- `customer_phone`: Customer phone

### Daily Revenue Table
- `date`: Date of revenue
- `total_orders`: Number of orders
- `total_revenue`: Daily revenue amount
- `total_fish_sold`: Fish units sold

### Machine Status Table
- `is_online`: Machine online status
- `temperature`: Current temperature
- `humidity`: Current humidity
- `last_maintenance`: Last maintenance date

## 🐛 Troubleshooting

### Backend Won't Start
```bash
# Check if port 5050 is already in use
sudo ss -tlnp | grep 5050

# Verify Python virtual environment
source backend/venv/bin/activate
python app.py

# Check database permissions
ls -la backend/instance/
```

### Frontend Won't Start
```bash
# Check if port 3000 is in use
sudo ss -tlnp | grep 3000

# Reinstall node modules
cd frontend && rm -rf node_modules package-lock.json && npm install

# Clear Vite cache
cd frontend && rm -rf node_modules/.vite
```

### ESP32 Not Connecting
```bash
# Verify ESP32 is on the same network
ping <ESP32_IP>

# Check if backend is accessible from ESP32
curl http://<FLASK_SERVER_IP>:5050/api/fish

# Verify ESP32 web server responds
curl http://<ESP32_IP>/status

# Check firewall allows port 80 (ESP32) and 5050 (Flask)
sudo ufw status
```

### Database Issues
```bash
# Reset database (WARNING: deletes all data)
cd backend
source venv/bin/activate
python database/init_db.py reset

# Re-seed data
python database/init_db.py
```

### MQTT / Adafruit IO Issues
```bash
# MQTT is optional - app runs without it. To enable:
# 1. Create account at https://io.adafruit.com
# 2. Get username and AIO key
# 3. Update .env with credentials
```

### Common Issues on Student Laptops

| Issue | Solution |
|-------|----------|
| `npm: command not found` | Run `./setup.sh` to install Node.js, or manually install from nodesource |
| `python3: command not found` | Run `sudo apt install python3 python3-pip python3-venv` |
| `Permission denied` on run.sh | Run `chmod +x run.sh` |
| Port already in use | Kill existing process or change ports in `.env` and `vite.config.js` |
| Cannot access from phone | Use `--host` flag (already in `run.sh`), ensure same WiFi network |
| ESP32 timeout | Check firewall, verify IP addresses, ensure both on same network |

## 📝 License

This project is licensed under the MIT License.

## 👥 Authors

- Smart Fish Vending Machine Team

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Open a Pull Request
