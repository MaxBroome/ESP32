Import("env")
import os

version_file = os.path.join(env.subst("$PROJECT_DIR"), "VERSION")
with open(version_file) as f:
    version = f.read().strip()

env.Append(BUILD_FLAGS=[f'-DFIRMWARE_VERSION=\\"{version}\\"'])
print(f"Firmware version: {version}")