# README

## Bootloader port

- if reset by app, extend backdoor timeout to wait developer flash new program.

## App port

- overwrite SCB->VTOR to 0x0800'8000 in system_stm32f4xx.c
- modify linker tell app start at 0x0800'8000


## Bootloader check software reset
```c
if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) {
    __HAL_RCC_CLEAR_RESET_FLAGS();
    BackDoorSetExtension(30e3 /* Timeout in ms*/);
}
```

## App trigger software reset
```c
if (true /* trigger condition*/) {
    // print log if need
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}
```