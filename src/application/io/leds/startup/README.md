# Custom startup routine for board variants

In order for board variant to have custom LED startup routine, add a .cpp file called the same way as board variant directory which implements
`LEDs::startUpAnimation` function.