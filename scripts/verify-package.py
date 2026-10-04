#!/usr/bin/env python3
"""Verify a Mission-Control ZIP against the local release build."""
import re
import struct
import sys
from pathlib import Path, PurePosixPath
import zipfile


def pfs_entries(data):
    if data[:4] != b"PFS0":
        raise ValueError("Invalid NSP header")
    count, strings_size = struct.unpack_from("<II", data, 4)
    strings = data[16 + count * 24:16 + count * 24 + strings_size]
    base = 16 + count * 24 + strings_size
    result = {}
    for index in range(count):
        offset, size, name_offset = struct.unpack_from("<QQI", data, 16 + 24 * index)
        if base + offset + size > len(data):
            raise ValueError("Invalid NSP entry bounds")
        name = strings[name_offset:].split(b"\0", 1)[0].decode()
        result[name] = data[base + offset:base + offset + size]
    return result


def verify(path):
    root = Path(__file__).resolve().parent.parent
    makefile = (root / "Makefile").read_text()
    version = re.search(r"^MC_VERSION := ([0-9.]+)$", makefile, re.M).group(1)
    tid = re.search(r"^MC_MITM_TID := ([0-9a-f]+)$", makefile, re.M).group(1)
    prefix = "atmosphere/contents/" + tid + "/"
    build = root / "mc_mitm/out/nintendo_nx_arm64_armv8a/release"
    with zipfile.ZipFile(path) as archive:
        if archive.testzip() is not None:
            raise ValueError("ZIP checksum failure")
        for name in archive.namelist():
            if PurePosixPath(name).is_absolute() or ".." in PurePosixPath(name).parts:
                raise ValueError("Unsafe ZIP entry")
        nsp = archive.read(prefix + "exefs.nsp")
        if nsp != (build / "mc_mitm.nsp").read_bytes():
            raise ValueError("Archive NSP differs from local build")
        entries = pfs_entries(nsp)
        if entries["main"] != (build / "mc_mitm.nso").read_bytes():
            raise ValueError("Embedded NSO mismatch")
        npdm = entries["main.npdm"]
        if npdm != (build / "mc_mitm.npdm").read_bytes() or npdm[:4] != b"META":
            raise ValueError("Embedded NPDM mismatch")
        offset = struct.unpack_from("<I", npdm, 0x70)[0]
        if npdm[offset:offset + 4] != b"ACI0" or struct.unpack_from("<Q", npdm, offset + 0x10)[0] != int(tid, 16):
            raise ValueError("Incorrect sysmodule TID")
        if archive.read(prefix + "mitm.lst").decode().splitlines() != ["btdrv", "btm"]:
            raise ValueError("Incorrect MITM services")
        if archive.read(prefix + "flags/boot2.flag") != b"":
            raise ValueError("Invalid boot flag")
        template = "config/MissionControl/missioncontrol.ini.template"
        if archive.read(template) != (root / "mc_mitm/config.ini").read_bytes():
            raise ValueError("Incorrect configuration template")
        if "config/MissionControl/missioncontrol.ini" in archive.namelist():
            raise ValueError("Archive would replace user configuration")
        for file in (root / "exefs_patches").rglob("*.ips"):
            data = archive.read("atmosphere/" + file.relative_to(root).as_posix())
            if data != file.read_bytes() or data[:5] != b"IPS32":
                raise ValueError("Incorrect Bluetooth patch: " + file.name)
            cursor = 5
            while data[cursor:cursor + 4] != b"EEOF":
                if cursor + 6 > len(data):
                    raise ValueError("Truncated IPS32 record")
                size = int.from_bytes(data[cursor + 4:cursor + 6], "big")
                cursor += 6 + (size if size else 3)
                if cursor > len(data) - 4:
                    raise ValueError("Invalid IPS32 record bounds")
            if cursor + 4 != len(data):
                raise ValueError("Invalid IPS32 end marker")
    if (version + "-").encode() not in (build / "mc_mitm.elf").read_bytes():
        raise ValueError("Compiled version does not match Makefile")
    print("Verified Mission-Control " + version + ": ZIP, NSP/NSO, TID, MITM, configuration, patches and version")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("Usage: verify-package.py RELEASE.zip")
    verify(sys.argv[1])
