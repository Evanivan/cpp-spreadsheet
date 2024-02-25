# cpp-spreadsheet

Дипломный проект: `Электронная таблица (Spreadsheet)`

Простая таблица аналог Microsoft Excel или Google Sheets, для работы только с текстом и формулами.

- В ячейках таблицы могут быть текст или формулы.
- Формулы, как и в существующих решениях, могут содержать индексы ячеек.
- Кэширование значений формул

# Требования
C++17 и выше

- CMake generated project and dependency files
- STL smart pointers
- CMake generated project and dependency files
-[Java SE Runtime Environment 8](https://www.oracle.com/java/technologies/downloads/#java8)
- Корректировки для использования парсера формулы с [ANTLR](https://www.antlr.org/)

## Сборка

1. Установите ANTLR согласно инструкции https://github.com/antlr/antlr4/blob/master/doc/getting-started.md.
2. Создайте пустую папку с именем `antlr4_runtime` в репозитории проекта и перенести содержимое архива antlr4-cpp-runtime*.zip.
3. Создайте папку для сборки программы.
4. Откройте консоль в данной папке и введите в консоли : `cmake <путь к файлу CMakeLists.txt> -DANTLR_EXECUTABLE=<путь к antlr-4.13.0-complete.jar>`.
5. Введите команду : `cmake --build .` .
6. После сборки в папке сборки появится исполняемый файл `spreadsheet.exe`.

## Работа с Spreadsheet

Работа с электронной таблицей реализована для прохождения тестов внутри функции main.
