from abc import abstractmethod, ABCMeta
from typing import TypeVar, Generic, Tuple


T = TypeVar('T')


class PeriodicAcquisitionSensorBase(Generic[T], metaclass=ABCMeta):
    @abstractmethod
    def get_sensor_type(self) -> str:
        # Returns a short plugin/sensor name for filename creation,
        # eg. "cvcam" for cv2 camera plugin
        pass

    @abstractmethod
    def get_sensor_id(self) -> str:
        # Returns a short, unique ID, used for differentiation
        # between sensors of the same type for filename creation.
        # If used multiple times, always has to return the same value.
        # First time called during the session.
        # If the acquisition is performed on many hosts it must be differentiated anyway.
        # It can be a random number, based on random numbers provided by the OS.
        pass

    @abstractmethod
    def get_file_extension(self) -> str:
        # Returns a file extension without a dot.
        # First time called during the session.
        pass

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
    def get_sample(self) -> Tuple[T, int]:
        # Returns a sample if and its timestamp, blocks if no samples in buffer,
        # releases immediatelly after new sample(s) arrival
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
    def open_file(self, full_path: str) -> str:
        pass

    @abstractmethod
    def close_file(self) -> None:
        pass

    @abstractmethod
    def write_sample_to_file(self, sample: T) -> None:
        pass
