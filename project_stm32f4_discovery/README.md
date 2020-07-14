# STM32F407-Discovery board demo project

This is a simple example project for the STM32F407-Discovery Board to show the usage
of the STM32F4 hardware abstraction layer of the `STM_HAL` project.

## Functionality

The project consists of four tasks/apps:
1. LedBlinker:

   The LedBlinker controls the four leds on the discovery board. They light up in
   clockwise direction.

2. TestRtc:

   This Tasks initializes the real time clock (RTC) to 12 o'clock on the 14th of
   September 2015 and then prints once a second the current RTC reading to the
   debug interface.

3. uart demo (anonymous, created in main procedure):

   This tasks uses another uart communication interface. It yields until the reading
   of two characters and then prints them back in reversed order.

4. adc demo (anonymous, created in main procedure):

   Every second this tasks prints the current  reading of the analog to digital
   converter (ADC) on pin C1 to the debug interface.

By pressing the "User Button" the direction of the LedBlinker animation can be
inversed.

All of these Tasks can be monitored by SEGGER SystemView. The interrupt tracing is
enabled as well. It is set by the startup script.

## Pin mapping

- Debug interface on
  - Pin A8 Tx
  - Pin A9 Rx

- Communication interface on
  - Pin C10 Tx
  - Pin C11 Rx

- Single Wire Debug on
  - Pin A14 SWD
  - Pin A13 Reset

- Discovery Board "User Button" on pin C0

- ADC analog input pin C1
