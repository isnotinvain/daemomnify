#!/usr/bin/env python3

import heapq
import sys
import time
import traceback
from dataclasses import dataclass, field
from typing import Any

import mido

from omnify import Omnify


@dataclass(order=True)
class ScheduledMessage:
    send_time: int
    msg: Any = field(compare=False)


class MessageScheduler:
    def __init__(self):
        self.queue = []

    def schedule(self, msg, delay_seconds):
        send_time = time.perf_counter() + delay_seconds
        heapq.heappush(self.queue, ScheduledMessage(send_time, msg))

    def send_overdue_messages(self, output_port):
        current_time = time.perf_counter()

        while self.queue and self.queue[0].send_time <= current_time:
            scheduled = heapq.heappop(self.queue)
            try:
                output_port.send(scheduled.msg)
            except OSError as e:
                print(f"Error sending scheduled message {scheduled.msg}: {e}")
                traceback.print_exc()


def list_midi_devices():
    devices = mido.get_input_names()
    if not devices:
        return []

    return devices


def select_device(devices):
    """Prompt user to select a MIDI device from the list."""
    if not devices:
        return None

    while True:
        try:
            choice = input(f"Select device (1-{len(devices)}): ")
            idx = int(choice) - 1

            if 0 <= idx < len(devices):
                return devices[idx]
            else:
                print(f"Please enter a number between 1 and {len(devices)}")
        except ValueError:
            print("Please enter a valid number")
        except KeyboardInterrupt:
            print("Cancelled.")
            return None


def create_virtual_output(port_name="Daemomnify"):
    try:
        return mido.open_output(port_name, virtual=True)
    except OSError as e:
        print(f"Error creating virtual output: {e}")
        traceback.print_exc()
        return None


def main():
    print("=== Welcome to Daemomnify. Let's Omnify some instruments! ===")

    # List and select input device
    devices = list_midi_devices()
    if not devices:
        print("No MIDI input devices found! Plug in / setup some midi controllers and try again.")
        sys.exit(1)

    print("Available MIDI input devices:")
    for idx, name in enumerate(devices, 1):
        print(f"  {idx}. {name}")

    selected_device = select_device(devices)
    if not selected_device:
        sys.exit(0)

    print(f"Selected: {selected_device}")

    # Create virtual output
    print("Creating virtual MIDI output...")
    virtual_output = create_virtual_output()
    if not virtual_output:
        print("Failed to create virtual output! Dang! See ya!")
        sys.exit(1)

    print(f"Virtual output created: {virtual_output.name}")

    # Create message scheduler
    scheduler = MessageScheduler()
    omnify = Omnify(scheduler)

    # Open input device and start listening
    print(f"Listening to {selected_device}...")
    print("Press Ctrl+C to stop")

    try:
        with mido.open_input(selected_device) as inport:
            while True:
                # Check for any pending incoming messages (non-blocking)
                for msg in inport.iter_pending():
                    to_send_now = omnify.handle_incoming_message(msg)
                    # Send immediate message to virtual output
                    try:
                        for m in to_send_now:
                            virtual_output.send(m)
                    except OSError as e:
                        print(f"Error sending message {m}: {e}")
                        traceback.print_exc()

                # Send any scheduled messages whose time has arrived
                scheduler.send_overdue_messages(virtual_output)

                # Sleep briefly to avoid spinning CPU (1ms = ~1000Hz poll rate)
                time.sleep(0.001)

    except KeyboardInterrupt:
        print("Stopping...")
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
    finally:
        if virtual_output:
            virtual_output.close()
        print("Daemomnify banished.")


if __name__ == "__main__":
    main()
