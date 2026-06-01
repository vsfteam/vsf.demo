/* Auto-generated from test_params.yml — do not edit manually */
#ifndef __TEST_PARAMS_GENERATED_H__
#define __TEST_PARAMS_GENERATED_H__

#define VSF_TEST_MARKER_DELAY_MS  2
#define VSF_TEST_USART_TX_BAUD_PAYLOAD          "Hello VSF\r\n"
#define VSF_TEST_USART_TX_BAUD_PAYLOAD_DRAIN_MS 15
#define VSF_TEST_USART_TX_MODE_PAYLOAD          "0123456789\r\n"
#define VSF_TEST_USART_TX_MODE_PAYLOAD_DRAIN_MS 10
#define VSF_TEST_USART_RX_DATA_PAYLOAD          "Hello VSF\r\n"
#define VSF_TEST_USART_RX_DATA_PAYLOAD_DRAIN_MS 500
#define VSF_TEST_USART_RX_BAUD_PAYLOAD          "Hello VSF\r\n"
#define VSF_TEST_USART_RX_BAUD_PAYLOAD_DRAIN_MS 500
#define VSF_TEST_USART_RX_MODE_PAYLOAD          "0123456789\r\n"
#define VSF_TEST_USART_RX_MODE_PAYLOAD_DRAIN_MS 500
#define VSF_TEST_USART_RX_IRQ_PAYLOAD          "Hello VSF\r\n"
#define VSF_TEST_USART_RX_IRQ_PAYLOAD_DRAIN_MS 500
#define VSF_TEST_USART_RX_TIMEOUT_PAYLOAD          "Hello VSF\r\n"
#define VSF_TEST_USART_RX_TIMEOUT_PAYLOAD_DRAIN_MS 500
#define VSF_TEST_USART_RX_PARITY_ERROR_PAYLOAD          "Hello VSF\r\n"
#define VSF_TEST_USART_RX_PARITY_ERROR_PAYLOAD_DRAIN_MS 500
#define VSF_TEST_USART_RX_FRAME_ERROR_PAYLOAD          "Hello VSF\r\n"
#define VSF_TEST_USART_RX_FRAME_ERROR_PAYLOAD_DRAIN_MS 500

/* === tx_baud (usart_tx_baud) === */

#define VSF_TEST_USART_TX_BAUD_TIMEOUT_MS  ((uint32_t)(10.5 * 1000))

#define VSF_TEST_USART_TX_BAUD_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_TX_BAUD_DEFAULT_MODE  (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_TX_ENABLE)
#define VSF_TEST_USART_TX_BAUD_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_TX_BAUD_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0, .baudrate = 9600 },  \
    { .expect_pass = true, .idx = 1, .baudrate = 19200 },  \
    { .expect_pass = true, .idx = 2, .baudrate = 38400 },  \
    { .expect_pass = true, .idx = 3, .baudrate = 57600 },  \
    { .expect_pass = true, .idx = 4, .baudrate = 115200 },  \
    { .expect_pass = true, .idx = 5, .baudrate = 230400 },  \
    { .expect_pass = true, .idx = 6, .baudrate = 460800 },  \
    { .expect_pass = true, .idx = 7, .baudrate = 921600 },  \
    { .expect_pass = false, .idx = 8, .baudrate = 0 },  \
    { .expect_pass = false, .idx = 9, .baudrate = 100000000 },  \
    { .expect_pass = true, .idx = 10, .baudrate = 115200, .data_size_bytes = 1024 }
#define VSF_TEST_USART_TX_BAUD_CASE_COUNT  11

#ifndef VSF_TEST_USART_TX_BAUD_ENABLE
#   define VSF_TEST_USART_TX_BAUD_ENABLE  ENABLED
#endif

/* === tx_mode (usart_tx_mode) === */

#define VSF_TEST_USART_TX_MODE_TIMEOUT_MS  ((uint32_t)(10.5 * 1000))

#define VSF_TEST_USART_TX_MODE_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_TX_MODE_DEFAULT_BAUDRATE  115200
#define VSF_TEST_USART_TX_MODE_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_TX_MODE_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0, .mode = (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 1, .mode = (VSF_USART_EVEN_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 2, .mode = (VSF_USART_ODD_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 3, .mode = (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_7_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 4, .mode = (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_6_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 5, .mode = (VSF_USART_NO_PARITY | VSF_USART_2_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 6, .mode = (VSF_USART_EVEN_PARITY | VSF_USART_1_STOPBIT | VSF_USART_7_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 7, .mode = (VSF_USART_ODD_PARITY | VSF_USART_2_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 8, .mode = (VSF_USART_FORCE_0_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 9, .mode = (VSF_USART_FORCE_1_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_TX_ENABLE) },  \
    { .expect_pass = true, .idx = 10, .mode = (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_5_BIT_LENGTH | VSF_USART_TX_ENABLE) }
#define VSF_TEST_USART_TX_MODE_CASE_COUNT  11

#ifndef VSF_TEST_USART_TX_MODE_ENABLE
#   define VSF_TEST_USART_TX_MODE_ENABLE  ENABLED
#endif

/* === rx_data (usart_rx_data) === */

#define VSF_TEST_USART_RX_DATA_TIMEOUT_MS  ((uint32_t)(1.5 * 1000))

#define VSF_TEST_USART_RX_DATA_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_DATA_DEFAULT_MODE  (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE)
#define VSF_TEST_USART_RX_DATA_DEFAULT_BAUDRATE  115200
#define VSF_TEST_USART_RX_DATA_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_RX_DATA_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0 },  \
    { .expect_pass = true, .idx = 1, .data_size_bytes = 64 },  \
    { .expect_pass = true, .idx = 2, .data_size_bytes = 256 },  \
    { .expect_pass = true, .idx = 3, .data_size_bytes = 1024 },  \
    { .expect_pass = true, .idx = 4, .data_size_bytes = 4096 }
#define VSF_TEST_USART_RX_DATA_CASE_COUNT  5

#ifndef VSF_TEST_USART_RX_DATA_ENABLE
#   define VSF_TEST_USART_RX_DATA_ENABLE  ENABLED
#endif

/* === rx_baud (usart_rx_baud) === */

#define VSF_TEST_USART_RX_BAUD_TIMEOUT_MS  ((uint32_t)(8.5 * 1000))

#define VSF_TEST_USART_RX_BAUD_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_BAUD_DEFAULT_MODE  (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE)
#define VSF_TEST_USART_RX_BAUD_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_RX_BAUD_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0, .baudrate = 9600 },  \
    { .expect_pass = true, .idx = 1, .baudrate = 19200 },  \
    { .expect_pass = true, .idx = 2, .baudrate = 38400 },  \
    { .expect_pass = true, .idx = 3, .baudrate = 57600 },  \
    { .expect_pass = true, .idx = 4, .baudrate = 115200 },  \
    { .expect_pass = true, .idx = 5, .baudrate = 230400 },  \
    { .expect_pass = true, .idx = 6, .baudrate = 460800 },  \
    { .expect_pass = true, .idx = 7, .baudrate = 921600 }
#define VSF_TEST_USART_RX_BAUD_CASE_COUNT  8

#ifndef VSF_TEST_USART_RX_BAUD_ENABLE
#   define VSF_TEST_USART_RX_BAUD_ENABLE  ENABLED
#endif

/* === rx_mode (usart_rx_mode) === */

#define VSF_TEST_USART_RX_MODE_TIMEOUT_MS  ((uint32_t)(8.5 * 1000))

#define VSF_TEST_USART_RX_MODE_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_MODE_DEFAULT_BAUDRATE  115200
#define VSF_TEST_USART_RX_MODE_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_RX_MODE_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0, .mode = (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE) },  \
    { .expect_pass = true, .idx = 1, .mode = (VSF_USART_EVEN_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE) },  \
    { .expect_pass = true, .idx = 2, .mode = (VSF_USART_ODD_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE) },  \
    { .expect_pass = true, .idx = 3, .mode = (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_7_BIT_LENGTH | VSF_USART_RX_ENABLE) },  \
    { .expect_pass = true, .idx = 4, .mode = (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_6_BIT_LENGTH | VSF_USART_RX_ENABLE) },  \
    { .expect_pass = true, .idx = 5, .mode = (VSF_USART_NO_PARITY | VSF_USART_2_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE) },  \
    { .expect_pass = true, .idx = 6, .mode = (VSF_USART_EVEN_PARITY | VSF_USART_1_STOPBIT | VSF_USART_7_BIT_LENGTH | VSF_USART_RX_ENABLE) },  \
    { .expect_pass = true, .idx = 7, .mode = (VSF_USART_ODD_PARITY | VSF_USART_2_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE) }
#define VSF_TEST_USART_RX_MODE_CASE_COUNT  8

#ifndef VSF_TEST_USART_RX_MODE_ENABLE
#   define VSF_TEST_USART_RX_MODE_ENABLE  ENABLED
#endif

/* === rx_irq (usart_rx_irq) === */

#define VSF_TEST_USART_RX_IRQ_TIMEOUT_MS  ((uint32_t)(1.5 * 1000))

#define VSF_TEST_USART_RX_IRQ_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_IRQ_DEFAULT_MODE  (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE)
#define VSF_TEST_USART_RX_IRQ_DEFAULT_BAUDRATE  115200
#define VSF_TEST_USART_RX_IRQ_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_RX_IRQ_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0 }
#define VSF_TEST_USART_RX_IRQ_CASE_COUNT  1

#ifndef VSF_TEST_USART_RX_IRQ_ENABLE
#   define VSF_TEST_USART_RX_IRQ_ENABLE  ENABLED
#endif

/* === rx_timeout (usart_rx_timeout) === */

#define VSF_TEST_USART_RX_TIMEOUT_TIMEOUT_MS  ((uint32_t)(1.5 * 1000))

#define VSF_TEST_USART_RX_TIMEOUT_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_TIMEOUT_DEFAULT_MODE  (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE)
#define VSF_TEST_USART_RX_TIMEOUT_DEFAULT_BAUDRATE  115200
#define VSF_TEST_USART_RX_TIMEOUT_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_RX_TIMEOUT_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0 }
#define VSF_TEST_USART_RX_TIMEOUT_CASE_COUNT  1

#ifndef VSF_TEST_USART_RX_TIMEOUT_ENABLE
#   define VSF_TEST_USART_RX_TIMEOUT_ENABLE  ENABLED
#endif

/* === rx_parity_error (usart_rx_parity_error) === */

#define VSF_TEST_USART_RX_PARITY_ERROR_TIMEOUT_MS  ((uint32_t)(1.5 * 1000))

#define VSF_TEST_USART_RX_PARITY_ERROR_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_PARITY_ERROR_DEFAULT_BAUDRATE  115200
#define VSF_TEST_USART_RX_PARITY_ERROR_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_RX_PARITY_ERROR_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0, .mode = (VSF_USART_EVEN_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE) }
#define VSF_TEST_USART_RX_PARITY_ERROR_CASE_COUNT  1

#ifndef VSF_TEST_USART_RX_PARITY_ERROR_ENABLE
#   define VSF_TEST_USART_RX_PARITY_ERROR_ENABLE  ENABLED
#endif

/* === rx_frame_error (usart_rx_frame_error) === */

#define VSF_TEST_USART_RX_FRAME_ERROR_TIMEOUT_MS  ((uint32_t)(1.5 * 1000))

#define VSF_TEST_USART_RX_FRAME_ERROR_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_FRAME_ERROR_DEFAULT_BAUDRATE  115200
#define VSF_TEST_USART_RX_FRAME_ERROR_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_RX_FRAME_ERROR_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0, .mode = (VSF_USART_NO_PARITY | VSF_USART_2_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE) }
#define VSF_TEST_USART_RX_FRAME_ERROR_CASE_COUNT  1

#ifndef VSF_TEST_USART_RX_FRAME_ERROR_ENABLE
#   define VSF_TEST_USART_RX_FRAME_ERROR_ENABLE  ENABLED
#endif

/* === rx_break_error (usart_rx_break_error) === */

#define VSF_TEST_USART_RX_BREAK_ERROR_TIMEOUT_MS  ((uint32_t)(1.5 * 1000))

#define VSF_TEST_USART_RX_BREAK_ERROR_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_BREAK_ERROR_DEFAULT_BAUDRATE  115200
#define VSF_TEST_USART_RX_BREAK_ERROR_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_RX_BREAK_ERROR_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0, .mode = (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE) }
#define VSF_TEST_USART_RX_BREAK_ERROR_CASE_COUNT  1

#ifndef VSF_TEST_USART_RX_BREAK_ERROR_ENABLE
#   define VSF_TEST_USART_RX_BREAK_ERROR_ENABLE  ENABLED
#endif

/* === rx_overflow_error (usart_rx_overflow_error) === */

#define VSF_TEST_USART_RX_OVERFLOW_ERROR_TIMEOUT_MS  ((uint32_t)(1.5 * 1000))

#define VSF_TEST_USART_RX_OVERFLOW_ERROR_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_OVERFLOW_ERROR_DEFAULT_BAUDRATE  115200
#define VSF_TEST_USART_RX_OVERFLOW_ERROR_DEFAULT_EXPECT_PASS  true

#define VSF_TEST_USART_RX_OVERFLOW_ERROR_PARAMS_INIT  \
    { .expect_pass = true, .idx = 0, .mode = (VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_8_BIT_LENGTH | VSF_USART_RX_ENABLE) }
#define VSF_TEST_USART_RX_OVERFLOW_ERROR_CASE_COUNT  1

#ifndef VSF_TEST_USART_RX_OVERFLOW_ERROR_ENABLE
#   define VSF_TEST_USART_RX_OVERFLOW_ERROR_ENABLE  ENABLED
#endif

/* === tx_fifo_irq (usart_tx_fifo_irq) === */

#define VSF_TEST_USART_TX_FIFO_IRQ_SUITE_TIMEOUT_MS  ((uint32_t)(30.0 * 1000))

#define VSF_TEST_USART_TX_FIFO_IRQ_PARAMS_INIT  \
    { .idx = 0, .refill_target = 4 },  \
    { .idx = 1, .refill_target = 8 },  \
    { .idx = 2, .refill_target = 16 },  \
    { .idx = 3, .refill_target = 32 },  \
    { .idx = 4, .refill_target = 64 },  \
    { .idx = 5, .refill_target = 128 },  \
    { .idx = 6, .refill_target = 256 },  \
    { .idx = 7, .refill_target = 512 },  \
    { .idx = 8, .refill_target = 1024 }
#define VSF_TEST_USART_TX_FIFO_IRQ_CASE_COUNT  9

#ifndef VSF_TEST_USART_TX_FIFO_IRQ_ENABLE
#   define VSF_TEST_USART_TX_FIFO_IRQ_ENABLE  ENABLED
#endif

/* === rx_fifo_irq (usart_rx_fifo_irq) === */

#define VSF_TEST_USART_RX_FIFO_IRQ_SUITE_TIMEOUT_MS  ((uint32_t)(30.0 * 1000))

#define VSF_TEST_USART_RX_FIFO_IRQ_PARAMS_INIT  \
    { .idx = 0, .refill_target = 4, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL },  \
    { .idx = 1, .refill_target = 4, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_NOT_EMPTY },  \
    { .idx = 2, .refill_target = 4, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_FULL },  \
    { .idx = 3, .refill_target = 8, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL },  \
    { .idx = 4, .refill_target = 16, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL },  \
    { .idx = 5, .refill_target = 32, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL },  \
    { .idx = 6, .refill_target = 64, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL },  \
    { .idx = 7, .refill_target = 128, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL },  \
    { .idx = 8, .refill_target = 256, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL },  \
    { .idx = 9, .refill_target = 512, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL },  \
    { .idx = 10, .refill_target = 1024, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL }
#define VSF_TEST_USART_RX_FIFO_IRQ_CASE_COUNT  11

#ifndef VSF_TEST_USART_RX_FIFO_IRQ_ENABLE
#   define VSF_TEST_USART_RX_FIFO_IRQ_ENABLE  ENABLED
#endif

/* === request_tx_irq (usart_request_tx_irq) === */

#define VSF_TEST_USART_REQUEST_TX_IRQ_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_REQUEST_TX_IRQ_PARAMS_INIT  \
    { .idx = 0, .refill_target = 4 }
#define VSF_TEST_USART_REQUEST_TX_IRQ_CASE_COUNT  1

#ifndef VSF_TEST_USART_REQUEST_TX_IRQ_ENABLE
#   define VSF_TEST_USART_REQUEST_TX_IRQ_ENABLE  ENABLED
#endif

/* === request_rx_irq (usart_request_rx_irq) === */

#define VSF_TEST_USART_REQUEST_RX_IRQ_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_REQUEST_RX_IRQ_PARAMS_INIT  \
    { .idx = 0, .refill_target = 4 }
#define VSF_TEST_USART_REQUEST_RX_IRQ_CASE_COUNT  1

#ifndef VSF_TEST_USART_REQUEST_RX_IRQ_ENABLE
#   define VSF_TEST_USART_REQUEST_RX_IRQ_ENABLE  ENABLED
#endif

/* === request_cancel (usart_request_cancel) === */

#define VSF_TEST_USART_REQUEST_CANCEL_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_REQUEST_CANCEL_PARAMS_INIT  \
    { .idx = 0, .refill_target = 8, .cancel_after_us = 2000 }
#define VSF_TEST_USART_REQUEST_CANCEL_CASE_COUNT  1

#ifndef VSF_TEST_USART_REQUEST_CANCEL_ENABLE
#   define VSF_TEST_USART_REQUEST_CANCEL_ENABLE  ENABLED
#endif

/* === usart_break_signal (usart_break_signal) === */

#define VSF_TEST_USART_BREAK_SIGNAL_TIMEOUT_MS  ((uint32_t)(1.5 * 1000))

#define VSF_TEST_USART_BREAK_SIGNAL_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_USART_BREAK_SIGNAL_PARAMS_INIT  \
    { .idx = 0, .baudrate = 115200, .hold_ms = 5 }
#define VSF_TEST_USART_BREAK_SIGNAL_CASE_COUNT  1

#ifndef VSF_TEST_USART_BREAK_SIGNAL_ENABLE
#   define VSF_TEST_USART_BREAK_SIGNAL_ENABLE  ENABLED
#endif

/* === usart_hw_flow_control (usart_hw_flow_control) === */

#define VSF_TEST_USART_HW_FLOW_CONTROL_TIMEOUT_MS  ((uint32_t)(1.5 * 1000))

#define VSF_TEST_USART_HW_FLOW_CONTROL_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_USART_HW_FLOW_CONTROL_PARAMS_INIT  \
    { .idx = 0, .flow_mode = VSF_USART_RTS_HWCONTROL },  \
    { .idx = 1, .flow_mode = VSF_USART_CTS_HWCONTROL },  \
    { .idx = 2, .flow_mode = VSF_USART_RTS_CTS_HWCONTROL }
#define VSF_TEST_USART_HW_FLOW_CONTROL_CASE_COUNT  3

#ifndef VSF_TEST_USART_HW_FLOW_CONTROL_ENABLE
#   define VSF_TEST_USART_HW_FLOW_CONTROL_ENABLE  ENABLED
#endif

/* === rx_bulk_irq (usart_rx_bulk_irq) === */

#define VSF_TEST_USART_RX_BULK_IRQ_TIMEOUT_MS  ((uint32_t)(30.0 * 1000))

#define VSF_TEST_USART_RX_BULK_IRQ_SUITE_TIMEOUT_MS  ((uint32_t)(30.0 * 1000))

#define VSF_TEST_USART_RX_BULK_IRQ_PARAMS_INIT  \
    { .idx = 0, .data_size_bytes = 64 },  \
    { .idx = 1, .data_size_bytes = 256 },  \
    { .idx = 2, .data_size_bytes = 1024 },  \
    { .idx = 3, .data_size_bytes = 4096 }
#define VSF_TEST_USART_RX_BULK_IRQ_CASE_COUNT  4

#ifndef VSF_TEST_USART_RX_BULK_IRQ_ENABLE
#   define VSF_TEST_USART_RX_BULK_IRQ_ENABLE  ENABLED
#endif

/* === rx_fifo_threshold (usart_rx_fifo_threshold) === */

#define VSF_TEST_USART_RX_FIFO_THRESHOLD_TIMEOUT_MS  ((uint32_t)(10.0 * 1000))

#define VSF_TEST_USART_RX_FIFO_THRESHOLD_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_USART_RX_FIFO_THRESHOLD_PARAMS_INIT  \
    { .idx = 0, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_HALF_FULL, .expected_bytes = 16 },  \
    { .idx = 1, .threshold_mode = VSF_USART_RX_FIFO_THRESHOLD_FULL, .expected_bytes = 28 }
#define VSF_TEST_USART_RX_FIFO_THRESHOLD_CASE_COUNT  2

#ifndef VSF_TEST_USART_RX_FIFO_THRESHOLD_ENABLE
#   define VSF_TEST_USART_RX_FIFO_THRESHOLD_ENABLE  ENABLED
#endif

/* === gpio_output_input (gpio_output_input) === */

#define VSF_TEST_GPIO_OUTPUT_INPUT_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_OUTPUT_INPUT_PARAMS_INIT  \
    { .idx = 0, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK },  \
    { .idx = 1, .out_pin = VSF_TEST_PIN_JUMPER_OUT, .in_pin = VSF_TEST_PIN_JUMPER_IN }
#define VSF_TEST_GPIO_OUTPUT_INPUT_CASE_COUNT  2

#ifndef VSF_TEST_GPIO_OUTPUT_INPUT_ENABLE
#   define VSF_TEST_GPIO_OUTPUT_INPUT_ENABLE  ENABLED
#endif

/* === gpio_toggle (gpio_toggle) === */

#define VSF_TEST_GPIO_TOGGLE_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_TOGGLE_PARAMS_INIT  \
    { .idx = 0, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK },  \
    { .idx = 1, .out_pin = VSF_TEST_PIN_JUMPER_OUT, .in_pin = VSF_TEST_PIN_JUMPER_IN }
#define VSF_TEST_GPIO_TOGGLE_CASE_COUNT  2

#ifndef VSF_TEST_GPIO_TOGGLE_ENABLE
#   define VSF_TEST_GPIO_TOGGLE_ENABLE  ENABLED
#endif

/* === gpio_direction (gpio_direction) === */

#define VSF_TEST_GPIO_DIRECTION_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_DIRECTION_PARAMS_INIT  \
    { .idx = 0, .pin = VSF_TEST_PIN_SELF_LOOPBACK }
#define VSF_TEST_GPIO_DIRECTION_CASE_COUNT  1

#ifndef VSF_TEST_GPIO_DIRECTION_ENABLE
#   define VSF_TEST_GPIO_DIRECTION_ENABLE  ENABLED
#endif

/* === gpio_atomic (gpio_atomic) === */

#define VSF_TEST_GPIO_ATOMIC_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_ATOMIC_PARAMS_INIT  \
    { .idx = 0, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK },  \
    { .idx = 1, .out_pin = VSF_TEST_PIN_JUMPER_OUT, .in_pin = VSF_TEST_PIN_JUMPER_IN }
#define VSF_TEST_GPIO_ATOMIC_CASE_COUNT  2

#ifndef VSF_TEST_GPIO_ATOMIC_ENABLE
#   define VSF_TEST_GPIO_ATOMIC_ENABLE  ENABLED
#endif

/* === gpio_pinmux (gpio_pinmux) === */

#define VSF_TEST_GPIO_PINMUX_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_PINMUX_DEFAULT_BAUDRATE  115200

#define VSF_TEST_GPIO_PINMUX_PARAMS_INIT  \
    { .idx = 0, .tx_pin = VSF_TEST_PIN_UART1_TX, .rx_pin = VSF_TEST_PIN_UART1_RX }
#define VSF_TEST_GPIO_PINMUX_CASE_COUNT  1

#ifndef VSF_TEST_GPIO_PINMUX_ENABLE
#   define VSF_TEST_GPIO_PINMUX_ENABLE  ENABLED
#endif

/* === gpio_multi_pin (gpio_multi_pin) === */

#define VSF_TEST_GPIO_MULTI_PIN_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_MULTI_PIN_PARAMS_INIT  \
    { .idx = 0, .out_pin_a = VSF_TEST_PIN_MULTI_A, .in_pin_a = VSF_TEST_PIN_MULTI_A, .out_pin_b = VSF_TEST_PIN_MULTI_B, .in_pin_b = VSF_TEST_PIN_MULTI_B },  \
    { .idx = 1, .out_pin_a = VSF_TEST_PIN_JUMPER_OUT, .in_pin_a = VSF_TEST_PIN_JUMPER_IN, .out_pin_b = VSF_TEST_PIN_MULTI_C, .in_pin_b = VSF_TEST_PIN_MULTI_D }
#define VSF_TEST_GPIO_MULTI_PIN_CASE_COUNT  2

#ifndef VSF_TEST_GPIO_MULTI_PIN_ENABLE
#   define VSF_TEST_GPIO_MULTI_PIN_ENABLE  ENABLED
#endif

/* === gpio_open_drain (gpio_open_drain) === */

#define VSF_TEST_GPIO_OPEN_DRAIN_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_OPEN_DRAIN_PARAMS_INIT  \
    { .idx = 0, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK },  \
    { .idx = 1, .out_pin = VSF_TEST_PIN_JUMPER_OUT, .in_pin = VSF_TEST_PIN_JUMPER_IN }
#define VSF_TEST_GPIO_OPEN_DRAIN_CASE_COUNT  2

#ifndef VSF_TEST_GPIO_OPEN_DRAIN_ENABLE
#   define VSF_TEST_GPIO_OPEN_DRAIN_ENABLE  ENABLED
#endif

/* === gpio_toggle_freq (gpio_toggle_freq) === */

#define VSF_TEST_GPIO_TOGGLE_FREQ_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_GPIO_TOGGLE_FREQ_PARAMS_INIT  \
    { .idx = 0, .pin = VSF_TEST_PIN_LA_MONITORED, .toggle_count = 1000 },  \
    { .idx = 1, .pin = VSF_TEST_PIN_LA_MONITORED, .toggle_count = 10000 },  \
    { .idx = 2, .pin = VSF_TEST_PIN_LA_MONITORED, .toggle_count = 100000 },  \
    { .idx = 3, .pin = VSF_TEST_PIN_I2C0_SDA, .toggle_count = 1000 },  \
    { .idx = 4, .pin = VSF_TEST_PIN_I2C0_SCL, .toggle_count = 1000 }
#define VSF_TEST_GPIO_TOGGLE_FREQ_CASE_COUNT  5

#ifndef VSF_TEST_GPIO_TOGGLE_FREQ_ENABLE
#   define VSF_TEST_GPIO_TOGGLE_FREQ_ENABLE  ENABLED
#endif

/* === gpio_write_throughput (gpio_write_throughput) === */

#define VSF_TEST_GPIO_WRITE_THROUGHPUT_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_GPIO_WRITE_THROUGHPUT_PARAMS_INIT  \
    { .idx = 0, .pin = VSF_TEST_PIN_LA_MONITORED, .duration_us = 10000 }
#define VSF_TEST_GPIO_WRITE_THROUGHPUT_CASE_COUNT  1

#ifndef VSF_TEST_GPIO_WRITE_THROUGHPUT_ENABLE
#   define VSF_TEST_GPIO_WRITE_THROUGHPUT_ENABLE  ENABLED
#endif

/* === gpio_toggle_stress (gpio_toggle_stress) === */

#define VSF_TEST_GPIO_TOGGLE_STRESS_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_GPIO_TOGGLE_STRESS_PARAMS_INIT  \
    { .idx = 0, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK, .stress_count = 10000 },  \
    { .idx = 1, .out_pin = VSF_TEST_PIN_JUMPER_OUT, .in_pin = VSF_TEST_PIN_JUMPER_IN, .stress_count = 10000 }
#define VSF_TEST_GPIO_TOGGLE_STRESS_CASE_COUNT  2

#ifndef VSF_TEST_GPIO_TOGGLE_STRESS_ENABLE
#   define VSF_TEST_GPIO_TOGGLE_STRESS_ENABLE  ENABLED
#endif

/* === gpio_concurrent_prio (gpio_concurrent_prio) === */

#define VSF_TEST_GPIO_CONCURRENT_PRIO_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_GPIO_CONCURRENT_PRIO_PARAMS_INIT  \
    { .idx = 0, .out_pin = VSF_TEST_PIN_CONCURRENT_OUT, .in_pin = VSF_TEST_PIN_CONCURRENT_IN, .duration_ms = 200, .callback_period_us = 100 }
#define VSF_TEST_GPIO_CONCURRENT_PRIO_CASE_COUNT  1

#ifndef VSF_TEST_GPIO_CONCURRENT_PRIO_ENABLE
#   define VSF_TEST_GPIO_CONCURRENT_PRIO_ENABLE  ENABLED
#endif

/* === gpio_exti (gpio_exti) === */

#define VSF_TEST_GPIO_EXTI_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_EXTI_PARAMS_INIT  \
    { .idx = 0, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK, .trigger_mode = VSF_GPIO_EXTI_MODE_FALLING },  \
    { .idx = 1, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK, .trigger_mode = VSF_GPIO_EXTI_MODE_RISING },  \
    { .idx = 2, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK, .trigger_mode = VSF_GPIO_EXTI_MODE_RISING_FALLING },  \
    { .idx = 3, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK, .trigger_mode = VSF_GPIO_EXTI_MODE_LOW_LEVEL },  \
    { .idx = 4, .out_pin = VSF_TEST_PIN_SELF_LOOPBACK, .in_pin = VSF_TEST_PIN_SELF_LOOPBACK, .trigger_mode = VSF_GPIO_EXTI_MODE_HIGH_LEVEL },  \
    { .idx = 5, .out_pin = VSF_TEST_PIN_EXTI_ALT_OUT, .in_pin = VSF_TEST_PIN_EXTI_ALT_IN, .trigger_mode = VSF_GPIO_EXTI_MODE_FALLING }
#define VSF_TEST_GPIO_EXTI_CASE_COUNT  6

#ifndef VSF_TEST_GPIO_EXTI_ENABLE
#   define VSF_TEST_GPIO_EXTI_ENABLE  ENABLED
#endif

/* === gpio_irq_latency (gpio_irq_latency) === */

#define VSF_TEST_GPIO_IRQ_LATENCY_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_IRQ_LATENCY_PARAMS_INIT  \
    { .idx = 0, .pin = VSF_TEST_PIN_SELF_LOOPBACK, .warn_latency_ns = 50000, .max_latency_ns = 150000 }
#define VSF_TEST_GPIO_IRQ_LATENCY_CASE_COUNT  1

#ifndef VSF_TEST_GPIO_IRQ_LATENCY_ENABLE
#   define VSF_TEST_GPIO_IRQ_LATENCY_ENABLE  ENABLED
#endif

/* === gpio_irq_lifecycle (gpio_irq_lifecycle) === */

#define VSF_TEST_GPIO_IRQ_LIFECYCLE_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_IRQ_LIFECYCLE_PARAMS_INIT  \
    { .idx = 0, .pin = VSF_TEST_PIN_SELF_LOOPBACK }
#define VSF_TEST_GPIO_IRQ_LIFECYCLE_CASE_COUNT  1

#ifndef VSF_TEST_GPIO_IRQ_LIFECYCLE_ENABLE
#   define VSF_TEST_GPIO_IRQ_LIFECYCLE_ENABLE  ENABLED
#endif

/* === gpio_systimer_health (gpio_systimer_health) === */

#define VSF_TEST_GPIO_SYSTIMER_HEALTH_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_SYSTIMER_HEALTH_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_SYSTIMER_HEALTH_PARAMS_INIT  \
    { .idx = 0, .pin = VSF_TEST_PIN_LA_MONITORED, .interval_ms = 10, .toggle_count = 10 },  \
    { .idx = 1, .pin = VSF_TEST_PIN_LA_MONITORED, .interval_ms = 50, .toggle_count = 10 },  \
    { .idx = 2, .pin = VSF_TEST_PIN_LA_MONITORED, .interval_ms = 100, .toggle_count = 5 }
#define VSF_TEST_GPIO_SYSTIMER_HEALTH_CASE_COUNT  3

#ifndef VSF_TEST_GPIO_SYSTIMER_HEALTH_ENABLE
#   define VSF_TEST_GPIO_SYSTIMER_HEALTH_ENABLE  ENABLED
#endif

/* === gpio_analog_mode (gpio_analog_mode) === */

#define VSF_TEST_GPIO_ANALOG_MODE_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_ANALOG_MODE_PARAMS_INIT  \
    { .idx = 0, .pin = VSF_TEST_PIN_SELF_LOOPBACK }
#define VSF_TEST_GPIO_ANALOG_MODE_CASE_COUNT  1

#ifndef VSF_TEST_GPIO_ANALOG_MODE_ENABLE
#   define VSF_TEST_GPIO_ANALOG_MODE_ENABLE  ENABLED
#endif

/* === gpio_io_check (gpio_io_check) === */

#define VSF_TEST_GPIO_IO_CHECK_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_IO_CHECK_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_GPIO_IO_CHECK_PARAMS_INIT  \
    { .idx = 0, .pin = VSF_TEST_PIN_UART1_TX, .baudrate = 115200 },  \
    { .idx = 1, .pin = VSF_TEST_PIN_UART1_RX, .baudrate = 115200 }
#define VSF_TEST_GPIO_IO_CHECK_CASE_COUNT  2

#ifndef VSF_TEST_GPIO_IO_CHECK_ENABLE
#   define VSF_TEST_GPIO_IO_CHECK_ENABLE  ENABLED
#endif

/* === i2c_eeprom_rw (i2c_eeprom_rw) === */

#define VSF_TEST_I2C_EEPROM_RW_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_I2C_EEPROM_RW_PARAMS_INIT  \
    { .idx = 0, .i2c_idx = 0, .eeprom_addr = 80, .mem_addr = 0, .data_len = 1 },  \
    { .idx = 1, .i2c_idx = 0, .eeprom_addr = 80, .mem_addr = 16, .data_len = 4 },  \
    { .idx = 2, .i2c_idx = 0, .eeprom_addr = 80, .mem_addr = 32, .data_len = 8 }
#define VSF_TEST_I2C_EEPROM_RW_CASE_COUNT  3

#ifndef VSF_TEST_I2C_EEPROM_RW_ENABLE
#   define VSF_TEST_I2C_EEPROM_RW_ENABLE  ENABLED
#endif

/* === i2c_bus_scan (i2c_bus_scan) === */

#define VSF_TEST_I2C_BUS_SCAN_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_I2C_BUS_SCAN_PARAMS_INIT  \
    { .idx = 0, .scl_pin = VSF_TEST_PIN_I2C0_SCL, .sda_pin = VSF_TEST_PIN_I2C0_SDA },  \
    { .idx = 1, .scl_pin = VSF_TEST_PIN_I2C1_SCL, .sda_pin = VSF_TEST_PIN_I2C1_SDA }
#define VSF_TEST_I2C_BUS_SCAN_CASE_COUNT  2

#ifndef VSF_TEST_I2C_BUS_SCAN_ENABLE
#   define VSF_TEST_I2C_BUS_SCAN_ENABLE  ENABLED
#endif

/* === i2c_eeprom_page (i2c_eeprom_page) === */

#define VSF_TEST_I2C_EEPROM_PAGE_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_I2C_EEPROM_PAGE_PARAMS_INIT  \
    { .idx = 0, .i2c_idx = 0, .eeprom_addr = 80, .mem_addr = 24, .data_len = 8 }
#define VSF_TEST_I2C_EEPROM_PAGE_CASE_COUNT  1

#ifndef VSF_TEST_I2C_EEPROM_PAGE_ENABLE
#   define VSF_TEST_I2C_EEPROM_PAGE_ENABLE  ENABLED
#endif

/* === i2c_slave (i2c_slave) === */

#define VSF_TEST_I2C_SLAVE_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_I2C_SLAVE_PARAMS_INIT  \
    { .idx = 0, .master_i2c_idx = 0, .slave_i2c_idx = 1 }
#define VSF_TEST_I2C_SLAVE_CASE_COUNT  1

#ifndef VSF_TEST_I2C_SLAVE_ENABLE
#   define VSF_TEST_I2C_SLAVE_ENABLE  ENABLED
#endif

/* === i2c_eeprom_rw_fifo (i2c_eeprom_rw_fifo) === */

#define VSF_TEST_I2C_EEPROM_RW_FIFO_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_I2C_EEPROM_RW_FIFO_PARAMS_INIT  \
    { .idx = 0, .i2c_idx = 0, .eeprom_addr = 80, .mem_addr = 0, .data_len = 1 },  \
    { .idx = 1, .i2c_idx = 0, .eeprom_addr = 80, .mem_addr = 16, .data_len = 4 },  \
    { .idx = 2, .i2c_idx = 0, .eeprom_addr = 80, .mem_addr = 32, .data_len = 8 }
#define VSF_TEST_I2C_EEPROM_RW_FIFO_CASE_COUNT  3

#ifndef VSF_TEST_I2C_EEPROM_RW_FIFO_ENABLE
#   define VSF_TEST_I2C_EEPROM_RW_FIFO_ENABLE  ENABLED
#endif

/* === i2c_slave_fifo (i2c_slave_fifo) === */

#define VSF_TEST_I2C_SLAVE_FIFO_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_I2C_SLAVE_FIFO_PARAMS_INIT  \
    { .idx = 0, .master_i2c_idx = 0, .slave_i2c_idx = 1 }
#define VSF_TEST_I2C_SLAVE_FIFO_CASE_COUNT  1

#ifndef VSF_TEST_I2C_SLAVE_FIFO_ENABLE
#   define VSF_TEST_I2C_SLAVE_FIFO_ENABLE  ENABLED
#endif

/* === rtc_set_get (rtc_set_get) === */

#define VSF_TEST_RTC_SET_GET_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_RTC_SET_GET_PARAMS_INIT  \
    { .idx = 0, .rtc_idx = 0 }
#define VSF_TEST_RTC_SET_GET_CASE_COUNT  1

#ifndef VSF_TEST_RTC_SET_GET_ENABLE
#   define VSF_TEST_RTC_SET_GET_ENABLE  ENABLED
#endif

/* === rtc_alarm (rtc_alarm) === */

#define VSF_TEST_RTC_ALARM_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_RTC_ALARM_PARAMS_INIT  \
    { .idx = 0, .rtc_idx = 0 }
#define VSF_TEST_RTC_ALARM_CASE_COUNT  1

#ifndef VSF_TEST_RTC_ALARM_ENABLE
#   define VSF_TEST_RTC_ALARM_ENABLE  ENABLED
#endif

/* === rtc_epoch (rtc_epoch) === */

#define VSF_TEST_RTC_EPOCH_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_RTC_EPOCH_PARAMS_INIT  \
    { .idx = 0, .rtc_idx = 0 }
#define VSF_TEST_RTC_EPOCH_CASE_COUNT  1

#ifndef VSF_TEST_RTC_EPOCH_ENABLE
#   define VSF_TEST_RTC_EPOCH_ENABLE  ENABLED
#endif

/* === flash_erase_program_read (flash_erase_program_read) === */

#define VSF_TEST_FLASH_ERASE_PROGRAM_READ_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_FLASH_ERASE_PROGRAM_READ_PARAMS_INIT  \
    { .idx = 0, .offset = 262144, .size = 256 }
#define VSF_TEST_FLASH_ERASE_PROGRAM_READ_CASE_COUNT  1

#ifndef VSF_TEST_FLASH_ERASE_PROGRAM_READ_ENABLE
#   define VSF_TEST_FLASH_ERASE_PROGRAM_READ_ENABLE  ENABLED
#endif

/* === flash_boundary (flash_boundary) === */

#define VSF_TEST_FLASH_BOUNDARY_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_FLASH_BOUNDARY_PARAMS_INIT  \
    { .idx = 0, .offset = 262144, .size = 512 }
#define VSF_TEST_FLASH_BOUNDARY_CASE_COUNT  1

#ifndef VSF_TEST_FLASH_BOUNDARY_ENABLE
#   define VSF_TEST_FLASH_BOUNDARY_ENABLE  ENABLED
#endif

/* === wdt_basic (wdt_basic) === */

#define VSF_TEST_WDT_BASIC_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_WDT_BASIC_PARAMS_INIT  \
    { .idx = 0, .timeout_ms = 500, .feed_count = 2, .feed_interval_ms = 10 }
#define VSF_TEST_WDT_BASIC_CASE_COUNT  1

#ifndef VSF_TEST_WDT_BASIC_ENABLE
#   define VSF_TEST_WDT_BASIC_ENABLE  ENABLED
#endif

/* === wdt_reboot (wdt_reboot) === */

#define VSF_TEST_WDT_REBOOT_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_WDT_REBOOT_PARAMS_INIT  \
    { .idx = 0, .timeout_ms = 200 }
#define VSF_TEST_WDT_REBOOT_CASE_COUNT  1

#ifndef VSF_TEST_WDT_REBOOT_ENABLE
#   define VSF_TEST_WDT_REBOOT_ENABLE  ENABLED
#endif

/* === wdt_timeout_accuracy (wdt_timeout_accuracy) === */

#define VSF_TEST_WDT_TIMEOUT_ACCURACY_SUITE_TIMEOUT_MS  ((uint32_t)(10.0 * 1000))

#define VSF_TEST_WDT_TIMEOUT_ACCURACY_PARAMS_INIT  \
    { .idx = 0, .timeout_ms = 500, .tolerance_pct = 10 }
#define VSF_TEST_WDT_TIMEOUT_ACCURACY_CASE_COUNT  1

#ifndef VSF_TEST_WDT_TIMEOUT_ACCURACY_ENABLE
#   define VSF_TEST_WDT_TIMEOUT_ACCURACY_ENABLE  ENABLED
#endif

/* === adc_oneshot (adc_oneshot) === */

#define VSF_TEST_ADC_ONESHOT_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_ADC_ONESHOT_PARAMS_INIT  \
    { .idx = 0, .channel = 0, .expected_min = 0, .expected_max = 4095 }
#define VSF_TEST_ADC_ONESHOT_CASE_COUNT  1

#ifndef VSF_TEST_ADC_ONESHOT_ENABLE
#   define VSF_TEST_ADC_ONESHOT_ENABLE  ENABLED
#endif

/* === adc_temperature (adc_temperature) === */

#define VSF_TEST_ADC_TEMPERATURE_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_ADC_TEMPERATURE_PARAMS_INIT  \
    { .idx = 0, .channel_count = 5, .sensor_channel = 4, .temp_raw_min = 500, .temp_raw_max = 1200 }
#define VSF_TEST_ADC_TEMPERATURE_CASE_COUNT  1

#ifndef VSF_TEST_ADC_TEMPERATURE_ENABLE
#   define VSF_TEST_ADC_TEMPERATURE_ENABLE  ENABLED
#endif

/* === adc_stream (adc_stream) === */

#define VSF_TEST_ADC_STREAM_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_ADC_STREAM_PARAMS_INIT  \
    { .idx = 0, .channel = 4, .sample_count = 100 }
#define VSF_TEST_ADC_STREAM_CASE_COUNT  1

#ifndef VSF_TEST_ADC_STREAM_ENABLE
#   define VSF_TEST_ADC_STREAM_ENABLE  ENABLED
#endif

/* === pwm_basic (pwm_basic) === */

#define VSF_TEST_PWM_BASIC_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_PWM_BASIC_PARAMS_INIT  \
    { .idx = 0, .slice = 0, .channel = 0, .gpio = VSF_TEST_PIN_PWM_GPIO_A, .freq_hz = 1000, .period = 100, .pulse = 50, .run_ms = 1000 }
#define VSF_TEST_PWM_BASIC_CASE_COUNT  1

#ifndef VSF_TEST_PWM_BASIC_ENABLE
#   define VSF_TEST_PWM_BASIC_ENABLE  ENABLED
#endif

/* === pwm_dual_channel (pwm_dual_channel) === */

#define VSF_TEST_PWM_DUAL_CHANNEL_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_PWM_DUAL_CHANNEL_PARAMS_INIT  \
    { .idx = 0, .slice = 0, .channel_a = 0, .channel_b = 1, .gpio_a = VSF_TEST_PIN_PWM_GPIO_A, .gpio_b = VSF_TEST_PIN_PWM_GPIO_B, .freq_hz = 1000, .period = 100, .pulse_a = 50, .pulse_b = 25, .run_ms = 1000 }
#define VSF_TEST_PWM_DUAL_CHANNEL_CASE_COUNT  1

#ifndef VSF_TEST_PWM_DUAL_CHANNEL_ENABLE
#   define VSF_TEST_PWM_DUAL_CHANNEL_ENABLE  ENABLED
#endif

/* === pwm_irq (pwm_irq) === */

#define VSF_TEST_PWM_IRQ_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_PWM_IRQ_PARAMS_INIT  \
    { .idx = 0, .slice = 0, .channel = 0, .freq_hz = 1000, .period = 65535, .pulse = 32768, .test_ms = 500 }
#define VSF_TEST_PWM_IRQ_CASE_COUNT  1

#ifndef VSF_TEST_PWM_IRQ_ENABLE
#   define VSF_TEST_PWM_IRQ_ENABLE  ENABLED
#endif

/* === timer_oneshot (timer_oneshot) === */

#define VSF_TEST_TIMER_ONESHOT_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_TIMER_ONESHOT_PARAMS_INIT  \
    { .idx = 0, .timer_idx = 0, .channel = 0, .period_us = 50000 }
#define VSF_TEST_TIMER_ONESHOT_CASE_COUNT  1

#ifndef VSF_TEST_TIMER_ONESHOT_ENABLE
#   define VSF_TEST_TIMER_ONESHOT_ENABLE  ENABLED
#endif

/* === timer_periodic (timer_periodic) === */

#define VSF_TEST_TIMER_PERIODIC_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_TIMER_PERIODIC_PARAMS_INIT  \
    { .idx = 0, .timer_idx = 0, .channel = 0, .period_us = 10000, .count = 5 }
#define VSF_TEST_TIMER_PERIODIC_CASE_COUNT  1

#ifndef VSF_TEST_TIMER_PERIODIC_ENABLE
#   define VSF_TEST_TIMER_PERIODIC_ENABLE  ENABLED
#endif

/* === timer_async (timer_async) === */

#define VSF_TEST_TIMER_ASYNC_SUITE_TIMEOUT_MS  ((uint32_t)(10.0 * 1000))

#define VSF_TEST_TIMER_ASYNC_PARAMS_INIT  \
    { .idx = 0, .timer_idx = 0, .channel = 0, .period_us = 10000 }
#define VSF_TEST_TIMER_ASYNC_CASE_COUNT  1

#ifndef VSF_TEST_TIMER_ASYNC_ENABLE
#   define VSF_TEST_TIMER_ASYNC_ENABLE  ENABLED
#endif

/* === spi_loopback (spi_loopback) === */

#define VSF_TEST_SPI_LOOPBACK_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_SPI_LOOPBACK_PARAMS_INIT  \
    { .idx = 0, .mode = VSF_SPI_MODE_0, .clock_hz = 1000000, .data_len = 8 },  \
    { .idx = 1, .mode = VSF_SPI_MODE_1, .clock_hz = 1000000, .data_len = 8 },  \
    { .idx = 2, .mode = VSF_SPI_MODE_2, .clock_hz = 1000000, .data_len = 8 },  \
    { .idx = 3, .mode = VSF_SPI_MODE_3, .clock_hz = 1000000, .data_len = 8 },  \
    { .idx = 4, .mode = VSF_SPI_MODE_0, .clock_hz = 100000, .data_len = 8 },  \
    { .idx = 5, .mode = VSF_SPI_MODE_0, .clock_hz = 500000, .data_len = 8 },  \
    { .idx = 6, .mode = VSF_SPI_MODE_0, .clock_hz = 2000000, .data_len = 8 },  \
    { .idx = 7, .mode = VSF_SPI_MODE_0, .clock_hz = 5000000, .data_len = 8 },  \
    { .idx = 8, .mode = VSF_SPI_MODE_0, .clock_hz = 1000000, .data_len = 16 },  \
    { .idx = 9, .mode = VSF_SPI_MODE_0, .clock_hz = 1000000, .data_len = 64 },  \
    { .idx = 10, .mode = VSF_SPI_MODE_0, .clock_hz = 1000000, .data_len = 256 }
#define VSF_TEST_SPI_LOOPBACK_CASE_COUNT  11

#ifndef VSF_TEST_SPI_LOOPBACK_ENABLE
#   define VSF_TEST_SPI_LOOPBACK_ENABLE  ENABLED
#endif

/* === spi_async (spi_async) === */

#define VSF_TEST_SPI_ASYNC_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_SPI_ASYNC_PARAMS_INIT  \
    { .idx = 0, .mode = VSF_SPI_MODE_0, .clock_hz = 1000000, .data_len = 16, .test_type = 0 },  \
    { .idx = 1, .mode = VSF_SPI_MODE_0, .clock_hz = 1000000, .data_len = 16, .test_type = 1 },  \
    { .idx = 2, .mode = VSF_SPI_MODE_0, .clock_hz = 1000000, .data_len = 16, .test_type = 2 },  \
    { .idx = 3, .mode = VSF_SPI_MODE_0, .clock_hz = 1000000, .data_len = 256, .test_type = 3 }
#define VSF_TEST_SPI_ASYNC_CASE_COUNT  4

#ifndef VSF_TEST_SPI_ASYNC_ENABLE
#   define VSF_TEST_SPI_ASYNC_ENABLE  ENABLED
#endif

/* === rng_basic (rng_basic) === */

#define VSF_TEST_RNG_BASIC_SUITE_TIMEOUT_MS  ((uint32_t)(5.0 * 1000))

#define VSF_TEST_RNG_BASIC_PARAMS_INIT  \
    { .idx = 0, .word_count = 16 }
#define VSF_TEST_RNG_BASIC_CASE_COUNT  1

#ifndef VSF_TEST_RNG_BASIC_ENABLE
#   define VSF_TEST_RNG_BASIC_ENABLE  ENABLED
#endif

/* === dma_mem2mem (dma_mem2mem) === */

#define VSF_TEST_DMA_MEM2MEM_TIMEOUT_MS  ((uint32_t)(10.0 * 1000))

#define VSF_TEST_DMA_MEM2MEM_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_DMA_MEM2MEM_PARAMS_INIT  \
    { .idx = 0, .expect_pass = true }
#define VSF_TEST_DMA_MEM2MEM_CASE_COUNT  1

#ifndef VSF_TEST_DMA_MEM2MEM_ENABLE
#   define VSF_TEST_DMA_MEM2MEM_ENABLE  ENABLED
#endif

/* === dma_mem2mem_irq (dma_mem2mem_irq) === */

#define VSF_TEST_DMA_MEM2MEM_IRQ_TIMEOUT_MS  ((uint32_t)(10.0 * 1000))

#define VSF_TEST_DMA_MEM2MEM_IRQ_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_DMA_MEM2MEM_IRQ_PARAMS_INIT  \
    { .idx = 0, .expect_pass = true }
#define VSF_TEST_DMA_MEM2MEM_IRQ_CASE_COUNT  1

#ifndef VSF_TEST_DMA_MEM2MEM_IRQ_ENABLE
#   define VSF_TEST_DMA_MEM2MEM_IRQ_ENABLE  ENABLED
#endif

/* === dma_scatter_gather (dma_scatter_gather) === */

#define VSF_TEST_DMA_SCATTER_GATHER_TIMEOUT_MS  ((uint32_t)(10.0 * 1000))

#define VSF_TEST_DMA_SCATTER_GATHER_SUITE_TIMEOUT_MS  ((uint32_t)(15.0 * 1000))

#define VSF_TEST_DMA_SCATTER_GATHER_PARAMS_INIT  \
    { .idx = 0, .expect_pass = true },  \
    { .idx = 1, .expect_pass = true },  \
    { .idx = 2, .expect_pass = true }
#define VSF_TEST_DMA_SCATTER_GATHER_CASE_COUNT  3

#ifndef VSF_TEST_DMA_SCATTER_GATHER_ENABLE
#   define VSF_TEST_DMA_SCATTER_GATHER_ENABLE  ENABLED
#endif

#endif /* __TEST_PARAMS_GENERATED_H__ */
