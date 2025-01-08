from os import EX_UNAVAILABLE
from time import sleep
from math import ceil
from typing import Optional, Any

from kespp_sensors_abc import KESPPPeriodicSensorBase
from kespp_python import kespp
# from cv2_camera import OpenCVCamera


class PeriodicAcquisitionEngine:
    NANO: float = 1e-9

    def __init__(self, sensor: KESPPPeriodicSensorBase, max_late_samples: int = 1, late_sample_doubling_period: float = 2.0, sync_off: bool = False):
        # sched_setscheduler(0, SCHED_FIFO, sched_param(1))
        self._client: kespp.KESPPClient = kespp.KESPPClient()
        self._sensor: KESPPPeriodicSensorBase = sensor
        self._max_late_samples: int = max_late_samples
        self._late_sample_doubling_period: float = late_sample_doubling_period
        self.sync_off: bool = sync_off

        self.verbose_run: bool = False

    def _v_print(self, *args, **kwargs):
        # Verbose print
        if self.verbose_run:
            print(*args, **kwargs)

    def _connect_client(self) -> None:
        connection_trials = 10
        i = 0
        while not self._client.try_connect():
            print(self._client.error_msg())
            print('Retrying...')
            sleep(2)
            if i >= connection_trials - 1:
                print('Could not connect\n')
                exit(EX_UNAVAILABLE)
            i += 1
        print('Connected!\n')

    def _new_control_frame_received(self, synchro_data, old_synchro_data, new_chunk_timepoint) -> bool:
        # New ControlFrame received so new chunk or stop is coming
        return (synchro_data.chunk_nanosec() != old_synchro_data.chunk_nanosec()) and ((new_chunk_timepoint is None) or synchro_data.stop_flag())

    @staticmethod
    def _new_timepoint_reached(new_chunk_timepoint: Optional[int]) -> bool:
        # Time passed and new chunk should be recorded or recording should stop
        return (new_chunk_timepoint is not None) and (kespp.KESPPClient.now_nanosec() >= new_chunk_timepoint)

    def _timeout_occurred(self, new_chunk_timepoint: Optional[int], synchro_data) -> bool:
        # Timeout occurred - samples before timeout should be saved
        return (new_chunk_timepoint is None) and (kespp.KESPPClient.now_nanosec() >= synchro_data.chunk_nanosec() + self._client.timeout_nanosec())

    def _samples_in_range(self, start_timepoint_ns, end_timepoint_ns) -> float:
        return (end_timepoint_ns - start_timepoint_ns) / self._sensor.sampling_period_s() * self.NANO

    def _record_session(self):
            self._sensor.start_buffering()
            synchro_data = self._client.get_info()
            session_start_time: int = synchro_data.chunk_nanosec()
            session_sample_counter: int = 0
            timeout_detected: bool = False
            first_non_ok_time: int = 0
            sample: Any = None
            sample_timestamp: int = 0
            sample_to_forward: bool = False

            read_samples_counter: int = 1  # 1 because first sample is gotten before naming the file
            time_interpolated_samples_counter: int = 0
            limit_interpolated_samples_counter: int = 0
            removed_samples_counter: int = 0

            type(self._sensor)
            if self._sensor.type_string=="cam":
                timestampy = open(f'{session_start_time}_timestamps_cam{self._sensor._camera_index}.txt', 'w')  # works only for cv2_camera plugin
            else:
                timestampy = open(f'{session_start_time}_timestamps_lidar.txt', 'w')

            # Session recording loop (between start and stop recording commands):
            while (not synchro_data.stop_flag()) and (not timeout_detected):
                if kespp.KESPPClient.now_nanosec() >= session_start_time:
                    filename = self._sensor.open_file(self._client.recording_timestamp(synchro_data.chunk_nanosec()))
                else:
                    filename = None  # Filename "None" means there is fake chunk before real recording

                self._v_print(filename if filename is not None else "Passed up samples")

                chunk_sample_counter: int = 0
                chunk_enough_samples: bool = False
                if filename is None:
                    new_chunk_timepoint: Optional[int] = synchro_data.chunk_nanosec()
                else:
                    new_chunk_timepoint: Optional[int] = None  # to store current chunk's timepoint, even if a newer one is received while sleeping

                # Chunk recording loop (multiple data chunks within one session):
                while True:
                    if not sample_to_forward:
                        # TODO: Buffer with timestamps for samples instead of fetching one by one?
                        sample, sample_timestamp = self._sensor.get_sample()  # blocking call
                        # sample_timestamp = self._client.now_nanosec()
                        if filename is not None:
                            read_samples_counter += 1
                    else:
                        sample_to_forward = False

                    if filename is not None:
                        diff: int = session_sample_counter + chunk_sample_counter + 1 - ceil(self._samples_in_range(session_start_time, sample_timestamp))
                        # TODO: Calculate samples per session instead of per chunk (here and in the other places)
                        # +1 for already fetched but not saved sample

                        if diff == 0 or self.sync_off:
                            # Ideal number of samples
                            first_non_ok_time = 0
                            self._sensor.write_sample_to_file(sample)
                            chunk_sample_counter += 1

                            # DODANE ZAPISYWANIE TIMESTAMPÓW
                            timestampy.write(f'{str(sample_timestamp)}\n')

                        elif diff < 0:
                            # Too few samples
                            if first_non_ok_time == 0:
                                first_non_ok_time = sample_timestamp

                            if diff < -self._max_late_samples:
                                self._sensor.write_sample_to_file(self._sensor.interpolate_sample(sample))
                                chunk_sample_counter += 1
                                limit_interpolated_samples_counter += 1

                                # DODANE ZAPISYWANIE TIMESTAMPÓW
                                timestampy.write(f'{str(sample_timestamp)}\n')

                                self._v_print("Too few samples - doubled because max exceeded", end="")
                                self._v_print(", last sample timestamp:", self._client.recording_timestamp(sample_timestamp))
                                sample_to_forward = True  # Current sample forwarded to next loop cycle

                                first_non_ok_time = 0 if diff == -1 else sample_timestamp  # "0" because it's on time right now or "now" because still late
                            elif self._samples_in_range(first_non_ok_time, sample_timestamp) > self._late_sample_doubling_period:
                                self._sensor.write_sample_to_file(self._sensor.interpolate_sample(sample))
                                chunk_sample_counter += 1
                                time_interpolated_samples_counter += 1

                                # DODANE ZAPISYWANIE TIMESTAMPÓW
                                timestampy.write(f'{str(sample_timestamp)}\n')

                                self._v_print("Too few samples - doubled because time exceeded", end="")
                                self._v_print(", last sample timestamp:", self._client.recording_timestamp(sample_timestamp))
                                sample_to_forward = True  # Current sample forwarded to next loop cycle

                                first_non_ok_time = 0 if diff == -1 else sample_timestamp  # "0" because it's on time right now or "now" because still late
                                # TODO: First non ok time should be rather measured from timepoint before the late sample arrives (sample range timepoint)
                            else:
                                # Not doubled
                                self._sensor.write_sample_to_file(sample)
                                chunk_sample_counter += 1

                                # DODANE ZAPISYWANIE TIMESTAMPÓW
                                timestampy.write(f'{str(sample_timestamp)}\n')

                        else:
                            # Too many samples: diff > 0
                            self._v_print("Too many samples - dropped sample, last sample timestamp:", self._client.recording_timestamp(sample_timestamp))
                            first_non_ok_time = 0
                            removed_samples_counter += 1

                        if new_chunk_timepoint is not None:
                            chunk_enough_samples = session_sample_counter + chunk_sample_counter >= round(self._samples_in_range(session_start_time, new_chunk_timepoint))
                    else:
                        chunk_sample_counter += 1

                    old_synchro_data = synchro_data
                    synchro_data = self._client.get_info()

                    if self._new_control_frame_received(synchro_data, old_synchro_data, new_chunk_timepoint):
                        # TODO: Receive control frame even if sensor period is longer than buffer period - cannot use synchronized receiving
                        new_chunk_timepoint = synchro_data.chunk_nanosec()

                    if self._new_timepoint_reached(new_chunk_timepoint) and filename is None:
                        if not synchro_data.stop_flag():
                            # If sleep was too long, and new frame has been received, but old chunk isn't finished yet,
                            # create new file with old timepoint name (wrong filename if commented):
                            synchro_data = old_synchro_data
                        chunk_sample_counter -= 1
                        sample_to_forward = True
                        break
                    elif self._timeout_occurred(new_chunk_timepoint, synchro_data):
                        timeout_detected = True
                        break

                    # Consistency assertion:
                    if filename is not None and chunk_enough_samples:
                        # TODO: Jeśli w algorytmie albo implementacji jest błąd to warto sprawdzić tu czy czas nie został wyprzedzony
                        break
                    else:
                        ...
                        # TODO: Jeśli w algorytmie albo implementacji jest błąd to warto sprawdzić tu czy czas nie został przekroczony

                if filename is not None:
                    self._sensor.close_file()
                    session_sample_counter += chunk_sample_counter
                self._v_print(chunk_sample_counter, 'samples\n')

            self._sensor.stop_buffering()

            self._v_print('Total samples written:', session_sample_counter)
            self._v_print('Total read samples:', read_samples_counter)
            self._v_print(
                f'Samples interpolated due to {self._late_sample_doubling_period} periods deficit time:',
                time_interpolated_samples_counter,
            )
            self._v_print(
                f'Samples interpolated due to max. {self._max_late_samples} samples limit:',
                limit_interpolated_samples_counter,
            )
            self._v_print('Removed samples:', removed_samples_counter)
            self._v_print()


            timestampy.close()

    def run(self, verbose: bool = False) -> None:
        self.verbose_run = verbose
        self._connect_client()
        self._sensor.init_sensor()

        # Per-connection recording loop (multi-session handling):
        while self._client.connected():
            self._client.wait_for_start_info()
            if not self._client.connected():
                break
            self._record_session()

        self._sensor.stop_sensor()
