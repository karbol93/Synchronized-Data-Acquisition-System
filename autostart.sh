#!/bin/bash

if [[ $BASH_SOURCE == */* ]]; then
    cd -- "${BASH_SOURCE%/*}/"
fi

cd src/examples/acquisition_client_example

gnome-terminal -- ../../../../kmd3-server/server_with_parameters.out --buffer_period_ms=100 --timeout_s=60 --chunk_s=60

gnome-terminal -- venv/bin/python cv2_camera.py --cam-index 0 --path "/home/leaf/kmd3_input" --visualize
gnome-terminal -- venv/bin/python cv2_camera.py --cam-index 2 --path "/home/leaf/kmd3_input" --visualize
gnome-terminal -- venv/bin/python cv2_camera.py --cam-index 4 --path "/home/leaf/kmd3_input" --visualize

gnome-terminal -- venv/bin/python pcap_lidar.py --path "/home/leaf/kmd3_input" --visualize True
# gnome-terminal -- venv/bin/python imu.py

gnome-terminal -- ../../../../kmd3-server/cli255.out
