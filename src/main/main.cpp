#include <iostream>
#include "../sources/interpreter.hpp"
#include "../sources/scanner.hpp"
#include "../sources/expr_parser.hpp"

using namespace std;
using namespace halo;

Interpreter interpreter;

void run_prompt();
void run_script(string s);

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        run_prompt();
    }
    else if (argc == 2)
    {
        run_script(argv[1]);
    }
    else
    {
        cout << "usage:\nhalo - for repl mode\nhalo script.halo - for file mode" << endl;
    }
}

void run(const string &c)
{
    try
    {
        Scanner scanner(c);
        auto t = scanner.scan();

        ExprParser parser(t);
        auto e = parser.parse();

        cout << interpreter.evaluate(e)->to_str() << endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void run_prompt()
{
    cout << ":> ";

    for (string line; getline(cin, line);)
    {
        run(line);
        cout << ":> ";
    }

    cout << endl;
}

void run_script(string)
{
    cerr << "script mode is not implemented yet" << endl;
}