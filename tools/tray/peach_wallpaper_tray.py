#!/usr/bin/env python3
"""Small tray controller for the PeachWallpaper runtime."""

from __future__ import annotations

import argparse
import os
import socket
import subprocess
import sys
from pathlib import Path

from PySide6.QtCore import QObject, QTimer
from PySide6.QtGui import QAction, QIcon
from PySide6.QtWidgets import QApplication, QMenu, QStyle, QSystemTrayIcon


STOP_COMMAND = 1


class IpcClient:
    def __init__(self) -> None:
        runtime_dir = os.environ.get("XDG_RUNTIME_DIR")
        if not runtime_dir:
            raise RuntimeError("XDG_RUNTIME_DIR is not set")

        self.socket_path = Path(runtime_dir) / "peach-wallpaper.sock"

    def send_command(self, command: int) -> None:
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as connection:
            connection.settimeout(1.0)
            connection.connect(str(self.socket_path))
            connection.sendall(bytes((command,)))


class WallpaperProcess(QObject):
    def __init__(self, executable: Path, config: Path, ipc: IpcClient) -> None:
        super().__init__()
        self.executable = executable
        self.config = config
        self.ipc = ipc
        self.process: subprocess.Popen[bytes] | None = None
        self.restart_requested = False
        self.status = "Stopped"

    def is_running(self) -> bool:
        return self.process is not None and self.process.poll() is None

    def start(self) -> None:
        if self.is_running():
            self.set_status("Running")
            return

        command = [str(self.executable), "--config", str(self.config)]
        try:
            self.process = subprocess.Popen(command)
        except OSError as error:
            self.process = None
            self.set_status(f"Launch failed: {error}")
            return

        self.restart_requested = False
        self.set_status("Starting")

    def stop(self) -> None:
        if not self.is_running():
            self.set_status("Stopped")
            return

        try:
            self.ipc.send_command(STOP_COMMAND)
            self.set_status("Stopping")
        except OSError as error:
            self.set_status(f"Stop failed: {error}")

    def restart(self) -> None:
        if not self.is_running():
            self.start()
            return

        self.restart_requested = True
        self.stop()

    def refresh(self) -> None:
        if self.process is None:
            return

        if self.process.poll() is None:
            if self.status == "Starting":
                self.set_status("Running")
            return

        self.process = None
        should_restart = self.restart_requested
        self.restart_requested = False
        self.set_status("Stopped")

        if should_restart:
            self.start()

    def set_status(self, status: str) -> None:
        self.status = status


class TrayApplication(QObject):
    def __init__(self, process: WallpaperProcess) -> None:
        super().__init__()
        self.process = process

        self.tray = QSystemTrayIcon(self._icon())
        self.tray.setToolTip("PeachWallpaper")

        menu = QMenu()
        self.status_action = QAction(self.process.status, menu)
        self.status_action.setEnabled(False)
        menu.addAction(self.status_action)
        menu.addSeparator()

        start_action = QAction("Start", menu)
        start_action.triggered.connect(self.process.start)
        menu.addAction(start_action)

        stop_action = QAction("Stop", menu)
        stop_action.triggered.connect(self.process.stop)
        menu.addAction(stop_action)

        restart_action = QAction("Restart", menu)
        restart_action.triggered.connect(self.process.restart)
        menu.addAction(restart_action)

        menu.addSeparator()
        quit_action = QAction("Quit tray", menu)
        quit_action.triggered.connect(QApplication.instance().quit)
        menu.addAction(quit_action)

        self.tray.setContextMenu(menu)
        self.tray.show()

        self.refresh_timer = QTimer(self)
        self.refresh_timer.setInterval(250)
        self.refresh_timer.timeout.connect(self.refresh)
        self.refresh_timer.start()

    def refresh(self) -> None:
        self.process.refresh()
        self.status_action.setText(self.process.status)

    @staticmethod
    def _icon() -> QIcon:
        for name in ("preferences-desktop-wallpaper", "application-x-executable"):
            icon = QIcon.fromTheme(name)
            if not icon.isNull():
                return icon
        return QApplication.style().standardIcon(QStyle.StandardPixmap.SP_DesktopIcon)


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--executable",
        type=Path,
        default=Path("peach-wallpaper"),
        help="Wallpaper executable to launch.",
    )
    parser.add_argument(
        "--config",
        type=Path,
        required=True,
        help="Wallpaper configuration passed to the executable.",
    )
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()

    try:
        ipc = IpcClient()
    except RuntimeError as error:
        print(f"peach-wallpaper-tray: {error}", file=sys.stderr)
        return 1

    application = QApplication(sys.argv)
    if not QSystemTrayIcon.isSystemTrayAvailable():
        print("peach-wallpaper-tray: no system tray is available", file=sys.stderr)
        return 1

    process = WallpaperProcess(arguments.executable, arguments.config, ipc)
    tray_application = TrayApplication(process)
    application.aboutToQuit.connect(tray_application.refresh_timer.stop)
    return application.exec()


if __name__ == "__main__":
    raise SystemExit(main())
