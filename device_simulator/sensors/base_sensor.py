import random
import datetime
from device_simulator.CONSTS import SensorType
from device_simulator.sensors.sensor_reading import SensorReading

class BaseSensor:
    def __init__(self, type: SensorType,initial=0.0, min_value=0.0, max_value=100.0, variance=1.0, ):
        self.value = initial
        self.min_value = min_value
        self.max_value = max_value
        self.variance = variance
        self.type = type

    def read(self):
        # Simulate sensor reading with random walk
        change = random.uniform(-self.variance, self.variance)
        self.value += change
        
        # Clamp the value within min and max bounds
        if self.value < self.min_value:
            self.value = self.min_value
        elif self.value > self.max_value:
            self.value = self.max_value
        
        return round(self.value, 2)
    
    def get_state(self) -> SensorReading:
        reading = SensorReading(
            type=self.type.value,
            current=self.value,
            min_value=self.min_value,
            max_value=self.max_value,
            timestamp=datetime.datetime.utcnow().isoformat() + "Z",
            unit="N/A"
        )
        return reading
        
    