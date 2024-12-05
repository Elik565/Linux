#include <iostream>
#include <thread>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include <mutex>

sem_t booking_sem;
sem_t limit_sem;

std::mutex output_mutex;  // для синхронизации вывода

const size_t LIMIT_THREADS = 20;  // всего 20 потоков
const size_t BOOKING_THREADS = 3;  // 3 потока бронирования

bool start_thread = false;

void get_info(size_t id) {
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Поток с id " << id << " ждет доступа для получения справки\n";
    }

    sem_wait(&limit_sem);  // проверяем, попадаем ли мы в лимит
    sem_wait(&booking_sem);  // блокируем бронирование

    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Поток с id " << id << " начал получать справку\n";
    }

    sleep(2);

    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Поток с id " << id << " закончил получать справку\n";
    }

    sem_post(&limit_sem);  // освобождаем семафор лимита потоков справок
    sem_post(&booking_sem);  // освобождаем семафор бронирования
}

void book_seat(size_t id) {
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Поток с id " << id << " ждет доступа для бронирования\n";
    }

    sem_wait(&booking_sem);  // проверяем, можно ли бронировать

    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Поток с id " << id << " начал бронирование\n";
    }

    sleep(4);

    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Поток с id " << id << " закончил бронирование\n";
    }

    sem_post(&booking_sem);
}

int main() {
    sem_init(&booking_sem, 0, 1);  // только один поток может бронировать
    sem_init(&limit_sem, 0, 10);  // одновременно обрабатываться может только 10 потоков

    std::vector<std::thread> threads;

    // создаем потоки справок 
    for (size_t i = 0; i < LIMIT_THREADS - BOOKING_THREADS; i++) {
        std::cout << "Создан поток справок с id " << i << std::endl; 
        threads.emplace_back(get_info, i);
    }

    // создаем потоки бронирования
    for (size_t i = LIMIT_THREADS - BOOKING_THREADS; i < LIMIT_THREADS; i++) {
        std::cout << "Создан поток бронирования с id " << i << std::endl; 
        threads.emplace_back(book_seat, i);
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