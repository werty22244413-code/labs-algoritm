#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <utility>

template <typename T>
class Vector {
    T* data = nullptr;
    int sz = 0, cap = 0;
public:
    Vector() = default;
    explicit Vector(int n) : sz(n), cap(n), data(n ? new T[n]() : nullptr) {}
    
    // Правило пяти
    Vector(const Vector& o) : sz(o.sz), cap(o.cap), data(o.cap ? new T[o.cap] : nullptr) { std::copy(o.data, o.data + o.sz, data); }
    Vector(Vector&& o) noexcept : data(o.data), sz(o.sz), cap(o.cap) { o.data = nullptr; o.sz = o.cap = 0; }
    ~Vector() { delete[] data; }
    
    Vector& operator=(const Vector& o) {
        if (this != &o) { Vector t(o); std::swap(data, t.data); std::swap(sz, t.sz); std::swap(cap, t.cap); }
        return *this;
    }
    Vector& operator=(Vector&& o) noexcept {
        if (this != &o) { delete[] data; data = o.data; sz = o.sz; cap = o.cap; o.data = nullptr; o.sz = o.cap = 0; }
        return *this;
    }

    // Методы доступа
    int size() const { return sz; }
    int capacity() const { return cap; }
    bool empty() const { return sz == 0; }
    T& operator[](int i) { if (i < 0 || i >= sz) throw std::out_of_range("Index out of range"); return data[i]; }
    const T& operator[](int i) const { if (i < 0 || i >= sz) throw std::out_of_range("Index out of range"); return data[i]; }
    T& front() { if (empty()) throw std::underflow_error("Empty"); return data[0]; }
    T& back() { if (empty()) throw std::underflow_error("Empty"); return data[sz - 1]; }

    // Итераторы
    T* begin() { return data; }
    T* end() { return data + sz; }
    const T* begin() const { return data; }
    const T* end() const { return data + sz; }

    // Вспомогательный метод
    void reserve(int newCap) {
        if (newCap <= cap) return;
        T* newData = new T[newCap];
        for (int i = 0; i < sz; ++i) newData[i] = std::move(data[i]);
        delete[] data; data = newData; cap = newCap;
    }

    // Основные операции
    void push_back(const T& val) {
        if (sz == cap) reserve(cap == 0 ? 1 : cap * 2);
        data[sz++] = val;
    }

    void pop_back() { if (!empty()) --sz; }

    // Продвинутый уровень: emplace_back
    template <typename... Args>
    void emplace_back(Args&&... args) {
        if (sz == cap) reserve(cap == 0 ? 1 : cap * 2);
        new (&data[sz++]) T(std::forward<Args>(args)...);
    }

    // Средний уровень: merge
    void merge(const Vector& other) {
        Vector res(sz + other.sz);
        int i = 0, j = 0, k = 0;
        while (i < sz && j < other.sz) res.data[k++] = (data[i] < other.data[j]) ? data[i++] : other.data[j++];
        while (i < sz) res.data[k++] = data[i++];
        while (j < other.sz) res.data[k++] = other.data[j++];
        *this = std::move(res);
    }
};

int main() {
    setlocale(LC_ALL, "ru");
    std::cout << "=== Тест 14 варианта (Продвинутый) ===\n\n";

    // 1. Тест конструкторов и push_back
    Vector<int> v1;
    for (int i = 1; i <= 5; ++i) v1.push_back(i * 10); // 10, 20, 30, 40, 50
    std::cout << "v1 (push_back): "; for (int x : v1) std::cout << x << " "; std::cout << "\n";

    // 2. Тест front/back/pop_back
    std::cout << "Front: " << v1.front() << ", Back: " << v1.back() << "\n";
    v1.pop_back();
    std::cout << "После pop_back, Back: " << v1.back() << "\n\n";

    // 3. Тест emplace_back (14 вариант)
    Vector<std::pair<int, std::string>> v2;
    v2.emplace_back(1, "one");
    v2.emplace_back(2, "two");
    std::cout << "v2 (emplace_back): ";
    for (const auto& p : v2) std::cout << "{" << p.first << "," << p.second << "} "; std::cout << "\n\n";

    // 4. Тест merge (средний уровень)
    Vector<int> a, b;
    for (int x : {1, 3, 5}) a.push_back(x);
    for (int x : {2, 4, 6}) b.push_back(x);
    std::cout << "a: "; for (int x : a) std::cout << x << " "; std::cout << "\n";
    std::cout << "b: "; for (int x : b) std::cout << x << " "; std::cout << "\n";
    a.merge(b);
    std::cout << "a.merge(b): "; for (int x : a) std::cout << x << " "; std::cout << "\n\n";

    // 5. Тест обработки исключений
    try { v1[100]; } catch (const std::exception& e) { std::cout << "Исключение (выход за границы): " << e.what() << "\n"; }

    return 0;
}