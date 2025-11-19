#include "uart_interface.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

static struct termios old_termios;
static int terminal_configured = 0;

// Инициализация UART интерфейса (mock версия)
void uart_init(void) {
    // Настройка терминала для неблокирующего ввода
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    
    // Отключаем канонический режим и эхо
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    // Устанавливаем неблокирующий режим для stdin
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    
    terminal_configured = 1;
}

// Отправка сообщения через UART (mock версия через printf)
void uart_send(const char* message) {
    // В реальной реализации здесь будет отправка через HAL_UART_Transmit
    printf("%s", message);
    fflush(stdout);
}

// Получение символа (неблокирующее)
char uart_receive_char(void) {
    char c = 0;
    
    // В реальной реализации здесь будет проверка буфера UART
    // через HAL_UART_Receive или прерывания
    
    // Mock версия: чтение из stdin
    if (read(STDIN_FILENO, &c, 1) == 1) {
        return c;
    }
    
    return 0; // Нет данных
}

// Восстановление настроек терминала (для корректного завершения)
void uart_deinit(void) {
    if (terminal_configured) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
        terminal_configured = 0;
    }
}

