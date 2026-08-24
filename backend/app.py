"""
Smart Fish Vending Machine - Flask Application Factory
Main application entry point with database initialization and route registration.
"""

import os
import logging
from pathlib import Path
from flask import Flask, request, jsonify, render_template, make_response
from flask_cors import CORS
from dotenv import load_dotenv

# Load environment variables from .env file in project root
project_root = Path(__file__).resolve().parent.parent
load_dotenv(project_root / ".env")

try:
    from .models import db, Fish, Order, DailyRevenue, MachineStatus
except ImportError:
    from models import db, Fish, Order, DailyRevenue, MachineStatus
try:
    from .routes import api_bp
except ImportError:
    from routes import api_bp
try:
    from .mqtt_handler import mqtt_handler, init_mqtt
except ImportError:
    from mqtt_handler import mqtt_handler, init_mqtt
try:
    from .esp32_client import init_esp32
except ImportError:
    from esp32_client import init_esp32
from datetime import datetime

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler('fish_vending.log')
    ]
)
logger = logging.getLogger(__name__)


def create_app():
    """Application factory for creating Flask app."""
    app = Flask(__name__)

    # Configuration
    basedir = os.path.abspath(os.path.dirname(__file__))
    app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URL', f'sqlite:///{os.path.join(basedir, "fish_vending.db")}')
    app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
    app.config['SECRET_KEY'] = os.getenv('SECRET_KEY', 'dev-secret-key-change-in-production')

    # Initialize extensions
    db.init_app(app)
    CORS(app)

    # Register blueprints
    app.register_blueprint(api_bp, url_prefix='/api')

    # Create database tables
    with app.app_context():
        db.create_all()
        with db.engine.connect() as conn:
            result = conn.execute(db.text("PRAGMA table_info(fish)"))
            columns = [row[1] for row in result]
            if 'compartment' not in columns:
                conn.execute(db.text('ALTER TABLE fish ADD COLUMN compartment INTEGER DEFAULT 0'))
                conn.commit()
                logger.info("Added compartment column to fish table")
        seed_initial_data()

    # Initialize MQTT
    with app.app_context():
        init_mqtt(app)

    # Initialize ESP32 communication
    with app.app_context():
        init_esp32()
        logger.info(f"ESP32 configured at {os.getenv('ESP32_IP', '192.168.1.100')}:{os.getenv('ESP32_PORT', '80')}")

    # ==================== ESP32 INTEGRATION ====================

    @app.route('/dispense', methods=['POST'])
    def dispense_fish():
        """
        ESP32 integration endpoint.
        Receives dispense request and triggers fish dispensing.
        
        Expected JSON: {orderId, fishType, qty}
        Returns: {success, message, orderId}
        """
        try:
            data = request.get_json()
            if not data or not data.get('orderId') or not data.get('fishType'):
                return jsonify({'success': False, 'message': 'Missing required fields'}), 400

            order_id = data['orderId']
            fish_type = data.get('fishType')
            qty = int(data.get('qty', 1))

            # Validate order exists
            order = Order.query.get(order_id)
            if not order:
                return jsonify({'success': False, 'message': 'Order not found'}), 404

            # Update order status
            order.status = 'dispensing'
            db.session.commit()

            # Publish MQTT notification to Adafruit IO
            mqtt_handler.publish_order_notification(order_id, fish_type, qty)
            mqtt_handler.publish_inventory_update(fish_type, 
                Fish.query.filter_by(name=fish_type).first().quantity if Fish.query.filter_by(name=fish_type).first() else 0)

            # Return success to ESP32
            return jsonify({
                'success': True,
                'message': f'Dispensing {qty}x {fish_type}',
                'orderId': order_id,
                'fishType': fish_type,
                'quantity': qty
            }), 200

        except Exception as e:
            db.session.rollback()
            return jsonify({'success': False, 'message': str(e)}), 500

    @app.route('/esp32/status', methods=['POST'])
    def update_esp32_status():
        """
        Update machine status from ESP32 sensor data.
        
        Expected JSON: {online: bool, temperature: float, humidity: float}
        """
        try:
            data = request.get_json()
            status = MachineStatus.query.first()
            if not status:
                status = MachineStatus()
                db.session.add(status)

            status.is_online = data.get('online', True)
            status.temperature = data.get('temperature')
            status.humidity = data.get('humidity')
            status.updated_at = datetime.utcnow()

            db.session.commit()

            # Publish status to Adafruit IO
            mqtt_handler.publish_machine_status(
                status.is_online,
                status.temperature,
                status.humidity
            )

            return jsonify({'success': True, 'message': 'Status updated'}), 200

        except Exception as e:
            db.session.rollback()
            return jsonify({'success': False, 'message': str(e)}), 500

    @app.route('/health', methods=['GET'])
    def health_check():
        """Health check endpoint."""
        return jsonify({'status': 'healthy', 'service': 'fish-vending-api'}), 200

    @app.route('/api/info', methods=['GET'])
    def api_info():
        """API information endpoint."""
        return jsonify({
            'service': 'Smart Fish Vending Machine API',
            'version': '1.0.0',
            'status': 'running',
            'endpoints': {
                'health': '/health',
                'fish': '/api/fish',
                'orders': '/api/orders',
                'dashboard': '/api/dashboard/stats',
                'dispense': '/dispense',
                'esp32_status': '/esp32/status'
            },
            'documentation': '/api/docs'
        }), 200

    @app.route('/', methods=['GET'])
    def index():
        """Serve the main frontend application."""
        response = make_response(render_template('index.html'))
        response.headers['Cache-Control'] = 'no-store, no-cache, must-revalidate, max-age=0'
        response.headers['Pragma'] = 'no-cache'
        response.headers['Expires'] = '0'
        return response

    return app


def seed_initial_data():
    """Seed or update initial fish data and machine status."""
    initial_fish = [
        {'name': 'Tilapia', 'price': 220.00, 'quantity': 20,
         'image_url': 'https://images.unsplash.com/photo-1544943910-4c1dc44aab44?w=400',
         'description': 'Fresh tilapia, farm-raised and sustainably sourced.', 'compartment': 1},
        {'name': 'Salmon', 'price': 450.00, 'quantity': 15,
         'image_url': 'https://images.unsplash.com/photo-1599084993091-1cb5c0721cc6?w=400',
         'description': 'Premium Atlantic salmon, rich in omega-3 fatty acids.', 'compartment': 2},
        {'name': 'Trout', 'price': 320.00, 'quantity': 12,
         'image_url': 'https://images.unsplash.com/photo-1510130387422-82bed34b37e9?w=400',
         'description': 'Rainbow trout, freshly caught from mountain streams.', 'compartment': 3},
        {'name': 'Catfish', 'price': 180.00, 'quantity': 18,
         'image_url': 'https://images.unsplash.com/photo-1759419048034-e82d12086df1?w=400',
         'description': 'Southern-style catfish, clean and flavorful.', 'compartment': 4},
    ]
    for fish_data in initial_fish:
        fish = Fish.query.filter_by(name=fish_data['name']).first()
        if fish:
            fish.price = fish_data['price']
            fish.quantity = fish_data['quantity']
            fish.image_url = fish_data['image_url']
            fish.description = fish_data['description']
            if 'compartment' in fish_data:
                fish.compartment = fish_data['compartment']
        else:
            fish = Fish(**fish_data)
            db.session.add(fish)

    if MachineStatus.query.count() == 0:
        status = MachineStatus(is_online=True, temperature=22.0, humidity=45.0)
        db.session.add(status)

    db.session.commit()


if __name__ == '__main__':
    app = create_app()
    port = int(os.getenv('PORT', 5050))
    debug = os.getenv('FLASK_ENV') == 'development'
    app.run(host='0.0.0.0', port=port, debug=debug)
