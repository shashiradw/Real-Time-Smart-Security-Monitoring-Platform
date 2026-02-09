from dataclasses import dataclass, asdict
from datetime import datetime

@dataclass
class SensorReading:
    type: str
    current: float
    min_value: float
    max_value: float
    timestamp: str
    unit: str = "N/A"

    def to_dict(self):
        return asdict(self)
