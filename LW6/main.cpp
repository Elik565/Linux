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

void open_file_with_dbus(const std::string& file_path) {

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

    return 0;
}