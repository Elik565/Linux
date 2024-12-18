#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>

void send_open_file_request(const char* file_path) {
    DBusConnection* conn;
    DBusMessage* msg;
    DBusError error;

    dbus_error_init(&error);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

    if (dbus_error_is_set(&error)) {
        fprintf(stderr, "Ошибка подключения к шине: %s\n", error.message);
        dbus_error_free(&error);
        return;
    }

    // Создаем сообщение
    msg = dbus_message_new_method_call("example.fileopener", // Имя сервиса
        "/example/fileopener", // Объект
        "example.fileopener",  // Интерфейс
        "OpenFile");           // Метод

    if (!msg) {
        fprintf(stderr, "Ошибка создания сообщения.\n");
        return;
    }

    // Добавляем аргумент (путь к файлу)
    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &file_path, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "Ошибка добавления аргументов.\n");
        dbus_message_unref(msg);
        return;
    }

    // Отправляем сообщение
    if (!dbus_connection_send(conn, msg, NULL)) {
        fprintf(stderr, "Ошибка отправки сообщения.\n");
    }

    printf("Запрос на открытие файла отправлен: %s\n", file_path);

    dbus_message_unref(msg);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <путь_к_файлу>\n", argv[0]);
        return 1;
    }

    send_open_file_request(argv[1]);
    return 0;
}
