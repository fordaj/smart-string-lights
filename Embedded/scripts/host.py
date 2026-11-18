Import("env")  # PlatformIO specific syntax error, fine to ignore
import sys

if sys.platform == "win32":
    env.Append(LIBS=["ws2_32"])  # Needed for cpp-httplib on Windows
