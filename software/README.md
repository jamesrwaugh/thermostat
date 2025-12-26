# Software Start

```
meson setup --wipe --cross-file avr-cross-compile.txt builddir -Dbuild.pkg_config_path=/usr/local/lib/pkgconfig
```

# Software Go

## Read Fuses

```
avrdude -p atmega644p -c usbtiny -U hfuse:r:-:h -U lfuse:r:-:h
```

## Go

```
avrdude -p atmega644p -c usbtiny -U flash:w:./builddir/src/app-screen/thermo-screen-app
```

# Simulator Bugs Hall of Fame

In line with tradition, we pay respects, and list difficult bugs that would have been very difficult to solve on real hardware, but were found using the simulator.

- [x] Fixed incorrect register usage with Timer1 counting 10 ms
- [x] Using TIMER1_OVF_vect instead of TIMER1_COMPA_vect
- [x] RTC IIC interface was taking left-shifted addresses, not right
- [x] TMP116 double-sending address in I2C
- [x] I2C pin config copied/pasted from 644 setup, and addition of reverse valve, set wrong DDRC pins, resulting in heat not turning off.
