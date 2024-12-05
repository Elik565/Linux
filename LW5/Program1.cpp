#include <iostream>
#include <thread>
#include <semaphore.h>
#include <vector>
#include <unistd.h>

sem_t booking_sem;
sem_t limit_sem;

const size_t LIMIT_THREADS = 20;  // всего 20 потоков
const size_t BOOKING_THREADS = 3;  // 3 потока бронирования

bool start_thread = false;

void get_info(size_t id) {
    while (!start_thread) {
        sleep(1);
    }
}

void book_seat(size_t id) {
    while (!start_thread) {
        sleep(1);
    }
}

int main() {
    sem_init(&booking_sem, 0, 1);  // только один поток может бронировать
    sem_init(&limit_sem, 0, 10);  // одновременно обрабатываться может только 10 потоков

    std::vector<std::thread> threads;

    // создаем потоки бронирования
    for (size_t i = 0; i < BOOKING_THREADS; i++) {
        std::cout << "Создан поток бронирования с id " << i << std::endl; 
        threads.emplace_back(book_seat, i);
    }

    // создаем потоки справок 
    for (size_t i = BOOKING_THREADS; i < LIMIT_THREADS; i++) {
        std::cout << "Создан поток справок с id " << i << std::endl; 
        threads.emplace_back(get_info, i);
    }

    std::string input;
    std::cout << "Введите 'start', чтобы одновременно запустить все потоки\n";
    while (true) {
        std::cout << "Ввод: ";
        std::cin >> input;
        if (input == "start") {
            start_thread = true;
            break;
        }
    }
    
    // ожидаем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }

    // уничтожаем семафоры
    sem_destroy(&booking_sem);
    sem_destroy(&limit_sem);

    return 0;
}