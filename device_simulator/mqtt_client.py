import paho.mqtt.client as mqtt
import logging
import json
from datetime import datetime

logger = logging.getLogger(__name__)

class MQTTClient:
    
    def __init__(self, broker_addr: str, port: int, device_id: str):
        self.broker_addr = broker_addr
        self.port = port
        self.device_id = device_id
        self.connected = False
        
        # Create a mqtt client
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=self.device_id)
        
        # Set callbacks
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_publish = self._on_publish
        
    # Public APIs to handle the connection
    def connect(self) -> bool:
        """Connect to MQTT broker"""
        try:
            logger.info(f"Connecting to {self.broker_addr}:{self.port}")
            self.client.connect(self.broker_addr, self.port, keepalive=60)
            self.client.loop_start() 
            return True
        except Exception as e:
            logger.error(f"Connection failed: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from broker"""
        logger.info("Disconnecting from MQTT broker")
        self.client.loop_stop()
        self.client.disconnect()
      
    def publish_data(self, topic_suffix: str, payload: dict) -> bool:
        """
        Publish arbitrary data to MQTT
        Topic format: devices/{device_id}/{topic_suffix}
        """
        topic = f"devices/{self.device_id}/{topic_suffix}"
        
        try:
            result = self.client.publish(
                topic=topic,
                payload=json.dumps(payload),
                qos=1
            )
            
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                logger.debug(f"Published to {topic}")
                return True
            else:
                logger.error(f"Publish failed: {result.rc}")
                return False
        
        except Exception as e:
            logger.error(f"Publish error: {e}")
            return False
        
    def publish_sensor_data(self, sensor_type: str, value, timestamp: str = None) -> bool:
        """
        Publish sensor reading to MQTT
        """
        if timestamp is None:
            timestamp = datetime.utcnow().isoformat() + "Z"
        
        payload = {
            "device_id": str(self.device_id),
            "sensor_type": sensor_type,
            "value": value,
            "timestamp": timestamp
        }
        
        return self.publish_data(sensor_type, payload)
        
    # CallBacks
    def _on_connect(self, client, userdata, connect_flags, rc, properties):
        """Called when client connects"""
        if rc == 0:
            logger.info("Connected to MQTT broker")
            self.connected = True
        else:
            logger.error(f"Connection failed with code {rc}")
            self.connected = False
    
    def _on_disconnect(self, client, userdata, rc, properties):
        """Called when client disconnects"""
        self.connected = False
        if rc != 0:
            logger.warning(f"Unexpected disconnect: {rc}")
    
    def _on_publish(self, client, userdata, mid, reason_code, properties):
        """Called when message is published"""
        logger.debug(f"Message published (ID: {mid})")
