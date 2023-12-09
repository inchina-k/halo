#include <iostream>
#include <fstream>
#include <streambuf>

#include "../sources/interpreter.hpp"
#include "../sources/scanner.hpp"
#include "../sources/parser.hpp"

using namespace std;
using namespace halo;

Interpreter interpreter;

void run_prompt();
void run_script(const string &s);
string copy_file(ifstream &f);

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

string to_str(Object *o)
{
    return !o ? "null"s : o->to_str();
}

void run(const string &c)
{
    try
    {
        Scanner scanner(c);
        auto t = scanner.scan();

        Parser parser(t);

        parser.parse();

        if (parser.had_errors())
        {
            cerr << "parse error" << endl;
        }
        else
        {
            try
            {
                interpreter.execute(parser.statements());
            }
            catch (const exception &e)
            {
                cerr << e.what() << endl;
            }
        }
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
        run(line);
        cout << ":> ";
    }

    cout << endl;
}

void run_script(const string &file)
{
    ifstream f(file);

    if (!f)
    {
        cerr << "create the file you twit" << endl;
    }
    else
    {
        // cout << copy_file(f) << endl;
        run(copy_file(f));
    }
}

string copy_file(ifstream &f)
{
    return string((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
}