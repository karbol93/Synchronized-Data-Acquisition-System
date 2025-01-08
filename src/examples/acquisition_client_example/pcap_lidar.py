import lidar

import argparse
from io import TextIOWrapper
from queue import SimpleQueue, Empty
from threading import Thread, Condition
from time import sleep

from typing import Optional
from kespp_sensors_abc import KESPPPeriodicSensorBase
from periodic_acquisition_engine import PeriodicAcquisitionEngine


class LidarSensor(KESPPPeriodicSensorBase[Optional[int]]):
    def __init__(self, address: str, port: int, visualize: bool) -> None:
        self._sampling_period: float = 0.001327104
        self._lidar = None
        self._address = address
        self._port = port
        self._counter = 0
        self._cond = Condition()
        self._initialized: bool = False
        self._buffering: bool = False
        self._visualize: bool = visualize
        self._generator_thread: Optional[Thread] = None

    def _sample_viewer(self) -> None:
        while self._initialized and self._visualize:
            with self._cond:
                self._cond.wait_for( lambda : not self._buffering )
            self.get_sample()
            self._lidar.visualize()


    def get_sensor_type(self) -> str:
        return "pcaplidar"


    def get_sensor_id(self) -> str:
        return '1'  # TODO: Change to ip and port instead of hardcoded value (currently manages only one lidar)


    def get_file_extension(self) -> str:
        return 'pcap'


    def init_sensor(self) -> None:
        # An init() method can be repeated - always check if sensor is already initialized
        if not self._initialized:
            self._initialized = True
            self._lidar = lidar.VLP16(self._address, self._port)
            self._generator_thread = Thread(target=self._sample_viewer)
            self._generator_thread.start()


    def start_buffering(self) -> None:
        self._buffering = True


    def stop_buffering(self) -> None:
        with self._cond:
            self._buffering = False
            self._cond.notify_all()


    def stop_sensor(self) -> None:
        # TODO zwolnienie socketu/destruktor czy cos
        #  A stop() method can be repeated - always check if sensor is already initialized
        self._initialized = False
        self._buffering = False
        self._generator_thread.join()

    def samples_in_buffer(self) -> int:
        return 1

    def get_sample(self) -> Optional[int]:
        self._counter += 1
        self._lidar.get_data()
        return self._counter

    def interpolate_sample(self, sample: Optional[int]) -> Optional[int]:
        return None

    def sampling_period_s(self) -> float:
        return self._sampling_period

    def open_file(self, full_path: str) -> str:
        self._lidar.pcap_file(full_path)
        return full_path

    def close_file(self) -> None:
        self._lidar.close_file()

    def write_sample_to_file(self, sample: Optional[int]) -> None:
        self._lidar.save_data()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--max-late-samples', type=int, help='Max late samples', default=1)
    parser.add_argument('--late-sample-doubling', type=float, help='Late sample doubling periods', default=20.0)
    parser.add_argument('--path', type=str, help='Path to a directory where the files will be saved', default='.')
    parser.add_argument('--visualize', type=bool, help='Show videostream', default=False)
    args = parser.parse_args()

    sensor: LidarSensor = LidarSensor(address="255.255.255.255", port=2368, visualize=args.visualize)

    acquisition_engine = PeriodicAcquisitionEngine(
        sensor,
        args.max_late_samples,
        args.late_sample_doubling,
        write_path=args.path,
    )
    acquisition_engine.run(verbose=True)


if __name__ == "__main__":
    main()