#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <string>
#include <map>
#include <cctype>

using namespace std;

// Таблица макросов
map<string, string> macros;

// Проверка допустимых символов (только для финального кода)
bool isValidChar(char c) {
    string valid = "><+-.,[]()#NSd:;{}@/0123456789 \n\t";
    return valid.find(c) != string::npos;
}

// Разворачивание макросов в тексте
string expandMacros(const string& src) {
    string result;
    size_t i = 0;
    while (i < src.size()) {
        if (src[i] == '@') {
            size_t j = i + 1;
            while (j < src.size() && isalnum(src[j])) j++;
            string name = src.substr(i + 1, j - i - 1);
            if (macros.count(name)) {
                result += macros[name];
            } else {
                cerr << "Ошибка: неизвестный макрос @" << name << endl;
                exit(1);
            }
            i = j;
        } else {
            result += src[i];
            i++;
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Использование: " << argv[0] << " <файл.bsh>" << endl;
        return 1;
    }

    // 1. Читаем программу
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << argv[1] << endl;
        return 1;
    }
    string program((istreambuf_iterator<char>(file)),
                    istreambuf_iterator<char>());
    file.close();

    // 2. Убираем комментарии // ...
    string noComments;
    for (size_t i = 0; i < program.size(); i++) {
        if (program[i] == '/' && i + 1 < program.size() && program[i + 1] == '/') {
            while (i < program.size() && program[i] != '\n') i++;
            noComments += '\n';
            continue;
        }
        noComments += program[i];
    }

    // 3. Читаем макросы @имя = ... и удаляем их из текста
    string noMacros;
    for (size_t i = 0; i < noComments.size(); i++) {
        if (noComments[i] == '@') {
            size_t j = i + 1;
            while (j < noComments.size() && isalnum(noComments[j])) j++;
            string name = noComments.substr(i + 1, j - i - 1);
            while (j < noComments.size() && isspace(noComments[j])) j++;
            if (j < noComments.size() && noComments[j] == '=') {
                j++;
                size_t start = j;
                while (j < noComments.size() && noComments[j] != '\n') j++;
                string body = noComments.substr(start, j - start);
                macros[name] = body;
                i = j;
                continue;
            }
        }
        noMacros += noComments[i];
    }

    // 4. Разворачиваем макросы
    program = expandMacros(noMacros);

    // 5. Проверка допустимых символов (только для команд, не для макросов)
    {
        int line = 1;
        for (size_t i = 0; i < program.size(); i++) {
            if (program[i] == '\n') line++;
            if (!isValidChar(program[i])) {
                cerr << "Строка " << line << ": недопустимый символ '" 
                     << program[i] << "'" << endl;
                return 1;
            }
        }
    }

    // 6. Проверка парности скобок
    {
        int balance = 0;
        int paren = 0;
        int line = 1;
        for (size_t i = 0; i < program.size(); i++) {
            if (program[i] == '\n') line++;
            if (program[i] == '[') balance++;
            if (program[i] == ']') {
                balance--;
                if (balance < 0) {
                    cerr << "Строка " << line << ": лишняя ']'" << endl;
                    return 1;
                }
            }
            if (program[i] == '(') paren++;
            if (program[i] == ')') {
                paren--;
                if (paren < 0) {
                    cerr << "Строка " << line << ": лишняя ')'" << endl;
                    return 1;
                }
            }
        }
        if (balance > 0) {
            cerr << "Ошибка: не закрыта '[' (" << balance << " шт.)" << endl;
            return 1;
        }
        if (paren > 0) {
            cerr << "Ошибка: не закрыта '(' (" << paren << " шт.)" << endl;
            return 1;
        }
    }

    // 7. Память
    vector<unsigned char> tape(30000, 0);
    int ptr = 0;
    int pc = 0;
    stack<int> loop_stack;

    // 8. Выполнение
    while (pc < (int)program.size()) {
        char cmd = program[pc];

        switch (cmd) {
            case '>': ptr++; break;
            case '<': ptr--; break;
            case '+': tape[ptr]++; break;
            case '-': tape[ptr]--; break;
            case '.': cout << tape[ptr]; break;
            case ',': cin >> tape[ptr]; break;

            case '#': cout << (int)tape[ptr] << endl; break;
            case 'N': tape[ptr] -= 48; break;
            case 'S': tape[ptr] += 48; break;

            case 'd': {
                tape[ptr + 1] = tape[ptr] % 10;
                break;
            }

            case ':': {
                pc++;
                int count = 0;
                if (pc >= (int)program.size() || !isdigit(program[pc])) {
                    cerr << "Ошибка: после ':' нужно число" << endl;
                    return 1;
                }
                while (pc < (int)program.size() && isdigit(program[pc])) {
                    count = count * 10 + (program[pc] - '0');
                    pc++;
                }
                for (int i = 0; i < count; i++) tape[ptr]++;
                pc--;
                break;
            }
            case ';': {
                pc++;
                int count = 0;
                if (pc >= (int)program.size() || !isdigit(program[pc])) {
                    cerr << "Ошибка: после ';' нужно число" << endl;
                    return 1;
                }
                while (pc < (int)program.size() && isdigit(program[pc])) {
                    count = count * 10 + (program[pc] - '0');
                    pc++;
                }
                for (int i = 0; i < count; i++) tape[ptr]--;
                pc--;
                break;
            }

            case '[':
                if (tape[ptr] == 0) {
                    int depth = 1;
                    while (depth > 0) {
                        pc++;
                        if (pc >= (int)program.size()) {
                            cerr << "Ошибка: не закрыта '['" << endl;
                            return 1;
                        }
                        if (program[pc] == '[') depth++;
                        else if (program[pc] == ']') depth--;
                    }
                } else {
                    loop_stack.push(pc);
                }
                break;

            case '(':
                // Выполняется один раз, если значение ячейки — цифра (48-57)
                if (tape[ptr] < 48 || tape[ptr] > 57) {
                    int depth = 1;
                    while (depth > 0) {
                        pc++;
                        if (pc >= (int)program.size()) {
                            cerr << "Ошибка: не закрыта '('" << endl;
                            return 1;
                        }
                        if (program[pc] == '(') depth++;
                        else if (program[pc] == ')') depth--;
                    }
                }
                break;

            case ']':
                if (tape[ptr] != 0) {
                    pc = loop_stack.top();
                } else {
                    loop_stack.pop();
                }
                break;

            default: break;
        }

        // Проверка границ ленты
        if (ptr < 0 || ptr >= (int)tape.size()) {
            cerr << "Ошибка: указатель вышел за границы ленты" << endl;
            return 1;
        }

        pc++;
    }

    return 0;
}