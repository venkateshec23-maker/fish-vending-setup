"""
Smart Fish Vending Machine - Backend Models
Defines SQLAlchemy models for inventory, orders, and revenue tracking.
"""

from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import uuid

db = SQLAlchemy()


class Fish(db.Model):
    """Fish inventory model representing available fish types in the vending machine."""
    __tablename__ = 'fish'

    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False, unique=True)
    price = db.Column(db.Float, nullable=False)
    quantity = db.Column(db.Integer, nullable=False, default=0)
    image_url = db.Column(db.String(255), nullable=True)
    description = db.Column(db.Text, nullable=True)
    compartment = db.Column(db.Integer, nullable=False, default=0)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

    def to_dict(self):
        """Convert fish object to dictionary for JSON serialization."""
        return {
            'id': self.id,
            'name': self.name,
            'price': self.price,
            'quantity': self.quantity,
            'image_url': self.image_url,
            'description': self.description,
            'compartment': self.compartment,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }


class Order(db.Model):
    """Order model tracking customer purchases."""
    __tablename__ = 'orders'

    id = db.Column(db.String(20), primary_key=True)
    fish_type = db.Column(db.String(100), nullable=False)
    fish_name = db.Column(db.String(100), nullable=False)
    compartment = db.Column(db.Integer, nullable=True)
    quantity = db.Column(db.Integer, nullable=False)
    total_price = db.Column(db.Float, nullable=False)
    status = db.Column(db.String(50), default='pending')
    payment_status = db.Column(db.String(50), default='pending')
    customer_name = db.Column(db.String(100), nullable=True)
    customer_phone = db.Column(db.String(20), nullable=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

    def to_dict(self):
        """Convert order object to dictionary for JSON serialization."""
        return {
            'id': self.id,
            'fish_type': self.fish_type,
            'fish_name': self.fish_name,
            'compartment': self.compartment,
            'quantity': self.quantity,
            'total_price': self.total_price,
            'status': self.status,
            'payment_status': self.payment_status,
            'customer_name': self.customer_name,
            'customer_phone': self.customer_phone,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }

    @staticmethod
    def generate_order_id():
        """Generate unique order ID in format ORDXXXX."""
        return f"ORD{uuid.uuid4().int % 10000:04d}"


class DailyRevenue(db.Model):
    """Daily revenue tracking model."""
    __tablename__ = 'daily_revenue'

    id = db.Column(db.Integer, primary_key=True)
    date = db.Column(db.Date, nullable=False, unique=True)
    total_orders = db.Column(db.Integer, default=0)
    total_revenue = db.Column(db.Float, default=0.0)
    total_fish_sold = db.Column(db.Integer, default=0)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    def to_dict(self):
        """Convert daily revenue object to dictionary."""
        return {
            'id': self.id,
            'date': self.date.isoformat() if self.date else None,
            'total_orders': self.total_orders,
            'total_revenue': self.total_revenue,
            'total_fish_sold': self.total_fish_sold
        }


class MachineStatus(db.Model):
    """Machine status model for tracking ESP32 and sensor status."""
    __tablename__ = 'machine_status'

    id = db.Column(db.Integer, primary_key=True)
    is_online = db.Column(db.Boolean, default=False)
    temperature = db.Column(db.Float, nullable=True)
    humidity = db.Column(db.Float, nullable=True)
    last_maintenance = db.Column(db.DateTime, nullable=True)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

    def to_dict(self):
        """Convert machine status object to dictionary."""
        return {
            'id': self.id,
            'is_online': self.is_online,
            'temperature': self.temperature,
            'humidity': self.humidity,
            'last_maintenance': self.last_maintenance.isoformat() if self.last_maintenance else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }
