"""
Smart Fish Vending Machine - Backend Routes
REST API endpoints for customer website and admin dashboard.
"""

from flask import Blueprint, request, jsonify
from datetime import datetime, date
import logging

try:
    from .models import db, Fish, Order, DailyRevenue, MachineStatus
except ImportError:
    from models import db, Fish, Order, DailyRevenue, MachineStatus

try:
    from .esp32_client import esp32_client, get_compartment_for_fish
except ImportError:
    from esp32_client import esp32_client, get_compartment_for_fish

logger = logging.getLogger(__name__)

api_bp = Blueprint('api', __name__)


# ==================== FISH / INVENTORY ====================

@api_bp.route('/fish', methods=['GET'])
def get_all_fish():
    """Get all available fish types with current inventory."""
    try:
        fish_list = Fish.query.all()
        return jsonify({
            'success': True,
            'data': [fish.to_dict() for fish in fish_list]
        }), 200
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/fish/<int:fish_id>', methods=['GET'])
def get_fish(fish_id):
    """Get a specific fish by ID."""
    try:
        fish = Fish.query.get_or_404(fish_id)
        return jsonify({'success': True, 'data': fish.to_dict()}), 200
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 404


@api_bp.route('/fish', methods=['POST'])
def add_fish():
    """Add a new fish type to inventory (admin)."""
    try:
        data = request.get_json()
        if not data or not data.get('name') or data.get('price') is None:
            return jsonify({'success': False, 'error': 'Name and price are required'}), 400

        fish = Fish(
            name=data['name'],
            price=float(data['price']),
            quantity=int(data.get('quantity', 0)),
            image_url=data.get('image_url'),
            description=data.get('description'),
            compartment=int(data.get('compartment', 0))
        )
        db.session.add(fish)
        db.session.commit()
        return jsonify({'success': True, 'data': fish.to_dict()}), 201
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/fish/<int:fish_id>', methods=['PUT'])
def update_fish(fish_id):
    """Update fish inventory details (admin)."""
    try:
        fish = Fish.query.get_or_404(fish_id)
        data = request.get_json()

        if 'name' in data:
            fish.name = data['name']
        if 'price' in data:
            fish.price = float(data['price'])
        if 'quantity' in data:
            fish.quantity = int(data['quantity'])
        if 'image_url' in data:
            fish.image_url = data['image_url']
        if 'description' in data:
            fish.description = data['description']
        if 'compartment' in data:
            fish.compartment = int(data['compartment'])

        db.session.commit()
        return jsonify({'success': True, 'data': fish.to_dict()}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/fish/<int:fish_id>', methods=['DELETE'])
def delete_fish(fish_id):
    """Remove a fish type from inventory (admin)."""
    try:
        fish = Fish.query.get_or_404(fish_id)
        db.session.delete(fish)
        db.session.commit()
        return jsonify({'success': True, 'message': 'Fish deleted successfully'}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


# ==================== CART / CHECKOUT ====================

@api_bp.route('/checkout', methods=['POST'])
def checkout():
    """
    Process checkout and create order.
    Expected JSON: {items: [{fishId, name, qty, price}], customerName, customerPhone}
    """
    try:
        data = request.get_json()
        if not data or not data.get('items'):
            return jsonify({'success': False, 'error': 'Cart items are required'}), 400

        items = data['items']
        customer_name = data.get('customerName', 'Guest')
        customer_phone = data.get('customerPhone', '')

        # Validate inventory and calculate totals
        orders = []
        total_amount = 0.0

        for item in items:
            fish = Fish.query.get(item['fishId'])
            if not fish:
                return jsonify({'success': False, 'error': f'Fish with ID {item["fishId"]} not found'}), 404

            qty = int(item.get('qty', 1))
            if qty < 1:
                return jsonify({
                    'success': False,
                    'error': f'Invalid quantity for {fish.name}. Minimum quantity is 1.'
                }), 400

            if fish.quantity < qty:
                return jsonify({
                    'success': False,
                    'error': f'Insufficient stock for {fish.name}. Available: {fish.quantity}'
                }), 400

            item_total = fish.price * qty
            total_amount += item_total

            order = Order(
                id=Order.generate_order_id(),
                fish_type=fish.name,
                fish_name=fish.name,
                compartment=fish.compartment if fish.compartment else get_compartment_for_fish(fish.name),
                quantity=qty,
                total_price=item_total,
                status='processing',
                payment_status='pending',
                customer_name=customer_name,
                customer_phone=customer_phone
            )
            orders.append(order)

        # Create all orders
        for order in orders:
            db.session.add(order)

        db.session.commit()

        return jsonify({
            'success': True,
            'message': 'Order placed successfully',
            'orders': [order.to_dict() for order in orders],
            'total_amount': total_amount
        }), 201

    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/payment/process', methods=['POST'])
def process_payment():
    """
    Process fake payment and communicate with ESP32 for dispensing.
    Expected JSON: {orderId}
    """
    try:
        data = request.get_json()
        if not data or not data.get('orderId'):
            return jsonify({'success': False, 'error': 'Order ID is required'}), 400

        order = Order.query.get(data['orderId'])
        if not order:
            return jsonify({'success': False, 'error': 'Order not found'}), 404

        if order.payment_status == 'completed':
            return jsonify({'success': False, 'error': 'Payment already processed'}), 400

        if order.payment_status == 'failed':
            return jsonify({'success': False, 'error': 'Payment failed. Please try again or contact support.'}), 400

        # Get fish details for compartment mapping
        fish = Fish.query.filter_by(name=order.fish_name).first()
        if not fish:
            return jsonify({'success': False, 'error': 'Fish not found in inventory'}), 404

        # Check inventory availability
        if fish.quantity < order.quantity:
            return jsonify({
                'success': False,
                'error': f'Insufficient stock. Available: {fish.quantity}, Required: {order.quantity}'
            }), 400

        compartment = fish.compartment if fish and fish.compartment else get_compartment_for_fish(order.fish_name)

        # Send dispense request to ESP32
        logger.info(f"Sending dispense request to ESP32 for order {order.id}")
        esp32_response = esp32_client.send_dispense_request(
            order_id=order.id,
            compartment=compartment,
            fish_name=order.fish_name,
            qty=order.quantity
        )

        if esp32_response['success']:
            # ESP32 responded successfully - update database
            order.payment_status = 'completed'
            order.status = 'dispensing'
            order.updated_at = datetime.utcnow()

            db.session.commit()

            logger.info(f"Payment processed and order {order.id} sent to ESP32")

            return jsonify({
                'success': True,
                'message': 'Dispensing Started',
                'order': order.to_dict(),
                'esp32_response': esp32_response.get('data', {})
            }), 200
        else:
            # ESP32 failed or offline - mark payment as failed
            order.payment_status = 'failed'
            order.status = 'failed'
            order.updated_at = datetime.utcnow()
            db.session.commit()

            logger.error(f"ESP32 dispense failed for order {order.id}: {esp32_response.get('message')}")

            return jsonify({
                'success': False,
                'error': esp32_response.get('message', 'Dispensing Failed'),
                'order': order.to_dict(),
                'retry_allowed': True
            }), 503

    except Exception as e:
        db.session.rollback()
        logger.error(f"Payment processing error: {str(e)}")
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/order/<string:order_id>', methods=['GET'])
def get_order(order_id):
    """Get order details by ID."""
    try:
        order = Order.query.get_or_404(order_id)
        return jsonify({'success': True, 'data': order.to_dict()}), 200
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 404


@api_bp.route('/orders', methods=['GET'])
def get_all_orders():
    """Get all orders with optional filtering."""
    try:
        status_filter = request.args.get('status')
        query = Order.query

        if status_filter:
            query = query.filter_by(status=status_filter)

        orders = query.order_by(Order.created_at.desc()).all()
        return jsonify({
            'success': True,
            'data': [order.to_dict() for order in orders]
        }), 200
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/order/<string:order_id>/complete', methods=['POST'])
def complete_order(order_id):
    """
    Mark order as completed (called after ESP32 dispenses fish).
    Also updates inventory and daily revenue.
    """
    try:
        order = Order.query.get_or_404(order_id)

        # Prevent double completion
        if order.status == 'completed':
            return jsonify({'success': False, 'error': 'Order already completed'}), 400

        # Only allow completion if order is in dispensing state
        if order.status != 'dispensing':
            return jsonify({'success': False, 'error': f'Order is in {order.status} state, cannot complete'}), 400

        # Update inventory
        fish = Fish.query.filter_by(name=order.fish_name).first()
        if fish:
            fish.quantity = max(0, fish.quantity - order.quantity)

        # Update order status
        order.status = 'completed'
        order.updated_at = datetime.utcnow()

        # Update daily revenue
        today = date.today()
        daily_rev = DailyRevenue.query.filter_by(date=today).first()
        if not daily_rev:
            daily_rev = DailyRevenue(date=today, total_orders=0, total_revenue=0.0, total_fish_sold=0)
            db.session.add(daily_rev)

        daily_rev.total_orders += 1
        daily_rev.total_revenue += order.total_price
        daily_rev.total_fish_sold += order.quantity

        db.session.commit()

        return jsonify({
            'success': True,
            'message': 'Order completed successfully',
            'order': order.to_dict()
        }), 200

    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


# ==================== DASHBOARD / STATS ====================

@api_bp.route('/esp32/sensor-data', methods=['POST'])
def update_sensor_data():
    """Update machine sensor data from ESP32 (temperature, humidity, online status)."""
    try:
        data = request.get_json()
        if not data:
            return jsonify({'success': False, 'error': 'No data provided'}), 400

        status = MachineStatus.query.first()
        if not status:
            status = MachineStatus()
            db.session.add(status)

        status.is_online = data.get('online', status.is_online)
        status.temperature = data.get('temperature')
        status.humidity = data.get('humidity')
        status.updated_at = datetime.utcnow()

        db.session.commit()

        return jsonify({
            'success': True,
            'message': 'Sensor data updated',
            'data': status.to_dict()
        }), 200

    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/dashboard/stats', methods=['GET'])
def get_dashboard_stats():
    """Get overall statistics for admin dashboard."""
    try:
        total_revenue = db.session.query(db.func.sum(DailyRevenue.total_revenue)).scalar() or 0.0
        total_orders = Order.query.count()
        total_fish_sold = db.session.query(db.func.sum(DailyRevenue.total_fish_sold)).scalar() or 0

        fish_inventory = Fish.query.all()
        total_inventory = sum(fish.quantity for fish in fish_inventory)

        machine_status = MachineStatus.query.first()

        from datetime import datetime, timedelta
        now = datetime.utcnow()
        if machine_status and machine_status.updated_at:
            time_since_update = (now - machine_status.updated_at).total_seconds()
            if time_since_update > 60:
                is_online = False
            else:
                is_online = machine_status.is_online
        else:
            is_online = False

        return jsonify({
            'success': True,
            'data': {
                'total_revenue': round(total_revenue, 2),
                'total_orders': total_orders,
                'total_fish_sold': total_fish_sold,
                'available_inventory': total_inventory,
                'machine_online': is_online,
                'fish_types': len(fish_inventory),
                'machine_status': machine_status.to_dict() if machine_status else None
            }
        }), 200
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/dashboard/revenue', methods=['GET'])
def get_revenue_data():
    """Get revenue data for charts (last 7 days)."""
    try:
        from datetime import timedelta
        revenue_data = []
        for i in range(6, -1, -1):
            d = date.today() - timedelta(days=i)
            rev = DailyRevenue.query.filter_by(date=d).first()
            revenue_data.append({
                'date': d.isoformat(),
                'revenue': rev.total_revenue if rev else 0.0,
                'orders': rev.total_orders if rev else 0
            })

        return jsonify({'success': True, 'data': revenue_data}), 200
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/dashboard/fish-stats', methods=['GET'])
def get_fish_stats():
    """Get fish type statistics."""
    try:
        stats = []
        fish_list = Fish.query.all()
        for fish in fish_list:
            sold = db.session.query(db.func.sum(Order.quantity)).filter(
                Order.fish_name == fish.name,
                Order.status == 'completed'
            ).scalar() or 0
            stats.append({
                'name': fish.name,
                'available': fish.quantity,
                'sold': sold,
                'revenue': sold * fish.price
            })

        return jsonify({'success': True, 'data': stats}), 200
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/customer/history', methods=['GET'])
def get_customer_history():
    """Get customer order history."""
    try:
        orders = Order.query.order_by(Order.created_at.desc()).all()
        return jsonify({
            'success': True,
            'data': [order.to_dict() for order in orders]
        }), 200
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/order/<string:order_id>', methods=['DELETE'])
def delete_order(order_id):
    """Delete an order by ID."""
    try:
        order = Order.query.get_or_404(order_id)
        db.session.delete(order)
        db.session.commit()
        return jsonify({'success': True, 'message': 'Order deleted successfully'}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500


@api_bp.route('/admin/reset-database', methods=['POST'])
def reset_database():
    """Reset database: drops all tables and recreates with seed data. Admin only."""
    try:
        db.drop_all()
        db.create_all()

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
                fish.compartment = fish_data['compartment']
            else:
                fish = Fish(**fish_data)
                db.session.add(fish)

        if MachineStatus.query.count() == 0:
            status = MachineStatus(is_online=True, temperature=22.0, humidity=45.0)
            db.session.add(status)

        db.session.commit()
        return jsonify({'success': True, 'message': 'Database reset to default successfully'}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'success': False, 'error': str(e)}), 500
