import logging
from device_simulator.sensors.sensor_reading import SensorReading
from ..CONSTS import SensorType
from . import BaseSensor

logger = logging.getLogger(__name__)

class HumiditySensor(BaseSensor):
    def __init__(self, initial=50.0, min_value=0.0, max_value=100.0, variance=1.0):
        logger.info("Initializing HumiditySensor")
        logger.debug(f"Initial: {initial}, Min: {min_value}, Max: {max_value}, Variance: {variance}")
        super().__init__(SensorType.HUMIDITY, initial, min_value, max_value, variance)
        
    def read_value(self) -> float:
        """
        Simulate reading the humidity value.
        The value fluctuates randomly within the specified variance.
        
        Returns:
            float: Current humidity value
        """
        self.value = super().read()
        logger.debug(f"HumiditySensor read value: {self.value}")
        return self.value
    
    def get_state(self) -> dict:
        """
        Get the current state of the sensor.
        Returns:
            dict: Current state including the humidity value
        """
        reading: SensorReading = super().get_state()
        reading.unit = "%"
        logger.debug(f"HumiditySensor state: {reading}")
        return reading.to_dict()