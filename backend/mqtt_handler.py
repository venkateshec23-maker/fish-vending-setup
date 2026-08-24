"""
Smart Fish Vending Machine - MQTT Handler for Adafruit IO
Handles publishing sensor data and order notifications to Adafruit IO.
"""

import paho.mqtt.client as mqtt
import json
import os
from datetime import datetime


class MQTTHandler:
    """
    MQTT client for communicating with Adafruit IO.
    Publishes machine status, sensor data, and order notifications.
    """

    def __init__(self):
        """Initialize MQTT client with Adafruit IO credentials."""
        self.broker = os.getenv('MQTT_BROKER', 'io.adafruit.com')
        self.port = int(os.getenv('MQTT_PORT', 1883))
        self.username = os.getenv('MQTT_USERNAME', '')
        self.password = os.getenv('MQTT_PASSWORD', '')
        self.client = mqtt.Client(client_id="FishVendingMachine")
        self.connected = False

        if self.username and self.password:
            self.client.username_pw_set(self.username, self.password)
            self.client.on_connect = self._on_connect
            self.client.on_disconnect = self._on_disconnect
        else:
            print("Warning: MQTT credentials not found in environment variables")

    def _on_connect(self, client, userdata, flags, rc):
        """Callback for successful MQTT connection."""
        if rc == 0:
            self.connected = True
            print(f"Connected to MQTT broker: {self.broker}")
        else:
            print(f"Failed to connect to MQTT broker. Return code: {rc}")

    def _on_disconnect(self, client, userdata, rc):
        """Callback for MQTT disconnection."""
        self.connected = False
        print(f"Disconnected from MQTT broker. Return code: {rc}")

    def connect(self):
        """Connect to MQTT broker."""
        if not self.username or not self.password:
            print("Cannot connect to MQTT: credentials missing")
            return False
        try:
            self.client.connect(self.broker, self.port, 60)
            self.client.loop_start()
            return True
        except Exception as e:
            print(f"MQTT connection error: {e}")
            return False

    def disconnect(self):
        """Disconnect from MQTT broker."""
        try:
            self.client.loop_stop()
            self.client.disconnect()
            self.connected = False
        except Exception as e:
            print(f"MQTT disconnection error: {e}")

    def publish(self, topic, payload, retain=False):
        """
        Publish message to MQTT topic.
        
        Args:
            topic: MQTT topic string
            payload: Message payload (dict or string)
            retain: Whether to retain the message on the broker
        """
        if not self.connected:
            print(f"Cannot publish: not connected to MQTT broker")
            return False

        try:
            if isinstance(payload, dict):
                payload = json.dumps(payload)

            result = self.client.publish(topic, payload, retain=retain)
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                print(f"Published to {topic}: {payload}")
                return True
            else:
                print(f"Failed to publish to {topic}")
                return False
        except Exception as e:
            print(f"Publish error: {e}")
            return False

    def publish_order_notification(self, order_id, fish_type, quantity):
        """
        Publish order notification to Adafruit IO.
        
        Args:
            order_id: Unique order ID
            fish_type: Type of fish ordered
            quantity: Quantity ordered
        """
        topic = f"{self.username}/feeds/fish-vending/order"
        payload = {
            "orderId": order_id,
            "fishType": fish_type,
            "quantity": quantity,
            "timestamp": datetime.utcnow().isoformat()
        }
        return self.publish(topic, payload)

    def publish_machine_status(self, is_online, temperature=None, humidity=None):
        """
        Publish machine status to Adafruit IO.
        
        Args:
            is_online: Whether the machine is online
            temperature: Current temperature (optional)
            humidity: Current humidity (optional)
        """
        topic = f"{self.username}/feeds/fish-vending/status"
        payload = {
            "online": is_online,
            "temperature": temperature,
            "humidity": humidity,
            "timestamp": datetime.utcnow().isoformat()
        }
        return self.publish(topic, payload)

    def publish_inventory_update(self, fish_name, remaining_quantity):
        """
        Publish inventory update to Adafruit IO.
        
        Args:
            fish_name: Name of the fish
            remaining_quantity: Remaining quantity
        """
        topic = f"{self.username}/feeds/fish-vending/inventory"
        payload = {
            "fish": fish_name,
            "remaining": remaining_quantity,
            "timestamp": datetime.utcnow().isoformat()
        }
        return self.publish(topic, payload)


# Global MQTT handler instance
mqtt_handler = MQTTHandler()


def init_mqtt(app):
    """Initialize MQTT with Flask app context."""
    with app.app_context():
        mqtt_handler.connect()
        # Publish initial machine status
        try:
            from .models import MachineStatus
        except ImportError:
            from models import MachineStatus
        status = MachineStatus.query.first()
        if status:
            mqtt_handler.publish_machine_status(
                status.is_online,
                status.temperature,
                status.humidity
            )
