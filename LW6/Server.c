#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Функция для открытия файла с соответствующим приложением
void open_file(const char* file_path) {
    const char* command = NULL;
    const char* extension = strrchr(file_path, '.');

    if (extension == NULL) {
        printf("Не удалось определить расширение файла: %s\n", file_path);
        return;
    }

    // Простое сопоставление расширений приложений
    if (strcmp(extension, ".txt") == 0) {
        command = "gedit";
    }
    else if (strcmp(extension, ".png") == 0 || strcmp(extension, ".jpg") == 0) {
        command = "feh";
    }
    else if (strcmp(extension, ".pdf") == 0) {
        command = "evince";
    }
    else {
        printf("Не найдено подходящее приложение для расширения: %s\n", extension);
        return;
    }

    // Формируем команду для запуска
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "%s \"%s\" &", command, file_path);
    system(cmd);
}

// Функция для обработки сообщений
void process_message(DBusMessage* msg) {
    if (dbus_message_is_method_call(msg, "example.fileopener", "OpenFile")) {
        const char* file_path;
        DBusError error;
        dbus_error_init(&error);

        // Извлекаем путь к файлу из сообщения
        if (!dbus_message_get_args(msg, &error, DBUS_TYPE_STRING, &file_path, DBUS_TYPE_INVALID)) {
            fprintf(stderr, "Ошибка получения аргументов: %s\n", error.message);
            dbus_error_free(&error);
            return;
        }

        printf("Открытие файла: %s\n", file_path);
        open_file(file_path);
    }
}

int main() {
    DBusConnection* conn;
    DBusError error;

    // Инициализация D-Bus
    dbus_error_init(&error);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

    if (dbus_error_is_set(&error)) {
        fprintf(stderr, "Ошибка подключения к шине: %s\n", error.message);
        dbus_error_free(&error);
        return 1;
    }

    // Запрашиваем имя сервиса
    int ret = dbus_bus_request_name(conn, "example.fileopener", DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    if (dbus_error_is_set(&error)) {
        fprintf(stderr, "Ошибка запроса имени: %s\n", error.message);
        dbus_error_free(&error);
        return 1;
    }

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        fprintf(stderr, "Не удалось получить имя сервиса.\n");
        return 1;
    }

    printf("D-Bus сервер запущен. Ожидание сообщений...\n");

    // Основной цикл обработки сообщений
    while (1) {
        dbus_connection_read_write(conn, 0);
        DBusMessage* msg = dbus_connection_pop_message(conn);

        if (msg != NULL) {
            process_message(msg);
            dbus_message_unref(msg);
        }
    }

    return 0;
}
