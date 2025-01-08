#!/bin/bash

gnome-terminal -- ../../../../kmd3-server/server.out --buffer_period_ms=50 --timeout_s=30 --chunk_s=30
gnome-terminal -- ../../../../kmd3-server/cli255.out

gnome-terminal -- venv/bin/python cv2_camera.py --cam-index 0 --visualize
gnome-terminal -- venv/bin/python cv2_camera.py --cam-index 2 --visualize
gnome-terminal -- venv/bin/python cv2_camera.py --cam-index 4 --visualize

gnome-terminal -- venv/bin/python pcap_lidar.py --visualize True
gnome-terminal -- venv/bin/python imu.py
