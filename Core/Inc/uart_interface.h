#ifndef UART_INTERFACE_H
#define UART_INTERFACE_H

// Отправка сообщения через UART
void uart_send(const char* message);

// Получение символа (неблокирующее, возвращает 0 если нет данных)
char uart_receive_char(void);

// Инициализация UART интерфейса
void uart_init(void);

// Деинициализация UART интерфейса (восстановление настроек)
void uart_deinit(void);

#endif // UART_INTERFACE_H

