#include <iostream>
#include <string>
#include <dbus-1.0/dbus/dbus.h>

std::string get_file_extension(const std::string& file_path) {
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return "";  // если расширение не найдено
    }

    return file_path.substr(dot_pos + 1);
}

void open_file_with_dbus(const std::string& file_path, const std::string& extension) {
    // инициализация 
    DBusError err;
    DBusConnection* conn;
    dbus_error_init(&err);

    // подключение к системной шине dbus
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "Ошибка подключения к D-Bus: " << err.message << "\n";
        dbus_error_free(&err);
        exit(1);
    }

    if (!conn) {
        std::cerr << "Не удалось подключиться к D-Bus\n";
        exit(1);
    }

    DBusMessage* msg = dbus_message_new_method_call(
        "org.freedesktop.FileService", // имя сервиса
        "/org/freedesktop/FileService", // объект
        "org.freedesktop.FileService",  // интерфейс
        "OpenFile"  // метод
    );
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Неправильное количество входных параметров\n";
        return 1;
    }

    std::string extension = get_file_extension(argv[1]);

    if (extension.empty()) {
        std::cerr << "Не удалось определить расширение файла\n";
        return 1;
    }

    open_file_with_dbus(argv[1], extension);

    return 0;
}