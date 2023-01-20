#if VSF_HAL_USE_IO == ENABLED
    io,
#endif
#if VSF_HAL_USE_GPIO == ENABLED
    gpio,
#endif
#if VSF_HAL_USE_I2C == ENABLED
    i2c,
#endif
#if VSF_HAL_USE_SPI == ENABLED
    spi,
#endif
#if VSF_HAL_USE_USART == ENABLED
    usart,
#endif
#if VSF_HAL_USE_MMC == ENABLED
    mmc,
#endif
#if VSF_HAL_USE_ADC == ENABLED
    adc,
#endif
#if VSF_HAL_USE_DAC == ENABLED
    dac,
#endif
#if VSF_HAL_USE_PWM == ENABLED
    pwm,
#endif
#if VSF_HAL_USE_I2S == ENABLED
    i2s,
#endif
#if VSF_USE_USB_DEVICE == ENABLED && VSF_HAL_USE_USBD == ENABLED
    usbd,
#endif
#if VSF_USE_USB_HOST == ENABLED && VSF_HAL_USE_USBH == ENABLED
    usbh,
#endif