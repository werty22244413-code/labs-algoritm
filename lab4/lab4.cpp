#include <iostream>
#include <stdexcept>
#include <chrono>
#include <vector>

// POOL ALLOCATOR (Пул-аллокатор для узлов)
template <typename T, size_t BlockSize = 1024>
class PoolAllocator {
    struct Block { Block* next; };
    Block* freeList = nullptr;
    std::vector<void*> chunks;

public:
    ~PoolAllocator() { for (void* p : chunks) ::operator delete(p); }

    T* allocate() {
        if (!freeList) {
            size_t size = sizeof(T) > sizeof(Block) ? sizeof(T) : sizeof(Block);
            char* raw = static_cast<char*>(::operator new(size * BlockSize));
            chunks.push_back(raw);
            for (size_t i = 0; i < BlockSize; ++i) {
                Block* b = reinterpret_cast<Block*>(raw + i * size);
                b->next = freeList;
                freeList = b;
            }
        }
        Block* b = freeList;
        freeList = freeList->next;
        return reinterpret_cast<T*>(b);
    }
    void deallocate(T* p) {
        if (!p) return;
        Block* b = reinterpret_cast<Block*>(p);
        b->next = freeList;
        freeList = b;
    }
};

// ОДНОСВЯЗНЫЙ СПИСОК
template <typename T>
class SinglyLinkedList {
    struct Node {
        T data; Node* next;
        Node(const T& v) : data(v), next(nullptr) {}
    };
    Node* head = nullptr; Node* tail = nullptr; int count = 0;
    PoolAllocator<Node> alloc;

public:
    ~SinglyLinkedList() { clear(); }

    void push_back(const T& val) {
        Node* n = new (alloc.allocate()) Node(val);
        if (!head) head = tail = n;
        else { tail->next = n; tail = n; }
        count++;
    }

    void clear() {
        Node* cur = head;
        while (cur) {
            Node* next = cur->next;
            alloc.deallocate(cur);
            cur = next;
        }
        head = tail = nullptr; count = 0;
    }

    // Итератор
    class Iterator {
        Node* ptr;
    public:
        Iterator(Node* p) : ptr(p) {}
        T& operator*() { return ptr->data; }
        Iterator& operator++() { ptr = ptr->next; return *this; }
        bool operator!=(const Iterator& o) const { return ptr != o.ptr; }
    };
    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
    int size() const { return count; }
};

// ДВУСВЯЗНЫЙ СПИСОК
template <typename T>
class DoublyLinkedList {
    struct Node {
        T data; Node* prev; Node* next;
        Node(const T& v) : data(v), prev(nullptr), next(nullptr) {}
    };
    Node* head = nullptr; Node* tail = nullptr; int count = 0;
    PoolAllocator<Node> alloc;

public:
    ~DoublyLinkedList() { clear(); }

    void push_back(const T& val) {
        Node* n = new (alloc.allocate()) Node(val);
        if (!tail) head = tail = n;
        else {
            n->prev = tail; tail->next = n; tail = n;
        }
        count++;
    }

    void clear() {
        Node* cur = head;
        while (cur) {
            Node* next = cur->next;
            alloc.deallocate(cur);
            cur = next;
        }
        head = tail = nullptr; count = 0;
    }

    // Итератор
    class Iterator {
        Node* ptr;
    public:
        Iterator(Node* p) : ptr(p) {}
        T& operator*() { return ptr->data; }
        Iterator& operator++() { ptr = ptr->next; return *this; }
        Iterator& operator--() { ptr = ptr->prev; return *this; }
        bool operator!=(const Iterator& o) const { return ptr != o.ptr; }
    };
    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
    int size() const { return count; }
};

// MAIN: Тесты и сравнение производительности
int main() {
    setlocale(LC_ALL, "ru");

    std::cout << "=== Тест SinglyLinkedList (итераторы + пул) ===\n";
    SinglyLinkedList<int> sList;
    for (int i = 1; i <= 5; ++i) sList.push_back(i * 10);
    std::cout << "Обход: ";
    for (auto it = sList.begin(); it != sList.end(); ++it) std::cout << *it << " ";
    std::cout << "\nРазмер: " << sList.size() << "\n\n";

    std::cout << "=== Тест DoublyLinkedList (итераторы + пул) ===\n";
    DoublyLinkedList<int> dList;
    for (int i = 1; i <= 5; ++i) dList.push_back(i * 10);
    std::cout << "Обход: ";
    for (auto it = dList.begin(); it != dList.end(); ++it) std::cout << *it << " ";
    std::cout << "\nРазмер: " << dList.size() << "\n\n";

    std::cout << "=== Сравнение производительности (1 000 000 вставок) ===\n";
    const int N = 1000000;

    auto start = std::chrono::high_resolution_clock::now();
    SinglyLinkedList<int> sBench;
    for (int i = 0; i < N; ++i) sBench.push_back(i);
    auto end = std::chrono::high_resolution_clock::now();
    auto timeS = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    DoublyLinkedList<int> dBench;
    for (int i = 0; i < N; ++i) dBench.push_back(i);
    end = std::chrono::high_resolution_clock::now();
    auto timeD = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "SinglyLinkedList: " << timeS << " мс\n";
    std::cout << "DoublyLinkedList: " << timeD << " мс\n";
    std::cout << "(Двусвязный медленнее из-за настройки 2 указателей на узел)\n";

    return 0;
}