#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <string>
#include <map>
#include <set>
#include <cctype>
#include <cstdlib>
#include <ctime>

using namespace std;

// Macro table
map<string, string> macros;

// Check for allowed characters (final code only)
bool isValidChar(char c) {
    string valid = "><+-.,[]()#NSdmr%:;@/0123456789 \n\t";
    return valid.find(c) != string::npos;
}

// Strip { ... } comments.
// They may span several lines and sit inline after code. They nest.
string stripComments(const string& src) {
    string out;
    int depth = 0;
    int line = 1;
    int openLine = 0;

    for (size_t i = 0; i < src.size(); i++) {
        char c = src[i];
        if (c == '\n') {
            line++;
            out += '\n';   // keep line numbers stable for later checks
            continue;
        }

        if (c == '{') {
            if (depth == 0) openLine = line;
            depth++;
            continue;
        }
        if (c == '}') {
            if (depth == 0) {
                cerr << "Строка " << line << ": лишняя '}'" << endl;
                exit(1);
            }
            depth--;
            continue;
        }

        if (depth == 0) out += c;
    }

    if (depth > 0) {
        cerr << "Ошибка: не закрыта '{' (открыта в строке " << openLine << ")" << endl;
        exit(1);
    }
    return out;
}

// Read a whole file into a string
bool readFile(const string& path, string& out) {
    ifstream f(path);
    if (!f.is_open()) return false;
    out.assign((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    return true;
}

// Characters allowed in an include path
bool isPathChar(char c) {
    return isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-' || c == '/';
}

// Inline %file.bsh directives recursively.
// Macros defined in the included files are picked up later by extractMacros.
string processIncludes(const string& src, const string& baseDir, set<string>& seen) {
    string out;
    for (size_t i = 0; i < src.size(); i++) {
        if (src[i] != '%') {
            out += src[i];
            continue;
        }

        // %"path" or %path (reads until whitespace or end of line)
        string path;
        bool quoted = (i + 1 < src.size() && src[i + 1] == '"');
        if (quoted) {
            size_t j = i + 2;
            while (j < src.size() && src[j] != '"' && src[j] != '\n') j++;
            if (j >= src.size() || src[j] != '"') {
                cerr << "Ошибка: не закрыта кавычка в имени файла" << endl;
                exit(1);
            }
            path = src.substr(i + 2, j - i - 2);
            i = j;
        } else {
            size_t j = i + 1;
            while (j < src.size() && !isspace((unsigned char)src[j])) j++;
            path = src.substr(i + 1, j - i - 1);
            i = j - 1;
            // Unquoted names must look like plain filenames
            for (char c : path) {
                if (!isPathChar(c)) {
                    cerr << "Ошибка: недопустимый символ '" << c << "' в имени файла"
                         << " (используй кавычки)" << endl;
                    exit(1);
                }
            }
        }

        if (path.empty()) {
            cerr << "Ошибка: после '%' нужно имя файла" << endl;
            exit(1);
        }

        // Resolve relative to the including file first, then the working directory
        string full = path;
        if (full[0] != '/' && !baseDir.empty()) {
            string candidate = baseDir + "/" + path;
            ifstream probe(candidate);
            if (probe.is_open()) full = candidate;
        }

        if (seen.count(full)) {
            // Already included, skip to avoid an infinite loop
            continue;
        }
        seen.insert(full);

        string content;
        if (!readFile(full, content)) {
            cerr << "Не удалось открыть файл: " << full << endl;
            exit(1);
        }

        out += processIncludes(stripComments(content), baseDir, seen);
        out += '\n';
    }
    return out;
}

// Expand macros in the source text.
// Bodies are expanded recursively, so a macro may call other macros.
string expandMacros(const string& src, set<string>& active) {
    string result;
    size_t i = 0;
    while (i < src.size()) {
        if (src[i] == '@') {
            size_t j = i + 1;
            while (j < src.size() && isalnum(src[j])) j++;
            string name = src.substr(i + 1, j - i - 1);
            if (!macros.count(name)) {
                cerr << "Ошибка: неизвестный макрос @" << name << endl;
                exit(1);
            }
            if (active.count(name)) {
                cerr << "Ошибка: макрос @" << name << " вызывает сам себя" << endl;
                exit(1);
            }
            active.insert(name);
            result += expandMacros(macros[name], active);
            active.erase(name);
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

    // 1. Read the program
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << argv[1] << endl;
        return 1;
    }
    string program((istreambuf_iterator<char>(file)),
                    istreambuf_iterator<char>());
    file.close();

    // 2. Strip { ... } comments
    string noComments = stripComments(program);

    // 2.5. Inline %file.bsh includes so their macros become available
    {
        string baseDir;
        string dir(argv[1]);
        size_t slash = dir.find_last_of('/');
        if (slash != string::npos) baseDir = dir.substr(0, slash);

        set<string> seen;
        seen.insert(dir);
        noComments = processIncludes(noComments, baseDir, seen);
    }

    // 3. Read @name = ... macros and remove them from the text
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

    // 4. Expand macros
    set<string> active;
    program = expandMacros(noMacros, active);

    // 5. Check for allowed characters (commands only, not macros)
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

    // 6. Check that brackets are balanced
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

    // 7. Memory
    vector<unsigned char> tape(30000, 0);
    int ptr = 0;
    int pc = 0;
    stack<int> loop_stack;

    // 8. Execution
    srand((unsigned)time(NULL));
    while (pc < (int)program.size()) {
        char cmd = program[pc];

        switch (cmd) {
            case '>': ptr++; break;
            case '<': ptr--; break;
            case '+': tape[ptr]++; break;
            case '-': tape[ptr]--; break;
            case 'm': {
                if (ptr + 1 >= (int)tape.size()) {
                    cerr << "Ошибка: для 'm' нужна соседняя ячейка справа" << endl;
                    return 1;
                }
                tape[ptr] = (unsigned char)(tape[ptr] * tape[ptr + 1]);
                break;
            }
            case 'r':
                tape[ptr] = (unsigned char)(1 + rand() % 255);
                break;
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
                // Runs once if the cell holds a digit (48-57)
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

        // Check tape bounds
        if (ptr < 0 || ptr >= (int)tape.size()) {
            cerr << "Ошибка: указатель вышел за границы ленты" << endl;
            return 1;
        }

        pc++;
    }

    return 0;
}