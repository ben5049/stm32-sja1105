# SJA1105 Driver for STM32

## Thread Safety

All the functions in sja1105.h are thread safe, with the exception of SJA1105_PortConfigure() which should only be called from a single thread at startup and before SJA1105_Init().
