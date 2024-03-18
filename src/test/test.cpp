#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../../doctest.h"

#include "../sources/token.hpp"
#include "../sources/scanner.hpp"
#include "../sources/expr.hpp"
#include "../sources/parser.hpp"
#include "../sources/interpreter.hpp"
#include "../sources/printer.hpp"

#include <fstream>

using namespace std;
using namespace halo;

TEST_CASE("Token constructor")
{
    Token t(TokenType::Plus, "+", 0, 0);

    REQUIRE(t.m_type == TokenType::Plus);
    REQUIRE(t.m_lexeme == "+");
    REQUIRE(t.m_line == 0);
    REQUIRE(t.m_offset == 0);
}

TEST_CASE("Scanner::read_one_symbol_lexeme")
{
    SUBCASE("Dot")
    {
        string s = "  .  \n .";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Dot);
        REQUIRE(res[0].m_lexeme == ".");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 2);

        REQUIRE(res[1].m_type == TokenType::Dot);
        REQUIRE(res[1].m_lexeme == ".");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }

    SUBCASE("Comma")
    {
        string s = ",\n.\n,";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 4);

        REQUIRE(res[0].m_type == TokenType::Comma);
        REQUIRE(res[0].m_lexeme == ",");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::Dot);
        REQUIRE(res[1].m_lexeme == ".");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 0);

        REQUIRE(res[2].m_type == TokenType::Comma);
        REQUIRE(res[2].m_lexeme == ",");
        REQUIRE(res[2].m_line == 3);
        REQUIRE(res[2].m_offset == 0);
    }

    SUBCASE("Colon")
    {
        string s = ":";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 2);

        REQUIRE(res[0].m_type == TokenType::Colon);
        REQUIRE(res[0].m_lexeme == ":");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);
    }

    SUBCASE("Semicolon")
    {
        string s = ";";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 2);

        REQUIRE(res[0].m_type == TokenType::Semicolon);
        REQUIRE(res[0].m_lexeme == ";");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);
    }

    SUBCASE("OpenPar && ClosePar")
    {
        string s = "(\n)";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::OpenPar);
        REQUIRE(res[0].m_lexeme == "(");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::ClosePar);
        REQUIRE(res[1].m_lexeme == ")");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 0);
    }

    SUBCASE("OpenBracket && CloseBracket")
    {
        string s = "[\n]";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::OpenBracket);
        REQUIRE(res[0].m_lexeme == "[");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::CloseBracket);
        REQUIRE(res[1].m_lexeme == "]");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 0);
    }
}

TEST_CASE("Scanner::read_two_symbol_lexeme")
{
    SUBCASE("+, +=")
    {
        string s = "+ \n +=";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Plus);
        REQUIRE(res[0].m_lexeme == "+");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::PlusEqual);
        REQUIRE(res[1].m_lexeme == "+=");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }

    SUBCASE("-, -=")
    {
        string s = "- \n -=";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Minus);
        REQUIRE(res[0].m_lexeme == "-");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::MinusEqual);
        REQUIRE(res[1].m_lexeme == "-=");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }

    SUBCASE("*, *=")
    {
        string s = "* \n *=";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Mul);
        REQUIRE(res[0].m_lexeme == "*");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::MulEqual);
        REQUIRE(res[1].m_lexeme == "*=");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }

    SUBCASE("/, /=")
    {
        string s = "/ \n /=";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Div);
        REQUIRE(res[0].m_lexeme == "/");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::DivEqual);
        REQUIRE(res[1].m_lexeme == "/=");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }

    SUBCASE("<, <=")
    {
        string s = "< \n <=";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Less);
        REQUIRE(res[0].m_lexeme == "<");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::LessEqual);
        REQUIRE(res[1].m_lexeme == "<=");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }

    SUBCASE(">, >=")
    {
        string s = "> \n >=";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Greater);
        REQUIRE(res[0].m_lexeme == ">");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::GreaterEqual);
        REQUIRE(res[1].m_lexeme == ">=");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }

    SUBCASE("=, ==")
    {
        string s = "= \n ==";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Equal);
        REQUIRE(res[0].m_lexeme == "=");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::EqualEqual);
        REQUIRE(res[1].m_lexeme == "==");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }

    SUBCASE("!, !=")
    {
        string s = "! \n !=";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Bad);
        REQUIRE(res[0].m_lexeme == "!");
        REQUIRE(res[0].m_line == 1);
        REQUIRE(res[0].m_offset == 0);

        REQUIRE(res[1].m_type == TokenType::BangEqual);
        REQUIRE(res[1].m_lexeme == "!=");
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }
}

TEST_CASE("Scanner::read_comment")
{
    string s = ". #I am comment\n +";
    Scanner scanner(s);
    auto res = scanner.scan();

    REQUIRE(res.size() == 3);

    REQUIRE(res[0].m_line == 1);
    REQUIRE(res[0].m_offset == 0);

    REQUIRE(res[1].m_line == 2);
    REQUIRE(res[1].m_offset == 1);
}

TEST_CASE("Scanner::read_quote")
{
    SUBCASE("simple string")
    {
        string s = "\"I am str\"";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 2);
        REQUIRE(res[0].m_type == TokenType::StrLiteral);
        REQUIRE(res[0].m_lexeme == "I am str");
    }

    SUBCASE("bad string")
    {
        string s = "\"I am a bad str, punish me, daddy master\n +";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);
        REQUIRE(res[0].m_type == TokenType::Bad);
        REQUIRE(res[1].m_type == TokenType::Plus);
        REQUIRE(res[1].m_line == 2);
        REQUIRE(res[1].m_offset == 1);
    }

    SUBCASE("empty string")
    {
        string s = "\"\"";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 2);
        REQUIRE(res[0].m_type == TokenType::StrLiteral);
        REQUIRE(res[0].m_lexeme == "");
    }
}

TEST_CASE("Scanner::read_number")
{
    SUBCASE("int")
    {
        string s = "+123;";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 4);

        REQUIRE(res[0].m_type == TokenType::Plus);

        REQUIRE(res[1].m_type == TokenType::IntLiteral);
        REQUIRE(res[1].m_lexeme == "123");

        REQUIRE(res[2].m_type == TokenType::Semicolon);
        REQUIRE(res[2].m_offset == 4);
    }

    SUBCASE("float")
    {
        string s = "+\n123.09\n;";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 4);

        REQUIRE(res[0].m_type == TokenType::Plus);

        REQUIRE(res[1].m_type == TokenType::FloatLiteral);
        REQUIRE(res[1].m_lexeme == "123.09");
        REQUIRE(res[1].m_line == 2);

        REQUIRE(res[2].m_type == TokenType::Semicolon);
        REQUIRE(res[2].m_line == 3);
        REQUIRE(res[2].m_offset == 0);
    }

    SUBCASE("bad number")
    {
        string s = "123.+";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Bad);
        REQUIRE(res[0].m_lexeme == "123.");

        REQUIRE(res[1].m_type == TokenType::Plus);
    }
}

TEST_CASE("Scanner::read_identifier")
{
    SUBCASE("identifier")
    {
        string s = "temp";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 2);

        REQUIRE(res[0].m_type == TokenType::Identifier);
        REQUIRE(res[0].m_lexeme == "temp");
    }

    SUBCASE("_identifier")
    {
        string s = "__t_e_m_p__";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 2);

        REQUIRE(res[0].m_type == TokenType::Identifier);
        REQUIRE(res[0].m_lexeme == "__t_e_m_p__");
    }

    SUBCASE("not")
    {
        string s = "not x";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 3);

        REQUIRE(res[0].m_type == TokenType::Not);
        REQUIRE(res[0].m_lexeme == "not");

        REQUIRE(res[1].m_type == TokenType::Identifier);
        REQUIRE(res[1].m_lexeme == "x");
    }

    SUBCASE("and")
    {
        string s = "x and y";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 4);

        REQUIRE(res[0].m_type == TokenType::Identifier);
        REQUIRE(res[0].m_lexeme == "x");

        REQUIRE(res[1].m_type == TokenType::And);
        REQUIRE(res[1].m_lexeme == "and");

        REQUIRE(res[2].m_type == TokenType::Identifier);
        REQUIRE(res[2].m_lexeme == "y");
    }

    SUBCASE("or")
    {
        string s = "_x or not _y";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 5);

        REQUIRE(res[0].m_type == TokenType::Identifier);
        REQUIRE(res[0].m_lexeme == "_x");

        REQUIRE(res[1].m_type == TokenType::Or);
        REQUIRE(res[1].m_lexeme == "or");

        REQUIRE(res[2].m_type == TokenType::Not);
        REQUIRE(res[2].m_lexeme == "not");

        REQUIRE(res[3].m_type == TokenType::Identifier);
        REQUIRE(res[3].m_lexeme == "_y");
    }

    SUBCASE("var")
    {
        string s = "var x = 42";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 5);

        REQUIRE(res[0].m_type == TokenType::Var);
        REQUIRE(res[0].m_lexeme == "var");

        REQUIRE(res[1].m_type == TokenType::Identifier);
        REQUIRE(res[1].m_lexeme == "x");

        REQUIRE(res[2].m_type == TokenType::Equal);
        REQUIRE(res[2].m_lexeme == "=");

        REQUIRE(res[3].m_type == TokenType::IntLiteral);
        REQUIRE(res[3].m_lexeme == "42");
    }

    SUBCASE("let")
    {
        string s = "let x = 9.7";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 5);

        REQUIRE(res[0].m_type == TokenType::Let);
        REQUIRE(res[0].m_lexeme == "let");

        REQUIRE(res[1].m_type == TokenType::Identifier);
        REQUIRE(res[1].m_lexeme == "x");

        REQUIRE(res[2].m_type == TokenType::Equal);
        REQUIRE(res[2].m_lexeme == "=");

        REQUIRE(res[3].m_type == TokenType::FloatLiteral);
        REQUIRE(res[3].m_lexeme == "9.7");
    }

    SUBCASE("true, false")
    {
        string s = "var x = true;\nlet y = not false;";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 12);

        REQUIRE(res[0].m_type == TokenType::Var);
        REQUIRE(res[0].m_lexeme == "var");
        REQUIRE(res[1].m_type == TokenType::Identifier);
        REQUIRE(res[1].m_lexeme == "x");
        REQUIRE(res[2].m_type == TokenType::Equal);
        REQUIRE(res[2].m_lexeme == "=");
        REQUIRE(res[3].m_type == TokenType::True);
        REQUIRE(res[3].m_lexeme == "true");
        REQUIRE(res[4].m_type == TokenType::Semicolon);
        REQUIRE(res[4].m_lexeme == ";");

        REQUIRE(res[5].m_type == TokenType::Let);
        REQUIRE(res[5].m_lexeme == "let");
        REQUIRE(res[6].m_type == TokenType::Identifier);
        REQUIRE(res[6].m_lexeme == "y");
        REQUIRE(res[7].m_type == TokenType::Equal);
        REQUIRE(res[7].m_lexeme == "=");
        REQUIRE(res[8].m_type == TokenType::Not);
        REQUIRE(res[8].m_lexeme == "not");
        REQUIRE(res[9].m_type == TokenType::False);
        REQUIRE(res[9].m_lexeme == "false");
        REQUIRE(res[10].m_type == TokenType::Semicolon);
        REQUIRE(res[10].m_lexeme == ";");
    }

    SUBCASE("null")
    {
        string s = "var x = null";
        Scanner scanner(s);
        auto res = scanner.scan();

        REQUIRE(res.size() == 5);

        REQUIRE(res[0].m_type == TokenType::Var);
        REQUIRE(res[0].m_lexeme == "var");

        REQUIRE(res[1].m_type == TokenType::Identifier);
        REQUIRE(res[1].m_lexeme == "x");

        REQUIRE(res[2].m_type == TokenType::Equal);
        REQUIRE(res[2].m_lexeme == "=");

        REQUIRE(res[3].m_type == TokenType::Null);
        REQUIRE(res[3].m_lexeme == "null");
    }
}

TEST_CASE("Expr classes")
{
    Printer ep;

    SUBCASE("BinaryExpr")
    {
        Token plus(TokenType::Plus, "+", 0, 0);
        Token a(TokenType::IntLiteral, "2", 0, 0);
        Token b(TokenType::IntLiteral, "3", 0, 0);

        Literal i1(a);
        Literal i2(b);

        Expr *e = new BinaryExpr(plus, &i1, &i2);

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "(2+3)");

        delete e;
    }
}

TEST_CASE("Expr parser")
{
    Printer ep;

    SUBCASE("2+3")
    {
        string s = "2+3";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "(2+3)");
    }

    SUBCASE("(2+3)*-4")
    {
        string s = "(2+3)*-4";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "((2+3)*(-4))");
    }

    SUBCASE("(2+3)*---4")
    {
        string s = "(2+3)*---4";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "((2+3)*(-(-(-4))))");
    }

    SUBCASE("(2+#)")
    {
        string s = "(2+#)";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS(p.parse(), runtime_error);
    }

    SUBCASE("(2+3")
    {
        string s = "(2+3";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS(p.parse(), runtime_error);
    }

    SUBCASE("2+3)")
    {
        string s = "2+3)";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "(2+3)");
    }

    SUBCASE("2+3 <= 5+7")
    {
        string s = "2+3 <= 5+7";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "((2+3)<=(5+7))");
    }

    SUBCASE("2.0+3.0 == 10.0-5.0")
    {
        string s = "2.0+3.0 == 10.0-5.0";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "((2.0+3.0)==(10.0-5.0))");
    }

    SUBCASE("Hello+World")
    {
        string s = "\"Hello\"+\"World\"";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "(Hello+World)");
    }
}

TEST_CASE("interpreter")
{
    SUBCASE("2+2")
    {
        string s = "2+2";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "4");
    }

    SUBCASE("Hello+World")
    {
        string s = "\"Hello\"+\"World\"";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "HelloWorld");
    }

    SUBCASE("--4")
    {
        string s = "--4";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "4");
    }

    SUBCASE("not true")
    {
        string s = "not true";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("2 + 2.0")
    {
        string s = "2 + 2.0";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "4.000000");
    }

    SUBCASE("2.0 + 2")
    {
        string s = "2.0 + 2";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "4.000000");
    }

    SUBCASE("2 + hello")
    {
        string s = "2 + \"hello\"";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        REQUIRE_THROWS_WITH(interpreter.evaluate(e), "line 1: incorrect operand types for '+'");
    }

    SUBCASE("20 / 3")
    {
        string s = "20 / 3";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "6");
    }

    SUBCASE("20 % 3")
    {
        string s = "20 % 3";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "2");
    }

    SUBCASE("\n20.0 % 3.0")
    {
        string s = "\n20.0 % 3.0";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        REQUIRE_THROWS_WITH(interpreter.evaluate(e), "line 2: incorrect operand types for '%'");
    }

    SUBCASE("2 < 3")
    {
        string s = "2 < 3";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "true");
    }

    SUBCASE("3 < 2")
    {
        string s = "3 < 2";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("3.0 < 2.0")
    {
        string s = "3.0 < 2.0";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("3.0 < 2")
    {
        string s = "3.0 < 2";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("3.0 <= 3")
    {
        string s = "3.0 <= 3";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "true");
    }

    SUBCASE("3 > 3")
    {
        string s = "3 > 3";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("3 >= 3")
    {
        string s = "3 >= 3";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "true");
    }

    SUBCASE("3 == 3")
    {
        string s = "3 == 3";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "true");
    }

    SUBCASE("3 == 2")
    {
        string s = "3 == 2";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("3.0 == 2.0")
    {
        string s = "3.0 == 2.0";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("hello == hello")
    {
        string s = "\"hello\" == \"hello\"";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "true");
    }

    SUBCASE("true == true")
    {
        string s = "true == true";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "true");
    }

    SUBCASE("true != true")
    {
        string s = "true != true";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("1 < 5 or 5 < 10")
    {
        string s = "1 < 5 or 5 < 10";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "true");
    }

    SUBCASE("1 < 5 and 5 < 10")
    {
        string s = "1 < 5 and 5 < 10";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "true");
    }

    SUBCASE("1 > 5 or 5 < 10")
    {
        string s = "1 > 5 or 5 < 10";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "true");
    }

    SUBCASE("1 > 5 and 5 < 10")
    {
        string s = "1 > 5 and 5 < 10";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("true and false")
    {
        string s = "true and false";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "false");
    }

    SUBCASE("0 and 1")
    {
        string s = "0 and 1";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "0");
    }

    SUBCASE("\"\" and \"\"")
    {
        string s = "\"\" and \"\"";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "");
    }

    SUBCASE("0.0 or 1")
    {
        string s = "0.0 or 1";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o->to_str() == "1");
    }

    SUBCASE("true and null")
    {
        string s = "true and null";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        Interpreter interpreter;
        Object *o = interpreter.evaluate(e);
        REQUIRE(o == nullptr);
    }
}

TEST_CASE("calls")
{
    Printer ep;

    SUBCASE("f()")
    {
        string s = "f()";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "f()");
    }

    SUBCASE("f(1)(2, 3)()")
    {
        string s = "f(1)(2, 3)()";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "f(1)(2, 3)()");
    }

    SUBCASE("f().method()")
    {
        string s = "f().method()";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "f().method()");
    }

    SUBCASE("f().method(x, y).field")
    {
        string s = "f().method(x, y).field";
        Scanner sc(s);
        auto v = sc.scan();
        Parser p(v);
        Expr *e = p.parse_expr();

        e->visit(&ep);

        REQUIRE(ep.m_data.str() == "f().method(x, y).field");
    }
}

TEST_CASE("scripts")
{
    SUBCASE("001")
    {
        ifstream file("scripts/001.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "Hello, World!\n");
    }

    SUBCASE("002")
    {
        ifstream file("scripts/002.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("Kamila");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "Hello, " + s_in.str() + "!\n");
    }

    SUBCASE("003")
    {
        ifstream file("scripts/003.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "10, 10.500000, 10.000000\n");
    }

    SUBCASE("004")
    {
        ifstream file("scripts/004.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "Hello");
    }

    SUBCASE("010")
    {
        ifstream file("scripts/010.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("21");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "enjoy your happy hour!\nyou're 21!!!\n");
    }

    SUBCASE("011")
    {
        ifstream file("scripts/011.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "10\n9\n8\n7\n6\n5\n4\n3\n2\n1\n");
    }

    SUBCASE("011")
    {
        ifstream file("scripts/011.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "10\n9\n8\n7\n6\n5\n4\n3\n2\n1\n");
    }

    SUBCASE("012")
    {
        ifstream file("scripts/012.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "1\n2\n3\n");
    }

    SUBCASE("013")
    {
        ifstream file("scripts/013.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "1\n2\n4\n5\n");
    }

    SUBCASE("014")
    {
        ifstream file("scripts/014.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "hello, world\n");
    }

    SUBCASE("015")
    {
        ifstream file("scripts/015.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("Johnson");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "hello, Johnson\nnull\n");
    }

    SUBCASE("016")
    {
        ifstream file("scripts/016.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "6\n");
    }

    SUBCASE("017")
    {
        ifstream file("scripts/017.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "1\n2\n3\n4\n");
    }

    SUBCASE("018")
    {
        ifstream file("scripts/018.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "6\n1\n1\n");
    }

    SUBCASE("019")
    {
        ifstream file("scripts/019.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS_MESSAGE(p.parse(), runtime_error, "line 1: return statement out of function");
    }

    SUBCASE("020")
    {
        ifstream file("scripts/020.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS_MESSAGE(p.parse(), runtime_error, "line 1: break statement out of loop");
    }

    SUBCASE("021")
    {
        ifstream file("scripts/021.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS_MESSAGE(p.parse(), runtime_error, "line 1: continue statement out of loop");
    }

    SUBCASE("022")
    {
        ifstream file("scripts/022.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);

        REQUIRE_THROWS_AS_MESSAGE(interp.execute(p.statements()), runtime_error, "max fun depth exceeded: 1024");
    }

    SUBCASE("023")
    {
        ifstream file("scripts/023.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "6");
    }

    SUBCASE("024")
    {
        ifstream file("scripts/024.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "1\n4\n9\n");
    }

    SUBCASE("025")
    {
        ifstream file("scripts/025.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "1\n4\n9\n");
    }

    SUBCASE("026")
    {
        ifstream file("scripts/026.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "1\n4\n9\n");
    }

    SUBCASE("027")
    {
        ifstream file("scripts/027.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "11 7 0\n11 17 1\n");
    }

    SUBCASE("028")
    {
        ifstream file("scripts/028.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "7 0\n");
        REQUIRE(s_out.str() != "7 11\n");
    }

    SUBCASE("029")
    {
        ifstream file("scripts/029.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "1\n4\n9\n");
    }

    SUBCASE("030")
    {
        ifstream file("scripts/030.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "4\n8\n");
    }

    SUBCASE("031")
    {
        ifstream file("scripts/031.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS_MESSAGE(p.parse(), runtime_error, "line 2: break statement out of loop");
    }

    SUBCASE("032")
    {
        ifstream file("scripts/032.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS_MESSAGE(p.parse(), runtime_error, "line 2: continue statement out of loop");
    }

    SUBCASE("033")
    {
        ifstream file("scripts/033.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS_MESSAGE(p.parse(), runtime_error, "line 2: fun must be global or a class member");
    }

    SUBCASE("034")
    {
        ifstream file("scripts/034.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS_MESSAGE(p.parse(), runtime_error, "line 1: lambda cannot be used in another lambda or as a class member");
    }

    SUBCASE("035")
    {
        ifstream file("scripts/035.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "5\n2\n3\n4\n1\n2\n");
    }

    SUBCASE("036")
    {
        ifstream file("scripts/036.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "<class Point>:\n\t_init_\n\tmethod\n");
    }

    SUBCASE("037")
    {
        ifstream file("scripts/037.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);

        REQUIRE_THROWS_AS_MESSAGE(interp.execute(p.statements()), runtime_error, "x name is not found\n");
    }

    SUBCASE("038")
    {
        ifstream file("scripts/038.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "hey\n");
    }

    SUBCASE("039")
    {
        ifstream file("scripts/039.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "1, 2\n");
    }

    SUBCASE("041")
    {
        ifstream file("scripts/041.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);

        REQUIRE_THROWS_AS_MESSAGE(p.parse(), runtime_error, "line 4: return statement must not return value in constructor\n");
    }

    SUBCASE("042")
    {
        ifstream file("scripts/042.halo");
        string src = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        Scanner sc(src);
        auto v = sc.scan();
        Parser p(v);
        p.parse();

        istringstream s_in("");
        ostringstream s_out;

        Interpreter interp(s_in, s_out);
        interp.execute(p.statements());

        REQUIRE(s_out.str() == "42\n");
    }
}
