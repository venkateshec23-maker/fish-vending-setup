"""
Smart Fish Vending Machine - ESP32 Communication Service
Handles HTTP communication with the ESP32 hardware controller.
"""

import requests
import os
import json
import logging
from datetime import datetime

logger = logging.getLogger(__name__)


class ESP32Client:
    """
    HTTP client for communicating with ESP32 hardware controller.
    Handles dispense requests and status checks.
    """

    def __init__(self):
        """Initialize ESP32 client with configuration from environment."""
        self.esp32_ip = os.getenv('ESP32_IP', '192.168.1.100')
        self.esp32_port = int(os.getenv('ESP32_PORT', 80))
        self.timeout = int(os.getenv('ESP32_TIMEOUT', '30'))
        self.retries = int(os.getenv('ESP32_RETRIES', '3'))
        self.retry_delay = int(os.getenv('ESP32_RETRY_DELAY', '5'))
        self.base_url = f"http://{self.esp32_ip}:{self.esp32_port}"

        logger.info(f"ESP32 Client initialized: {self.base_url}")

    def _post_with_retry(self, url, payload):
        """POST to ESP32 with retry/backoff."""
        last_error = None
        for attempt in range(1, self.retries + 1):
            try:
                logger.info(f"ESP32 request attempt {attempt}/{self.retries} to {url}")
                response = requests.post(
                    url,
                    json=payload,
                    headers={'Content-Type': 'application/json'},
                    timeout=self.timeout
                )
                logger.info(f"ESP32 response status: {response.status_code}")
                logger.debug(f"ESP32 response body: {response.text}")
                return response
            except requests.exceptions.Timeout:
                last_error = 'TIMEOUT'
                logger.error(f"ESP32 request timeout on attempt {attempt}/{self.retries}")
            except requests.exceptions.ConnectionError:
                last_error = 'CONNECTION_ERROR'
                logger.error(f"Cannot connect to ESP32 on attempt {attempt}/{self.retries}")
            except requests.exceptions.RequestException as e:
                last_error = 'REQUEST_ERROR'
                logger.error(f"ESP32 request failed on attempt {attempt}/{self.retries}: {str(e)}")

            if attempt < self.retries:
                logger.info(f"Retrying in {self.retry_delay}s...")
                import time
                time.sleep(self.retry_delay)

        return last_error

    def send_dispense_request(self, order_id, compartment, fish_name, qty):
        """
        Send dispense request to ESP32.

        Args:
            order_id: Unique order ID
            compartment: Compartment number (1-4)
            fish_name: Name of fish to dispense
            qty: Quantity to dispense

        Returns:
            dict: Response from ESP32 with status and details
        """
        url = f"{self.base_url}/dispense"
        payload = {
            "orderId": order_id,
            "compartment": compartment,
            "fishName": fish_name,
            "qty": qty
        }

        logger.info(f"Sending dispense request to ESP32: {payload}")

        result = self._post_with_retry(url, payload)

        if isinstance(result, str):
            return {
                'success': False,
                'message': f'Machine did not respond ({result})',
                'error': result
            }

        if result.status_code == 200:
            try:
                data = result.json()
                if data.get('status') == 'success':
                    logger.info(f"Dispense request successful for order {order_id}")
                    return {
                        'success': True,
                        'message': data.get('message', 'Dispensing started'),
                        'order_id': order_id,
                        'data': data
                    }
                else:
                    logger.error(f"ESP32 returned error: {data}")
                    return {
                        'success': False,
                        'message': data.get('message', 'Unknown error from ESP32'),
                        'error': 'ESP32_ERROR'
                    }
            except json.JSONDecodeError:
                logger.error(f"Invalid JSON response from ESP32: {result.text}")
                return {
                    'success': False,
                    'message': 'Invalid response from machine',
                    'error': 'INVALID_RESPONSE'
                }
        else:
            logger.error(f"ESP32 returned HTTP {result.status_code}")
            return {
                'success': False,
                'message': f'Machine returned error code {result.status_code}',
                'error': 'HTTP_ERROR',
                'status_code': result.status_code
            }

    def check_status(self):
        """
        Check ESP32 machine status.

        Returns:
            dict: Status information from ESP32
        """
        url = f"{self.base_url}/status"

        try:
            response = requests.get(url, timeout=self.timeout)
            if response.status_code == 200:
                return {
                    'online': True,
                    'data': response.json()
                }
            else:
                return {
                    'online': False,
                    'error': f'HTTP {response.status_code}'
                }
        except Exception as e:
            return {
                'online': False,
                'error': str(e)
            }

    def is_machine_online(self):
        """
        Check if ESP32 is online.

        Returns:
            bool: True if machine is online
        """
        status = self.check_status()
        return status.get('online', False)


# Global ESP32 client instance
esp32_client = ESP32Client()


def init_esp32():
    """Initialize ESP32 client with app context."""
    logger.info("ESP32 communication service initialized")


def get_compartment_for_fish(fish_name):
    """
    Map fish name to compartment number.
    In production, this could be stored in database or config.

    Args:
        fish_name: Name of the fish

    Returns:
        int: Compartment number (1-4)
    """
    mapping = {
        'Tilapia': 1,
        'Salmon': 2,
        'Trout': 3,
        'Catfish': 4
    }
    return mapping.get(fish_name, 1)  # Default to compartment 1
