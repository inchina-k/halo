#include <iostream>
#include <fstream>
#include <streambuf>

#include "../sources/interpreter.hpp"
#include "../sources/scanner.hpp"
#include "../sources/parser.hpp"

using namespace std;
using namespace halo;

void run_prompt();
void run_script(const string &s);
string copy_file(ifstream &f);

Interpreter interpreter;
vector<unique_ptr<Parser>> parsers;

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
        cout << "Usage:\n    halo - REPL mode\n    halo script.halo - file mode" << endl;
    }
}

string to_str(Object *o)
{
    return !o ? "null"s : o->to_str();
}

void prompt(const string &c)
{
    vector<string> errors;

    try
    {
        Scanner scanner(c);
        auto t = scanner.scan();

        parsers.emplace_back(make_unique<Parser>(t));

        try
        {
            parsers.back()->parse();
        }
        catch (const exception &e)
        {
            errors.push_back(e.what());
        }

        if (parsers.back()->had_errors())
        {
            parsers.pop_back();
            parsers.emplace_back(make_unique<Parser>(t));

            Expr *expr = nullptr;

            try
            {
                expr = parsers.back()->parse_expr();
            }
            catch (const std::exception &e)
            {
                errors.push_back(e.what());
            }

            if (errors.size() == 2)
            {
                parsers.pop_back();
                cerr << "Parse as stmt: " << errors[0] << endl;
                cerr << "Parse as expr: " << errors[1] << endl;
                return;
            }

            cout << to_str(interpreter.evaluate(expr)) << endl;
        }
        else
        {
            interpreter.execute(parsers.back()->statements());
        }
    }
    catch (const exception &e)
    {
        cerr << e.what() << endl;
    }
}

void script(const string &c)
{
    try
    {
        Scanner scanner(c);
        auto t = scanner.scan();

        Parser parser(t);
        parser.parse();

        interpreter.execute(parser.statements());
    }
    catch (const exception &e)
    {
        cerr << e.what() << endl;
    }
}

void run_prompt()
{
    cout << ":> ";

    for (string line; getline(cin, line);)
    {
        prompt(line);
        cout << ":> ";
    }

    cout << endl;
}

void run_script(const string &file)
{
    ifstream f(file);

    if (!f)
    {
        cerr << "The file was not found" << endl;
    }
    else
    {
        interpreter.set_script(file);
        script(copy_file(f));
    }
}

string copy_file(ifstream &f)
{
    return string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
}