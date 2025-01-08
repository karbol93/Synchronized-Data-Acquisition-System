#!/bin/bash
echo "Parameter 1: path to dir where to write files (relative to home directory; default: kmd3_input)"

if [[ $BASH_SOURCE == */* ]]; then
    cd -- "${BASH_SOURCE%/*}/"
fi

if [[ $# -gt 0 ]]; then
    write_path="$HOME/$1"
else
    write_path="$HOME/kmd3_input"
fi

if [[ -d $write_path ]]; then
    echo "Write dir set to: $write_path"
    cd examples/python_acquisition_engine || exit

    gnome-terminal -- ../../../kmd3-server/server_with_parameters.out --buffer_period_ms=100 --timeout_s=60 --chunk_s=60

    # gnome-terminal -- ../../../cam-detect-plugin/venv/bin/python camera_detect_plugin.py --cam-index 0 --path "$write_path" --visualize
    gnome-terminal -- venv/bin/python cv2_camera.py --cam-index 0 --path "$write_path" --visualize
    gnome-terminal -- venv/bin/python cv2_camera.py --cam-index 2 --path "$write_path" --visualize
    gnome-terminal -- venv/bin/python cv2_camera.py --cam-index 4 --path "$write_path" --visualize

    gnome-terminal -- venv/bin/python pcap_lidar.py --path "$write_path" --visualize True
    # gnome-terminal -- venv/bin/python imu.py
    # gnome-terminal -- venv/bin/python dummy_sensor.py --path "$write_path"

    gnome-terminal -- ../../../kmd3-server/cli255.out
else
    echo "WRITE DIRECTORY $write_path DOES NOT EXIST; press enter"
    read
fi
