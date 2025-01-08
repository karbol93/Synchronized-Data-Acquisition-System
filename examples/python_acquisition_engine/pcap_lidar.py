import lidar

import argparse
# from io import TextIOWrapper
# from queue import SimpleQueue, Empty
# from threading import Thread

from time import monotonic_ns
from typing import Optional, Tuple
from pa_sensors_abc import PeriodicAcquisitionSensorBase
from periodic_acquisition_engine import PeriodicAcquisitionEngine


class LidarSensor(PeriodicAcquisitionSensorBase[Optional[int]]):
    def __init__(self, address, port) -> None:
        self._sampling_period: float = 0.001327104
        self._lidar = None
        self._address = address
        self._port = port
        # self._samples_buffer: SimpleQueue[int] = SimpleQueue()
        self._initialized: bool = False
        self._buffering: bool = False

        # self._generator_thread: Optional[Thread] = None
        # self._file

    def init(self) -> None:
        # An init() method can be repeated - always check if sensor is already initialized
        if not self._initialized:
            self._initialized = True
            # self._generator_thread = Thread(target=self._sample_generator)
            # self._generator_thread.start()
            self._lidar = lidar.VLP16(self._address, self._port)

    def stop(self) -> None:
        # TODO zwolnienie socketu/destruktor czy cos
        #  A stop() method can be repeated - always check if sensor is already initialized
        # if self._generator_thread is not None:
            # self._initialized = False
        self._buffering = False
            # self._generator_thread.join()

    def start_buffering(self) -> None:
        self._buffering = True

    def stop_buffering(self) -> None:
        self._buffering = False
        # Flush buffer
        # if not self._samples_buffer.empty():
        #     self._samples_buffer.get(block=False)

    def samples_in_buffer(self) -> int:
        return 1

    def get_sample(self) -> Tuple[int, int]:
        return self._lidar.get_data(), monotonic_ns()

    def interpolate_sample(self, sample: Optional[int]) -> Optional[int]:
        return None

    def sampling_period(self) -> float:
        return self._sampling_period

    def open_file(self, timestamp: str) -> str:
        filename = f'{timestamp}_lidar.pcap'
        self._lidar.pcap_file(filename)
        return filename

    def close_file(self) -> None:
        print("a")
        self._lidar.close_file()

    def write_sample_to_file(self, sample: Optional[int]) -> None:
        self._lidar.save_data()


def main():
    parser = argparse.ArgumentParser()
    # parser.add_argument('--sps', type=int, help='Samples per second', default=30)
    # parser.add_argument('--drift', type=float, help='Sensor clock drift', default=-0.0004)
    parser.add_argument('--max-late-samples', type=int, help='Max late samples', default=1)
    parser.add_argument('--late-sample-doubling', type=float, help='Late sample doubling periods', default=20.0)
    args = parser.parse_args()

    sensor: LidarSensor = LidarSensor("255.255.255.255", 2368)

    acquisition_engine = PeriodicAcquisitionEngine(sensor, args.max_late_samples, args.late_sample_doubling)
    acquisition_engine.run(verbose=True)


if __name__ == "__main__":
    main()