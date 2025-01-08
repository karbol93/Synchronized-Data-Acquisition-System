import argparse
from io import TextIOWrapper
from queue import SimpleQueue, Empty
from threading import Thread
from time import sleep
# from time import time_ns
from typing import Optional

from kespp_sensors_abc import KESPPPeriodicSensorBase
from periodic_acquisition_engine import PeriodicAcquisitionEngine


class DummySensor(KESPPPeriodicSensorBase[Optional[int]]):
    def __init__(self, sps: int, sample_drift_s: float) -> None:
        self._sps: int = sps
        self._sample_drift_s: float = sample_drift_s
        self._sampling_period: float = 1/self._sps

        self._samples_buffer: SimpleQueue[int] = SimpleQueue()
        self._initialized: bool = False
        self._buffering: bool = False
        self._counter: int = 0  # Data source object

        self._generator_thread: Optional[Thread] = None
        self._file: TextIOWrapper

    def _sample_generator(self):
        while self._initialized:
            if self._buffering:
                self._samples_buffer.put(self._counter)
                self._counter += 1
            else:
                self._counter += 1

            sleep(1 / self._sps + self._sample_drift_s)

    def init_sensor(self) -> None:
        # An init() method can be repeated - always check if sensor is already initialized
        if not self._initialized:
            self._counter = 0
            self._initialized = True
            self._generator_thread = Thread(target=self._sample_generator)
            self._generator_thread.start()

    def stop_sensor(self) -> None:
        #  A stop() method can be repeated - always check if sensor is already initialized
        if self._generator_thread is not None:
            self._initialized = False
            self._buffering = False
            self._generator_thread.join()

    def start_buffering(self) -> None:
        self._buffering = True

    def stop_buffering(self) -> None:
        self._buffering = False
        # Flush buffer
        if not self._samples_buffer.empty():
            self._samples_buffer.get(block=False)

    def get_sample(self) -> Optional[int]:
        return self._samples_buffer.get(block=True)

    def interpolate_sample(self, succeeding_sample: Optional[int]) -> Optional[int]:
        return succeeding_sample

    def sampling_period_s(self) -> float:
        return self._sampling_period

    def open_file(self, timestamp: str) -> str:
        self._file = open(f'{timestamp}_dummy{self._sample_drift_s}.txt', 'x')
        return self._file.name

    def close_file(self) -> None:
        self._file.close()

    def write_sample_to_file(self, sample: Optional[int]) -> None:
        self._file.write(str(sample)+'\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sps', type=float, help='Samples per second', default=30.0)
    parser.add_argument('--drift', type=float, help='Sensor clock drift', default=-0.0004)
    parser.add_argument('--max-late-samples', type=int, help='Max late samples', default=1)
    parser.add_argument('--late-sample-doubling', type=float, help='Late sample doubling periods', default=20.0)
    parser.add_argument('--sync-off', action='store_true', help='Turn off the sync algorithm')
    args = parser.parse_args()

    sensor: DummySensor = DummySensor(sps=args.sps, sample_drift_s=args.drift)

    acquisition_engine = PeriodicAcquisitionEngine(
        sensor,
        max_late_samples=args.max_late_samples,
        late_sample_doubling_period=args.late_sample_doubling,
        sync_off=args.sync_off,
    )
    acquisition_engine.run(verbose=True)


if __name__ == "__main__":
    main()
