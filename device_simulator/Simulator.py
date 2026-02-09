import argparse
import yaml
import time
import logging
import sys
import os
import threading

# Add the current directory to sys.path to ensure imports work correctly
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from mqtt_client import MQTTClient
from sensors.temperature import TemperatureSensor
from sensors.humidity import HumiditySensor
from CONSTS import SIMULATION_INTERVAL

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

def load_config(config_path):
    """Load simulator configuration from YAML file"""
    with open(config_path, 'r') as f:
        return yaml.safe_load(f)

def create_sensors(sensors_cfg):
    """Factory function to create sensor instances from configuration"""
    sensors = []
    for sensor_type, cfg in sensors_cfg.items():
        if sensor_type == 'temperature':
            sensors.append(TemperatureSensor(
                initial=cfg.get('initial', 25.0),
                min_value=cfg.get('min', -40.0),
                max_value=cfg.get('max', 85.0),
                variance=cfg.get('variance', 0.5)
            ))
        elif sensor_type == 'humidity':
            sensors.append(HumiditySensor(
                initial=cfg.get('initial', 50.0),
                min_value=cfg.get('min', 0.0),
                max_value=cfg.get('max', 100.0),
                variance=cfg.get('variance', 1.0)
            ))
        else:
            logger.warning(f"Unknown sensor type in config: {sensor_type}")
    return sensors

class DeviceSimulator(threading.Thread):
    def __init__(self, config):
        super().__init__()
        self.config = config
        self.device_id = config.get('device_id', 'unknown_device')
        self.mqtt_cfg = config.get('mqtt', {})
        self.pub_cfg = config.get('publishing', {})
        self.sensors_cfg = config.get('sensors', {})
        
        self.running = False
        self.daemon = True

    def run(self):
        broker = self.mqtt_cfg.get('broker', 'localhost')
        port = self.mqtt_cfg.get('port', 1883)
        
        logger.info(f"Starting simulator for device: {self.device_id}")
        
        client = MQTTClient(broker, port, self.device_id)
        
        if not client.connect():
            logger.error(f"Device {self.device_id} failed to connect to MQTT broker.")
            return
        
        sensors = create_sensors(self.sensors_cfg)
        if not sensors:
            logger.error(f"No sensors configured for device {self.device_id}.")
            client.disconnect()
            return
        
        # Determine interval: prefer config, then fallback to CONSTS.SIMULATION_INTERVAL
        # The user's code had SIMULATION_INTERVAL.value, but SIMULATION_INTERVAL is an int.
        interval = self.pub_cfg.get('interval', SIMULATION_INTERVAL)
        
        self.running = True
        try:
            while self.running:
                for sensor in sensors:
                    sensor.read_value()
                    state = sensor.get_state()
                    
                    sensor_type = state['type']
                    state['device_id'] = self.device_id
                    
                    success = client.publish_data(sensor_type, state)
                    
                    if success:
                        logger.info(f"[{self.device_id}] Published {sensor_type}: {state['current']} {state['unit']}")
                
                time.sleep(interval)
        except Exception as e:
            logger.error(f"Simulation error in device {self.device_id}: {e}")
        finally:
            logger.info(f"Cleaning up device {self.device_id}...")
            client.disconnect()

    def stop(self):
        self.running = False

def main():
    parser = argparse.ArgumentParser(description="IoT Device Simulator")
    parser.add_argument("--config", type=str, nargs='+', help="Path to YAML config file(s)")
    parser.add_argument("--dir", type=str, help="Directory containing YAML config files")
    args = parser.parse_args()
    
    configs = []
    if args.config:
        for cfg_path in args.config:
            configs.append(load_config(cfg_path))
    
    if args.dir:
        for file in os.listdir(args.dir):
            if file.endswith(".yaml") or file.endswith(".yml"):
                configs.append(load_config(os.path.join(args.dir, file)))

    if not configs:
        logger.error("No configuration files provided. Use --config or --dir.")
        return

    simulators = []
    for config in configs:
        sim = DeviceSimulator(config)
        sim.start()
        simulators.append(sim)

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        logger.info("Simulation interrupted by user")
        for sim in simulators:
            sim.stop()
        for sim in simulators:
            sim.join()

if __name__ == "__main__":
    main()