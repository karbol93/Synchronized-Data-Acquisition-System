import argparse
import cv2
from queue import SimpleQueue
from threading import Thread
from typing import Optional, Tuple
from time import monotonic_ns

from pa_sensors_abc import PeriodicAcquisitionSensorBase
from periodic_acquisition_engine import PeriodicAcquisitionEngine


class OpenCVCamera(PeriodicAcquisitionSensorBase[Optional[cv2.Mat]]):
    def __init__(self, camera_index: int, capture_params: tuple, writer_fourcc: int, visualize: bool) -> None:
        self._camera_index: int = camera_index
        self._params: tuple = capture_params

        self._frames_buffer: SimpleQueue[cv2.Mat] = SimpleQueue()
        self._initialized: bool = False
        self._buffering: bool = False
        self._sampling_period: float = 0.0
        self._capture: cv2.VideoCapture = cv2.VideoCapture()  # Data source object
        self._writer_fourcc = writer_fourcc
        self._visualize: Optional[str] = f'Cam {self._camera_index}' if visualize else None
        self._file: cv2.VideoWriter = cv2.VideoWriter()

        self._broken_frames_counter: int = 0

        self._generator_thread: Optional[Thread] = None

    def get_sensor_type(self) -> str:
        return 'cvcam'

    def get_sensor_id(self) -> str:
        return str(self._camera_index)

    def get_file_extension(self) -> str:
        return 'avi'

    def _sample_generator(self) -> None:
        if self._visualize:
            cv2.namedWindow(self._visualize, cv2.WINDOW_GUI_NORMAL)

        while self._initialized:
            if self._buffering:
                ret, frame = self._capture.read()
                if ret:
                    self._frames_buffer.put(frame)
                else:
                    self._broken_frames_counter += 1
            else:
                if not self._visualize:
                    self._capture.grab()
                else:
                    ret, frame = self._capture.read()
                    if ret:
                        cv2.imshow(self._visualize, mat=frame)
                        cv2.pollKey()


    def init_sensor(self) -> None:
        # An init() method can be repeated - always check if sensor is already initialized
        if (not self._initialized) and (self._capture.open(self._camera_index, apiPreference=cv2.CAP_ANY, params=self._params)):
            print(int(self._capture.get(cv2.CAP_PROP_FRAME_WIDTH)), 'x', int(self._capture.get(cv2.CAP_PROP_FRAME_HEIGHT)), sep='')
            print(self._capture.get(cv2.CAP_PROP_FPS), 'FPS')
            self._capture.set(cv2.CAP_PROP_FOCUS, 0.0)
            self._sampling_period = 1.0 / self._capture.get(cv2.CAP_PROP_FPS)
            self._initialized = True
            self._generator_thread = Thread(target=self._sample_generator)
            self._generator_thread.start()

    def stop_sensor(self) -> None:
        #  A stop() method can be repeated - always check if sensor is already initialized
        if self._generator_thread is not None:
            self._initialized = False
            self._buffering = False
            self._generator_thread.join()
            self._capture.release()

    def start_buffering(self) -> None:
        self._broken_frames_counter = 0
        self._buffering = True

    def stop_buffering(self) -> None:
        self._buffering = False
        # Flush buffer
        while not self._frames_buffer.empty():
            self._frames_buffer.get(block=False)
        print(f'OpenCV couldn\'t decode {self._broken_frames_counter} frames from the camera\n')

    def get_sample(self) -> Tuple[cv2.Mat, int]:
        return self._frames_buffer.get(block=True), monotonic_ns()

    def interpolate_sample(self, succeeding_sample: Optional[cv2.Mat]) -> Optional[cv2.Mat]:
        return succeeding_sample

    def sampling_period_s(self) -> float:
        return self._sampling_period

    def open_file(self, full_path: str) -> str:
        self._file = cv2.VideoWriter(
            full_path,
            self._writer_fourcc,
            self._capture.get(cv2.CAP_PROP_FPS),
            (
                int(self._capture.get(cv2.CAP_PROP_FRAME_WIDTH)),
                int(self._capture.get(cv2.CAP_PROP_FRAME_HEIGHT)),
            ),
        )
        return full_path

    def close_file(self) -> None:
        self._file.release()

    def write_sample_to_file(self, sample: Optional[cv2.Mat]) -> None:
        self._file.write(sample)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cam-index', type=int, help='Camera index from /dev/video', default=0)
    parser.add_argument('--cam-fps', type=int, help='Desired FPS', default=30)
    parser.add_argument('--path', type=str, help='Path to a directory where the files will be saved', default='.')
    parser.add_argument('--sync-off', action='store_true', help='Turn off the sync algorithm')
    parser.add_argument('--visualize', action='store_true', help='Show the video stream')
    args = parser.parse_args()

    capture_fourcc = cv2.VideoWriter_fourcc(*'MJPG')
    writer_fourcc = cv2.VideoWriter_fourcc(*'MJPG')
    params = (
        cv2.CAP_PROP_FOURCC, capture_fourcc,
        cv2.CAP_PROP_FRAME_WIDTH, 1920,
        cv2.CAP_PROP_FRAME_HEIGHT, 1080,
        cv2.CAP_PROP_FPS, args.cam_fps,
        cv2.CAP_PROP_AUTOFOCUS, 0,
    )
    cam: OpenCVCamera = OpenCVCamera(args.cam_index, capture_params=params, writer_fourcc=writer_fourcc, visualize=args.visualize)

    acquisition_engine = PeriodicAcquisitionEngine(
        sensor=cam,
        max_late_samples=2,
        late_sample_doubling_period=20,
        sync_off=args.sync_off,
        write_path=args.path,
    )
    acquisition_engine.run(verbose=True)


if __name__ == "__main__":
    main()
