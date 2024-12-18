#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <mutex>
#include <condition_variable>

std::mutex output_mutex;  // для синхронизации вывода

class Monitor {
private:
    std::mutex mutex;

    // поток переходит в режим ожидания на условной переменной
    std::condition_variable booking_cv;  
    std::condition_variable limit_cv;

    size_t active_booking = 0;  // кол-во активных бронирующих потоков
    size_t active_info = 0;  // кол-во активных справочных потоков
    const size_t limit_info;  // кол-во справочных потоков для одновременного обслуживания

public:
    Monitor(size_t limit_info_threads) : limit_info(limit_info_threads) {}

    void start_getting_info(size_t id) {
        std::unique_lock<std::mutex> lock(mutex);
        if (active_info >= limit_info) {
            std::cout << "Уже обрабатываются " << limit_info << " потоков\n";
        }
        while (active_booking > 0 || active_info >= limit_info) {
            limit_cv.wait(lock);  // ждем доступа к инфо
        }

        active_info++;
        std::cout << "Поток с id " << id << " начал получение справки\n";
    }

    void end_getting_info(size_t id) {
        std::unique_lock<std::mutex> lock(mutex);
        active_info--;
        std::cout << "Поток с id " << id << " закончил получение справки\n";
        limit_cv.notify_all();  // сообщаем всем ждущим справочным потокам
        booking_cv.notify_one();  // сообщаем одному ждущему потоку бронирования
    }

    void start_booking(size_t id) {
        std::unique_lock<std::mutex> lock(mutex);
        while (active_booking > 0 || active_info > 0) {
            booking_cv.wait(lock);  // ждем доступа к бронированию
        }

        active_booking++;
        std::cout << "Поток с id " << id << " начал бронирование\n";
    }

    void end_booking(size_t id) {
        std::unique_lock<std::mutex> lock(mutex);
        active_booking--;
        std::cout << "Поток с id " << id << " закончил бронирование\n";
        booking_cv.notify_one();  // сообщаем одному бронирующему потоку
    }
};

void get_info(Monitor& monitor, size_t id) {
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Поток с id " << id << " ждет доступа для получения справки\n";
    }

    monitor.start_getting_info(id);
    sleep(2);
    monitor.end_getting_info(id);
}

void book_seat(Monitor& monitor, size_t id) {
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Поток с id " << id << " ждет доступа для бронирования\n";
    }

    monitor.start_booking(id);
    sleep(4);
    monitor.end_booking(id);
}

int main() {
    const size_t LIMIT_THREADS = 20;  // всего потоков
    const size_t BOOKING_THREADS = 3;  // кол-во потоков бронирования

    Monitor monitor(10);  // одновременно обрабатывается 10 справочных потоков

    std::vector<std::thread> threads;

    // создаем потоки справок 
    for (size_t i = 0; i < LIMIT_THREADS - BOOKING_THREADS; i++) {
        std::cout << "Создан поток справок с id " << i << std::endl; 
        // используем ref для передачи по ссылке
        threads.emplace_back(get_info, std::ref(monitor), i);
    }

    // создаем потоки бронирования
    for (size_t i = LIMIT_THREADS - BOOKING_THREADS; i < LIMIT_THREADS; i++) {
        std::cout << "Создан поток бронирования с id " << i << std::endl; 
        // используем ref для передачи по ссылке
        threads.emplace_back(book_seat, std::ref(monitor), i);
    }

    // ожидаем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}