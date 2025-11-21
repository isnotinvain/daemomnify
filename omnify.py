import time

import mido

from chords import Chord


def clamp_note(note):
    if note > 127:
        return (note % 12) + 108  # Clamp to highest octave
    return note


class Omnify:
    def __init__(self, scheduler, chord_channel=0, strum_channel=1, strum_cooldown=0.3, strum_gate_time=0.5):
        self.scheduler = scheduler
        self.current_chord = Chord.MAJOR
        self.note_on_events_of_current_chord = []
        self.hold_chords_indefinately = False
        self.chord_channel = chord_channel
        self.strum_channel = strum_channel
        self.last_strum_time = 0
        self.strum_cooldown = strum_cooldown
        self.last_strum_zone = -1
        self.strum_gate_time = strum_gate_time

    def stop_notes_of_current_chord(self):
        events = []
        for note in self.note_on_events_of_current_chord:
            events.append(mido.Message("note_off", note=note.note, velocity=note.velocity, channel=note.channel))
        self.note_on_events_of_current_chord.clear()
        return events

    def handle_note_on(self, msg):
        # stop currently plaing chord
        events = self.stop_notes_of_current_chord()

        # now we generate the chord note on events
        # (offsets includes 0, so no need to add msg itself,
        # and it might have the wrong channel anyway)
        root = msg.note
        for offset in self.current_chord.value.offsets:
            on = mido.Message("note_on", note=clamp_note(root + offset), velocity=msg.velocity, channel=self.chord_channel)
            events.append(on)
            self.note_on_events_of_current_chord.append(on)

        return events

    def handle_note_off(self, msg):
        if self.note_on_events_of_current_chord and self.note_on_events_of_current_chord[0].note == msg.note:
            # we have released the root of the current chord
            # either stop the chord now or do nothing depending on hold mode
            print(self.note_on_events_of_current_chord[0])
            if not self.hold_chords_indefinately:
                return self.stop_notes_of_current_chord()

        return []

    def handle_control_change(self, msg):
        if msg.is_cc(1):
            if not self.note_on_events_of_current_chord:
                # Theres no current chord, so we don't do anything
                return []

            # we be strummin
            events = []

            now = time.perf_counter()
            cool_down_ready = now >= self.last_strum_time + self.strum_cooldown

            # each zone is 13/128ths in size (plus some floor-ing from integer division)
            strum_plate_zone = int((msg.value * 13) / 128)

            root = self.note_on_events_of_current_chord[0].note % 12
            velocity = self.note_on_events_of_current_chord[0].velocity

            if strum_plate_zone != self.last_strum_zone or cool_down_ready:
                # we'll allow it, we've crossed into the next strumming area or it's been long enough
                note_to_play = self.current_chord.value.arp[root][strum_plate_zone]
                events.append(mido.Message("note_on", note=note_to_play, velocity=velocity, channel=self.strum_channel))
                self.scheduler.schedule(mido.Message("note_off", note=note_to_play, channel=self.strum_channel), self.strum_gate_time)
                self.last_strum_time = now
                self.last_strum_zone = strum_plate_zone

            return events
        return []

    def handle_incoming_message(self, msg):
        match msg.type:
            case "note_on":
                if msg.velocity == 0:
                    # dumb dumb dumb
                    return self.handle_note_off(msg)
                else:
                    return self.handle_note_on(msg)
            case "note_off":
                return self.handle_note_off(msg)
            case "control_change":
                return self.handle_control_change(msg)
            case _:
                return []
