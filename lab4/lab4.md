# Лабораторная работа №3
## Реализация динамического массива (аналог vector). Итераторы

**Студент:** Черников Роман Сергеевич  
**Группа:** ПИЖ-6-о-25-1  
**Дисциплина:** Алгоритмы и структуры данных  
**Вариант:** 14  
**Уровень:** Продвинутый

---

## Цель работы

1. Реализовать шаблонный класс односвязного списка
(SinglyLinkedList) с базовыми операциями
2. Реализовать шаблонный класс двусвязного списка
(DoublyLinkedList) с базовыми операциями
3. Освоить операции вставки, удаления, поиска и обхода элементов
в списках
4. Сравнить производительность и особенности использования
односвязного и двусвязного списков
5. Научиться правильно управлять памятью при работе со списками
(удаление узлов)

## Код программы

```cpp
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <vector>

// POOL ALLOCATOR (выделения памяти большими блоками)
template <typename T, size_t BlockSize = 1024>
class PoolAllocator {
    struct Block { Block* next; };
    Block* freeList = nullptr;
    std::vector<void*> chunks;

public:
    ~PoolAllocator() { for (void* p : chunks) ::operator delete(p); }

    // выделение нового чанка если нету в freelist, разбиваем на кусочки добавляем в цепочку через next, затем берем первый из freelist и возвращаем
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
    // вовращаем блок памяти обратно в freelist
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
```
## Контрольные вопросы

1. **Отличие от массива:** Список: элементы разбросаны по памяти, связаны указателями. Массив: данные лежат в непрерывной области памяти.
2. **O(1) в односвязном:** Вставка/удаление в начало, вставка после заданного узла.
3. **Удаление из конца O(n):** Нужно найти предпоследний узел. Без указателя `prev` приходится перебирать весь список с начала.
4. **Указатель tail:** Позволяет добавлять элементы в конец за O(1) вместо O(n).
5. **Двусвязный vs Односвязный:** В двусвязном списке каждый узел хранит два указателя: на следующий (`next`) и предыдущий (`prev`).
6. **Что быстрее в двусвязном:** Удаление из конца, удаление по ссылке на конкретный узел, обход с конца.
7. **Head и Tail:** `head` — первый элемент списка, `tail` — последний.
8. **Удаление из середины (односвязный):** Найти предыдущий узел и перенаправить его `next` на `next.next` удаляемого узла.
9. **Удаление из середины (двусвязный):** Связать `prev.next` и `next.prev`. Проще, так как не нужно искать предыдущий узел — он уже доступен через `node.prev`.
10. **Забыть обновить tail:** `tail` будет указывать на невалидный/удалённый узел, что сломает добавление в конец и обратный обход.
11. **Реверс:** Итеративно проходить по списку, перенаправляя `next` каждого узла на его предыдущий узел (используя временную переменную).
12. **Обнаружение цикла:** Алгоритм Флойда (два указателя: «быстрый» и «медленный»). Если они встретятся — цикл есть.
13. **Список вместо массива:** Когда нужны частые вставки/удаления в начало или середину и не требуется быстрый доступ по индексу.
14. **Доп. память:** Память под указатель(и) `next` (и `prev`) + накладные расходы на выравнивание памяти.
15. **Доступ по индексу O(n):** Элементы не лежат в непрерывной памяти, поэтому к i-му элементу нуж