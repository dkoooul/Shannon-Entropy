#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <iomanip>
#include <random>
#include <locale>
#include <codecvt>
#include <string>

using namespace std;

// Функция для безопасной проверки знаков препинания
bool isCustomPunct(char32_t c) {
    const char32_t puncts[] = U".!?,;:\"'()-";
    for (char32_t p : puncts) {
        if (c == p) return true;
    }
    return false;
}

// Нормализация символов с учетом требований
char32_t normalizeChar(char32_t c, bool isTextFile) {
    if (!isTextFile) return c;

    // Приведение русских букв к нижнему регистру
    if (c >= U'А' && c <= U'Я') {
        return c + (U'а' - U'А');
    }

    // Обработка специфических символов
    switch (c) {
    case U'Ё': case U'ё': return U'е';
    case U'Ъ': case U'ъ': return U'ь';
    default:
        if (isCustomPunct(c)) return U'.';
        return tolower(c);
    }
}

// Функция вычисления энтропии
double calculateEntropy(const wstring& content, int n, bool isTextFile) {
    if (content.length() < n) return 0.0;

    unordered_map<wstring, int> frequencies;
    int total = 0;

    for (size_t i = 0; i <= content.length() - n; ++i) {
        wstring ngram;
        for (int j = 0; j < n; ++j) {
            char32_t c = normalizeChar(content[i + j], isTextFile);
            if (c == U'\t') continue;
            ngram += static_cast<wchar_t>(c);
        }

        if (ngram.length() == n) {
            frequencies[ngram]++;
            total++;
        }
    }

    double entropy = 0.0;
    for (const auto& pair : frequencies) {
        double probability = static_cast<double>(pair.second) / total;
        entropy -= probability * log2(probability);
    }

    return n > 1 ? entropy / n : entropy;
}

// Генерация файла с равномерным распределением
void generateUniformFile(const string& filename, int size) {
    wofstream file(filename, ios::binary);
    file.imbue(locale(file.getloc(), new codecvt_utf8<wchar_t>));

    const wstring chars = L"абвгдежзийклмнопрстуфхцчшщьыъэюя0123456789";
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, chars.size() - 1);

    for (int i = 0; i < size; ++i) {
        file << chars[dist(gen)];
    }
}

// Генерация файла с неравномерным распределением
void generateNonUniformFile(const string& filename, int size) {
    wofstream file(filename, ios::binary);
    file.imbue(locale(file.getloc(), new codecvt_utf8<wchar_t>));

    vector<pair<wchar_t, double>> symbols = {
        {L'а', 0.4}, {L'б', 0.2}, {L'в', 0.15}, {L'г', 0.1}, {L'д', 0.05},
        {L'е', 0.03}, {L'ж', 0.02}, {L'з', 0.015}, {L'и', 0.01}, {L'й', 0.005}
    };

    random_device rd;
    mt19937 gen(rd());
    discrete_distribution<> dist({ 40, 20, 15, 10, 5, 3, 2, 1, 1, 0 });

    for (int i = 0; i < size; ++i) {
        file << symbols[dist(gen)].first;
    }
}

// Анализ файла
void analyzeFile(const string& filename, bool isTextFile) {
    wifstream file(filename, ios::binary);
    file.imbue(locale(file.getloc(), new codecvt_utf8<wchar_t>));

    if (!file.is_open()) {
        wcerr << L"Ошибка открытия файла: " << filename.c_str() << endl;
        return;
    }

    // Проверка размера
    file.seekg(0, ios::end);
    streampos size = file.tellg();
    file.seekg(0, ios::beg);

    if (size < 10240) {
        wcout << L"Файл слишком мал (<10KB), но анализ будет продолжен" << endl;
    }

    // Чтение содержимого
    wstring content((istreambuf_iterator<wchar_t>(file)), istreambuf_iterator<wchar_t>());
    file.close();

    // Вычисление энтропии
    double entropy1 = calculateEntropy(content, 1, isTextFile);
    double entropy2 = calculateEntropy(content, 2, isTextFile);

    // Вывод результатов
    wcout << L"\nАнализ файла: " << filename.c_str() << endl;
    wcout << L"Размер: " << size / 1024 << L" KB" << endl;
    wcout << L"Энтропия (1-граммы): " << fixed << setprecision(4) << entropy1 << L" бит/символ" << endl;
    wcout << L"Энтропия (2-граммы): " << fixed << setprecision(4) << entropy2 << L" бит/символ" << endl;
    wcout << L"Отношение (2/1): " << fixed << setprecision(4) << (entropy1 > 0 ? entropy2 / entropy1 : 0) << endl;
    wcout << L"----------------------------------------" << endl;
}

int main() {
    // Настройка локали
    locale::global(locale(""));
    wcout.imbue(locale());
    wcin.imbue(locale());

    // Генерация тестовых файлов
    wcout << L"Генерация тестовых файлов..." << endl;
    generateUniformFile("uniform.txt", 15000);
    generateNonUniformFile("non_uniform.txt", 12000);

    // Анализ файлов
    wcout << L"\nАнализ файлов..." << endl;
    analyzeFile("uniform.txt", false);
    analyzeFile("non_uniform.txt", false);
    analyzeFile("literature.txt", true);

    return 0;
}