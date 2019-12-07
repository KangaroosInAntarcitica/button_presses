## Button presses counter
Group: _Andriy Dmytruk_, _Anatolii Iatsuk_

To start button presses counter:
```bash
# build on device
make CROSS=0
# start the modules
insmod gpio_lkm.ko
insmod button-listener.ko
# start the consumer to find out button counts
insmod button-listener-consumer.ko
```
Button listener listens to button presses (On GPIO19 by default. Other side through resistor +3V). If the number of presses is greater than 10, it turns on the LED (On GPIO16, other side to GND).  

The `button_listener` module uses `gpio_lkm` `gpio_lkm_free_gpio` function. It will stop using the gpio and delete the user file, so `button_listener` can use it. Now user cannot use the cooresponding GPIOs. Button listener than creates an inerrupt handler for `HIGH_EDGE` on the GPIO. It creates a file at `/sys/press_counter/button19` named `press_count` that you can read from. 

The `gpio_lkm` exports symbol `gpio_unavailable` that is used to mark that the gpios are broken and cannot be used. It also export 2 symbols, that can be used to read a GPIO or command it from other kernel modules. These are not used in `button-listener` 

The `button_listener_consumer` module uses the exported function `get_count()` to get the number of received presses and show it to user.

