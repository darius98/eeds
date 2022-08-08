#!/usr/bin/env python3

import os.path

working_dir = os.path.dirname(__file__)

latest = None
latest_timestamp = None

for entry in os.listdir(working_dir):
    entry_path = os.path.join(working_dir, entry)
    if "crash-" in entry:
        if "crash-latest" in entry:
            print("crash-latest already exists. Fix it and remove it before invoking this!!")
            exit(0)
        entry_creat_time = os.stat(entry_path).st_ctime
        if latest_timestamp is None or entry_creat_time > latest_timestamp:
            latest = entry
            latest_timestamp = entry_creat_time

if latest is None:
    print("No crash entry found.")
    exit(0)

os.rename(os.path.join(working_dir, latest), os.path.join(working_dir, "crash-latest"))
print("Renamed latest crash.")
