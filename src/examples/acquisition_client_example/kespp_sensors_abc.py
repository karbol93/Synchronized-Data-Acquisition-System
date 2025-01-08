from abc import abstractmethod, ABCMeta
from typing import TypeVar, Generic


T = TypeVar('T')


class KESPPPeriodicSensorBase(Generic[T], metaclass=ABCMeta):
    @abstractmethod
    def init_sensor(self) -> None:  # Separate init allows for sensor re-initialization
        # An init() method can be repeated - always check if sensor is already initialized
        pass

    @abstractmethod
    def stop_sensor(self) -> None:
        # A stop() method can be repeated - always check if sensor is already initialized
        pass

    @abstractmethod
    def start_buffering(self) -> None:
        pass

    @abstractmethod
    def stop_buffering(self) -> None:
        pass

    @abstractmethod
    def get_sample(self) -> tuple[T, int]:
        # Returns a sample and its timestamp,
        # blocks if no samples in buffer,
        # releases immediately after new sample(s) arrival
        pass

    @abstractmethod
    def interpolate_sample(self, succeeding_sample: T) -> T:
        # Interpolated sample is written first,
        # then the succeeding_sample
        pass

    @abstractmethod
    def sampling_period_s(self) -> float:
        # Returned in seconds
        pass

    @abstractmethod
    def open_file(self, timestamp: str) -> str:
        pass

    @abstractmethod
    def close_file(self) -> None:
        pass

    @abstractmethod
    def write_sample_to_file(self, sample: T) -> None:
        pass
