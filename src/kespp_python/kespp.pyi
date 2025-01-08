class ControlFrame:
    def __init__(self, chunk_nanosec: int = ..., stop_flag: int = ...) -> None: ...
    def chunk_nanosec(self) -> int: ...
    def stop_flag(self) -> int: ...


class KESPPClient:
    def __init__(self, host: str = ..., port: int = ...) -> None: ...
    def connected(self) -> bool: ...
    def realtime_diff_nanosec(self) -> int: ...
    def timeout_nanosec(self) -> int: ...
    def error_msg(self) -> str: ...
    def try_connect(self) -> bool: ...
    def wait_for_start_info(self) -> None: ...
    def get_info(self) -> ControlFrame: ...
    @staticmethod
    def now_nanosec() -> int: ...
    def recording_timestamp(self, chunk_nanosec: int) -> str: ...
