# Compile options

VERBOSE=y
OPT=g
USE_NANO=y
SEMIHOST=n
USE_FPU=y

# Libraries

USE_LPCOPEN=y
USE_SAPI=y
USE_FREERTOS=y
FREERTOS_HEAP_TYPE=4

USE_FATFS=y

# Let FSSDC/FSUSB print their own status messages by DEBUGSTR()
DEFINES+=DEBUG_ENABLE
