#include <iostream>
#include <memory>
#include <algorithm>

#include "parser.hpp"

using namespace std;
using namespace halo;

// Expr *Parser::alloc_grouping(Expr *e)
// {
//     m_nodes.push_back(make_unique<Grouping>(e));
//     return m_nodes.back().get();
// }

Expr *Parser::alloc_binary_expr(Token t, Expr *l, Expr *r)
{
    m_nodes.push_back(make_unique<BinaryExpr>(t, l, r, t.m_line));
    return m_nodes.back().get();
}

Expr *Parser::alloc_logical_expr(Token t, Expr *l, Expr *r)
{
    m_nodes.push_back(make_unique<LogicalExpr>(t, l, r, t.m_line));
    return m_nodes.back().get();
}

Expr *Parser::alloc_unary_expr(Token t, Expr *e)
{
    m_nodes.push_back(make_unique<UnaryExpr>(t, e, t.m_line));
    return m_nodes.back().get();
}

Expr *Parser::alloc_call_expr(Expr *e, const std::vector<Expr *> &p, size_t line)
{
    m_nodes.push_back(make_unique<Call>(e, p, line));
    return m_nodes.back().get();
}

Expr *Parser::alloc_dot_expr(Expr *e, Token t)
{
    m_nodes.push_back(make_unique<Dot>(e, t, t.m_line));
    return m_nodes.back().get();
}

Expr *Parser::alloc_subscript_expr(Expr *e, Expr *index, size_t line)
{
    m_nodes.push_back(make_unique<Subscript>(e, index, line));
    return m_nodes.back().get();
}

Expr *Parser::alloc_literal(Token t)
{
    m_nodes.push_back(make_unique<Literal>(t, t.m_line));
    return m_nodes.back().get();
}

Expr *Parser::alloc_var(Token t)
{
    m_nodes.push_back(make_unique<Var>(t, t.m_line));
    return m_nodes.back().get();
}

Expr *Parser::alloc_lambda(const std::vector<Token> &capture, const std::vector<Token> &params, std::vector<std::unique_ptr<Stmt>> body, size_t line)
{
    m_nodes.push_back(make_unique<Lambda>(capture, params, move(body), line));
    return m_nodes.back().get();
}

Expr *Parser::alloc_list(const std::vector<Expr *> &params, size_t line)
{
    m_nodes.push_back(make_unique<ListExpr>(params, line));
    return m_nodes.back().get();
}

Expr *Parser::parse_expr()
{
    return expr();
}

void Parser::parse()
{
    try
    {
        while (peek().m_type != TokenType::Eof)
        {
            m_stmts.emplace_back(statement(peek().m_line));
        }
    }
    catch (const exception &e)
    {
        m_had_errors = true;
        throw;
    }
}

Stmt *Parser::statement(size_t line)
{
    if (match(TokenType::Var))
    {
        return var_statement(line);
    }

    if (match(TokenType::Let))
    {
        return assignment_statement(line);
    }

    if (match(TokenType::If))
    {
        return if_statement(line);
    }

    if (match(TokenType::While))
    {
        return while_statement(line);
    }

    if (match(TokenType::For))
    {
        return for_statement(line);
    }

    if (match(TokenType::Break))
    {
        return break_statement(line);
    }

    if (match(TokenType::Continue))
    {
        return continue_statement(line);
    }

    if (match(TokenType::Fun))
    {
        return fun_statement(line);
    }

    if (match(TokenType::Return))
    {
        return return_statement(line);
    }

    if (match(TokenType::Class))
    {
        return class_statement(line);
    }

    return expression_statement(line);
    // throw runtime_error("unknown statement");
}

Stmt *Parser::var_statement(size_t line)
{
    Token name = consume(TokenType::Identifier, "Parse error\n    line " + to_string(peek().m_line) + ": <var statement> expected variable name");

    Expr *e = nullptr;

    if (match(TokenType::Equal))
    {
        e = expr();
    }

    consume(TokenType::Semicolon, "Parse error\n    line " + to_string(peek().m_line) + ": <var statement> expected ';' symbol");

    return new VarStmt(name, e, line);
}

Stmt *Parser::assignment_statement(size_t line)
{
    // Token name = consume(TokenType::Identifier, "Parse error\nline " + to_string(peek().m_line) + ": <assignment statement> expected variable name");
    Expr *lval = call();
    auto p = dynamic_cast<Var *>(lval);
    auto p2 = dynamic_cast<Dot *>(lval);
    auto p3 = dynamic_cast<Subscript *>(lval);

    if (p || p2 || p3)
    {
        consume(TokenType::Equal, "Parse error\n    line " + to_string(peek().m_line) + ": <assignment statement> expected '=' symbol");
        Expr *e = expr();
        consume(TokenType::Semicolon, "Parse error\n    line " + to_string(peek().m_line) + ": <assignment statement> expected ';' symbol");

        return new AssignmentStmt(lval, e, line);
    }

    throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <assignment statement> can be used only with variables or object fields");
}

Stmt *Parser::expression_statement(size_t line)
{
    Expr *e = expr();

    consume(TokenType::Semicolon, "Parse error\n    line " + to_string(peek().m_line) + ": <expression statement> expected ';' symbol");

    return new ExpressionStmt(e, line);
}

Stmt *Parser::if_statement(size_t line)
{
    m_scopes.push_back(Scopes::If);

    vector<Expr *> conds;
    vector<vector<unique_ptr<Stmt>>> then_branches;
    vector<unique_ptr<Stmt>> else_branch;

    do
    {
        conds.push_back(expr());

        consume(TokenType::Colon, "Parse error\n    line " + to_string(peek().m_line) + ": <if statement> expected ':' symbol");

        then_branches.emplace_back();

        while (peek().m_type != TokenType::Eof &&
               peek().m_type != TokenType::End &&
               peek().m_type != TokenType::Elif &&
               peek().m_type != TokenType::Else)
        {
            then_branches.back().emplace_back(statement(peek().m_line));
        }

    } while (match(TokenType::Elif));

    if (peek().m_type == TokenType::Eof)
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <if statement> unexpected end of statement");
    }

    if (match(TokenType::Else))
    {
        consume(TokenType::Colon, "Parse error\n    line " + to_string(peek().m_line) + ": <else statement> expected ':' symbol");

        while (peek().m_type != TokenType::Eof && peek().m_type != TokenType::End)
        {
            else_branch.emplace_back(statement(peek().m_line));
        }

        if (peek().m_type == TokenType::Eof)
        {
            throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <else statement> unexpected end of statement");
        }
    }

    consume(TokenType::End, "Parse error\n    line " + to_string(peek().m_line) + ": <if statement> missing end of statement");

    m_scopes.pop_back();

    return new IfStmt(conds, move(then_branches), move(else_branch), line);
}

Stmt *Parser::while_statement(size_t line)
{
    m_scopes.push_back(Scopes::While);

    Expr *cond = expr();
    vector<unique_ptr<Stmt>> do_branch;

    consume(TokenType::Colon, "Parse error\n    line " + to_string(peek().m_line) + ": <while statement> expected ':' symbol");

    while (peek().m_type != TokenType::Eof &&
           peek().m_type != TokenType::End)
    {
        do_branch.emplace_back(statement(peek().m_line));
    }

    if (peek().m_type == TokenType::Eof)
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <while statement> unexpected end of statement");
    }

    consume(TokenType::End, "Parse error\n    line " + to_string(peek().m_line) + ": <while statement> missing end of statement");

    m_scopes.pop_back();

    return new WhileStmt(cond, move(do_branch), line);
}

Stmt *Parser::for_statement(size_t line)
{
    m_scopes.push_back(Scopes::For);

    Token identifier = consume(TokenType::Identifier, "Parse error\n    line " + to_string(peek().m_line) + ": <for statement> expected variable identifier");

    consume(TokenType::In, "Parse error\n    line " + to_string(peek().m_line) + ": <for statement> expected 'in' keyword");

    Expr *begin = nullptr;
    Expr *end = nullptr;
    Expr *step = nullptr;
    Expr *iterable = nullptr;
    vector<unique_ptr<Stmt>> do_branch;

    if (match(TokenType::OpenPar))
    {
        vector<Expr *> range_params;

        if (peek().m_type != TokenType::ClosePar)
        {
            do
            {
                Expr *e = expr();
                range_params.push_back(e);

            } while (match(TokenType::Comma));
        }

        consume(TokenType::ClosePar, "Parse error\n    line " + to_string(peek().m_line) + ": <for statement> expected ')' symbol");

        if (range_params.size() == 2)
        {
            begin = range_params[0];
            end = range_params[1];
        }
        else if (range_params.size() == 3)
        {
            begin = range_params[0];
            end = range_params[1];
            step = range_params[2];
        }
        else
        {
            throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <for statement> invalid number of range arguments");
        }
    }
    else
    {
        iterable = expr();
    }

    consume(TokenType::Colon, "Parse error\n    line " + to_string(peek().m_line) + ": <for statement> expected ':' symbol");

    while (peek().m_type != TokenType::Eof &&
           peek().m_type != TokenType::End)
    {
        do_branch.emplace_back(statement(peek().m_line));
    }

    if (peek().m_type == TokenType::Eof)
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <for statement> unexpected end of statement");
    }

    consume(TokenType::End, "Parse error\n    line " + to_string(peek().m_line) + ": <for statement> missing end of statement");

    m_scopes.pop_back();

    return new ForStmt(identifier, begin, end, step, iterable, move(do_branch), line);
}

Stmt *Parser::break_statement(size_t line)
{
    if (!is_in_loop())
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <break statement> out of loop");
    }

    consume(TokenType::Semicolon, "Parse error\n    line " + to_string(peek().m_line) + ": <break statement> expected ';' symbol");

    return new BreakStmt(line);
}

Stmt *Parser::continue_statement(size_t line)
{
    if (!is_in_loop())
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <continue statement> out of loop");
    }

    consume(TokenType::Semicolon, "Parse error\n    line " + to_string(peek().m_line) + ": <continue statement> expected ';' symbol");

    return new ContinueStmt(line);
}

Stmt *Parser::fun_statement(size_t line)
{
    if (!is_fun_allowed())
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <fun statement> must be global or a class member");
    }

    Token name = consume(TokenType::Identifier, "Parse error\n    line " + to_string(peek().m_line) + ": <fun statement> expected function name");

    if (m_scopes.size() > 0 && m_scopes.back() == Scopes::Class && name.m_lexeme == "_init_")
    {
        m_scopes.push_back(Scopes::Init);
    }
    else
    {
        m_scopes.push_back(Scopes::Fun);
    }

    consume(TokenType::OpenPar, "Parse error\n    line " + to_string(peek().m_line) + ": <fun statement> expected '(' symbol in function '" + name.m_lexeme + "'");
    vector<Token> params;
    if (peek().m_type == TokenType::Identifier)
    {
        params.push_back(advance());
    }

    while (peek().m_type == TokenType::Comma)
    {
        advance();
        params.push_back(consume(TokenType::Identifier, "Parse error\n    line " + to_string(peek().m_line) + ": <fun statement> expected parameter name after ',' symbol in function '" + name.m_lexeme + "'"));

        if (count_if(begin(params), end(params), [&params](const auto &p)
                     { return p.m_lexeme == params.back().m_lexeme; }) > 1)
        {
            throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <fun statement> duplicate parameter '" + params.back().m_lexeme + "' in function '" + name.m_lexeme + "'");
        }
    }
    consume(TokenType::ClosePar, "Parse error\n    line " + to_string(peek().m_line) + ": <fun statement> expected ')' symbol in function '" + name.m_lexeme + "'");

    consume(TokenType::Colon, "Parse error\n    line " + to_string(peek().m_line) + ": <fun statement> expected ':' symbol in function '" + name.m_lexeme + "'");

    vector<unique_ptr<Stmt>> body;

    while (peek().m_type != TokenType::Eof &&
           peek().m_type != TokenType::End)
    {
        body.emplace_back(statement(peek().m_line));
    }

    if (peek().m_type == TokenType::Eof)
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <fun statement> unexpected end of function '" + name.m_lexeme + "'");
    }

    consume(TokenType::End, "Parse error\n    line " + to_string(peek().m_line) + ": <fun statement> missing end of function '" + name.m_lexeme + "'");

    m_scopes.pop_back();

    return new FunStmt(name, params, move(body), line);
}

Stmt *Parser::return_statement(size_t line)
{
    if (!is_in_callable())
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <return statement> out of function");
    }

    Expr *exp = nullptr;

    if (peek().m_type != TokenType::Semicolon)
    {
        if (is_constructor())
        {
            throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <return statement> must not have return value in constructor");
        }
        exp = expr();
    }

    consume(TokenType::Semicolon, "Parse error\n    line " + to_string(peek().m_line) + ": <return statement> expected ';' symbol");

    return new ReturnStmt(exp, line);
}

Stmt *Parser::class_statement(size_t line)
{
    if (!is_class_allowed())
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <class statement> can only be global");
    }

    m_scopes.push_back(Scopes::Class);

    Token name = consume(TokenType::Identifier, "Parse error\n    line " + to_string(peek().m_line) + ": <class statement> expected class name");
    consume(TokenType::Colon, "Parse error\n    line " + to_string(peek().m_line) + ": <class statement> expected ':' symbol in class '" + name.m_lexeme + "'");

    set<string> fields;
    vector<unique_ptr<FunStmt>> methods;

    while (true)
    {
        size_t fun_line = peek().m_line;

        if (match(TokenType::Var))
        {
            Token field = consume(TokenType::Identifier, "Parse error\n    line " + to_string(peek().m_line) + ": <class statement> expected variable name");
            consume(TokenType::Semicolon, "Parse error\n    line " + to_string(peek().m_line) + ": <class statement> expected ';' symbol");
            fields.insert(field.m_lexeme);
        }
        else if (match(TokenType::Fun))
        {
            methods.emplace_back(static_cast<FunStmt *>(fun_statement(fun_line)));
        }
        else
        {
            break;
        }
    }

    consume(TokenType::End, "Parse error\n    line " + to_string(peek().m_line) + ": <class statement> missing end of class '" + name.m_lexeme + "'");

    m_scopes.pop_back();

    return new ClassStmt(name, fields, move(methods), line);
}

/*
    EXPRESSIONS
*/

Expr *Parser::expr()
{
    return or_expr();
}

Expr *Parser::or_expr()
{
    Expr *left = and_expr();

    while (m_tokens[m_curr].m_type == TokenType::Or)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = and_expr();
        left = alloc_logical_expr(op, left, right);
    }

    return left;
}

Expr *Parser::and_expr()
{
    Expr *left = equality();

    while (m_tokens[m_curr].m_type == TokenType::And)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = equality();
        left = alloc_logical_expr(op, left, right);
    }

    return left;
}

Expr *Parser::equality()
{
    Expr *left = comparison();

    while (m_tokens[m_curr].m_type == TokenType::EqualEqual ||
           m_tokens[m_curr].m_type == TokenType::BangEqual)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = comparison();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *Parser::comparison()
{
    Expr *left = term();

    while (m_tokens[m_curr].m_type == TokenType::Greater ||
           m_tokens[m_curr].m_type == TokenType::GreaterEqual ||
           m_tokens[m_curr].m_type == TokenType::Less ||
           m_tokens[m_curr].m_type == TokenType::LessEqual)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = term();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *Parser::term()
{
    Expr *left = factor();

    while (m_tokens[m_curr].m_type == TokenType::Plus || m_tokens[m_curr].m_type == TokenType::Minus)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = factor();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *Parser::factor()
{
    Expr *left = unary();

    while (m_tokens[m_curr].m_type == TokenType::Mul || m_tokens[m_curr].m_type == TokenType::Div || m_tokens[m_curr].m_type == TokenType::Mod)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = unary();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *Parser::unary()
{
    if (m_tokens[m_curr].m_type == TokenType::Minus ||
        m_tokens[m_curr].m_type == TokenType::Not)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = unary();
        return alloc_unary_expr(op, right);
    }

    return call();
}

Expr *Parser::call()
{
    Expr *callee = primary();

    while (true)
    {
        size_t line = peek().m_line;

        if (match(TokenType::OpenPar))
        {
            vector<Expr *> args;

            if (peek().m_type != TokenType::ClosePar)
            {
                do
                {
                    Expr *e = expr();
                    args.push_back(e);

                } while (match(TokenType::Comma));
            }

            consume(TokenType::ClosePar, "Parse error\n    line " + to_string(peek().m_line) + ": <call expression> expected ')' symbol");
            callee = alloc_call_expr(callee, args, line);
        }
        else if (match(TokenType::Dot))
        {
            Token name = consume(TokenType::Identifier, "Parse error\n    line " + to_string(peek().m_line) + ": <call expression> expected member name");
            callee = alloc_dot_expr(callee, name);
        }
        else if (match(TokenType::OpenBracket))
        {
            Expr *e = expr();
            consume(TokenType::CloseBracket, "Parse error\n    line " + to_string(peek().m_line) + ": <call expression> expected ']' symbol");
            callee = alloc_subscript_expr(callee, e, line);
        }
        else
        {
            break;
        }
    }

    return callee;
}

Expr *Parser::primary()
{
    if (m_tokens[m_curr].m_type == TokenType::IntLiteral ||
        m_tokens[m_curr].m_type == TokenType::FloatLiteral ||
        m_tokens[m_curr].m_type == TokenType::True || m_tokens[m_curr].m_type == TokenType::False ||
        m_tokens[m_curr].m_type == TokenType::StrLiteral ||
        m_tokens[m_curr].m_type == TokenType::Null)
    {
        return alloc_literal(m_tokens[m_curr++]);
    }
    else if (match(TokenType::Lambda))
    {
        return lambda();
    }
    else if (m_tokens[m_curr].m_type == TokenType::Identifier)
    {
        return alloc_var(m_tokens[m_curr++]);
    }
    else if (m_tokens[m_curr].m_type == TokenType::OpenPar)
    {
        ++m_curr;
        Expr *e = expr();

        if (m_tokens[m_curr].m_type == TokenType::ClosePar)
        {
            ++m_curr;
            return e;
        }

        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <primary expression> expected ')' symbol");
    }
    else if (match(TokenType::OpenBracket))
    {
        return list();
    }

    throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <primary expression> unknown expression");
}

Expr *Parser::lambda()
{
    size_t line = m_tokens[m_curr - 1].m_line;

    if (!is_lambda_allowed())
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> cannot be used in another lambda or as a class member");
    }

    m_scopes.push_back(Scopes::Lambda);

    vector<Token> capture;
    if (match(TokenType::OpenBracket))
    {
        if (peek().m_type == TokenType::Identifier)
        {
            capture.push_back(advance());
        }

        while (peek().m_type == TokenType::Comma)
        {
            advance();
            capture.push_back(consume(TokenType::Identifier, "Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> expected capture element name after ',' symbol"));

            if (count_if(begin(capture), end(capture), [&capture](const auto &e)
                         { return e.m_lexeme == capture.back().m_lexeme; }) > 1)
            {
                throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> duplicate capture element '" + capture.back().m_lexeme + "'");
            }
        }
        consume(TokenType::CloseBracket, "Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> expected ']' symbol");
    }

    consume(TokenType::OpenPar, "Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> expected '(' symbol");
    vector<Token> params;
    if (peek().m_type == TokenType::Identifier)
    {
        params.push_back(advance());
    }

    while (peek().m_type == TokenType::Comma)
    {
        advance();
        params.push_back(consume(TokenType::Identifier, "Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> expected parameter name after ',' symbol"));

        if (count_if(begin(params), end(params), [&params](const auto &p)
                     { return p.m_lexeme == params.back().m_lexeme; }) > 1)
        {
            throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> duplicate parameter '" + params.back().m_lexeme + "'");
        }
    }
    consume(TokenType::ClosePar, "Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> expected ')' symbol");

    consume(TokenType::Colon, "Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> expected ':' symbol");

    vector<unique_ptr<Stmt>> body;

    while (peek().m_type != TokenType::Eof &&
           peek().m_type != TokenType::End)
    {
        body.emplace_back(statement(peek().m_line));
    }

    if (peek().m_type == TokenType::Eof)
    {
        throw runtime_error("Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> unexpected end of lambda");
    }

    consume(TokenType::End, "Parse error\n    line " + to_string(peek().m_line) + ": <lambda expression> missing end of lambda");

    m_scopes.pop_back();

    return alloc_lambda(capture, params, move(body), line);
}

Expr *Parser::list()
{
    size_t line = m_tokens[m_curr - 1].m_line;

    vector<Expr *> args;

    if (peek().m_type != TokenType::CloseBracket)
    {
        do
        {
            Expr *e = expr();
            args.push_back(e);

        } while (match(TokenType::Comma));
    }

    consume(TokenType::CloseBracket, "Parse error\n    line " + to_string(peek().m_line) + ": <list expression> expected ']' symbol");

    return alloc_list(args, line);
}

/*
    HELPING FUNCTIONS
*/

bool Parser::match(TokenType t)
{
    if (m_tokens[m_curr].m_type == t)
    {
        ++m_curr;
        return true;
    }

    return false;
}

const Token &Parser::consume(TokenType t, std::string err)
{
    if (m_tokens[m_curr].m_type == t)
    {
        ++m_curr;
        return m_tokens[m_curr - 1];
    }

    throw runtime_error(err);
}

const Token &Parser::peek() const
{
    return m_tokens[m_curr];
}

const Token &Parser::advance()
{
    ++m_curr;
    return m_tokens[m_curr - 1];
}

bool Parser::is_in_loop() const
{
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it)
    {
        if (*it == Scopes::Fun || *it == Scopes::Lambda || *it == Scopes::Class)
        {
            return false;
        }

        if (*it == Scopes::While || *it == Scopes::For)
        {
            return true;
        }
    }

    return false;
}

bool Parser::is_in_callable() const
{
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it)
    {
        if (*it == Scopes::Fun || *it == Scopes::Lambda || *it == Scopes::Init)
        {
            return true;
        }

        if (*it == Scopes::Class)
        {
            return false;
        }
    }

    return false;
}

bool Parser::is_constructor() const
{
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it)
    {
        if (*it == Scopes::Fun || *it == Scopes::Lambda || *it == Scopes::Class)
        {
            return false;
        }

        if (*it == Scopes::Init)
        {
            return true;
        }
    }

    return false;
}

bool Parser::is_fun_allowed() const
{
    return m_scopes.empty() || m_scopes.front() == Scopes::Class;
}

bool Parser::is_lambda_allowed() const
{
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it)
    {
        if (*it == Scopes::Fun)
        {
            return true;
        }

        if (*it == Scopes::Lambda || *it == Scopes::Class)
        {
            return false;
        }
    }

    return true;
}

bool Parser::is_class_allowed() const
{
    return m_scopes.empty();
}