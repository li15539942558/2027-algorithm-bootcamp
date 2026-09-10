@echo off
chcp 65001 >nul
title RM Algorithm Bootcamp - WSL Ubuntu 22.04
echo.
echo   ============================================================
echo    2027 RM Algorithm Bootcamp - Week 1 Environment
echo    WSL distro : Ubuntu-22.04  (ROS2 Humble + Docker ready)
echo    Workspace  : ~/ros2_ws  (ROS2)   ~/morph  (C++ / cmake)
echo   ============================================================
echo.
wsl -d Ubuntu-22.04 --cd /home/ubuntu
