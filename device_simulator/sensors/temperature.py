import logging
from device_simulator.CONSTS import SensorType
from device_simulator.sensors.sensor_reading import SensorReading
from . import BaseSensor

logger = logging.getLogger(__name__)

class TemperatureSensor(BaseSensor):

    def __init__(self, initial=25.0, min_value=-40.0, max_value=85.0, variance=0.5):
        logger.info("Initializing TemperatureSensor")
        logger.debug(f"Initial: {initial}, Min: {min_value}, Max: {max_value}, Variance: {variance}")
        super().__init__(SensorType.TEMPERATURE, initial, min_value, max_value, variance)
       
    def read_value(self) -> float:
        """
        Simulate reading the temperature value.
        The value fluctuates randomly within the specified variance.
        
        Returns:
            float: Current temperature value
        """
        self.current_value = super().read()
        logger.debug(f"TemperatureSensor read value: {self.current_value}")
        return self.current_value
    
    def get_state(self) -> dict:
        """
        Get the current state of the sensor.
        
        Returns:
            dict: Current state including the temperature value
        """
        reading: SensorReading = super().get_state()
        reading.unit = "°C"
        logger.debug(f"TemperatureSensor state: {reading}")
        return reading.to_dict()
        