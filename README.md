# Транспортный справочник
Интерактивная база данных (транспортный каталог) с модулями ввода/вывода данных о маршрутах и остановках в/из базы данных в формате json и рендеринга карты всех маршрутов в формате svg.

Документация в процессе разработки.

### Реализованый функционал
- Парсер JSON
- Конструктор JSON
- SVG библиотека
- Рендеринг карт маршрутов
- Поддержка цветовых палитр для отрисовки линий маршрутов и автоматический рендеринг цвета для маршрутов
- "Умная"" база данных маршрутов и остановок
- Вычисление географических расстояний между маршрутами
- Проецирование географических координат на плоскость

### Использованные технологии
- C++ 17
- STL
- JSON
- SVG
- CMake 3.25.2

### Содержание файлов
- `search_server.h | .cpp` класс SearchServer, функционал поисковой системы
- `concurrent_map.h` реализация потокобезопасного контейнера ConcurrentMap
- `document.h` структуры Document и enum-класс DocumentStatus
- `log_duration.h` профилировщик, позволяющий измерять время выполнения программы целиком или отдельных функций
- `paginator.h` реализация функционала пагинатора страниц
- `process_queries.h | .cpp` реализация очереди запросов
- `read_input_functions.h | .cpp` функции чтения и обработки входного потока
- `string_processing.h | .cpp` вспомогательные функции обработки текста
- `text_example_functions.h | .cpp` функции с примерами использования и бенчмарки
- `main.cpp` точки входа для запуска функций примеров работы программы и бенчмарков
- `CMakeLists.txt` файл с информацией для сборки проекта с помощью CMake

### Сборка проекта
Проект можно удобно собрать с помощью CMake, используя файл CMakeLists.txt

### Руководство по использованию
1. Для начала работы с транспортным справочником необходимо создать пустую  базу данных (экземпляр класса TransportCatalogue)
```C++
data_base::TransportCatalogue db; // create empty data base
```
2. Создать парсер (экземпляр класса JsonReader), проинициализировав его созданным объектом базы данных:
```C++
json_reader::JsonReader json_reader(db); 
```
3. Программа готова к работе, дальeе необходимо передать методу LoadJsonAndSetDB класса JsonReader запросы во входной поток std::istream&:
Сигнатура метода:
```C++
void LoadJsonAndSetDB(std::istream& input)
```
Пример реализации в main.cpp:
```C++
json_reader.LoadJsonAndSetDB(std::cin)
```
4. С помощью метода GetCompleteOutputJSON() класса JsonReader можно получить в выходной поток сгенерированный комплексный json, который включает в себя:
- информацию об остановке (json);
- сгенерированную карту маршрутов (формат svg);
- информацию о маршруте (json).
Сигнатура метода:
```C++
void GetCompleteOutputJSON(std::ostream& out);
```
Пример реализации в main.cpp
```C++
json_reader.GetCompleteOutputJSON(std::cout);
```
Пример использования. Возможный результат рендеринга карты.
![Example svg](ex.png)

### Планы по доработке
- Добавление функционала расчета расстояний между остановками
- Добавление функционала расчета времени до прибытия автобуса
- Рефакторинг кода
