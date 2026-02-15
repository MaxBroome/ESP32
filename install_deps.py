Import("env")

import subprocess
import sys

try:
    import fatfs
    if not hasattr(fatfs, 'create_extended_partition'):
        print("Uninstalling incorrect fatfs package...")
        subprocess.check_call([sys.executable, "-m", "pip", "uninstall", "-y", "fatfs"])
except ImportError:
    pass

packages = [
    ("littlefs-python>=0.16.0", "littlefs"),
    ("fatfs-ng>=0.1.14", "fatfs"),
    ("pyyaml>=6.0.2", "yaml")
]

for package_spec, module_name in packages:
    try:
        mod = __import__(module_name)
        if module_name == "fatfs" and not hasattr(mod, 'create_extended_partition'):
            raise ImportError("Wrong fatfs package")
    except ImportError:
        print(f"Installing {package_spec}...")
        subprocess.check_call([sys.executable, "-m", "pip", "install", package_spec])