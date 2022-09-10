import os
import pathlib
import re
import socket
import subprocess
import tempfile

import pytest

TEST_DIR = pathlib.Path(__file__).parent

FD_PATTERN = re.compile(r"(\d+) (\d+) (\d+) (.+)")
UNIX_NET_PATTERN = re.compile(r"(\d+) ([01]) (.*)")


def test_fds():
    with tempfile.NamedTemporaryFile() as f:
        st = os.stat(f.name)
        p = subprocess.run(
            [TEST_DIR / "test_binary_fds", f.name],
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            encoding="utf-8",
        )
    assert p.returncode == 0

    print(p.stdout)
    mapping = {}
    for line in p.stdout.splitlines():
        assert (match := FD_PATTERN.match(line))
        fd, dev, ino, file = match.groups()
        mapping[int(fd)] = (int(dev), int(ino), file)

    assert (stdin_result := mapping.get(0)) is not None
    assert stdin_result[2] == "/dev/null"

    assert (stderr_result := mapping.get(2)) is not None
    assert stderr_result[2] == "/dev/null"

    assert any(key > 2 and value == (st.st_dev, st.st_ino, f.name) for key, value in mapping.items())


def test_net_socketpair():
    p = subprocess.run([TEST_DIR / "test_binary_net_socketpair"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    inodes = [int(line) for line in p.stdout.splitlines()]

    assert set(inodes[:2]).issubset(set(inodes[2:]))


def test_net_unix_listener():
    domain = "/reap/test"
    p = subprocess.run(
        [TEST_DIR / "test_binary_net_unix_listener", domain], stdout=subprocess.PIPE, encoding="utf-8"
    )
    print(p.stdout)
    assert p.returncode == 0
    lines = p.stdout.splitlines()

    inode = int(lines[0])

    domain = "@" + domain
    for line in lines[1:]:
        values = line.split()
        if len(values) == 2 and int(values[0]) == inode:
            assert values[1] == domain
            break
    else:
        assert False, "Inode not found in output"


def test_net_unix_accepted():
    domain = "/reap/test"
    p = subprocess.run(
        [TEST_DIR / "test_binary_net_unix_accepted", domain], stdout=subprocess.PIPE, encoding="utf-8"
    )
    print(p.stdout)
    assert p.returncode == 0
    lines = p.stdout.splitlines()

    inode = int(lines[0])

    mapping = {}
    domain = "@" + domain
    for line in lines[1:]:
        assert (match := UNIX_NET_PATTERN.match(line))
        ino, connected, addr = match.groups()
        if addr == domain:
            mapping[connected] = int(ino)
            if len(mapping) == 2:
                break

    assert mapping.get("0") == inode
    assert mapping.get("1") is not None
