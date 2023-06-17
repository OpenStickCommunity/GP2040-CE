"""Visualize the protobuf config from a picotool flash dump.

This is more manual than I'd like at the moment, but this demonstrates/documents the means
to display the config on a board. It requires a checkout that matches the version of the
firmware on the board (so that the protobuf messages match the flash, though protobuf
tolerates differences well, so it'll probably work, just be misnamed or incomplete).

Env setup:
% python -m venv venv
% source ./venv/bin/activate
% pip install -Ur requirements.txt
% mkdir build

Generating the Python proto code:
[env setup]
% protoc  ../../proto/* -I../../proto/ -I../../lib/nanopb/generator/proto --python_out=build

Usage:
[env setup]
% picotool save -r 101FE000 101FFFF4 build/memory.bin   # 101FE000 = storage start, 101FFFF4 storage end - footer
% export PYTHONPATH=../../lib/nanopb/generator/proto:build
% python visualize.py
"""
import pprint

import click
from config_pb2 import Config


def get_config(filename):
    """Load the protobuf section of an flash and display the contents."""
    with open(filename, 'rb') as dump:
        # read off the unused space
        while True:
            byte = dump.read(1)
            if byte != b'\x00':
                break
        content = byte + dump.read()
    config = Config()
    config.ParseFromString(content)
    return config


@click.command()
@click.argument('filename')
@click.option('--redump', default=None, help="re-serialize the config to the specified filename")
def visualize(filename, redump):
    """Read FILENAME (a .bin dump of a GP2040-CE's Protobuf storage section (commonly 101FE000-101FFFF4))
    and print out its contents."""
    config = get_config(filename)
    pprint.pprint(config)

    if redump:
        print(f"writing config to new binary file {redump}...")
        with open(redump, 'wb') as dump:
            dump.write(config.SerializeToString())


if __name__ == '__main__':
    visualize()
